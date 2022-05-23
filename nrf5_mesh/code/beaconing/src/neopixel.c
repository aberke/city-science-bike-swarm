/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */

#include <stdint.h>
#include <string.h>
#include <math.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_util_platform.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "easing.h"

#include "boards.h"

#include "nrf_drv_i2s.h"
#include "nrfx_i2s.h"
#include "neopixel.h"
#include "app_timer.h"
#include "buttons.h"
#include "leds.h"

//volatile uint8_t g_demo_mode = 0;
//volatile bool g_i2s_start = true;
//volatile bool g_i2s_running = false;
volatile bool neopixel_running = false;
bool init = false;

#define NLEDS 72
#define RESET_BITS 6
#define I2S_BUFFER_SIZE 3 * NLEDS + RESET_BITS

static uint32_t m_buffer_tx[I2S_BUFFER_SIZE];
static volatile int nled = 1;

static float m_led_strip_meteor_decay[NLEDS];
static float last_seen_phase_progress = 1.f;
static uint8_t m_led_strip_heat[NLEDS];
static uint32_t meteor_timer;
static uint16_t last_seen_meteor_position;

APP_TIMER_DEF(my_timer_id);

// Reverses a string 'str' of length 'len'
void reverse(char *str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

// Converts a given integer x to string str[].
// d is the number of digits required in the output.
// If d is more than the number of digits in x,
// then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x)
    {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}

// Converts a floating-point/double number to a string.
void ftoa(float n, char *res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;

    // convert integer part to string
    int i = intToStr(ipart, res, 0);

    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.'; // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter
        // is needed to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);

        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

static void timeout_handler(void *p_context)
{

    nrf_drv_i2s_stop();
    nrf_drv_i2s_uninit();
    neopixel_running = false;
}

// This is the I2S data handler - all data exchange related to the I2S transfers
// is done here.

static void data_handler(nrfx_i2s_buffers_t const *p_released, uint32_t status)
{

    // Non-NULL value in 'p_data_to_send' indicates that the driver needs
    // a new portion of data to send.
    if (p_released->p_tx_buffer != NULL)
    {
        // do nothing - buffer is updated elsewhere
    }
}

/*

    calcChannelValue()

    Sets up a 32 bit value for a channel (R/G/B). 

    A channel has 8 x 4-bit codes. Code 0xe is HIGH and 0x8 is LOW.

    So a level of 128 would be represented as:

    0xe8888888

    The 16 bit values need to be swapped because of the way I2S sends data - right/left channels.

    So for the above example, final value sent would be:

    0x8888e888

*/
uint32_t calcChannelValue(uint8_t level)
{
    uint32_t val = 0;

    // 0
    if (level == 0)
    {
        val = 0x88888888;
    }
    // 255
    else if (level == 255)
    {
        val = 0xeeeeeeee;
    }
    else
    {
        // apply 4-bit 0xe HIGH pattern wherever level bits are 1.
        val = 0x88888888;
        for (uint8_t i = 0; i < 8; i++)
        {
            if ((1 << i) & level)
            {
                uint32_t mask = ~(0x0f << 4 * i);
                uint32_t patt = (0x0e << 4 * i);
                val = (val & mask) | patt;
            }
        }

        // swap 16 bits
        val = (val >> 16) | (val << 16);
    }

    return val;
}

// set LED data
void set_led_data(uint8_t position, uint8_t r_level, uint8_t g_level, uint8_t b_level)
{
    m_buffer_tx[position * 3] = calcChannelValue(g_level);
    m_buffer_tx[position * 3 + 1] = calcChannelValue(r_level);
    m_buffer_tx[position * 3 + 2] = calcChannelValue(b_level);

    // // reset
    // for (int i = 3 * position; i < I2S_BUFFER_SIZE; i++)
    // {
    //     m_buffer_tx[i] = 0;
    // }
}

uint32_t millis(void)
{
    return (app_timer_cnt_get() / 32.768);
}

uint32_t compareMillis(uint32_t previousMillis, uint32_t currentMillis)
{
    if (currentMillis < previousMillis)
        return (currentMillis + OVERFLOW + 1 - previousMillis);
    return (currentMillis - previousMillis);
}

/**@brief Application main function.
 */
void neopixel(int amplitude, int phase)
{
    uint32_t err_code;
    static bool init = false;

    if (init == false)
    {
        err_code = app_timer_create(&my_timer_id, APP_TIMER_MODE_REPEATED, timeout_handler);
        APP_ERROR_CHECK(err_code);
        //
        err_code = app_timer_start(my_timer_id, APP_TIMER_TICKS(10), NULL);
        APP_ERROR_CHECK(err_code);
        init = true;
    }

    float amplitude_progress = (float)amplitude / 255.f;
    float phase_progress = (float)phase / PHASE_DURATION;

    uint8_t r_level;
    uint8_t g_level;
    uint8_t b_level;
    btn_color_t current_color = btn_current_color();
    btn_pattern_t current_pattern = btn_current_pattern();

    if (current_pattern == BUTTON_PATTERN_WHOLE_FADE)
    {
        for (int i = 0; i < NLEDS; i += 1)
        {
            r_level = current_color.r * amplitude_progress;
            g_level = current_color.g * amplitude_progress;
            b_level = current_color.b * amplitude_progress;

            m_led_strip_meteor_decay[i] = amplitude_progress;

            set_led_data(i, r_level, g_level, b_level);
        }
    }
    else if (current_pattern == BUTTON_PATTERN_METEOR)
    {
        if (phase_progress < last_seen_phase_progress)
        {
            // New phase, throw meteor
            meteor_timer = millis();
        }

        uint32_t meteor_duration = (int)PHASE_DURATION / 2;

        float meteor_progress = (millis() - meteor_timer) / (float)meteor_duration;
        uint16_t meteor_position = floor(QuadraticEaseOut(meteor_progress) * NLEDS);
        if (meteor_position < last_seen_meteor_position)
        {
            last_seen_meteor_position = 0;
        }
        // char phase_progress_str[20];
        // ftoa(phase_progress, phase_progress_str, 2);
        // char last_seen_phase_progress_str[20];
        // ftoa(last_seen_phase_progress, last_seen_phase_progress_str, 2);
        char meteor_progress_str[20];
        ftoa(meteor_progress, meteor_progress_str, 2);
        // __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " ---> Meteor progress %s (%d), %d (%d)\n", meteor_progress_str, millis() - meteor_timer, meteor_duration, meteor_timer);
        // __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " ---> Time: %d, Meteor duration %d, timer %d,  %d %s/%s\n",
        //   millis(), meteor_duration, meteor_timer, phase, phase_progress_str, last_seen_phase_progress_str);

        for (int i = 0; i < NLEDS; i += 1)
        {
            // Animate meteor forward
            if (meteor_progress <= 1 && i <= meteor_position && i >= last_seen_meteor_position)
            {
                last_seen_meteor_position = i;
                // Relight pixels
                if (i == 10)
                {
                    char m_led_strip_meteor_decay_str[20];
                    ftoa(m_led_strip_meteor_decay[i], m_led_strip_meteor_decay_str, 2);
                    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " ---> Pixel %d not yet lit, lighting %s\n", i, m_led_strip_meteor_decay_str);
                }
                r_level = current_color.r * 1;
                g_level = current_color.g * 1;
                b_level = current_color.b * 1;

                m_led_strip_meteor_decay[i] = 1.f;
                set_led_data(i, r_level, g_level, b_level);
            }

            // Pixels behind meteor
            if (m_led_strip_meteor_decay[i] > 0)
            {
                // Pixel already lit, now decay
                uint8_t random = 0;
                sd_rand_application_vector_get((uint8_t *)&random, sizeof(random));
                if (random % 100 < 20)
                {
                    float decay = (1 - (random % 20) / 100.f);
                    r_level = MAX(1, floor(m_led_strip_meteor_decay[i] * current_color.r * decay));
                    g_level = MAX(1, floor(m_led_strip_meteor_decay[i] * current_color.g * decay));
                    b_level = MAX(1, floor(m_led_strip_meteor_decay[i] * current_color.b * decay));
                    if (i == 10)
                    {
                        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " ---> Pixel %d decay %d%d%d\n", i, r_level, g_level, b_level);
                    }
                    m_led_strip_meteor_decay[i] *= decay;
                    m_led_strip_heat[i] = m_led_strip_meteor_decay[i] * 255;
                    set_led_data(i, r_level, g_level, b_level);
                }
            }
        }
    }
    else if (current_pattern == BUTTON_PATTERN_FIRE)
    {
        int cooldown;
        uint8_t random = 0;

        // Cool down
        for (int i = 0; i < NLEDS; i += 1)
        {
            sd_rand_application_vector_get((uint8_t *)&random, sizeof(random));
            if (random % 100 < 50)
            {
                cooldown = random % 15;
                if (cooldown > m_led_strip_heat[i])
                {
                    m_led_strip_heat[i] = 0;
                }
                else
                {
                    m_led_strip_heat[i] = m_led_strip_heat[i] - cooldown;
                }
            }
        }

        // Advance and diffuse heat from below
        for (int k = NLEDS - 1; k >= 2; k--)
        {
            m_led_strip_heat[k] = (m_led_strip_heat[k - 1] + 2 * m_led_strip_heat[k - 2]) / 3;
        }

        // Ignite new sparks near bottom
        sd_rand_application_vector_get((uint8_t *)&random, sizeof(random));
        if ((random % 255) < (amplitude_progress * 255))
        {
            m_led_strip_heat[random % 7] = (random % 100) + 155; // 150-255
        }

        // Translate heat to colors
        for (int i = 0; i < NLEDS; i += 1)
        {
            r_level = m_led_strip_heat[i] / 255.f * current_color.r;
            g_level = m_led_strip_heat[i] / 255.f * current_color.g;
            b_level = m_led_strip_heat[i] / 255.f * current_color.b;
            set_led_data(i, r_level, g_level, b_level);
        }
    }

    if (neopixel_running == false)
    {

        nrf_drv_i2s_config_t config = NRF_DRV_I2S_DEFAULT_CONFIG;
        config.sdin_pin = NULL;
        config.sdout_pin = 4;
        config.mck_setup = I2S_CONFIG_MCKFREQ_MCKFREQ_32MDIV10; ///< 32 MHz / 10 = 3.2 MHz.
        config.ratio = NRF_I2S_RATIO_32X;                       //NRF_I2S_RATIO_32X;    ///< LRCK = MCK / 32.
        config.channels = I2S_CONFIG_CHANNELS_CHANNELS_Stereo;

        err_code = nrf_drv_i2s_init(&config, data_handler);

        APP_ERROR_CHECK(err_code);
        init = true;

        nrf_drv_i2s_buffers_t const initial_buffers = {
            .p_tx_buffer = &m_buffer_tx[0],
        };
        err_code = nrf_drv_i2s_start(&initial_buffers, I2S_BUFFER_SIZE, 0);

        // for (int i = 0; i < NLEDS; i += 1)
        // {
        //     set_led_data(i, r_level, g_level, b_level);
        // }
        neopixel_running = true;
    }

    last_seen_phase_progress = phase_progress;
}

/**
 * @}
 */
