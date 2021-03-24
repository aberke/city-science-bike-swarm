#ifndef _SWARM_BUTTONS_H
#define _SWARM_BUTTONS_H

#include <stdbool.h>
#include <stdint.h>

#include "nrf.h"
#include "nordic_common.h"
#include "boards.h"

// Header files that needs to be included for PPI/GPIOTE/TIMER/TWI example.

// Peripheral driver header files
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_twi.h"
#include "nrf_drv_clock.h"

//#include "nrf_drv_uart.h"

// Library header files
#include "app_timer.h"
#include "app_button.h"
#include "app_pwm.h"

// Headers and defines needed by the logging interface
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_delay.h"
#include "nrf_gpio.h"


void button_handler(uint8_t pin_no, uint8_t button_action);
void buttons_init();

#endif
