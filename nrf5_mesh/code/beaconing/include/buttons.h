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

#include "color.h"

#define BUTTON_PATTERNS_COUNT 3
#define BUTTON_COLORS_COUNT 7
static btn_color_t button_colors[BUTTON_COLORS_COUNT] = {
    {0xFF, 0x00, 0x00},
    {0x00, 0xFF, 0x00},
    {0x00, 0x00, 0xFF},
    {0xFF, 0xFF, 0x00},
    {0xFF, 0x00, 0xFF},
    {0x00, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF},
};

static uint8_t selected_button_color = 0;
static uint8_t selected_button_pattern = 0;
static btn_color_t m_current_color = {0xFF, 0x00, 0x00};
static btn_color_t m_next_color = {0x00, 0x00, 0x00};

uint8_t btn_current_pattern();
void set_button_pattern(uint8_t next_pattern);
void advance_button_pattern(uint8_t direction);

btn_color_t btn_current_color();
btn_color_t btn_next_color();
void set_next_color(btn_color_t next_color);
void advance_button_color(uint8_t direction);

void bsp_evt_handler(bsp_event_t evt);
void bsp_configuration();
void button_handler(uint8_t pin_no, uint8_t button_action);

#endif
