#ifndef _SWARM_BUTTONS_H
#define _SWARM_BUTTONS_H

#include <stdbool.h>
#include <stdint.h>

#include "nrf.h"
#include "nordic_common.h"
#include "boards.h"

#include "bsp.h"

// Headers and defines needed by the logging interface
#include "log.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


void bsp_evt_handler(bsp_event_t evt);
void bsp_configuration();
void button_handler(uint8_t pin_no, uint8_t button_action);

#endif
