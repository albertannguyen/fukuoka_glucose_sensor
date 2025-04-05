/*
 ****************************************************************************************
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "user_periph_setup.h"
#include "datasheet.h"
#include "system_library.h"
#include "rwip_config.h"
#include "gpio.h"
#include "uart.h"
#include "syscntl.h"

/*
 ****************************************************************************************
 * GPIO RESERVATIONS
 ****************************************************************************************
		i.e. to reserve P0_1 as Generic Purpose I/O:
		RESERVE_GPIO(DESCRIPTIVE_NAME, GPIO_PORT_0, GPIO_PIN_1, PID_GPIO);
		last argument is pin function, do not change from PID_GPIO
 */

#if DEVELOPMENT_DEBUG

void GPIO_reservations(void)
{
// template
#if defined (CFG_PRINTF_UART2)
    RESERVE_GPIO(UART2_TX, UART2_TX_PORT, UART2_TX_PIN, PID_UART2_TX);
#endif

#if !defined (__DA14586__)
    RESERVE_GPIO(SPI_EN, SPI_EN_PORT, SPI_EN_PIN, PID_SPI_EN);
#endif
	
// custom
// reserve pins 8 and 9 as GPIO
// not sure what ports do so left it alone at port 0, seems to be the default for most things
RESERVE_GPIO(UVP_TRIGGER, UVP_TRIGGER_PORT, UVP_TRIGGER_PIN, PID_GPIO);
RESERVE_GPIO(MAX_SHDN, MAX_SHDN_PORT, MAX_SHDN_PIN, PID_GPIO);
}

#endif

/*
 ****************************************************************************************
 * GPIO CONFIGURATION
 ****************************************************************************************
		i.e. to set P0_1 as Generic purpose Output:
		GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_1, OUTPUT, PID_GPIO, false);
		last argument sets level of digital output, true is high and false is low
		last argument is ignored if pin is configured as an input
		this info can be found in gpio.c and gpio.h
 */

void set_pad_functions(void)
{
// template
#if defined (__DA14586__)
    // Disallow spontaneous DA14586 SPI Flash wake-up
    GPIO_ConfigurePin(GPIO_PORT_2, GPIO_PIN_3, OUTPUT, PID_GPIO, true);
#else
    // Disallow spontaneous SPI Flash wake-up
    GPIO_ConfigurePin(SPI_EN_PORT, SPI_EN_PIN, OUTPUT, PID_SPI_EN, true);
#endif

#if defined (CFG_PRINTF_UART2)
    // Configure UART2 TX Pad
    GPIO_ConfigurePin(UART2_TX_PORT, UART2_TX_PIN, OUTPUT, PID_UART2_TX, false);
#endif

// custom
// set pin 8 (UVP_TRIGGER) as input
GPIO_ConfigurePin(UVP_TRIGGER_PORT, UVP_TRIGGER_PIN, INPUT, PID_GPIO, false);
// set pin 9 (MAX_SHDN) as digital output high
GPIO_ConfigurePin(MAX_SHDN_PORT, MAX_SHDN_PIN, OUTPUT, PID_GPIO, true);
}

#if defined (CFG_PRINTF_UART2)
// Configuration struct for UART2
static const uart_cfg_t uart_cfg = {
    .baud_rate = UART2_BAUDRATE,
    .data_bits = UART2_DATABITS,
    .parity = UART2_PARITY,
    .stop_bits = UART2_STOPBITS,
    .auto_flow_control = UART2_AFCE,
    .use_fifo = UART2_FIFO,
    .tx_fifo_tr_lvl = UART2_TX_FIFO_LEVEL,
    .rx_fifo_tr_lvl = UART2_RX_FIFO_LEVEL,
    .intr_priority = 2,
};
#endif

void periph_init(void)
{
#if defined (__DA14531__)
    // In Boost mode enable the DCDC converter to supply VBAT_HIGH for the used GPIOs
		// #WIP need to check this function and use it to set the DCDC converter
    syscntl_dcdc_turn_on_in_boost(SYSCNTL_DCDC_LEVEL_3V0);
#else
    // Power up peripherals' power domain
    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 0);
    while (!(GetWord16(SYS_STAT_REG) & PER_IS_UP));
    SetBits16(CLK_16M_REG, XTAL16_BIAS_SH_ENABLE, 1);
#endif

    // ROM patch
    patch_func();

    // Initialize peripherals
#if defined (CFG_PRINTF_UART2)
    // Initialize UART2
    uart_initialize(UART2, &uart_cfg);
#endif

    // Set pad functionality
    set_pad_functions();

    // Enable the pads
    GPIO_set_pad_latch_en(true);
}
