#include "app_telemetry.h"

#include "common.h"
#include "usart.h"

#include <string.h>

#define TELEMETRY_PERIOD_MS 67U
#define TELEMETRY_MAGIC_0   0x42U
#define TELEMETRY_MAGIC_1   0x43U
#define TELEMETRY_VERSION   1U

typedef struct __attribute__((packed))
{
    uint8_t magic[2];
    uint8_t version;
    uint8_t packet_size;
    uint16_t sequence;
    uint32_t time_ms;
    float roll_deg;
    float roll_ref_deg;
    float gx_dps;
    float gx_ref_dps;
    float omega_ref;
    float omega_l;
    float omega_r;
    float speed_fb_mps;
    float speed_corr_deg;
    uint16_t crc16;
} TelemetryPacket;

_Static_assert(sizeof(TelemetryPacket) == 48U, "Unexpected telemetry packet size");

static TelemetryPacket tx_packet;
static uint16_t sequence;

static uint16_t CRC16_CCITT(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFFU;

    while (length-- > 0U)
    {
        crc ^= (uint16_t)(*data++) << 8;
        for (uint8_t bit = 0U; bit < 8U; ++bit)
        {
            crc = (crc & 0x8000U) ? (uint16_t)((crc << 1) ^ 0x1021U)
                                  : (uint16_t)(crc << 1);
        }
    }
    return crc;
}

void App_Telemetry_Init(void)
{
    memset(&tx_packet, 0, sizeof(tx_packet));
    tx_packet.magic[0] = TELEMETRY_MAGIC_0;
    tx_packet.magic[1] = TELEMETRY_MAGIC_1;
    tx_packet.version = TELEMETRY_VERSION;
    tx_packet.packet_size = (uint8_t)sizeof(tx_packet);
    sequence = 0U;
}

void App_Telemetry_Proc(float roll_deg,
                        float roll_ref_deg,
                        float gx_dps,
                        float gx_ref_dps,
                        float omega_ref,
                        float omega_l,
                        float omega_r,
                        float speed_fb_mps,
                        float speed_corr_deg)
{
    PERIODIC(TELEMETRY_PERIOD_MS);

    /* Never hold up the 5 ms control loop; a late frame is simply skipped. */
    if (huart3.gState != HAL_UART_STATE_READY)
    {
        return;
    }

    tx_packet.sequence = sequence;
    tx_packet.time_ms = HAL_GetTick();
    tx_packet.roll_deg = roll_deg;
    tx_packet.roll_ref_deg = roll_ref_deg;
    tx_packet.gx_dps = gx_dps;
    tx_packet.gx_ref_dps = gx_ref_dps;
    tx_packet.omega_ref = omega_ref;
    tx_packet.omega_l = omega_l;
    tx_packet.omega_r = omega_r;
    tx_packet.speed_fb_mps = speed_fb_mps;
    tx_packet.speed_corr_deg = speed_corr_deg;
    tx_packet.crc16 = CRC16_CCITT((const uint8_t *)&tx_packet,
                                 (uint16_t)(sizeof(tx_packet) - sizeof(tx_packet.crc16)));

    if (HAL_UART_Transmit_IT(&huart3, (const uint8_t *)&tx_packet,
                             (uint16_t)sizeof(tx_packet)) == HAL_OK)
    {
        ++sequence;
    }
}
