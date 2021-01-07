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
#include "nordic_common.h"
#include "nrf.h"
#include "app_util_platform.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"

#include "boards.h"

#include "nrf_drv_i2s.h"
#include "nrfx_i2s.h"
#include "neopixel.h"

volatile uint8_t g_demo_mode = 0;
volatile bool g_i2s_start = true;
volatile bool g_i2s_running = false;

#define NLEDS 30
#define RESET_BITS 6
#define I2S_BUFFER_SIZE 3*NLEDS + RESET_BITS

static uint32_t m_buffer_tx[I2S_BUFFER_SIZE];
static volatile int nled = 1;

// This is the I2S data handler - all data exchange related to the I2S transfers
// is done here.

static void data_handler( nrfx_i2s_buffers_t const * p_released,uint32_t status)
{
    // Non-NULL value in 'p_data_to_send' indicates that the driver needs
    // a new portion of data to send.
    if (p_released->p_tx_buffer != NULL)
    {
        // do nothing - buffer is updated elsewhere
    }
}

/*

    caclChannelValue()

    Sets up a 32 bit value for a channel (R/G/B). 

    A channel has 8 x 4-bit codes. Code 0xe is HIGH and 0x8 is LOW.

    So a level of 128 would be represented as:

    0xe8888888

    The 16 bit values need to be swapped because of the way I2S sends data - right/left channels.

    So for the above example, final value sent would be:

    0x8888e888

*/
uint32_t caclChannelValue(uint8_t level)
{
    uint32_t val = 0;

    // 0 
    if(level == 0) {
        val = 0x88888888;
    }
    // 255
    else if (level == 255) {
        val = 0xeeeeeeee;
    }
    else {
        // apply 4-bit 0xe HIGH pattern wherever level bits are 1.
        val = 0x88888888;
        for (uint8_t i = 0; i < 8; i++) {
            if((1 << i) & level) {
                uint32_t mask = ~(0x0f << 4*i);
                uint32_t patt = (0x0e << 4*i);
                val = (val & mask) | patt;
            }
        }

        // swap 16 bits
        val = (val >> 16) | (val << 16);
    }

    return val;
}

// set LED data
void set_led_data(uint8_t r_level, uint8_t g_level, uint8_t b_level)
{
    for(int i = 0; i < 3*NLEDS; i += 3) {
            m_buffer_tx[i] = caclChannelValue(g_level);
            m_buffer_tx[i+1] = caclChannelValue(r_level);
            m_buffer_tx[i+2] = caclChannelValue(b_level);
       
    }

    // reset 
    for(int i = 3*NLEDS; i < I2S_BUFFER_SIZE; i++) {
        m_buffer_tx[i] = 0;
    }
}

/**@brief Application main function.
 */
void neopixel(int phase)
{
    uint32_t err_code;

      float multiplier = (float)phase/(float)255;
     
      uint8_t r_level = 255*multiplier;
      uint8_t g_level = 120*multiplier;
      uint8_t b_level = 35*multiplier;

    set_led_data(r_level,g_level,b_level);

    nrf_drv_i2s_config_t config = NRF_DRV_I2S_DEFAULT_CONFIG;
    config.sdin_pin  = NULL;
    config.sdout_pin = 5;
    config.mck_setup = NRF_I2S_MCK_32MDIV10; ///< 32 MHz / 10 = 3.2 MHz.
    config.ratio     = NRF_I2S_RATIO_32X;    ///< LRCK = MCK / 32.
    config.channels  = NRF_I2S_CHANNELS_STEREO;
    
   for (int i=0;i<3;i++)
    {
        err_code = nrf_drv_i2s_init(&config, data_handler);
        APP_ERROR_CHECK(err_code);
        // start I2S
        if(g_i2s_start && !g_i2s_running) 
        {
            nrf_drv_i2s_buffers_t const initial_buffers = {
            .p_tx_buffer = &m_buffer_tx[0],
            };
            err_code = nrf_drv_i2s_start(&initial_buffers, I2S_BUFFER_SIZE, 0);
            APP_ERROR_CHECK(err_code);
            nrf_delay_ms(5);
            nrf_drv_i2s_stop();
  
        }
         nrf_delay_ms(5);
        nrf_drv_i2s_uninit();
        set_led_data(r_level,g_level,b_level);
  
    }

}


/**
 * @}
 */