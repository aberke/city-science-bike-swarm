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
 * @defgroup ble_sdk_app_beacon_main main.c
 * @{
 * @ingroup ble_sdk_app_beacon
 * @brief Beacon Transmitter Sample Application main file.
 *
 * This file contains the source code for an Beacon transmitter sample application.
 */

#include <stdbool.h>
#include <stdint.h>
#include "ble_advdata.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "bsp.h"
#include "app_timer.h"
#if defined(DYNAMIC_BEACON)
#include "ble_radio_notification.h"
#endif // DYNAMIC_BEACON

#define CENTRAL_LINK_COUNT              0                                 /**<number of central links used by the application. When changing this number remember to adjust the RAM settings*/

#if defined(DYNAMIC_BEACON)
#define PERIPHERAL_LINK_COUNT           1                                 /**<number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/
#else // DYNAMIC_BEACON
#define PERIPHERAL_LINK_COUNT           0                                 /**<number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/
#endif //DYNAMIC_BEACON

#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                 /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define APP_CFG_NON_CONN_ADV_TIMEOUT    0                                 /**< Time for which the device must be advertising in non-connectable mode (in seconds). 0 disables timeout. */
#define NON_CONNECTABLE_ADV_INTERVAL    MSEC_TO_UNITS(100, UNIT_0_625_MS) /**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100ms to 10.24s). */

#define APP_BEACON_INFO_LENGTH          0x17                              /**< Total length of information advertised by the Beacon. */
#define APP_ADV_DATA_LENGTH             0x15                              /**< Length of manufacturer specific data in the advertisement. */
#define APP_DEVICE_TYPE                 0x02                              /**< 0x02 refers to Beacon. */
#define APP_MEASURED_RSSI               0xC3                              /**< The Beacon's measured RSSI at 1 meter distance in dBm. */
#define APP_COMPANY_IDENTIFIER          0x0059                            /**< Company identifier for Nordic Semiconductor ASA. as per www.bluetooth.org. */
#define APP_MAJOR_VALUE                 0x01, 0x02                        /**< Major value used to identify Beacons. */ 
#define APP_MINOR_VALUE                 0x03, 0x04                        /**< Minor value used to identify Beacons. */ 
#define APP_BEACON_UUID                 0x01, 0x12, 0x23, 0x34, \
                                        0x45, 0x56, 0x67, 0x78, \
                                        0x89, 0x9a, 0xab, 0xbc, \
                                        0xcd, 0xde, 0xef, 0xf0            /**< Proprietary UUID for Beacon. */

#define DEAD_BEEF                       0xDEADBEEF                        /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define APP_TIMER_PRESCALER             0                                 /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                 /**< Size of timer operation queues. */

#if defined(USE_UICR_FOR_MAJ_MIN_VALUES)
#define UICR_ADDRESS                    0x10001080                        /**< Address of the UICR register used by this example. The major and minor versions to be encoded into the advertising data will be picked up from this location. */
#endif

#if (defined(USE_UICR_FOR_MAJ_MIN_VALUES) || defined(DYNAMIC_BEACON))
#define MAJ_VAL_OFFSET_IN_BEACON_INFO   18                                /**< Position of the MSB of the Major Value in m_beacon_info array. */
#endif // (USE_UICR_FOR_MAJ_MIN_VALUES || DYNAMIC_BEACON)

#if defined(DYNAMIC_BEACON)
#define CONNECTABLE_ADV_INTERVAL        MSEC_TO_UNITS(20, UNIT_0_625_MS)  /**< The advertising interval for connectable advertisement (20 ms). This value can vary between 20ms to 10.24s). */

static uint32_t m_dynamic_adv_counter = 0;                                /**< Parameter used ac dynamic counter inside iBeacon format advertising data. */
#endif // DYNAMIC_BEACON

static ble_gap_adv_params_t m_adv_params;                                 /**< Parameters to be passed to the stack when starting advertising. */
static uint8_t m_beacon_info[APP_BEACON_INFO_LENGTH] =                    /**< Information advertised by the Beacon. */
{
    APP_DEVICE_TYPE,     // Manufacturer specific information. Specifies the device type in this 
                         // implementation. 
    APP_ADV_DATA_LENGTH, // Manufacturer specific information. Specifies the length of the 
                         // manufacturer specific data in this implementation.
    APP_BEACON_UUID,     // 128 bit UUID value. 
    APP_MAJOR_VALUE,     // Major arbitrary value that can be used to distinguish between Beacons. 
    APP_MINOR_VALUE,     // Minor arbitrary value that can be used to distinguish between Beacons. 
    APP_MEASURED_RSSI    // Manufacturer specific information. The Beacon's measured TX power in 
                         // this implementation. 
};

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
#if defined(DYNAMIC_BEACON)
    ble_advdata_t scanrspdata;
    int8_t        tx_power = RADIO_TXPOWER_TXPOWER_0dBm;
    uint8_t       flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
#else // DYNAMIC_BEACON
    uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;
#endif // DYNAMIC_BEACON

    ble_advdata_manuf_data_t manuf_specific_data;

    manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;

#if defined(USE_UICR_FOR_MAJ_MIN_VALUES)
    // If USE_UICR_FOR_MAJ_MIN_VALUES is defined, the major and minor values will be read from the
    // UICR instead of using the default values. The major and minor values obtained from the UICR
    // are encoded into advertising data in big endian order (MSB First).
    // To set the UICR used by this example to a desired value, write to the address 0x10001080
    // using the nrfjprog tool. The command to be used is as follows.
    // nrfjprog --snr <Segger-chip-Serial-Number> --memwr 0x10001080 --val <your major/minor value>
    // For example, for a major value and minor value of 0xabcd and 0x0102 respectively, the
    // the following command should be used.
    // nrfjprog --snr <Segger-chip-Serial-Number> --memwr 0x10001080 --val 0xabcd0102
    uint16_t major_value = ((*(uint32_t *)UICR_ADDRESS) & 0xFFFF0000) >> 16;
    uint16_t minor_value = ((*(uint32_t *)UICR_ADDRESS) & 0x0000FFFF);

    uint8_t index = MAJ_VAL_OFFSET_IN_BEACON_INFO;

    m_beacon_info[index++] = MSB_16(major_value);
    m_beacon_info[index++] = LSB_16(major_value);

    m_beacon_info[index++] = MSB_16(minor_value);
    m_beacon_info[index++] = LSB_16(minor_value);
#endif

#if defined(DYNAMIC_BEACON)
    // Fill iBeacon Major and Minor Version worlds (16bit each) with value changed dynamically
    // for each advertising interval (3 packets each sent over different radio channel, all within 1ms).
    uint8_t info_index = MAJ_VAL_OFFSET_IN_BEACON_INFO;

    m_beacon_info[info_index++] = ((m_dynamic_adv_counter & 0xFF000000) >> 24);
    m_beacon_info[info_index++] = ((m_dynamic_adv_counter & 0x00FF0000) >> 16);
    m_beacon_info[info_index++] = ((m_dynamic_adv_counter & 0x0000FF00) >> 8);
    m_beacon_info[info_index++] = (m_dynamic_adv_counter & 0x000000FF);
#endif // DYNAMIC_BEACON

    manuf_specific_data.data.p_data = (uint8_t *) m_beacon_info;
    manuf_specific_data.data.size   = APP_BEACON_INFO_LENGTH;

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type             = BLE_ADVDATA_NO_NAME;
    advdata.flags                 = flags;
    advdata.p_manuf_specific_data = &manuf_specific_data;

#if defined(DYNAMIC_BEACON)
    // Build and set scan response data.
    memset(&scanrspdata, 0, sizeof(scanrspdata));
    scanrspdata.name_type         = BLE_ADVDATA_FULL_NAME;
    scanrspdata.p_tx_power_level  = &tx_power;
#endif // DYNAMIC_BEACON

#if defined(DYNAMIC_BEACON)
    err_code = ble_advdata_set(&advdata, &scanrspdata);
#else // DYNAMIC_BEACON
    err_code = ble_advdata_set(&advdata, NULL);
#endif // DYNAMIC_BEACON
    APP_ERROR_CHECK(err_code);

    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));

