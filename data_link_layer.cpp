#include "data_link_layer.h"
#include "network_layer.h"
#include "knx-transceiver.h"
#include "uart1.h"
#include "knx.h"

extern unsigned char tx_ongoing;

union addresses addresses;
unsigned char address_type;
unsigned char ack_request;
unsigned char frame_format;
unsigned char octet_count;
unsigned char priority;
unsigned char lsdu[17];
unsigned char count;
unsigned char xor_checksum;
unsigned char xor_checksum_tx;
unsigned char frame_tx[24];

void (*statefunc)(void) = waiting;
void (*service)(void) = configure;
unsigned char service_requested = true;

unsigned char data_to_transmit = 0; //count of bytes to transmit
unsigned char data_transmitted = 0; //count of bytes, which already transmitted 

unsigned char get_lsdu(unsigned char i) {
  return addresses.array[i];
}

void waiting(void) {
  unsigned char c = ugetchar1();
  if((c & STANDARD_FRAME_MASK) == STANDARD_FRAME_PATTERN) {
    priority = (c & PRIO_MASK) >> 2;
    //TODO check if repetition 
    count = 0;
    xor_checksum = c;
    statefunc = st_addresses;
//  } else if((c & EXTENDED_FRAME_MASK) == EXTENDED_FRAME_PATTERN) {
//    //ringbuffer
//    statefunc = extended_frame;
//  } else if(c == POLL_FRAME_PATTERN) {
//    //todo
//  } else if((c & ACKN_MASK) == ACKN_PATTERN) {
//    //todo
//  } else if((c & STATE_MASK) == STATE_PATTERN) {
//  } else if((c & FRAMESTATE_MASK) == FRAMESTATE_PATTERN) {
  } else if((c & DATA_CON_MASK) == DATA_CON_PATTERN) {
    L_Data_con(address_type, addresses.dest_addr, STANDARD_FRAME, octet_count, priority, addresses.src_addr, lsdu, c >> 7); // 1 = positive; 0 = negative;
    tx_ongoing = false;
    data_transmitted = 0;
  } else if(c == RESET_PATTERN) {
  } else if(c == STOPMODE_PATTERN) {
  } else if(c == SYSTEMSTATE_PATTERN) {
  } else if(c == FRAMEEND_PATTERN) {
  } else  {
  }
}

void configure(void) {
  uputchar1(RESET);
  while(ugetchar1() != RESET_PATTERN) {}
  uputchar1(CONFIGURE | (1 << END_MARKER));
  while((ugetchar1() & CONFIGURE_MASK) != CONFIGURE_PATTERN) {}
  statefunc = waiting;
//  uputchar1(CONFIGURE | (1 << END_MARKER));
//  statefunc = waiting_configure;
}

void waiting_configure(void) {
  if((ugetchar1() & CONFIGURE_MASK) == CONFIGURE_PATTERN) {
    //TODO check returned value
    statefunc = waiting;
  }
}

void reset(void) {
  uputchar1(RESET);
  statefunc = waiting_reset;
}

void waiting_reset(void) {
  if(ugetchar1() == RESET_PATTERN) {
    statefunc = waiting;
    configure_req();  
  }//TODO else error
}


void st_addresses(void) {
  unsigned char c = ugetchar1();
  addresses.array[count] = c;
  xor_checksum ^= c;
  count++;
  if(count > 3) {
    statefunc = st_length;
  }
}

void st_length(void) {
  unsigned char c = ugetchar1();
  xor_checksum ^= c;
  octet_count = c & LENGTH_MASK;
  address_type = c>>7;
  lsdu[0] = c>>4 & 0x7;
  count = 1;
  if(tx_ongoing) { //telegram is from me
    DEBUGGING("TXO");
    statefunc = st_lsdu;
    return;
  }
  bool addressed = false;
  if(GROUP_ADDR == address_type) {
    GroupAddressTable grat;
    EEPROM.get(GRAT_ADDR, grat);
    if(0 == addresses.dest_addr_high && 0 == addresses.dest_addr_low) { //if broadcast address
      addressed = true;
    } else {
      for(char i = 1; i < grat.length; i++) { // i=0 is individual address //TODO optimize search algorithm
        if(grat.addresses[i][ADDRHIGH] == addresses.dest_addr_high
            && grat.addresses[i][ADDRLOW] == addresses.dest_addr_low) {
          addressed = true;
          break;
        }
      }
    }
  } else {
    unsigned char individualAddr[2];
    EEPROM.get(GRAT_ADDR + 1, individualAddr);
    if((individualAddr[ADDRHIGH] == addresses.dest_addr_high)
        && (individualAddr[ADDRLOW] == addresses.dest_addr_low)) {
      DEBUGGING("Lia");
      addressed = true;
    }
  }

  if(addressed) {
    uputchar1(ADDRESSED);
    statefunc = st_lsdu;
  } else {
    uputchar1(NOT_ADDRESSED);
    statefunc = st_not_addressed;
  }
}

