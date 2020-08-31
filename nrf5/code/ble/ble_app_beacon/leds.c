#include "leds.h"

void swarm_leds_init(void) {
    ret_code_t err_code;

    const led_sb_init_params_t led_sb_init_param = LED_SB_INIT_DEFAULT_PARAMS(LEDS_MASK | PIN_MASK(ARDUINO_5_PIN) | PIN_MASK(ARDUINO_4_PIN) | PIN_MASK(ARDUINO_3_PIN));

    err_code = led_softblink_init(&led_sb_init_param);
    APP_ERROR_CHECK(err_code);

    err_code = led_softblink_start(LEDS_MASK | PIN_MASK(ARDUINO_5_PIN) | PIN_MASK(ARDUINO_4_PIN) | PIN_MASK(ARDUINO_3_PIN));
    APP_ERROR_CHECK(err_code);
}

void swarm_leds_loop(void) {

}
