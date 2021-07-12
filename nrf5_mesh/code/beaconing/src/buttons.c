#include "buttons.h"


/**@brief Function for handling bsp events.
 */
void bsp_evt_handler(bsp_event_t evt)
{
    switch (evt)
    {
        case BSP_EVENT_KEY_0:
            bsp_board_led_invert(1);
            break;

        case BSP_EVENT_KEY_1:
            bsp_board_led_invert(1);
            break;

        case BSP_EVENT_KEY_2:
            bsp_board_led_invert(1);
            break;

        case BSP_EVENT_KEY_3:
            bsp_board_led_invert(1);
            break;

        default:
            return; // no implementation needed
    }
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "BSP: %u\n", evt);

    advance_color_pattern();
}

void advance_color_pattern() {
    uint8_t next_color_pattern = (selected_color_pattern + 1) % COLOR_PATTERNS_COUNT;

    selected_color_pattern = next_color_pattern;
    m_current_color = color_patterns[selected_color_pattern];
}

btn_color_t btn_current_color() {
    return m_current_color;
}
/**@brief Function for initializing bsp module.
 */
void bsp_configuration()
{
    uint32_t err_code;

    err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_evt_handler);
    APP_ERROR_CHECK(err_code);

    m_current_color = color_patterns[selected_color_pattern];
}

void button_handler(uint8_t pin_no, uint8_t button_action)
{
    NRF_LOG_INFO("Button press detected \r\n");
    if(pin_no == BUTTON_1 && button_action == APP_BUTTON_PUSH)
    {
        NRF_LOG_INFO("Button 1 pressed \r\n");
    }
    if(pin_no == BUTTON_4 && button_action == APP_BUTTON_PUSH)
    {
        NRF_LOG_INFO("Button 4 pressed \r\n");
    }

}

