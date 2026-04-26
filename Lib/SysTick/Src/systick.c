#include "systick.h"
#include <stdint.h>

static volatile uint32_t t_ms = 0;

/*!
    @brief Returns the current uptime in milliseconds
    @return milliseconds since system has been up
*/
uint32_t get_ms(void) {
    return t_ms;
}

/*!
    @brief Blocks the processor for a given amount of milliseconds
    @param ms Number of milliseconds to block the processor
*/
void delay_ms(const uint32_t ms) {
    /*
        t_ms gets updated at a defined 1 ms interval so if delay_ms is called
        shortly before t_ms is updated, then the actual delayed time will be
        less than the desired delay. To ensure minimum possible delay, add 1 to
        the delay so that delay_ms will delay minimum 1 ms and maximum ms + 1.
    */
    uint32_t end_t = t_ms + ms + 1;
    while (t_ms != end_t);
}

/*!
    @brief SysTick IRQ handler
    @details Currently this function is called in the ACTUAL SysTick IRQ because
    it is defined in the HAL. Eventually if I move away from HAL entirely this
    function will become the actual SysTick IRQ handler.
*/
void systick_handler(void) {
    t_ms++;
}