void st_not_addressed(void) {
  unsigned char c = ugetchar1();
  if(c == FRAMEEND) {
    statefunc = st_frameend_not_addressed;
  }
}

void st_frameend_not_addressed(void) {
  unsigned char c = ugetchar1();
  if(c == FRAMEEND) {
    statefunc = st_not_addressed;
  } else {
    statefunc = waiting;
  }
}

void st_lsdu(void) {
  unsigned char c = ugetchar1();
  if(c == FRAMEEND) {
    statefunc = st_frameend;
  } else {
    xor_checksum ^= c;
    lsdu[count++] = c;
  }
}

void st_frameend(void) {
  unsigned char c = ugetchar1(); 
  if(c == FRAMEEND) {
    lsdu[count++] = c;
    xor_checksum ^= c;
    statefunc = st_lsdu;
  } else { //U_Frame_State_ind
    if(xor_checksum == 0xFF) {
      if(!data_transmitted) { //telegram is not from me
        //call network layer
        L_Data_ind(true, address_type, addresses.dest_addr, STANDARD_FRAME, lsdu, octet_count, priority, addresses.src_addr);
      }
    } else {
      // wrong checksum
    }
    if(data_transmitted) {
      //state = waiting L_data.con//TODO
    }
    statefunc = waiting;
  }
}

void send_checksum(void) {
  if(utx1_ready3()) {
    uputchar1(L_DATAEND | data_transmitted);
    uputchar1(~xor_checksum_tx);
    statefunc = waiting;
  } else {
    service_requested = true;
  }
}


void configure_req(void) {
  service = configure;
  service_requested = true; 
}

void reset_req(void) {
  service = reset;
  service_requested = true;
}

void transmit_data() {
  if(data_transmitted) { //DataStart already send
    unsigned char octet = frame_tx[data_transmitted];
    uputchar1(L_DATACON | data_transmitted);
    uputchar1(octet);
    data_transmitted++;
    xor_checksum_tx ^= octet;
    if(data_transmitted == data_to_transmit) {
      service = send_checksum;
      service_requested = true;
      data_to_transmit = 0;
    }
  } else { // send DATASTART
    unsigned char control_byte = frame_tx[0];
    uputchar1(L_DATASTART);
    uputchar1(control_byte);
    xor_checksum_tx = control_byte; 
    data_transmitted++;
  } 
}

bool L_Data_req(bool ack_request, unsigned char address_type, unsigned char destination_address[2], unsigned char frame_format, unsigned char lsdu[], unsigned char octet_count, unsigned char priority, unsigned char source_address[2]) {
  DEBUGGING("LDR");
  frame_tx[0] = STANDARD_FRAME_PATTERN | (1 << NOT_REPEAT) | (priority << 2); //TODO frameformat
  frame_tx[1] = source_address[ADDRHIGH]; 
  frame_tx[2] = source_address[ADDRLOW];
  frame_tx[3] = destination_address[ADDRHIGH];
  frame_tx[4] = destination_address[ADDRLOW];
  frame_tx[5] = (address_type << 7) | (lsdu[0] << 4) | octet_count;
  octet_count++;
  for(unsigned char i = 1; i <= octet_count; i++) {
    frame_tx[i+5] = lsdu[i];
  }
  data_transmitted = 0;
  data_to_transmit = octet_count + 6;
  
  tx_ongoing = true;
  return true;
}

//void L_SystemBroadcast_req(bool ack_request, unsigned char address_type, unsigned char destination_address[2], unsigned char frame_format, unsigned char lsdu[], unsigned char octet_count, unsigned char priority) {
//  
//  data_to_transmit = true;
//}


void check_link_layer(void) {
  if(statefunc == waiting && service_requested) {
    statefunc = service;
    service_requested = false;
    (*statefunc)();  
  } else if(ukbhit1()) {	//data in uart puffer
    while(ukbhit1()) {
      (*statefunc)();  
    }
  } else if(data_to_transmit && utx1_ready3()) {   //transmitpuffer ready and data to transmit
    char i[2] = {0, 0};
    while(data_to_transmit && utx1_ready3()) {
      transmit_data();
      i[0]++;
    }
  }
}

