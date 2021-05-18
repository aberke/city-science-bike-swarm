/* Copyright (c) 2010 - 2020, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "ad_type_filter.h"
#include "advertiser.h"
#include "app_timer.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_conn_state.h"
#include "ble_hci.h"
#include "ble_softdevice_support.h"
#include "ble_srv_common.h"
#include "ble.h"
#include "boards.h"
#include "example_common.h"
#include "fds.h"
#include "leds.h"
#include "log.h"
#include "mesh_app_utils.h"
#include "mesh_provisionee.h"
#include "mesh_stack.h"
#include "neopixel.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_mesh_config_examples.h"
#include "nrf_mesh_configure.h"
#include "nrf_mesh.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh.h"
#include "peer_manager.h"
#include "sensorsim.h"
#include "simple_hal.h"

#if defined(NRF51) && defined(NRF_MESH_STACK_DEPTH)
#include "stack_depth.h"
#endif

/*****************************************************************************
 * Definitions
 *****************************************************************************/
#define ADVERTISER_BUFFER_SIZE (64)

#define DEVICE_NAME "Swarm"                                  /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME "Starlings Technology, Inc."       /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL 300                                 /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */
#define APP_ADV_DURATION 18000                               /**< The advertising duration (180 seconds) in units of 10 milliseconds. */
#define APP_BLE_OBSERVER_PRIO 3                              /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG 1                               /**< A tag identifying the SoftDevice BLE configuration. */
#define MIN_CONN_INTERVAL MSEC_TO_UNITS(100, UNIT_1_25_MS)   /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL MSEC_TO_UNITS(200, UNIT_1_25_MS)   /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY 4                                      /**< Slave latency. */
#define CONN_SUP_TIMEOUT MSEC_TO_UNITS(4000, UNIT_10_MS)     /**< Connection supervisory timeout (4 seconds). */
#define FIRST_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(5000) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(30000) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT 3                       /**< Number of attempts before giving up the connection parameter negotiation. */
#define SEC_PARAM_BOND 1                                     /**< Perform bonding. */
#define SEC_PARAM_MITM 0                                     /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC 0                                     /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS 0                                 /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES BLE_GAP_IO_CAPS_NONE       /**< No I/O capabilities. */
#define SEC_PARAM_OOB 0                                      /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE 7                             /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE 16                            /**< Maximum encryption key size. */
#define DEAD_BEEF 0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
NRF_BLE_GATT_DEF(m_gatt);                                    /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                      /**< Context for the Queued Write module.*/
BLE_LIGHT_DEF(m_light_service);                              /**< Light service instance. */
BLE_ADVERTISING_DEF(m_advertising);                          /**< Advertising module instance. */
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;     /**< Handle of the current connection. */
static ble_uuid_t m_adv_uuids[] =                            /**< Universally unique service identifiers. */
    {
        {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}};

/*****************************************************************************
 * Forward declaration of static functions
 *****************************************************************************/

static void advertising_start(bool erase_bonds);

/*****************************************************************************
 * Static variables
 *****************************************************************************/
/** Single advertiser instance. May periodically transmit one packet at a time. */
static advertiser_t m_advertiser;

static uint8_t m_adv_buffer[ADVERTISER_BUFFER_SIZE];
static bool m_device_provisioned;
unsigned long int timealive = 0;

advertiser_tx_complete_cb_t tx_complete_cb;

APP_TIMER_DEF(mytimealive);

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const *p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_id)
    {
    case PM_EVT_BONDED_PEER_CONNECTED:
    {
        NRF_LOG_INFO("Connected to a previously bonded device.");
    }
    break;

    case PM_EVT_CONN_SEC_SUCCEEDED:
    {
        NRF_LOG_INFO("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d.",
                     ble_conn_state_role(p_evt->conn_handle),
                     p_evt->conn_handle,
                     p_evt->params.conn_sec_succeeded.procedure);
    }
    break;

    case PM_EVT_CONN_SEC_FAILED:
    {
        /* Often, when securing fails, it shouldn't be restarted, for security reasons.
             * Other times, it can be restarted directly.
             * Sometimes it can be restarted, but only after changing some Security Parameters.
             * Sometimes, it cannot be restarted until the link is disconnected and reconnected.
             * Sometimes it is impossible, to secure the link, or the peer device does not support it.
             * How to handle this error is highly application dependent. */
    }
    break;

    case PM_EVT_CONN_SEC_CONFIG_REQ:
    {
        // Reject pairing request from an already bonded peer.
        pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
        pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
    }
    break;

    case PM_EVT_STORAGE_FULL:
    {
        // Run garbage collection on the flash.
        err_code = fds_gc();
        if (err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
        {
            // Retry.
        }
        else
        {
            APP_ERROR_CHECK(err_code);
        }
    }
    break;

    case PM_EVT_PEERS_DELETE_SUCCEEDED:
    {
        advertising_start(false);
    }
    break;

    case PM_EVT_PEER_DATA_UPDATE_FAILED:
    {
        // Assert.
        APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
    }
    break;

    case PM_EVT_PEER_DELETE_FAILED:
    {
        // Assert.
        APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
    }
    break;

    case PM_EVT_PEERS_DELETE_FAILED:
    {
        // Assert.
        APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
    }
    break;

    case PM_EVT_ERROR_UNEXPECTED:
    {
        // Assert.
        APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
    }
    break;

    case PM_EVT_CONN_SEC_START:
    case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
    case PM_EVT_PEER_DELETE_SUCCEEDED:
    case PM_EVT_LOCAL_DB_CACHE_APPLIED:
    case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
        // This can happen when the local DB has changed.
    case PM_EVT_SERVICE_CHANGED_IND_SENT:
    case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
    default:
        break;
    }
}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    ret_code_t err_code;
    ble_gap_conn_params_t gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    /* YOUR_JOB: Use an appearance value matching the application's use case.
       err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_);
       APP_ERROR_CHECK(err_code); */

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

