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

// Single pixel RGB data structure. Make an array out of this to store RGB data for a string.
typedef struct
{

    uint8_t r;
    uint8_t g;
    uint8_t b;
} color;
typedef color btn_color_t;

#define COLOR_PATTERNS_COUNT 6
static btn_color_t color_patterns[COLOR_PATTERNS_COUNT] = {
    {0xFF, 0x00, 0x00},
    {0x00, 0xFF, 0x00},
    {0x00, 0x00, 0xFF},
    {0xFF, 0xFF, 0x00},
    {0xFF, 0x00, 0xFF},
    {0x00, 0xFF, 0xFF},
};

static uint8_t selected_color_pattern = 0;
static btn_color_t m_current_color = {0xFF, 0x00, 0x00};
static btn_color_t m_new_color;

btn_color_t btn_current_color();
void bsp_evt_handler(bsp_event_t evt);
void bsp_configuration();
void button_handler(uint8_t pin_no, uint8_t button_action);

#endif
