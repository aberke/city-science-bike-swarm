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
/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2011 Stephen Caudle <scaudle@doceme.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include "log.h"
#include <string.h>
#include "neopixel_SPI.h"
#include "buttons.h"

#define N_LEDS 1

void update_string(btn_color_t *data, uint16_t len);

float multiply;

#define SPI_INSTANCE 0                                               /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE); /**< SPI instance. */
static volatile bool spi_xfer_done;                                  /**< Flag used to indicate that SPI instance completed the transfer. */
#define TEST_STRING "Nordic"
static uint8_t m_tx_buf[] = TEST_STRING;          /**< TX buffer. */
static uint8_t m_rx_buf[sizeof(TEST_STRING) + 1]; /**< RX buffer. */
static const uint8_t m_length = sizeof(m_tx_buf); /**< Transfer length. */

bool spi_initialized = false;

/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrf_drv_spi_evt_t const *p_event,
                       void *p_context)
{
    spi_xfer_done = true;
    // __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Transfer completed.\n");
    if (m_rx_buf[0] != 0)
    {
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " Received:");
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, m_rx_buf);
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "\n");
    }
}

void setup_spi()
{

    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_LSB_FIRST;
    spi_config.frequency = 0x54000000UL;
    spi_config.ss_pin = SPI_SS_PIN;
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = 6;
    spi_config.sck_pin = SPI_SCK_PIN;
    spi_config.mode = NRF_DRV_SPI_MODE_1;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL));
}

void neopixel_SPI(int phase)
{
    uint32_t i, j;

    if (!spi_initialized)
    {
        setup_spi();
        spi_initialized = true;
    }

    btn_color_t led_data[N_LEDS];
    btn_color_t scratch[N_LEDS];
    btn_color_t current_color = btn_current_color();

    //union leds

    int x = 0;

    multiply = (float)phase / (float)255;

    // multiply = 1;

    for (x = 0; x < N_LEDS; x++)
    {
        led_data[x].r = current_color.r * multiply; //red max 128?
        led_data[x].g = current_color.g * multiply; //green
        led_data[x].b = current_color.b * multiply; //blue?
    }

    j = 0;

    //gpio_toggle(GPIOD, GPIO12);	/* LED on/off */

    // Send the new data to the LED string
    update_string(led_data, N_LEDS);

    //nrf_delay_us(50);

    return;
}

/* turn bits into pulses of the correct ratios for the WS2811 by making *
 * bytes with the correct number of ones & zeros in the right order.    */
void update_string(btn_color_t *data, uint16_t len)
{
    uint8_t *bytearray = (uint8_t *)data;

    uint16_t i = 0;
    int16_t j = 0;
    uint8_t tmp = 0;
    uint8_t sendbuf[17];

    len = len * 3;
    spi_xfer_done = false;

    //  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, bytearray, 3, NULL, 0));
    //  while (!spi_xfer_done)
    //{
    //    __WFE();
    //}
    //APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, &green, 1, NULL, 0));
    //APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, &blue, 1, NULL, 0));

    __NOP();
    for (i = 0; i < len; i++)
    {
        tmp = bytearray[i];
        int i = 0;
        for (j = 7; j >= 0; j--)
        {
            spi_xfer_done = false;

            if (tmp & (0x01 << j))
            {
                // generate the sequence to represent a 'one' to the WS2811.
                //	spi_send(SPI1, 0xFFF0);
                uint8_t one = 0x3F;

                sendbuf[i] = one;
                //     sendbuf[i+1] = 0x00;
                i++;
                //  i++;

                //0b1111 1111 1111 0000
            }
            else
            {
                // generate the sequence to represent a 'zero'.
                //	spi_send(SPI1, 0xE000);
                uint8_t zero = 0x03;
                sendbuf[i] = zero;
                //sendbuf[i+1] = 0x00;
                //i=i+2;
                i++;
                __NOP();
                //0b1110 0000 0000 0000
            }
        }

        //      sendbuf[9] = 0x00;

        APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, sendbuf, 8, NULL, 0));

        while (!spi_xfer_done)
        {
            __WFE();
        }

        __NOP();
    }
}
