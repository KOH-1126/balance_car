#!/usr/bin/env python3
"""Receive and decode balance-car telemetry over BLE or a serial COM port."""

from __future__ import annotations

import argparse
import asyncio
import csv
import math
import struct
import sys
import time
from datetime import datetime
from pathlib import Path

import serial
from bleak import BleakClient, BleakScanner
from serial.tools import list_ports


MAGIC = b"BC"
PACKET = struct.Struct("<2sBBHI9fH")
DEFAULT_BLE_NAME = "BT_ECB0D8010F49"
DEFAULT_NOTIFY_UUID = "0000fff1-0000-1000-8000-00805f9b34fb"
HEADER = [
    "type", "t_ms", "roll_deg", "roll_ref_deg", "gx_dps", "gx_ref_dps",
    "omega_ref", "omega_l", "omega_r", "speed_fb_mps", "speed_corr_deg", "sequence",
]


def crc16_ccitt(data: bytes) -> int:
    crc = 0xFFFF
    for value in data:
        crc ^= value << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


class Decoder:
    def __init__(self, writer: csv.writer) -> None:
        self.writer = writer
        self.buffer = bytearray()
        self.good = 0
        self.bad_crc = 0
        self.lost = 0
        self.last_sequence: int | None = None

    def feed(self, chunk: bytes | bytearray) -> None:
        self.buffer.extend(chunk)
        while True:
            marker = self.buffer.find(MAGIC)
            if marker < 0:
                if len(self.buffer) > 1:
                    del self.buffer[:-1]
                return
            if marker:
                del self.buffer[:marker]
            if len(self.buffer) < PACKET.size:
                return
            if self.buffer[2] != 1 or self.buffer[3] != PACKET.size:
                del self.buffer[0]
                continue

            raw = bytes(self.buffer[:PACKET.size])
            if crc16_ccitt(raw[:-2]) != int.from_bytes(raw[-2:], "little"):
                self.bad_crc += 1
                del self.buffer[0]
                continue

            values = PACKET.unpack(raw)
            del self.buffer[:PACKET.size]
            _, _, _, sequence, t_ms, *rest = values
            floats = rest[:-1]
            if not all(math.isfinite(value) for value in floats):
                continue
            if self.last_sequence is not None:
                self.lost += (sequence - self.last_sequence - 1) & 0xFFFF
            self.last_sequence = sequence
            self.writer.writerow(["DATA", t_ms, *[f"{value:.6f}" for value in floats], sequence])
            self.good += 1


def report(decoder: Decoder, started: float) -> None:
    elapsed = max(time.monotonic() - started, 1e-6)
    print(
        f"接收 {decoder.good} 帧，平均 {decoder.good / elapsed:.1f} Hz，"
        f"丢帧 {decoder.lost}，CRC错误 {decoder.bad_crc}"
    )


async def find_ble_device(name: str, address: str | None):
    if address:
        return await BleakScanner.find_device_by_address(address, timeout=15.0)
    return await BleakScanner.find_device_by_filter(
        lambda device, advertisement: device.name == name or advertisement.local_name == name,
        timeout=15.0,
    )


async def receive_ble(
    name: str, address: str | None, characteristic: str, duration: float, output: Path
) -> int:
    print(f"正在搜索 BLE 设备 {address or name}...")
    device = await find_ble_device(name, address)
    if device is None:
        raise RuntimeError(f"未发现 BLE 设备 {address or name}，请确认小车已上电且未连接其他手机。")

    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", newline="", encoding="utf-8-sig") as stream:
        writer = csv.writer(stream)
        writer.writerow(HEADER)
        decoder = Decoder(writer)

        async with BleakClient(device, timeout=15.0) as client:
            print(f"已连接 {device.name}（{device.address}），无需 Windows 配对。")
            print(f"数据文件：{output.resolve()}")
            await client.start_notify(characteristic, lambda _, data: decoder.feed(data))
            started = time.monotonic()
            next_report = started + 1.0
            try:
                while duration <= 0 or time.monotonic() - started < duration:
                    await asyncio.sleep(0.10)
                    now = time.monotonic()
                    if now >= next_report:
                        report(decoder, started)
                        stream.flush()
                        next_report = now + 1.0
            finally:
                await client.stop_notify(characteristic)

        if decoder.good == 0:
            print("BLE 已连接，但未收到有效遥测数据。", file=sys.stderr)
            return 2
        print(f"采集完成：{decoder.good} 帧，丢帧 {decoder.lost}，已保存 {output.resolve()}")
        return 0


def choose_serial_port(requested: str | None) -> str:
    if requested:
        return requested
    ports = [
        item for item in list_ports.comports()
        if "bluetooth" in (item.description or "").lower()
        or "bthenum" in (item.hwid or "").lower()
        or "蓝牙" in (item.description or "")
    ]
    if len(ports) == 1:
        return ports[0].device
    raise RuntimeError("没有唯一的蓝牙串口，请用 --port 指定；本车推荐使用默认 BLE 模式。")


def receive_serial(port: str, baud: int, duration: float, output: Path) -> int:
    output.parent.mkdir(parents=True, exist_ok=True)
    with serial.Serial(port, baudrate=baud, timeout=0.20) as link, output.open(
        "w", newline="", encoding="utf-8-sig"
    ) as stream:
        writer = csv.writer(stream)
        writer.writerow(HEADER)
        decoder = Decoder(writer)
        started = time.monotonic()
        next_report = started + 1.0
        print(f"已打开 {port}（{baud} 8N1），数据文件：{output.resolve()}")
        while duration <= 0 or time.monotonic() - started < duration:
            decoder.feed(link.read(max(link.in_waiting, 1)))
            now = time.monotonic()
            if now >= next_report:
                report(decoder, started)
                stream.flush()
                next_report = now + 1.0
        if decoder.good == 0:
            return 2
        print(f"采集完成：{decoder.good} 帧，丢帧 {decoder.lost}，已保存 {output.resolve()}")
        return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--transport", choices=("ble", "serial"), default="ble")
    parser.add_argument("--name", default=DEFAULT_BLE_NAME, help="BLE 广播名称")
    parser.add_argument("--address", help="BLE 地址；不填时按名称搜索")
    parser.add_argument("--characteristic", default=DEFAULT_NOTIFY_UUID, help="BLE 通知特征 UUID")
    parser.add_argument("--port", help="serial 模式的 COM 口")
    parser.add_argument("--baud", type=int, default=9600, help="serial 模式波特率")
    parser.add_argument("--duration", type=float, default=20.0, help="采集秒数；0 表示持续采集")
    parser.add_argument("--output", type=Path, help="输出 CSV 路径")
    args = parser.parse_args()
    output = args.output or Path(f"telemetry_wireless_{datetime.now():%Y%m%d_%H%M%S}.csv")

    try:
        if args.transport == "ble":
            return asyncio.run(
                receive_ble(args.name, args.address, args.characteristic, args.duration, output)
            )
        return receive_serial(choose_serial_port(args.port), args.baud, args.duration, output)
    except (RuntimeError, serial.SerialException, OSError) as exc:
        print(f"错误：{exc}", file=sys.stderr)
        return 2
    except KeyboardInterrupt:
        print("采集已停止。")
        return 130


if __name__ == "__main__":
    raise SystemExit(main())
