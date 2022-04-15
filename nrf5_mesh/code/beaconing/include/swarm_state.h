 #ifndef SWARM_STATE_H
 #define SWARM_STATE_H

typedef struct
{
    unsigned long start_time;
    unsigned long duration;
} swarm_transition_t;
// swarm_transition_t swarm_transition;

typedef enum
{
    STATE_SYNC_INACTIVE = 0,
    STATE_SYNC_ACTIVATING = 1,
    STATE_SYNC_ACTIVE = 2,
    STATE_SYNC_DEACTIVATING = 3
} state_sync_t;
static state_sync_t m_sync_state;

static unsigned long int m_timealive = 0;
static unsigned long int m_lastsync = 0;

void swarm_state_init();
void set_updated_timealive(unsigned long rxTimealive);
void jump_timealive(int amount);
unsigned long timealive_duration();

void touch_lastsync();
unsigned long time_since_lastsync();

#endif
