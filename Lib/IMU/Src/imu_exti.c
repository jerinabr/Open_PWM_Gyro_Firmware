#include "imu_exti.h"
#include "hw_config.h"
#include "stm32g431xx.h"
#include "stm32g4xx.h"
#include <stdint.h>

static volatile uint8_t int1_flag = 0;

/***********************************************************************
-- PUBLIC FUNCTIONS --
***********************************************************************/

/*!
    @brief Configure the IMU INT1 MCU pin as an input and enable the
    falling-edge interrupt
*/
void imu_exti_init(void) {
    /* Configure PB0 as an input */
    MODIFY_REG(
        GPIOB->MODER,
        GPIO_MODER_MODE0_Msk,
        GPIO_PB0_INPUT_MODE
    );

    /* Connect PB0 to the EXTI0 input mux */
    SET_BIT(SYSCFG->EXTICR[0], SYSCFG_EXTICR1_EXTI0_PB);

    /* Mask the EXTI0 interrupt */
    SET_BIT(EXTI->IMR1, EXTI_IMR1_IM0_Msk);

    /* Enable falling-edge trigger for EXTI0 interrupt */
    SET_BIT(EXTI->FTSR1, EXTI_FTSR1_FT0_Msk);

    /* Enable the EXTI0 interrupt */
    NVIC_SetPriority(IMU_INT1_IRQn, IMU_INT1_IRQ_PRIORITY);
    NVIC_EnableIRQ(IMU_INT1_IRQn);
}

/*!
    @brief Indicate if IMU INT1 is active
    @return 1 if the interrupt is active, 0 otherwise
*/
uint8_t is_int1_active(void) {
    return int1_flag;
}

/*!
    @brief Clear the int1 flag
*/
void clear_int1(void) {
    int1_flag = 0;
}

/***********************************************************************
-- INTERRUPT HANDLER --
***********************************************************************/

/*!
    @brief Set the IMU INT1 flag on the falling-edge of the INT1 strobe
*/
void EXTI0_IRQHandler(void) {
    /* Verify the interrupt source was EXTI0 */
    if (EXTI->PR1 & EXTI_PR1_PIF0_Msk) {
        SET_BIT(EXTI->PR1, EXTI_PR1_PIF0_Msk);
        int1_flag = 1;
    }
}