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


// Albert: for GPIO settings
#include "gpio.h"
#include "user_periph_setup.h"

// Albert: for UART serial port output
#include "arch_console.h"
// #include "uart.h"

// Albert: for ADC functions
#include "adc.h"
#include "adc_531.h"

// Albert: for timer functions
#include "timer0_2.h"
#include "timer2.h"

/*
 ****************************************************************************************
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
 
 // Albert: debug watch variables
 bool uart_busy_status __SECTION_ZERO("retention_mem_area0");
 
 // Albert: define timer for ADC data collection
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
	
	// #FIXME bad code
	uart_busy_status = GetBits32(UART2_USR_REG, UART_BUSY);
	while(uart_busy_status){
		arch_printf_process();
	}
	
	// if voltage supervisor drives pin low, then start system shutdown
	if(GPIO_GetPinStatus(UVP_TRIGGER_PORT, UVP_TRIGGER_PIN) == false){
		// shutdown MAX9913 by driving pin low
		GPIO_SetInactive(UVP_MAX_SHDN_PORT, UVP_MAX_SHDN_PIN);
		// #TODO set DA14531 to hibernate (lowest power mode)
	}
	
	// start the default initialization process for BLE user application
	default_app_on_init();
}

// Albert: will run if DA14531 is connected
void user_on_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
	default_app_on_connection(connection_idx, param);
}

// Albert: will run if DA14531 is disconnected
void user_on_disconnect(struct gapc_disconnect_ind const *param )
{
	default_app_on_disconnect(param);
}

// template code that handles unhandled messages from BLE
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

// Albert: ADC initialization function
// #TODO remember to use adc_disable(); to stop ADC later based on callback function tree and desired behavior
// #TODO ANY CHANGES TO ADC CONFIG MUST BE APPLIED WHEN ADC IS OFF
void adc_initialize(void)
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

// Albert: ADC collect data function
// ADC is 10 bits long, but can be extended to 16 bits via oversampling
// #WIP call this repeatedly based on BLE GATT profile specification
uint16_t adc_collect_sample(void)
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

// Albert: code taken and adjusted from ADC peripheral driver example section 10
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

// Albert: config timer 2 and PWM frequencies
void timer2_initialize_pwm(void)
{
	// #TODO move config files outside of these functions, maybe in header file?
	// Define input clock division factor
	tim0_2_clk_div_config_t timer_clk_config = {
		.clk_div = TIM0_2_CLK_DIV_1
	};
	
	timer0_2_clk_div_set(&timer_clk_config);
	
	// Define timer 2 config for max performance
	tim2_config_t timer_hw_config =
	{
    .hw_pause = TIM2_HW_PAUSE_OFF,
		.clk_source = TIM2_CLK_SYS
	};
	
	timer2_config(&timer_hw_config);

	// System clock (16 MHz), divided by a factor defined by the timer_clk_config, is the input frequency of this function
	// #TODO make a header variable for the PWM frequency (set at the max output frequency of the timer according to the datasheet for now)
	timer2_pwm_freq_set(16000000 / 2, 16000000 / 1);
}

// Albert: enable timer 2's PWM 2 and PWM 3 output
// #TODO pass config parameters as arguments in this function and call it in app_init
void timer2_enable_pwm(void)
{
	// Set PWM parameters
	tim2_pwm_config_t pwm_2_config = {
		.pwm_signal = TIM2_PWM_2,
		.pwm_dc = 50,
		.pwm_offset = 0
	};
	
	timer2_pwm_signal_config(&pwm_2_config);
	
	// Enable timer input clock
	timer0_2_clk_enable();
	
	// Enable PWM signal
	timer2_start();
	
	// Disbale PWM signal
	timer2_stop();

	// Disable the input clock
	timer0_2_clk_disable();
}

/// @} APP