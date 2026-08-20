#include "common.h"


uint64_t GetUs(void)
{
    uint32_t primask;
    uint32_t tick_ms; // 当前毫秒数
    uint32_t systick_value; // SysTick 当前倒计时值
    uint32_t systick_period; // SysTick 一个完整周期（1ms）有多少个计数
    uint32_t tick_period_us; // 一个 SysTick 周期（1ms）对应多少微秒

    primask = __get_PRIMASK(); // 看看中断有没有被禁止
    __disable_irq(); // 禁止中断，防止在读取 SysTick->VAL 时被中断打断

    // 获取当前毫秒数和 SysTick 当前倒计时值 必须连着读取，防止被中断打断
    tick_ms = HAL_GetTick();
    systick_value = SysTick->VAL;

    systick_period = SysTick->LOAD + 1U; // 获取systick倒数计数周期
    tick_period_us = (uint32_t)HAL_GetTickFreq() * 1000U; // 注意HAL_GetTickFreq()返回的是每个systick倒数周期的毫秒数

    /* SysTick已回卷但中断尚未执行时，补上本次HAL时基周期。 */
    if ((SCB->ICSR & SCB_ICSR_PENDSTSET_Msk) != 0U) // 如果systick在上面的过程中倒数到0，需要补上本次HAL时基周期
    {
        tick_ms += (uint32_t)HAL_GetTickFreq();
        systick_value = SysTick->VAL;
    }

    if (primask == 0U) // 如果中断之前是开启的，那么在返回前重新开启中断
    {
        __enable_irq();
    }

    return ((uint64_t)tick_ms * 1000ULL) //毫秒转换成微秒
         + (((uint64_t)((systick_period - 1U) - systick_value) * tick_period_us)
            / systick_period);
}
