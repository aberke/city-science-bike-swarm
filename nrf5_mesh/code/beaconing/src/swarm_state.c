#include <stdio.h>
#include "swarm_state.h"
#include "app_timer.h"

APP_TIMER_DEF(timer_timealive);

static void timealive_handler(void *p_context)
{
    m_timealive = m_timealive + 1;
}

void swarm_state_init() {
    ret_code_t err_code = app_timer_create(&timer_timealive, APP_TIMER_MODE_REPEATED, timealive_handler);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(timer_timealive, APP_TIMER_TICKS(1000), NULL);
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

void touch_lastsync()
{
    m_lastsync = m_timealive;
}

unsigned long time_since_lastsync()
{
    return m_timealive - m_lastsync;
}
