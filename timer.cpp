#include "timer.h"
#include "knx.h"

volatile uint8_t knx_timer_ticks;

void init_knx_timer(void) {
  TCCR3A = 0;
  TCCR3B = ((1<< CS31) | (1<< WGM32)); //prescaler 8, clk/8 and clear timer on compare match A for timer 3
  OCR3A = 30000; //set output compare A match for timer 3 // interrupt every 30ms
  //TIMSK3 = (1<< OCIE3A); //enable output compare A match interrupt for timer 3
  TIMSK3 = ((1<< TOIE3) | (1<< OCIE3A));
}

uint8_t get_knx_timer_ticks(void) {
  return knx_timer_ticks;
}

void set_knx_timer_ticks(uint8_t ticks) {
  knx_timer_ticks = ticks;
}

ISR(TIMER3_COMPA_vect) {
  knx_timer_ticks++;
}

//ISR(TIMER3_OVF_vect) {
//  knx_timer_ticks = 1;
//}
