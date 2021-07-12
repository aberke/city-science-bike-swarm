 #ifndef SWARM_STATE_H
 #define SWARM_STATE_H


static unsigned long int m_timealive = 0;

void swarm_state_init();
void set_updated_timealive(unsigned long rxTimealive);
void jump_timealive(int amount);
unsigned long timealive_duration();

#endif
