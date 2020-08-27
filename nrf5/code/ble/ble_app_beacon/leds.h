#ifndef _LEDS_H
#define _LEDS_H

#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "led_softblink.h"
#include "app_error.h"
#include "sdk_errors.h"
#include "app_timer.h"
#include "nrf_drv_clock.h"
#include "app_util_platform.h"

void swarm_leds_init(void);
void swarm_leds_loop(void);

#endif /* _LEDS_H */
