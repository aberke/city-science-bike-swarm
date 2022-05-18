#ifndef _SWARM_COLOR_H
#define _SWARM_COLOR_H

#include <stdbool.h>
#include <stdint.h>

#include "nrf.h"
#include "nordic_common.h"
#include "boards.h"

// Single pixel RGB data structure. Make an array out of this to store RGB data for a string.
typedef struct
{

    uint8_t r;
    uint8_t g;
    uint8_t b;
} COLOR;
typedef COLOR btn_color_t;

btn_color_t change_hsv_c(
    const btn_color_t in,
    const float fHue,
    const float fSat,
    const float fVal);

#endif
