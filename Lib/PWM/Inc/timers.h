/*!
  @file		timers.h
  @brief	Configure timers to generate a 50Hz PWM output on the channel headers

  --TIMER CONFIGURATION--
  Timer 2 (32-bit):
    CH1 -> PA0
    CH2 -> PA1
    CH3 -> PA2
    CH4 -> PA3

  Timer 4 (16-bit):
    CH1 -> PB6
    CH2 -> PB7

  --OVERVIEW--
  This library configures timers 2 and 4 in PWM mode to output a 50Hz PWM signal
  on the pins shown above. When the timers are initialized, the outputs are disabled.

  The outputs are only enabled if these conditions happen in order:
    1. The compare register values are >0
    2. The timer enable function is called

  The timers are configured in a way such that the value in the compare register is
  the exact pulse width of the output in microseconds
    - e.g. a CC value of 1500 would output a 1500us pulse width
*/
#ifndef TIMERS_H
#define TIMERS_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------
// TYPES
// ----------------------------------------------------------------------

// Struct to hold requested timer compare register values
typedef struct {
  uint32_t tim2_cc[4]; /*!< TIM2 compare registers values (0 - 4,294,967,295)
                              Channels 0-3 */
  uint32_t tim4_cc[2]; /*!< TIM4 compare registers values (0 - 65,535)
                              Channels 0-1 */
} timers_s;

// Global instance of timers
extern timers_s timers;

// ----------------------------------------------------------------------
// FUNCTIONS
// ----------------------------------------------------------------------

void timers_init(void);
void timers_output_enable(void);
void timers_output_disable(void);
void timers_update_cc(void);

#ifdef __cplusplus
}
#endif

#endif