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

// for GPIO settings
#include "gpio.h"
#include "user_periph_setup.h"

// for UART serial port debugging
#include "arch_console.h"
// #include "systick.h"
// #include "uart.h"

// for ADC functions
#include "adc.h"
#include "adc_531.h"

// for timer functions
#include "timer0_2.h"
#include "timer2.h"

// for DCDC converter debug
#include "syscntl.h"

/*
 ****************************************************************************************
 * DEFINES
 ****************************************************************************************
 */

// datasheet values
#define MIN_PWM_DIV 2
#define MAX_PWM_DIV 16383
#define SYS_CLK_FREQ_HZ 16000000
#define LP_CLK_FREQ_HZ 32000

// clamp macro
#define CLAMP(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

/*
 ****************************************************************************************
 * GLOBAL RETENTION VARIABLE DEFINITIONS
 ****************************************************************************************
 */

// ADC variables
timer_hnd adc_timer __SECTION_ZERO("retention_mem_area0");
uint16_t adc_input __SECTION_ZERO("retention_mem_area0");
uint16_t adc_input_volt __SECTION_ZERO("retention_mem_area0");
bool adc_timer_started __SECTION_ZERO("retention_mem_area0");

/*
 ****************************************************************************************
 * UVP FUNCTIONS
 ****************************************************************************************
*/

void uvp_shdn(void)
{
	// if voltage supervisor drives pin low, then start system shutdown
	if(GPIO_GetPinStatus(UVP_TRIGGER_PORT, UVP_TRIGGER_PIN) == false){
		// shutdown MAX9913 by driving pin low
		GPIO_SetInactive(UVP_MAX_SHDN_PORT, UVP_MAX_SHDN_PIN);
		// TODO set DA14531 to hibernate (lowest power mode)
	}
}

/*
 ****************************************************************************************
 * ADC FUNCTIONS
 ****************************************************************************************
*/

// ADC main code, reads and prints to UART terminal in a timer callback loop
// WIP single mode works but not continuous
// Single mode output: Raw = 9, Volt = 31 mV with no connection (valid floating output)
void gpadc_timer_cb(void)
{
	// Read and print ADC value to UART
	adc_input = gpadc_collect_sample();
	adc_input_volt = gpadc_sample_to_mv(adc_input);
	arch_printf("Register Value: %d | Voltage: %d mV \n\r", adc_input, adc_input_volt);
	
	// Restart the timer
	adc_timer = app_easy_timer(100, gpadc_timer_cb);
}

// WIP email company about how to implement this as interrupt is not being triggered after conversion in continuous mode
void gpadc_interrupt(void)
{
	// Read and print ADC value
	adc_input = gpadc_collect_sample();
	adc_input_volt = gpadc_sample_to_mv(adc_input);
	
	// arch_printf will only print once callback function returns
	arch_printf("Register Value: %d | Voltage: %d mV \n\r", adc_input, adc_input_volt);
	
	// Clear the interrupt
	adc_clear_interrupt();
}

// TODO play with settings and see which gives the most accurate reading
// TODO read datasheet and calculate manual mode settings that gives highest sampling rate and accuracy
void gpadc_init(void)
{
	// ADC config structure, details about range of inputs for parameters found in adc_531.h
	adc_config_t adc_config_struct =
	{
			// Measure from 1 pin with respect to ground
			.input_mode = ADC_INPUT_MODE_SINGLE_ENDED,
			// Set pin 6 for single ended input mode
			.input = ADC_INPUT_SE_P0_6,
		
			// Sets sample time multiplier, see adc_set_sample_time
			.smpl_time_mult = 2, // set at lowest for highest sampling rate
		
			// Set continuous measurement mode
			.continuous = false,
			// Set interval time between conversions, see adc_set_interval
			.interval_mult = 0,
			
			// Set attenuation factor
			.input_attenuator = ADC_INPUT_ATTN_4X, // less noise
			
			// Enable chopping algorithm, refer to datasheet
			.chopping = false, // more accuracy at cost of sampling rate
			
			// Set oversampling mode, see adc_set_oversampling
			.oversampling = 0 // not necessary as stated by grad student Jialiang
	};
	
	// Initialize ADC with structure defined above
	// ANY CHANGES TO ADC CONFIG MUST BE APPLIED WHEN ADC IS OFF
	adc_init(&adc_config_struct);
	
	// Disable input shifter (only used for measuring negative values)
	adc_input_shift_disable();
	// Disable die temperature sensor
	adc_temp_sensor_disable();

	// Perform offset calibration of the ADC
	adc_reset_offsets();
	adc_offset_calibrate(ADC_INPUT_MODE_SINGLE_ENDED);
	
	// FIXME Register interrupt function to be used when ADC is on in continuous mode
	// adc_register_interrupt(gpadc_interrupt);
	
	// consider using adc_ldo_const_current_enable() if getting noisy readings at lower voltage
}

