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

// Albert: for ADC functions
#include "adc.h"
#include "adc_531.h"

/*
 ****************************************************************************************
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
 
 // Albert: define global variable to watch when debugging
 bool uvp_status __SECTION_ZERO("retention_mem_area0");
 int uart_status __SECTION_ZERO("retention_mem_area0");
 
 // Albert: define timer for ADC data collection
 timer_hnd timer_id __SECTION_ZERO("retention_mem_area0");

/*
 ****************************************************************************************
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/

// Albert: will run if DA14531 is connected
void user_on_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
	default_app_on_connection(connection_idx, param);
	
	// print statements
	// process needed to push string to UART during debugging
	// #FIXME loop logic is not working to print whole long statement
	arch_printf("UVP Check Running and test string: JKABEGIJSDKFGIAWKBGDSFBWAIELBEWIOBFJK \n \r");
	uart_status = GetBits32(UART2_USR_REG, UART_BUSY);
	while(uart_status != 0){
		arch_printf_process();
	}

	// set global watch variable
	uvp_status = GPIO_GetPinStatus(UVP_TRIGGER_PORT, UVP_TRIGGER_PIN);
	
	// if voltage supervisor drives pin low, then start system shutdown
	if(uvp_status == false){
		// shutdown MAX9913 by driving pin low
		GPIO_SetInactive(UVP_MAX_SHDN_PORT, UVP_MAX_SHDN_PIN);
		// #TODO set DA14531 to hibernate (lowest power mode)
	}
}

// ALbert: will run if DA14531 is disconnected
void user_on_disconnect( struct gapc_disconnect_ind const *param )
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
// #WIP go through the adc_531.h file for additional settings and confirm with Jerry and datasheet for the specifications
// #TODO remember to use adc_disable(); to stop ADC later based on callback function tree and desired behavior
// #FIXME ANY CHANGES TO ADC CONFIG MUST BE APPLIED WHEN ADC IS OFF
void adc_init_continuous(void)
{
    // ADC config structure
    adc_config_t adc_config_struct =
    {
				// Measure from 1 pin with respect to ground
        .input_mode = ADC_INPUT_MODE_SINGLE_ENDED,
				// Set pin 6 for single ended input mode
        .input = ADC_INPUT_SE_P0_6,
			
				// #WIP Sets sample time multiplier i.e. how long ADC samples before converting data to digital
        .smpl_time_mult = 1,
			
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
        .oversampling = 0,
    };
		
		// Initialize ADC with structure defined above
    adc_init(&adc_config_struct);

    // Perform offset calibration of the ADC
    adc_offset_calibrate(ADC_INPUT_MODE_SINGLE_ENDED);
		
		// Start the ADC
    adc_start();
}

// Albert: ADC collect data function
// #WIP call this occasionally based on BLE code specification, check with Dhruv
uint16_t adc_collect_sample(void)
{
	uint16_t sample = adc_correct_sample(adc_get_sample());
	return (sample);
}

/*
Albert: template code to edit later
You can also use arch print variable to print data as value in UART terminal for debugging

timer_id = app_easy_timer(200, timer_cb);
static uint16_t gpadc_sample_to_mv(uint16_t sample)
{
    // Resolution of ADC sample depends on oversampling rate
    uint32_t adc_res = 10 + ((6 < adc_get_oversampling()) ? 6 : adc_get_oversampling());

    // Reference voltage is 900mv but scale based in input attenation
    uint32_t ref_mv = 900 * (GetBits16(GP_ADC_CTRL2_REG, GP_ADC_ATTN) + 1);

    return (uint16_t)((((uint32_t)sample) * ref_mv) >> adc_res);
}
static void timer_cb(void)
{
    // Perform single ADC conversion
    uint16_t result = gpadc_read();

    arch_printf("\n\radc result: %dmv", gpadc_sample_to_mv(result));

    // Restart the timer
    timer_id = app_easy_timer(200, timer_cb);
}
*/

/// @} APP