static void light_evt_handler(ble_light_service_t *p_light_service, ble_light_evt_t *p_evt)
{
    // Action to perform when the Data I/O characteristic notifications are enabled
    // Add your implementation here
    if (p_evt->evt_type == BLE_DATA_IO_EVT_NOTIFICATION_ENABLED)
    {
        // Possibly save to a global variable to know that notifications are ENABLED
        NRF_LOG_INFO("Notifications ENABLED on Data I/O Characteristic");
    }
    else if (p_evt->evt_type == BLE_DATA_IO_EVT_NOTIFICATION_DISABLED)
    {
        // Possibly save to a global variable to know that notifications are DISABLED
        NRF_LOG_INFO("Notifications DISABLED on Data I/O Characteristic");
    }

    // Handle any other events necessary...
}

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    ret_code_t err_code;
    ble_light_service_init_t light_init;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize the light service
    memset(&light_init, 0, sizeof(light_init));
    light_init.evt_handler = light_evt_handler;

    err_code = ble_light_service_init(&m_light_service, &light_init);
    NRF_LOG_INFO("Done with services_init()\r\n");
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t *p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    ret_code_t err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail = false;
    cp_init.evt_handler = on_conn_params_evt;
    cp_init.error_handler = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;

    switch (ble_adv_evt)
    {
    case BLE_ADV_EVT_FAST:
        NRF_LOG_INFO("Fast advertising.");
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_ADV_EVT_IDLE:
        sleep_mode_enter();
        break;

    default:
        break;
    }
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context)
{
    ret_code_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_DISCONNECTED:
        NRF_LOG_INFO("Disconnected.");
        // LED indication will be changed when advertising starts.
        break;

    case BLE_GAP_EVT_CONNECTED:
        NRF_LOG_INFO("Connected.");
        err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
        APP_ERROR_CHECK(err_code);
        m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
    {
        NRF_LOG_DEBUG("PHY update request.");
        ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
        err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
        APP_ERROR_CHECK(err_code);
    }
    break;

    case BLE_GATTC_EVT_TIMEOUT:
        // Disconnect on GATT Client timeout event.
        NRF_LOG_DEBUG("GATT Client Timeout.");
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTS_EVT_TIMEOUT:
        // Disconnect on GATT Server timeout event.
        NRF_LOG_DEBUG("GATT Server Timeout.");
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;

    default:
        // No implementation needed.
        break;
    }
}

/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond = SEC_PARAM_BOND;
    sec_param.mitm = SEC_PARAM_MITM;
    sec_param.lesc = SEC_PARAM_LESC;
    sec_param.keypress = SEC_PARAM_KEYPRESS;
    sec_param.io_caps = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob = SEC_PARAM_OOB;
    sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc = 1;
    sec_param.kdist_own.id = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    ret_code_t err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = true;
    init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_complete.p_uuids = m_adv_uuids;

    init.config.ble_adv_fast_enabled = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout = APP_ADV_DURATION;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**@brief Function for starting advertising.
 */
static void advertising_start(bool erase_bonds)
{
    if (erase_bonds == true)
    {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETED_SUCEEDED event
    }
    else
    {
        ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);

        APP_ERROR_CHECK(err_code);
    }
}

static void timealive_handler(void *p_context)
{
    timealive = timealive + 1;
}

