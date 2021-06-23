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
    }
    if(pin_no == BUTTON_4 && button_action == APP_BUTTON_PUSH)
    {
        NRF_LOG_INFO("Button 4 pressed \r\n");
    }

}


void buttons_init()
{

}
