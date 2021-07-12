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
#include "nrf_gpiote.h"
#include "stdlib.h"
#include "nrf_drv_pwm.h"
#include "nrf_drv_timer.h"
#include <math.h>

#define PI 3.14159265358979323846

#ifdef LED_SB_INIT_PARAMS_OFF_TIME_TICKS // Set to 65536
#undef LED_SB_INIT_PARAMS_OFF_TIME_TICKS
#define LED_SB_INIT_PARAMS_OFF_TIME_TICKS 5000
#endif






void swarm_leds_init(void);
void swarm_leds_loop(void);
void swarm_leds_restart(void);
void light(int amplitude);
void ledloop();
int current_phase(void);
void set_phase (int phasecnt);
unsigned long millis();


void pwm_init(void);
void timer_initalize(void);






#endif /* _LEDS_H */
