/*
 *  ======== ti_drivers_config.h ========
 *  Configured TI-Drivers module declarations
 *
 *  The macros defines herein are intended for use by applications which
 *  directly include this header. These macros should NOT be hard coded or
 *  copied into library source code.
 *
 *  Symbols declared as const are intended for use with libraries.
 *  Library source code must extern the correct symbol--which is resolved
 *  when the application is linked.
 *
 *  DO NOT EDIT - This file is generated for the LP_CC2652RB
 *  by the SysConfig tool.
 */
#ifndef ti_drivers_config_h
#define ti_drivers_config_h

#define CONFIG_SYSCONFIG_PREVIEW

#define CONFIG_LP_CC2652RB
#ifndef DeviceFamily_CC26X2
#define DeviceFamily_CC26X2
#endif

#include <ti/devices/DeviceFamily.h>

#include <stdint.h>

/* support C++ sources */
#ifdef __cplusplus
extern "C" {
#endif


/*
 *  ======== CCFG ========
 */
#include DeviceFamily_constructPath(driverlib/osc.h)


/*
 *  ======== GPIO ========
 */

/* DIO6, LaunchPad LED Red */
extern const uint_least8_t              CONFIG_LED_STALL_CONST;
#define CONFIG_LED_STALL                0
/* DIO7, LaunchPad LED Green */
extern const uint_least8_t              CONFIG_LED_DIR_CONST;
#define CONFIG_LED_DIR                  1
/* DIO13, LaunchPad Button BTN-1 (Left) */
extern const uint_least8_t              CONFIG_GPIO_STEP_EN_CONST;
#define CONFIG_GPIO_STEP_EN             2
/* DIO14, LaunchPad Button BTN-2 (Right) */
extern const uint_least8_t              CONFIG_GPIO_MOTOR_DIR_CONST;
#define CONFIG_GPIO_MOTOR_DIR           3
/* DIO0 */
extern const uint_least8_t              PWM_Sense_Stall_CONST;
#define PWM_Sense_Stall                 4
#define CONFIG_TI_DRIVERS_GPIO_COUNT    5

/* LEDs are active high */
#define CONFIG_GPIO_LED_ON  (1)
#define CONFIG_GPIO_LED_OFF (0)

#define CONFIG_LED_ON  (CONFIG_GPIO_LED_ON)
#define CONFIG_LED_OFF (CONFIG_GPIO_LED_OFF)


/*
 *  ======== PIN ========
 */
#include <ti/drivers/PIN.h>

extern const PIN_Config BoardGpioInitTable[];

/* LaunchPad LED Red, Parent Signal: CONFIG_LED_STALL GPIO Pin, (DIO6) */
#define CONFIG_PIN_2                   0x00000006
/* LaunchPad LED Green, Parent Signal: CONFIG_LED_DIR GPIO Pin, (DIO7) */
#define CONFIG_PIN_3                   0x00000007
/* LaunchPad Button BTN-1 (Left), Parent Signal: CONFIG_GPIO_STEP_EN GPIO Pin, (DIO13) */
#define CONFIG_PIN_4                   0x0000000d
/* LaunchPad Button BTN-2 (Right), Parent Signal: CONFIG_GPIO_MOTOR_DIR GPIO Pin, (DIO14) */
#define CONFIG_PIN_5                   0x0000000e
/* Parent Signal: PWM_Sense_Stall GPIO Pin, (DIO0) */
#define CONFIG_PIN_11                   0x00000000
/* Parent Signal: CONFIG_GPTIMER_0 PWM Pin, (DIO12) */
#define CONFIG_PIN_9                   0x0000000c
/* MX25R8035F SPI Flash, Parent Signal: CONFIG_SPI_MASTER SCLK, (DIO10) */
#define CONFIG_PIN_6                   0x0000000a
/* MX25R8035F SPI Flash, Parent Signal: CONFIG_SPI_MASTER MISO, (DIO8) */
#define CONFIG_PIN_7                   0x00000008
/* MX25R8035F SPI Flash, Parent Signal: CONFIG_SPI_MASTER MOSI, (DIO9) */
#define CONFIG_PIN_8                   0x00000009
/* XDS110 UART, Parent Signal: CONFIG_UART_0 TX, (DIO3) */
#define CONFIG_PIN_0                   0x00000003
/* MX25R8035F SPI Flash, Parent Signal: CONFIG_SPI_MASTER SS, (DIO20) */
#define CONFIG_PIN_1                   0x00000014
#define CONFIG_TI_DRIVERS_PIN_COUNT    11


/*
 *  ======== PWM ========
 */

/* DIO12 */
extern const uint_least8_t              CONFIG_PWM_1_CONST;
#define CONFIG_PWM_1                    0
#define CONFIG_TI_DRIVERS_PWM_COUNT     1


/*
 *  ======== SPI ========
 */

/*
 *  MOSI: DIO9
 *  MISO: DIO8
 *  SCLK: DIO10
 *  SS: DIO20
 *  MX25R8035F SPI Flash
 */
extern const uint_least8_t              CONFIG_SPI_MASTER_CONST;
#define CONFIG_SPI_MASTER               0
#define CONFIG_TI_DRIVERS_SPI_COUNT     1


/*
 *  ======== Timer ========
 */

extern const uint_least8_t                  CONFIG_TIMER_0_CONST;
#define CONFIG_TIMER_0                      0
#define CONFIG_TI_DRIVERS_TIMER_COUNT       1


/*
 *  ======== UART ========
 */

/*
 *  TX: DIO3
 *  RX: Unassigned
 *  XDS110 UART
 */
extern const uint_least8_t              CONFIG_UART_0_CONST;
#define CONFIG_UART_0                   0
#define CONFIG_TI_DRIVERS_UART_COUNT    1


/*
 *  ======== GPTimer ========
 */

extern const uint_least8_t                  CONFIG_GPTIMER_0_CONST;
#define CONFIG_GPTIMER_0                    0
extern const uint_least8_t                  CONFIG_GPTIMER_1_CONST;
#define CONFIG_GPTIMER_1                    1
#define CONFIG_TI_DRIVERS_GPTIMER_COUNT     2


/*
 *  ======== Board_init ========
 *  Perform all required TI-Drivers initialization
 *
 *  This function should be called once at a point before any use of
 *  TI-Drivers.
 */
extern void Board_init(void);

/*
 *  ======== Board_initGeneral ========
 *  (deprecated)
 *
 *  Board_initGeneral() is defined purely for backward compatibility.
 *
 *  All new code should use Board_init() to do any required TI-Drivers
 *  initialization _and_ use <Driver>_init() for only where specific drivers
 *  are explicitly referenced by the application.  <Driver>_init() functions
 *  are idempotent.
 */
#define Board_initGeneral Board_init

#ifdef __cplusplus
}
#endif

#endif /* include guard */
