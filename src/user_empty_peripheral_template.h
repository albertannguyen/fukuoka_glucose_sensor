/**
 ****************************************************************************************
 * @file user_empty_peripheral_template.h
 * @brief Empty peripheral template project header file.
 ****************************************************************************************
 */

#ifndef _USER_EMPTY_PERIPHERAL_TEMPLATE_H_
#define _USER_EMPTY_PERIPHERAL_TEMPLATE_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 * @brief 
 * @{
 ****************************************************************************************
 */

/*
 ****************************************************************************************
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"
#include "app_task.h"                  // application task
#include "gapc_task.h"                 // gap functions and messages
#include "gapm_task.h"                 // gap functions and messages
#include "app.h"                       // application definitions
#include "co_error.h"                  // error code definitions

/****************************************************************************
Add here supported profiles' application header files.
i.e.
#if (BLE_DIS_SERVER)
#include "app_dis.h"
#include "app_dis_task.h"
#endif
*****************************************************************************/

// Albert: includes for PWM functions
#include "timer0_2.h"
#include "timer2.h"

/*
 ****************************************************************************************
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
 
// Albert: Custom rerouted callback functions from user_callback_config.h
void user_app_on_init(void);

// Albert: UVP functions
void uvp_shdn(void);

// Albert: ADC functions
void gpadc_init(void);
uint16_t gpadc_collect_sample(void);
uint16_t gpadc_sample_to_mv(uint16_t sample);
void gpadc_timer_cb(void);

// Albert: PWM functions
void timer2_pwm_init(tim0_2_clk_div_t clk_div, tim2_clk_src_t clk_src, tim2_hw_pause_t hw_pause, uint16_t pwm_div);
void timer2_pwm_enable(uint8_t dc_pwm2, uint8_t offset_pwm2, uint8_t dc_pwm3, uint8_t offset_pwm3);
void timer2_pwm_disable(void);

/**
 ****************************************************************************************
 * @brief Connection function.
 * @param[in] conidx        Connection Id index
 * @param[in] param         Pointer to GAPC_CONNECTION_REQ_IND message
 ****************************************************************************************
		Albert: yellow warning only shows up after building the project (they can be ignored for now)
		Runs when Bluethooth is initially connected
*/
void user_on_connection(const uint8_t conidx, struct gapc_connection_req_ind const *param);

/**
 ****************************************************************************************
 * @brief Disconnection function.
 * @param[in] param         Pointer to GAPC_DISCONNECT_IND message
 ****************************************************************************************
*/
void user_on_disconnect(struct gapc_disconnect_ind const *param);

/**
 ****************************************************************************************
 * @brief Handles the messages that are not handled by the SDK internal mechanisms.
 * @param[in] msgid   Id of the message received.
 * @param[in] param   Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id  ID of the sending task instance.
 ****************************************************************************************
*/
void user_catch_rest_hndl(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);

/// @} APP

#endif