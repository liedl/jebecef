#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>


#define KNX_TIMEOUT_3S 100     //ticks up 3s reached
#define KNX_TIMEOUT_6S 200     //ticks up 6s reached


void init_knx_timer(void);
uint8_t get_knx_timer_ticks(void);
void set_knx_timer_ticks(uint8_t ticks);



#endif //TIMER_H