uint16_t gpadc_collect_sample(void)
{
	// adc_get_sample() is only for single mode
	
	// Read data from ADC register, which always holds the latest conversion results and can be read at any time
	
	// Single mode operation
	uint16_t sample = adc_correct_sample(adc_get_sample());
	
	// Continuous mode operation
	// uint16_t sample = adc_correct_sample(GetWord16(GP_ADC_RESULT_REG));
	
	return (sample);
}

// code snippet given by Renesas
uint16_t gpadc_sample_to_mv(uint16_t sample)
{
    // Effective resolution of ADC sample based on oversampling rate	
    uint32_t adc_resolution = 10 + ((6 < adc_get_oversampling()) ? 6 : adc_get_oversampling());

    // Reference voltage is 900mv but can be scaled based on input attenation
    uint32_t ref_mv = 900 * (GetBits16(GP_ADC_CTRL2_REG, GP_ADC_ATTN) + 1);
		
		// Returns mV value read by the ADC
    return (uint16_t)((((uint32_t)sample) * ref_mv) >> adc_resolution);
}

/*
 ****************************************************************************************
 * PWM FUNCTIONS
 ****************************************************************************************
*/

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
	uint8_t clk_div_int = 1 << clk_div;
	uint32_t clk_freq = (clk_src == TIM2_CLK_SYS) ? SYS_CLK_FREQ_HZ : LP_CLK_FREQ_HZ,
					 input_freq = clk_freq / clk_div_int;

	// Clamp pwm_div if beyond datasheet range of 2 to (2^14 - 1)
	pwm_div = CLAMP(pwm_div, MIN_PWM_DIV, MAX_PWM_DIV);
	
	// TODO fully determine compile bug if arch_printf is included
	/*
	if (pwm_div < MIN_PWM_DIV) {
    pwm_div = MIN_PWM_DIV;
    // arch_printf("pwm_div is below minimum, clamped to min value %d \n\r", MIN_PWM_DIV);
		// __BKPT(0); // force breakpoint here because debugger cannot stop in here, it has been optimized out of build
	} else if (pwm_div > MAX_PWM_DIV) {
    pwm_div = MAX_PWM_DIV;
    // arch_printf("pwm_div is above maximum, clamped to max value %d \n\r", MAX_PWM_DIV);
	}
	*/

	// Set PWM frequency based on datasheet for Timer 2
	timer2_pwm_freq_set(input_freq / pwm_div, input_freq);
}

void timer2_pwm_enable(uint8_t dc_pwm2, uint8_t offset_pwm2, uint8_t dc_pwm3, uint8_t offset_pwm3)
{
	// Clamp values if inputs are outside of 0-100% range
	/*
	dc_pwm2 = CLAMP(dc_pwm2, 0, 100);
	offset_pwm2 = CLAMP(offset_pwm2, 0, 100);
	dc_pwm3 = CLAMP(dc_pwm3, 0, 100);
	offset_pwm3 = CLAMP(offset_pwm3, 0, 100);
	*/
	
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
	// This function already comes with ASSERT_WARNING input protection, no need for clamping
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

/*
 ****************************************************************************************
 * DEFAULT CALLBACK FUNCTIONS
 ****************************************************************************************
*/

void user_on_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
	default_app_on_connection(connection_idx, param);
	
	// Ensures that code only runs once
	if (!adc_timer_started)
	{
		// ADC test code
		gpadc_init();
		adc_enable(); // powers on ADC
		adc_timer = app_easy_timer(100, gpadc_timer_cb);
		adc_timer_started = true;
	}
}

void user_on_disconnect(struct gapc_disconnect_ind const *param )
{
	default_app_on_disconnect(param);
}

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
 * STARTUP LOOP
 ****************************************************************************************
*/

void user_app_on_init(void)
{
	// start the default initialization process for BLE user application
	default_app_on_init();
	
	// TODO make changes to DCDC converter and observe how it changes output of GPIOs
	syscntl_dcdc_level_t vdd = syscntl_dcdc_get_level();
	adc_timer_started = false;
	
	// PWM test code
	// max voltage is 3.3 V on LP clock source, min is 0 V
	// this is because GPIO is supplied by VBAT_HIGH or the 3.3 V LDO on devkit
	
	/*
	timer2_pwm_init(TIM0_2_CLK_DIV_8, TIM2_CLK_LP, TIM2_HW_PAUSE_OFF, 0xFFFF);
	timer2_pwm_enable(50, 0, 25, 0);
	*/
	
	// UVP test code
	// if condition passes if trigger pin is driven low and turns off the MAX SHDN pin
	// GPIO high is around 3 V, and low is 0 V
	
	/*
	uvp_trigger_status = GPIO_GetPinStatus(UVP_TRIGGER_PORT, UVP_TRIGGER_PIN);
	while(1){
		uvp_shdn();
	}
	*/
}

/// @} APP