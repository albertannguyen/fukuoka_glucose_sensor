/*
 ****************************************************************************************
 * INCLUDE FILES - TEMPLATE (DO NOT MODIFY)
 ****************************************************************************************
 */
 
#include "rwip_config.h" // SW configuration
#include "gattc_task.h"
#include "app_api.h"
#include "user_empty_peripheral_template.h"

/*
 ****************************************************************************************
 * INCLUDE FILES - ALBERT NGUYEN
 ****************************************************************************************
 */

// for GPIO settings
#include "gpio.h"
#include "user_periph_setup.h"

// for UART serial port output
#include "arch_console.h"

/*
 ****************************************************************************************
 * GLOBAL VARIABLE DEFINITIONS - ALBERT NGUYEN
 ****************************************************************************************
 */
 
 // bool my_led_state __SECTION_ZERO("retention_mem_area0"); // @RETENTION MEMORY

/*
 ****************************************************************************************
 * FUNCTION DEFINITIONS - TEMPLATE & ALBERT NGUYEN
 ****************************************************************************************
		You can modify this one, treat it as the int main loop
*/

// Will run if DA14531 is connected
void user_on_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
	// template code - DO NOT MODIFY
	default_app_on_connection(connection_idx, param);
	
	// print statements
	// process needed to push string to UART during debugging
	arch_printf("UVP Check Running \n \r");
	arch_printf_process();
	arch_printf_process();

	// Albert's code
	// if voltage supervisor drives pin low, then shutdown MAX9913 and go to sleep/hibernate #WIP
	if(GPIO_GetPinStatus(UVP_TRIGGER_PORT, UVP_TRIGGER_PIN) == false){
		GPIO_SetInactive(MAX_SHDN_PORT, MAX_SHDN_PIN);
	}
}

// Will run if DA14531 is disconnected
void user_on_disconnect( struct gapc_disconnect_ind const *param )
{
	// template code - DO NOT MODIFY
	default_app_on_disconnect(param);
}

void user_catch_rest_hndl(ke_msg_id_t const msgid,
                          void const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id)
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