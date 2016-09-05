#include <jebecef.h>

void init_jebecef(void) {
  init_uart1(); 
  init_knx_timer();
}

void handle_jebecef(void) {
  handleAIL();
  check_link_layer();
  check_transport_layer();
}
