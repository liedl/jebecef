#ifndef NETWORK_LAYER_H
#define NETWORK_LAYER_H

#include "data_link_layer.h"
#include "transport_layer.h"
#include "knx.h"





//services

//requests
bool N_Data_Individual_req(bool ack_request, unsigned char destination_address[2], unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char nsdu[]);
bool N_Data_Group_req(bool ack_request, unsigned char destination_address[2], unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char nsdu[]);
bool N_Data_Broadcast_req(bool ack_request, unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char nsdu[]);
//void N_Data_SystemBroadcast_req(bool ack_request, unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char nsdu[]);

//indications

void L_Data_ind(bool ack_request, unsigned char address_type, unsigned char destination_address[2], unsigned char frame_format, unsigned char lsdu[], unsigned char octet_count, unsigned char priority, unsigned char source_address[2]);
//void L_SystemBroadcast_ind(bool ack_request, unsigned char address_type, unsigned char destination_address[2], unsigned char lsdu[], unsigned char octet_count, unsigned char priority, unsigned char source_address[2]);

//confirmations

//L_Data_con(unsigned char address_type, unsigned char destination_address[2], unsigned char frame_format, unsigned char octet_count, unsigned char priority, unsigned char source_address[2], unsigned char lsdu[], unsigned char l_status);

void L_Data_con(unsigned char l_status);
void L_Data_con(unsigned char address_type, unsigned char destination_address[2], unsigned char frame_format, unsigned char octet_count, unsigned char priority, unsigned char source_address[2], unsigned char lsdu[], unsigned char l_status);

#endif //NETWORK_LAYER_H