static void rx_cb(const nrf_mesh_adv_packet_rx_data_t *p_rx_data)
{
    LEDS_OFF(BSP_LED_0_MASK); /* @c LED_RGB_RED_MASK on pca10031 */
    char msg[128];

    // sprintf(msg, "%02x", p_rx_data->p_payload);
    uint8_t *word = "SWARM";
    // printf("%s", p_rx_data->p_payload);
    if (strstr(p_rx_data->p_payload, word) != NULL)
    {
        //    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " ---> Target Pack RCV\n");

        // (void) sprintf(msg, "RX [@%u]: RSSI: %3d ADV TYPE: %x ADDR: [%02x:%02x:%02x:%02x:%02x:%02x]",
        //               p_rx_data->p_metadata->params.scanner.timestamp,
        //               p_rx_data->p_metadata->params.scanner.rssi,
        //               p_rx_data->adv_type,
        //               p_rx_data->p_metadata->params.scanner.adv_addr.addr[0],
        //               p_rx_data->p_metadata->params.scanner.adv_addr.addr[1],
        //               p_rx_data->p_metadata->params.scanner.adv_addr.addr[2],
        //               p_rx_data->p_metadata->params.scanner.adv_addr.addr[3],
        //               p_rx_data->p_metadata->params.scanner.adv_addr.addr[4],
        //               p_rx_data->p_metadata->params.scanner.adv_addr.addr[5]);
        //  __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, msg, p_rx_data->p_payload, p_rx_data->length);

        unsigned long int rxTimeAlive = 0;

        rxTimeAlive = ((p_rx_data->p_payload[7 + 0] << 24) + (p_rx_data->p_payload[7 + 1] << 16) + (p_rx_data->p_payload[7 + 2] << 8) + (p_rx_data->p_payload[7 + 3]));
        //  printf("rxtimealive: %l ",rxTimeAlive);
        //   __NOP();
        if (rxTimeAlive > timealive)
        {
            sprintf(msg, " ---> RX %d > my timealive %d\n", rxTimeAlive, timealive);
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, msg);

            setPhase(0);
        }
    }

    //  LEDS_ON(BSP_LED_0_MASK);  /* @c LED_RGB_RED_MASK on pca10031 */
}

static void mesh_adv_init(void)
{
    advertiser_instance_init(&m_advertiser, NULL, m_adv_buffer, ADVERTISER_BUFFER_SIZE);
}

// static void mesh_adv_start(void)
// {
//     /* Let scanner accept Complete Local Name AD Type. */
//     bearer_adtype_add(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME);

//     advertiser_enable(&m_advertiser);
//     static const uint8_t adv_data[] =
//         {
//             0x06,                                /* AD data length (including type, but not itself) */
//             BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, /* AD data type (Complete local name) */
//             'S',                                 /* AD data payload (Name of device) */
//             'W',
//             'A',
//             'R',
//             'M',
//         };

//     /* Allocate packet */
//     adv_packet_t *p_packet = advertiser_packet_alloc(&m_advertiser, sizeof(adv_data));
//     if (p_packet)
//     {
//         /* Construct packet contents */
//         memcpy(p_packet->packet.payload, adv_data, sizeof(adv_data));
//         /* Repeat forever */
//         p_packet->config.repeats = 0x01;

//         advertiser_packet_send(&m_advertiser, p_packet);
//     }

//     //   advertiser_disable(&m_advertiser);
// }

static void mesh_adv_start(void)
{

    //unsigned long timealive = millis();

    /* Let scanner accept Complete Local Name AD Type. */
    bearer_adtype_add(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME);
    advertiser_enable(&m_advertiser);
    advertiser_flush(&m_advertiser);
    uint8_t adv_data[] =
        {
            0x06 + 0x04,                         /* AD data length (including type, but not itself) 6 bytes plus sie of the long */
            BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, /* AD data type (Complete local name) */
            'S',                                 /* AD data payload (Name of device) */
            'W',
            'A',
            'R',
            'M',
            (int)((timealive >> 8) & 0xFF),
            (int)((timealive & 0xFF)),
        };

    /* Allocate packet */
    adv_packet_t *p_packet = advertiser_packet_alloc(&m_advertiser, sizeof(adv_data));
    if (p_packet)
    {
        /* Construct packet contents */
        memcpy(p_packet->packet.payload, adv_data, sizeof(adv_data));
        /* Repeat forever */
        p_packet->config.repeats = 0x01;

        advertiser_packet_send(&m_advertiser, p_packet);
        //    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " ---> PHASE AT ZERO packet sent  \n" );
    }
    //   __NOP();
    //    advertiser_disable(&m_advertiser);
}

static void node_reset(void)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " ---> ----- Node reset -----\n");
    hal_led_blink_ms(LEDS_MASK, LED_BLINK_INTERVAL_MS, LED_BLINK_CNT_RESET);
    /* This function may return if there are ongoing flash operations. */
    mesh_stack_device_reset();
}

static void config_server_evt_cb(const config_server_evt_t *p_evt)
{
    if (p_evt->type == CONFIG_SERVER_EVT_NODE_RESET)
    {
        node_reset();
    }
}

