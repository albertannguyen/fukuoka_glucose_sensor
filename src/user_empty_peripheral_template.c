/**
 ****************************************************************************************
 * @file user_empty_peripheral_template.c
 * @brief Empty peripheral template project source code.
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 ****************************************************************************************
 * INCLUDE FILES
 ****************************************************************************************
 */
 
#include "rwip_config.h" // SW configuration
#include "gattc_task.h"
#include "app_api.h"
#include "user_empty_peripheral_template.h"

// Albert:
// for GPIO settings
#include "gpio.h"
#include "user_periph_setup.h"
// for UART serial port output
#include "arch_console.h"
// #include "uart.h"
// for ADC functions
#include "adc.h"
#include "adc_531.h"
// for timer functions
#include "timer0_2.h"
#include "timer2.h"

/*
 ****************************************************************************************
 * DEFINES
 ****************************************************************************************
 */

#define MIN_PWM_DIV 2
#define MAX_PWM_DIV 16383
#define SYS_CLK_FREQ_HZ 16000000
#define LP_CLK_FREQ_HZ 32000

/*
 ****************************************************************************************
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
 
 // Albert:
 // debug watch variables
 bool uart_busy_status __SECTION_ZERO("retention_mem_area0");
 // define timer for ADC data collection
 // timer_hnd timer_id __SECTION_ZERO("retention_mem_area0");

/*
 ****************************************************************************************
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/

// Albert: callback function runs on startup/reset
void user_app_on_init(void)
{
	// #TODO find way to flush UART string buffer to terminal during debugging line by line
	arch_printf("UVP Check Running and test string: JKABEGIJSDKFGIAWKBGDSFBWAIELBEWIOBFJK \n \r");
	uart_busy_status = GetBits32(UART2_USR_REG, UART_BUSY);
	while(uart_busy_status){
		arch_printf_process();
	}
	
	uvp_shdn();
	
	// start the default initialization process for BLE user application
	default_app_on_init();
}

// will run if DA14531 is connected
void user_on_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
	default_app_on_connection(connection_idx, param);
}

// will run if DA14531 is disconnected
void user_on_disconnect(struct gapc_disconnect_ind const *param )
{
	default_app_on_disconnect(param);
}

// template code that catches unhandled messages from BLE
void user_catch_rest_hndl(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    switch(msgid)
    {
        case GATTC_EVENT_REQ_IND:
        {
            // Confirm unhandled indication to avoid GATT timeout
            struct gattc_event_ind const *ind = (struct gattc_event_ind const *) param;
            struct gattc_event_cfm *cfm = KE_MSG_ALLOC(GATTC_EVENT_CFM, src_id, dest_id, gattc_event_cfm);
            cfm->handle = ind->handle;
            KE_MSG_SEND(cfm);
        } break;

        default:
            break;
    }
}

/*
 ****************************************************************************************
 * UVP FUNCTIONS (Albert)
 ****************************************************************************************
*/

void uvp_shdn(void)
{
	// if voltage supervisor drives pin low, then start system shutdown
	if(GPIO_GetPinStatus(UVP_TRIGGER_PORT, UVP_TRIGGER_PIN) == false){
		// shutdown MAX9913 by driving pin low
		GPIO_SetInactive(UVP_MAX_SHDN_PORT, UVP_MAX_SHDN_PIN);
		// #TODO set DA14531 to hibernate (lowest power mode)
	}
}

/*
 ****************************************************************************************
 * ADC FUNCTIONS (Albert)
 ****************************************************************************************
*/

// #TODO remember to use adc_disable(); to stop ADC later based on callback function tree and desired behavior
// #TODO ANY CHANGES TO ADC CONFIG MUST BE APPLIED WHEN ADC IS OFF
void gpadc_init(void)
{
    // ADC config structure
    adc_config_t adc_config_struct =
    {
				// Measure from 1 pin with respect to ground
        .input_mode = ADC_INPUT_MODE_SINGLE_ENDED,
				// Set pin 6 for single ended input mode
        .input = ADC_INPUT_SE_P0_6,
				// Sets sample time multiplier, see adc_set_sample_time() function
        .smpl_time_mult = 0,
				// Set continous measurement mode
        .continuous = true,
				// Set conversion to have no wait interval
        .interval_mult = 0,
				// Set no attenuation of input
        .input_attenuator = ADC_INPUT_ATTN_NO,
				
				// Enable chopping algorithm, refer to datasheet
				// #WIP check with sponsor if this is desired
        .chopping = true,
					
				// Disables oversampling, which can improve accuracy at cost of sample rate
        .oversampling = 0
    };
		// Initialize ADC with structure defined above
    adc_init(&adc_config_struct);
		// Disable input shifter (for measuring negative values)
		adc_input_shift_disable();
		// Disable die temperature sensor
		adc_temp_sensor_disable();

    // Perform offset calibration of the ADC
		adc_reset_offsets();
    adc_offset_calibrate(ADC_INPUT_MODE_SINGLE_ENDED);
		
		// #WIP consider using adc_ldo_const_current_enable() if getting noisy readings at lower voltage
}

