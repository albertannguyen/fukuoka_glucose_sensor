#ifndef _USER_EMPTY_PERIPHERAL_TEMPLATE_H_
#define _USER_EMPTY_PERIPHERAL_TEMPLATE_H_

/*
 ****************************************************************************************
 * INCLUDE FILES
 ****************************************************************************************
 */

// template
#include "rwble_config.h"
#include "app_task.h"                  // application task
#include "gapc_task.h"                 // gap functions and messages
#include "gapm_task.h"                 // gap functions and messages
#include "app.h"                       // application definitions
#include "co_error.h"                  // error code definitions

/*
 ****************************************************************************************
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

// template
// yellow warning only shows up after building the project (they can safely be ignored)
void user_on_connection(const uint8_t conidx, struct gapc_connection_req_ind const *param);

void user_on_disconnect(struct gapc_disconnect_ind const *param);

void user_catch_rest_hndl(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);

#endif