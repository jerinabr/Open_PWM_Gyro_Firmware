#include "timers.h"
#include "stm32g431xx.h"
#include "stm32g4xx.h"
#include <stdint.h>

/*
  GPIO configuration defines
*/
#define GPIO_PA0_AF1_TIM2_CH1 (0x1UL << GPIO_AFRL_AFSEL0_Pos)
#define GPIO_PA1_AF1_TIM2_CH2 (0x1UL << GPIO_AFRL_AFSEL1_Pos)
#define GPIO_PA2_AF1_TIM2_CH3 (0x1UL << GPIO_AFRL_AFSEL2_Pos)
#define GPIO_PA3_AF1_TIM2_CH4 (0x1UL << GPIO_AFRL_AFSEL3_Pos)
#define GPIO_PB6_AF2_TIM4_CH1 (0x2UL << GPIO_AFRL_AFSEL6_Pos)
#define GPIO_PB7_AF2_TIM4_CH2 (0x2UL << GPIO_AFRL_AFSEL7_Pos)

#define GPIO_PA0_AF_MODE      (0x2UL << GPIO_MODER_MODE0_Pos)
#define GPIO_PA1_AF_MODE      (0x2UL << GPIO_MODER_MODE1_Pos)
#define GPIO_PA2_AF_MODE      (0x2UL << GPIO_MODER_MODE2_Pos)
#define GPIO_PA3_AF_MODE      (0x2UL << GPIO_MODER_MODE3_Pos)
#define GPIO_PB6_AF_MODE      (0x2UL << GPIO_MODER_MODE6_Pos)
#define GPIO_PB7_AF_MODE      (0x2UL << GPIO_MODER_MODE7_Pos)
/*
  Timer defines
*/
#define TIM_PRESCALER         (159UL)   // Prescaler for the timers
#define TIM_PERIOD            (20000UL) // Timer period in microseconds

// Initialize the timers instance
timers_s timers = {
  .tim2_cc = {0},
  .tim4_cc = {0}
};

// ----------------------------------------------------------------------
// PRIVATE FUNCTIONS
// ----------------------------------------------------------------------

/*!
  @brief Configure GPIO pins PA0-PA3, PB6, and PB7 as timer outputs
*/
static void configure_pins(void) {
  /*
    Configure PA0-PA3 for use by timer 2
  */
  SET_BIT(    // Set alternate function for PA0-PA3 as timer 2 output pins
    GPIOA->AFR[0],
    GPIO_PA0_AF1_TIM2_CH1 |
    GPIO_PA1_AF1_TIM2_CH2 |
    GPIO_PA2_AF1_TIM2_CH3 |
    GPIO_PA3_AF1_TIM2_CH4
  );
  MODIFY_REG( // Set PA0-PA3 into alternate function mode
    GPIOA->MODER,
    GPIO_MODER_MODE0 |
    GPIO_MODER_MODE1 |
    GPIO_MODER_MODE2 |
    GPIO_MODER_MODE3,
    GPIO_PA0_AF_MODE |
    GPIO_PA1_AF_MODE |
    GPIO_PA2_AF_MODE |
    GPIO_PA3_AF_MODE
  );

  /*
    Configure PB6 and PB7 for use by timer 4
  */
  SET_BIT(    // Set alternate function for PB6 and PB7 as timer 4 output pins
    GPIOB->AFR[0],
    GPIO_PB6_AF2_TIM4_CH1 |
    GPIO_PB7_AF2_TIM4_CH2
  );
  MODIFY_REG( // Set PB6 and PB7 into alternate function mode
    GPIOB->MODER,
    GPIO_MODER_MODE6 |
    GPIO_MODER_MODE7,
    GPIO_PB6_AF_MODE |
    GPIO_PB7_AF_MODE
  );
}