#if defined(DYNAMIC_BEACON)
    if (CONNECTABLE_ADV_INTERVAL < BLE_GAP_ADV_NONCON_INTERVAL_MIN) {
        m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    }
    else {
        m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_NONCONN_IND;
    }
#else // DYNAMIC_BEACON
    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_NONCONN_IND;
#endif // DYNAMIC_BEACON
    m_adv_params.p_peer_addr = NULL;                             // Undirected advertisement.
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
#if defined(DYNAMIC_BEACON)
    m_adv_params.interval    = CONNECTABLE_ADV_INTERVAL;
#else // DYNAMIC_BEACON
    m_adv_params.interval    = NON_CONNECTABLE_ADV_INTERVAL;
#endif // DYNAMIC_BEACON
    m_adv_params.timeout     = APP_CFG_NON_CONN_ADV_TIMEOUT;
}

#if defined(DYNAMIC_BEACON)
/**
 * @brief     Function managing GAP Peripheral parameters and their changes if
 *            dynamic advertisement is active.
 * @details   This function is called from the System event interrupt handler
 *            after a system event has been received.
 * @param[in] radio_active <code>true</code> if radio interface is currently
 *                         active, <code>false</code> otherwise.
 * @retval    None.
 */
static void on_ble_radio_active_evt(bool radio_active) {
    // Increment dynamic part of the counter => only once per
    // activate/deactivate sequence of radio event (when radio activity is
    // finished).
    if (radio_active) {
        // First update the advertisement and scan response data with
        // current values.
        advertising_init();
        // Increment counter for next window (no need to protect against
        // overflow => it will restart from zero again).
        m_dynamic_adv_counter++;
    }
}
#endif // DYNAMIC_BRACON

/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t err_code;

#if defined(DYNAMIC_BEACON)
    // Enable radio notification events to be able to change advertising data after each broadcast.
    err_code = ble_radio_notification_init(NRF_APP_PRIORITY_LOW,
                                           NRF_RADIO_NOTIFICATION_DISTANCE_1740US,
                                           on_ble_radio_active_evt);
    APP_ERROR_CHECK(err_code);
#endif // DYNAMIC_BEACON

    err_code = sd_ble_gap_adv_start(&m_adv_params);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
     uint32_t err_code;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);
    
    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);
    
    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for doing power management.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code;
    // Initialize.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
    err_code = bsp_init(BSP_INIT_LED, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), NULL);
    APP_ERROR_CHECK(err_code);
    ble_stack_init();
    advertising_init();

    // Start execution.
    advertising_start();

    // Enter main loop.
    for (;; )
    {
        power_manage();
    }
}

/**
 * @}
 */
