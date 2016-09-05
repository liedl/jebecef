#ifndef APPLICATION_LAYER_H
#define APPLICATION_LAYER_H

#include "eeprom.h"
#include "knx.h"
#include "transport_layer.h"
#include "application_interface_layer.h"


//get octet_count from type_octet
unsigned char getObjectLength(unsigned char type_octet);

//services

bool A_GroupValue_Read_req      (bool ack_request, unsigned char asap, unsigned char priority, unsigned char hop_count_type);
bool A_GroupValue_Read_res      (bool ack_request, unsigned char asap, unsigned char priority, unsigned char hop_count_type, unsigned char data[]);
bool A_GroupValue_Write_req     (bool ack_request, unsigned char asap, unsigned char priority, unsigned char hop_count_type, unsigned char data[]);

void A_Memory_Read_res          (bool ack_request, unsigned char asap[2], unsigned char priority, unsigned char number, unsigned char memory_address[2], unsigned char data[]);
void A_DeviceDescriptor_Read_res(bool ack_request, unsigned char asap[2], unsigned char priority, unsigned char hop_count_type, unsigned char descriptor_type, unsigned char device_descriptor[2]);

//indications

void T_Data_Group_ind           (unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[], unsigned char tsap);
void T_Data_Tag_Group_ind       (unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[], unsigned char source_address[2], unsigned char destinationAddress[2], unsigned char frameFormat);
void T_Data_Broadcast_ind       (unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[], unsigned char source_address[2]);
void T_Data_SystemBroadcast_ind (unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[], unsigned char source_address[2]);
void T_Data_Individual_ind      (unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[], unsigned char tsap[2]);
void T_Connect_ind              (unsigned char tsap[2]);
void T_Disconnect_ind           (unsigned char tsap[2]);
void T_Data_Connected_ind       (unsigned char tsap[2], unsigned char octet_count, unsigned char priority, unsigned char tsdu[]);

//confirmations

void T_Data_con          (unsigned char t_status);
void T_Data_Group_con    (unsigned char ack_request, unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsap, unsigned char tsdu[], unsigned char t_status);
void T_Data_Connected_con(unsigned char tsap[2]);

#endif //TRANSPORT_LAYER_H
