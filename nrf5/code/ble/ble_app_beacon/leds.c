#include "leds.h"

void swarm_leds_init(void) {
    ret_code_t err_code;

    const led_sb_init_params_t led_sb_init_param = LED_SB_INIT_DEFAULT_PARAMS(LEDS_MASK | PIN_MASK(ARDUINO_5_PIN));

    err_code = led_softblink_init(&led_sb_init_param);
    APP_ERROR_CHECK(err_code);

    err_code = led_softblink_start(LEDS_MASK | PIN_MASK(ARDUINO_5_PIN));
    APP_ERROR_CHECK(err_code);

    nrf_gpio_cfg_input(BSP_BUTTON_0, BUTTON_PULL);
    nrf_gpiote_event_configure(GPIOTE_CHANNEL_0, BSP_BUTTON_0, NRF_GPIOTE_POLARITY_HITOLO); 
	NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Enabled; //Set GPIOTE interrupt register on channel 0
	NVIC_EnableIRQ(GPIOTE_IRQn); //Enable interrupts
    // nrf_gpio_cfg_output(ARDUINO_0_PIN);
    // nrf_gpio_cfg_output(bsp_board_led_idx_to_pin(3));
}

void swarm_leds_loop(void) {
    // nrf_gpio_pin_toggle(ARDUINO_0_PIN);
    // nrf_gpio_pin_toggle(bsp_board_led_idx_to_pin(3));
}
