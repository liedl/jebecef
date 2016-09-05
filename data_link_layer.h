#ifndef DATA_LINK_LAYER_H
#define DATA_LINK_LAYER_H

#include "uart1.h"
#include <stdbool.h>
#include "eeprom.h"

void write(unsigned char);


void setSerial(void);

extern unsigned char lsdu[17];

unsigned char get_lsdu(unsigned char i);

void check_link_layer(void);

//States
void waiting_reset(void);
void st_addresses(void);
void st_length(void);
void st_lsdu(void);
void st_frameend(void);
void st_framestate(void);
void st_not_addressed(void); 
void st_frameend_not_addressed(void);

void send_checksum(void);

void extended_frame(void);
void waiting(void);

void configure(void);
void waiting_configure(void);
void reset(void);
void waiting_reset(void);

void transmit_data(void); 

//Services

void configure_req(void);
void reset_req(void);

bool L_Data_req(bool ack_request, unsigned char address_type, unsigned char destination_address[2], unsigned char frame_format, unsigned char lsdu[], unsigned char octet_count, unsigned char priority, unsigned char source_address[2]);

//void L_SystemBroadcast_req(bool ack_request, unsigned char address_type, unsigned char destination_address[2], unsigned char frame_format, unsigned char lsdu[], unsigned char octet_count, unsigned char priority);

//void L_Poll_Data_req(unsigned short destination, unsigned char no_of_expected_poll_data);

//void L_Poll_Update_req(unsigned char Poll_Data);


#endif //DATA_LINK_LAYER_H
