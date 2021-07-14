 #ifndef SWARM_STATE_H
 #define SWARM_STATE_H


typedef struct {
    unsigned long start_time;
    unsigned long duration;
} swarm_transition_t;

static unsigned long int m_timealive = 0;
// swarm_transition_t swarm_transition;

void swarm_state_init();
void set_updated_timealive(unsigned long rxTimealive);
void jump_timealive(int amount);
unsigned long timealive_duration();

#endif
