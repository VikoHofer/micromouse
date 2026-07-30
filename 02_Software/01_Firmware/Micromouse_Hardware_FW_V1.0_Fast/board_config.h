#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/* ----------------------------------------------------------
 * Clock tree per board
 * ---------------------------------------------------------- */
 
#if TARGET_HW == HW_VERSION_1

    /*
     * V1:
     * unstable at 180 MHz, therefore reduced clock
     *
     * SYSCLK = 84 MHz
     * HCLK   = 84 MHz
     * APB1   = 42 MHz  -> APB1 timer clocks = 84 MHz
     * APB2   = 84 MHz  -> APB2 timer clocks = 84 MHz
     */
    #define BOARD_SYSCLK_HZ         84000000UL
    #define BOARD_HCLK_HZ           84000000UL
    #define BOARD_APB1_HZ           42000000UL
    #define BOARD_APB2_HZ           84000000UL

    #define TIM_APB1_CLOCK_HZ       84000000UL
    #define TIM_APB2_CLOCK_HZ       84000000UL

    /*
	 * TIM4 PWM:
	 * f_pwm = timer_clk / ((PSC + 1) * (ARR + 1))
	 *
	 * 84 MHz / (3 + 1) / (1023 + 1) = 20.51 kHz
	 */
	#define PWM_PERIOD              1023
	#define PWM_PRESCALER           3

#endif

/* ----------------------------------------------------------
 * Generic timer base config
 * ---------------------------------------------------------- */
#define TIMER_BASE_TICK_HZ          10000U   /* 0.1 ms tick */
#define TIMER_TICKS_PER_MS          10U

#endif