// #WIP call this repeatedly based on BLE transmission specifications
uint16_t gpadc_collect_sample(void)
{
	// Power on the ADC
	adc_enable();
	
	// Start a conversion and collect sample
	adc_start();
	uint16_t sample = adc_correct_sample(adc_get_sample());
	
	// Power down the ADC
	adc_disable();
	
	return (sample);
}

// code taken and adjusted from ADC peripheral driver example section 10
// #TODO you can also use arch print variable to print data as value in UART terminal for debugging
uint16_t gpadc_sample_to_mv(uint16_t sample)
{
    // Effective resolution of ADC sample based on oversampling rate	
    uint32_t adc_resolution = 10 + ((6 < adc_get_oversampling()) ? 6 : adc_get_oversampling());

    // Reference voltage is 900mv but can be scaled based on input attenation
    uint32_t ref_mv = 900 * (GetBits16(GP_ADC_CTRL2_REG, GP_ADC_ATTN) + 1);

    return (uint16_t)((((uint32_t)sample) * ref_mv) >> adc_resolution);
}

/*
// Test callback code
void timer_cb(void)
{
    // Perform single ADC conversion
    uint16_t result = adc_init_continuous();

    arch_printf("\n\radc result: %dmv", gpadc_sample_to_mv(result));

    // Restart the timer
    timer_id = app_easy_timer(200, timer_cb);
}
*/

/*
 ****************************************************************************************
 * PWM FUNCTIONS (Albert)
 ****************************************************************************************
*/

// NOTE THAT pwm_div IS ONLY LIMITED TO VALUES FROM 2 TO (2^14 - 1)
// #TODO implement a protection for this so that pwm_div never goes outside of this range
void timer2_pwm_init(tim0_2_clk_div_t clk_div, tim2_clk_src_t clk_src, tim2_hw_pause_t hw_pause, uint16_t pwm_div)
{
	// Define timer parameters in struct
	tim0_2_clk_div_config_t clk_cfg = {
		.clk_div = clk_div
	};
	
	tim2_config_t tmr_cfg =
	{
		.clk_source = clk_src,
    .hw_pause = hw_pause
	};
	
	// Set timer parameters
	timer0_2_clk_div_set(&clk_cfg);
	timer2_config(&tmr_cfg);
	
	// Define PWM parameters
	uint32_t clk_freq = (clk_src == TIM2_CLK_SYS) ? SYS_CLK_FREQ_HZ : LP_CLK_FREQ_HZ,
					 clk_div_int = (clk_div == TIM0_2_CLK_DIV_1) ? 1 :
																(clk_div == TIM0_2_CLK_DIV_2) ? 2 :
																(clk_div == TIM0_2_CLK_DIV_4) ? 4 : 8,
					 input_freq = clk_freq / clk_div_int;

	// Set PWM parameters
	// Input and output frequency of this function is defined and set based on datasheet for timer 2
	timer2_pwm_freq_set(input_freq / pwm_div, input_freq);
}

void timer2_pwm_enable(uint8_t dc_pwm2, uint8_t offset_pwm2, uint8_t dc_pwm3, uint8_t offset_pwm3)
{
	// Define PWM parameters in struct
	tim2_pwm_config_t pwm2_cfg = {
		.pwm_signal = TIM2_PWM_2,
		.pwm_dc = dc_pwm2,
		.pwm_offset = offset_pwm2
	};
	
	tim2_pwm_config_t pwm3_cfg = {
		.pwm_signal = TIM2_PWM_3,
		.pwm_dc = dc_pwm3,
		.pwm_offset = offset_pwm3
	};
	
	// Set PWM parameters
	timer2_pwm_signal_config(&pwm2_cfg);
	timer2_pwm_signal_config(&pwm3_cfg);
	
	// Enable timer input clock
	timer0_2_clk_enable();
	
	// Enable PWM signal
	timer2_start();
}

void timer2_pwm_disable(void)
{
	// Disable PWM signal
	timer2_stop();

	// Disable timer input clock
	timer0_2_clk_disable();
}

/// @} APP