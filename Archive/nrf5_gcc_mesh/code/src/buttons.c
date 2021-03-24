#include "buttons.h"

// PMW instance
APP_PWM_INSTANCE(PWM2, 2);  // Setup a PWM instance with TIMER 2

static volatile int ready_flag = true;

void button_handler(uint8_t pin_no, uint8_t button_action)
{
    NRF_LOG_INFO("Button press detected \r\n");
    if(pin_no == BUTTON_1 && button_action == APP_BUTTON_PUSH)
    {
        NRF_LOG_INFO("Button 1 pressed \r\n");
        nrf_drv_gpiote_out_task_trigger(LED_1);
        APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM2, 0, 10));
        ready_flag = false;
         nrf_delay_ms(1000);
        while(!ready_flag){APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM2, 0, 0));}
    }
    if(pin_no == BUTTON_2 && button_action == APP_BUTTON_PUSH)
    {
        NRF_LOG_INFO("Button 2 pressed \r\n");
        nrf_drv_gpiote_out_task_trigger(LED_2);
        APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM2, 0, 5));
        ready_flag = false;
        nrf_delay_ms(1000);
        while(!ready_flag){APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM2, 0, 0));}
    }

}


void buttons_init()
{
    ret_code_t err_code;
/*
    static app_button_cfg_t button_cfg = {
      .pin_no = BUTTON_1,
      .active_state = APP_BUTTON_ACTIVE_LOW,
      .pull_cfg = NRF_GPIO_PIN_PULLUP,
      .button_handler = button_handler 
    };
*/
    static app_button_cfg_t button_cfg[2] = {
        {BUTTON_1,APP_BUTTON_ACTIVE_LOW,NRF_GPIO_PIN_PULLUP,button_handler},
        {BUTTON_2,APP_BUTTON_ACTIVE_LOW,NRF_GPIO_PIN_PULLUP,button_handler}
    };

    err_code = app_button_init(button_cfg,2,5);
    APP_ERROR_CHECK(err_code);

    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}
