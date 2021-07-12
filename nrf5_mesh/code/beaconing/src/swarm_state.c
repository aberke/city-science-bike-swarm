#include <stdio.h>
#include "swarm_state.h"
#include "app_timer.h"

APP_TIMER_DEF(mytimealive);

static void timealive_handler(void *p_context)
{
    m_timealive = m_timealive + 1;
}

void swarm_state_init() {
    ret_code_t err_code = app_timer_create(&mytimealive, APP_TIMER_MODE_REPEATED, timealive_handler);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(mytimealive, APP_TIMER_TICKS(1000), NULL);
    APP_ERROR_CHECK(err_code);
}

void jump_timealive(int amount) {
    m_timealive = m_timealive + amount;
}

void set_updated_timealive(unsigned long rxTimealive) {
    m_timealive = rxTimealive;
}

unsigned long timealive_duration() {
    return m_timealive;
}
