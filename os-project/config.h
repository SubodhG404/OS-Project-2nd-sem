#ifndef CONFIG_H
#define CONFIG_H

constexpr int BUFFER_SIZE = 5;

extern int PRODUCER_DELAY_MS;
extern int CONSUMER_DELAY_MS;
extern bool ENABLE_DEADLOCK_MODE;
extern bool ENABLE_RACE_CONDITION_MODE;

void configure_rates(int mode_choice);

#endif