static void device_identification_start_cb(uint8_t attention_duration_s)
{
    hal_led_mask_set(LEDS_MASK, false);
    hal_led_blink_ms(BSP_LED_2_MASK | BSP_LED_3_MASK,
                     LED_BLINK_ATTENTION_INTERVAL_MS,
                     LED_BLINK_ATTENTION_COUNT(attention_duration_s));
}

static void provisioning_aborted_cb(void)
{
    hal_led_blink_stop();
}

static void unicast_address_print(void)
{
    dsm_local_unicast_address_t node_address;
    dsm_local_unicast_addresses_get(&node_address);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " ---> Node Address: 0x%04x \n", node_address.address_start);
}

static void provisioning_complete_cb(void)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " ---> Successfully provisioned\n");

    unicast_address_print();
    hal_led_blink_stop();
    hal_led_mask_set(LEDS_MASK, LED_MASK_STATE_OFF);
    hal_led_blink_ms(LEDS_MASK, LED_BLINK_INTERVAL_MS, LED_BLINK_CNT_PROV);
}

static void mesh_init(void)
{
    mesh_stack_init_params_t init_params =
        {
            .core.irq_priority = NRF_MESH_IRQ_PRIORITY_THREAD,
            .core.lfclksrc = DEV_BOARD_LF_CLK_CFG,
            .models.config_server_cb = config_server_evt_cb};

    uint32_t status = mesh_stack_init(&init_params, &m_device_provisioned);
    switch (status)
    {
    case NRF_ERROR_INVALID_DATA:
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " ***> Data in the persistent memory was corrupted. Device starts as unprovisioned.\n");
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " ***> Reset device before starting of the provisioning process.\n");
        break;
    case NRF_SUCCESS:
        break;
    default:
        ERROR_CHECK(status);
    }

    /* Start listening for incoming packets */
    nrf_mesh_rx_cb_set(rx_cb);

    /* Initialize the advertiser */
    mesh_adv_init();
}

static void initialize(void)
{
#if defined(NRF51) && defined(NRF_MESH_STACK_DEPTH)
    stack_depth_paint_stack();
#endif

    ERROR_CHECK(app_timer_init());
    hal_leds_init();

    __LOG_INIT(LOG_SRC_APP, LOG_LEVEL_INFO, log_callback_rtt);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " ---> ----- Swarm Bluetooth Mesh Beacon Initializing... -----\n");

    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
    peer_manager_init();
    advertising_start(false);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);

    mesh_init();

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " ---> Initialization complete.\n");
}

static void start(void)
{
    if (!m_device_provisioned)
    {
        static const uint8_t static_auth_data[NRF_MESH_KEY_SIZE] = STATIC_AUTH_DATA;
        mesh_provisionee_start_params_t prov_start_params =
            {
                .p_static_data = static_auth_data,
                .prov_complete_cb = provisioning_complete_cb,
                .prov_device_identification_start_cb = device_identification_start_cb,
                .prov_device_identification_stop_cb = NULL,
                .prov_abort_cb = provisioning_aborted_cb,
                .p_device_uri = EX_URI_BEACON};
        ERROR_CHECK(mesh_provisionee_prov_start(&prov_start_params));
    }
    else
    {
        unicast_address_print();
    }

    /* Start advertising own beacon */
    /* Note: If application wants to start beacons at later time, adv_start() API must be called
     * from the same IRQ priority context same as that of the Mesh Stack. */
    mesh_adv_start();

    mesh_app_uuid_print(nrf_mesh_configure_device_uuid_get());

    ERROR_CHECK(mesh_stack_start());

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " ---> Swarm Bluetooth Mesh Beacon started.\n");

    //   hal_led_mask_set(LEDS_MASK, LED_MASK_STATE_OFF);
    //  hal_led_blink_ms(LEDS_MASK, LED_BLINK_INTERVAL_MS, LED_BLINK_CNT_START);
}

int main(void)
{
    initialize();
    start();

    // ERROR_CHECK(app_timer_init());

    // neopixel();

    pwm_init();
    timer_initalize();

    ret_code_t err_code = app_timer_create(&mytimealive, APP_TIMER_MODE_REPEATED, timealive_handler);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(mytimealive, APP_TIMER_TICKS(1000), NULL);
    APP_ERROR_CHECK(err_code);

    for (;;)
    {
        ledloop();
        int y = currentPhase();
        if (currentPhase() > 2080 && currentPhase() < 2100)
        {
            // __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " ---> PHASE AT ZERO  \n" );
            mesh_adv_start();
        }

        //(void)sd_app_evt_wait();
        bool done = nrf_mesh_process();
        if (done)
        {
            sd_app_evt_wait();
        }
    }
}
