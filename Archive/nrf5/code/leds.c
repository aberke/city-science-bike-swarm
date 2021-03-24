#include "leds.h"

void swarm_leds_init(void) {
    ret_code_t err_code;

    const led_sb_init_params_t led_sb_init_param = LED_SB_INIT_DEFAULT_PARAMS(BSP_LED_1_MASK | PIN_MASK(NRF_GPIO_PIN_MAP(0, 22)) | PIN_MASK(NRF_GPIO_PIN_MAP(0, 19)) | PIN_MASK(NRF_GPIO_PIN_MAP(0, 25)));

    err_code = led_softblink_init(&led_sb_init_param);
    APP_ERROR_CHECK(err_code);

    err_code = led_softblink_start(BSP_LED_1_MASK | PIN_MASK(NRF_GPIO_PIN_MAP(0, 22)) | PIN_MASK(NRF_GPIO_PIN_MAP(0, 19)) | PIN_MASK(NRF_GPIO_PIN_MAP(0, 25)));
    APP_ERROR_CHECK(err_code);
}

void swarm_leds_loop(void) {

}

void swarm_leds_restart(void) {
    ret_code_t err_code;
    
    err_code = led_softblink_uninit();
    APP_ERROR_CHECK(err_code);

    swarm_leds_init();
}