/*!
  @brief Configure timers 2 and 4 to output a 50Hz PWM signal
  @details All 4 channels of timer 2 are used but only channels 1 and 2 of timer 4 are
  used so the other 2 channels of timer 4 aren't configured

  The timers start with the pin outputs held low until the timer enable function is called
*/
static void configure_timers(void) {
  /*
    Set prescaler to 159 for a 1MHz counter clock

    Section 30.5.14 of the reference manual states that the counter clock frequency
    is equal to f_tim_psc_ck / (PSC[15:0] + 1), which in our case would be
    160MHz / (159 + 1) = 1MHz

    This makes the counter increment every 1us which is very convenient because that
    means the PWM pulse width (in microseconds) is the value in the compare register
  */
  SET_BIT(TIM2->PSC, TIM_PRESCALER);
  SET_BIT(TIM4->PSC, TIM_PRESCALER);

  /*
    Set output compare mode to PWM mode 1

    Unfortunately, the OCxM bits aren't contiguous in this register, so we have to
    set the bits individually
  */
  uint32_t oc1m_pwm_mode_1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // 0b0110
  uint32_t oc2m_pwm_mode_1 = TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1; // 0b0110
  uint32_t oc3m_pwm_mode_1 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1; // 0b0110
  uint32_t oc4m_pwm_mode_1 = TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1; // 0b0110
  SET_BIT(TIM2->CCMR1, oc2m_pwm_mode_1 | oc1m_pwm_mode_1);
  SET_BIT(TIM2->CCMR2, oc4m_pwm_mode_1 | oc3m_pwm_mode_1);
  SET_BIT(TIM4->CCMR1, oc2m_pwm_mode_1 | oc1m_pwm_mode_1);

  /*
    Set autoreload register to 19999 for a 50Hz PWM frequency

    50Hz has a period of 20000us and the counter updates every 1us. However, since the
    counter starts at 0, the counter needs to overflow at 19999 for a total of 20000
    counts
  */
  SET_BIT(TIM2->ARR, TIM_PERIOD-1);
  SET_BIT(TIM4->ARR, TIM_PERIOD-1);

  /*
    Enable preloading for the compare registers

    The ensures that any updates to the PWM pulse widths only take effect AFTER a cycle
    has completed and not DURING the cycle
  */
  SET_BIT(TIM2->CCMR1, TIM_CCMR1_OC2PE | TIM_CCMR1_OC1PE);
  SET_BIT(TIM2->CCMR2, TIM_CCMR2_OC4PE | TIM_CCMR2_OC3PE);
  SET_BIT(TIM4->CCMR1, TIM_CCMR1_OC2PE | TIM_CCMR1_OC1PE);

  /*
    Preload the compare registers by generating an update event
  */
  SET_BIT(TIM2->EGR, TIM_EGR_UG);
  SET_BIT(TIM4->EGR, TIM_EGR_UG);

  /*
    Enable the timer pin outputs
  */
  uint32_t tim2_ccer = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
  uint32_t tim4_ccer = TIM_CCER_CC1E | TIM_CCER_CC2E;
  SET_BIT(TIM2->CCER, tim2_ccer);
  SET_BIT(TIM4->CCER, tim4_ccer);

  /*
    Enable the counters
  */
  SET_BIT(TIM2->CR1, TIM_CR1_CEN);
  SET_BIT(TIM4->CR1, TIM_CR1_CEN);
}

// ----------------------------------------------------------------------
// PUBLIC FUNCTIONS
// ----------------------------------------------------------------------

/*!
  @brief Initialize the timers to output 50Hz PWM on the timer pins
*/
void timers_init() {
  configure_pins();
  configure_timers();
}

/*!
  @brief Update the compare registers for the timers
  @details Timer 2 compare registers are 32-bit so the values can be directly written,
  whereas timer 4 compare registers are 16-bit so the values need to be masked before
  being written

  The compare registers are also preloaded which means that the new values don't take
  effect till AFTER a PWM cycle has completed. This preserves the previous pulse to
  prevent potential unwanted behavior from connected devices
*/
void timers_update_cc() {
  WRITE_REG(TIM2->CCR1, timers.tim2_cc[0]);
  WRITE_REG(TIM2->CCR2, timers.tim2_cc[1]);
  WRITE_REG(TIM2->CCR3, timers.tim2_cc[2]);
  WRITE_REG(TIM2->CCR4, timers.tim2_cc[3]);
  WRITE_REG(TIM4->CCR1, timers.tim4_cc[0] & 0xFFFF);
  WRITE_REG(TIM4->CCR2, timers.tim4_cc[1] & 0xFFFF);
}