#ifndef TRANSPORT_LAYER_H
#define TRANSPORT_LAYER_H

#include "network_layer.h"
#include "application_layer.h"
#include "timer.h"
#include "knx.h"

#define CONNECTION_TIMEOUT_TIMER     0x01
#define ACKNOWLEDGMENT_TIMEOUT_TIMER 0x02

void check_transport_layer(void);

//services
bool T_Data_Group_req           (bool ack_request, unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[], unsigned char tsap[2]);
bool T_Data_Tag_Group_req       (bool ack_request, unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[], unsigned char destination_address[2], unsigned char frame_format);
bool T_Data_Broadcast_req       (bool ack_request, unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[]);
bool T_Data_SystemBroadcast_req (bool ack_request, unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[]);
bool T_Data_Individual_req      (bool ack_request, unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[], unsigned char tsap[2]);
bool T_Connect_req              (unsigned char priority, unsigned char destination_address[2]);
bool T_Disconnect_req           (unsigned char priority, unsigned char tsap[2]);
bool T_Data_Connected_req       (unsigned char priority, unsigned char tsap[2], unsigned char octet_count, unsigned char tsdu[]);





//indications

void N_Data_Individual_ind(unsigned char destination_address[2], unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char source_address[2], unsigned char nsdu[]);

void N_Data_Group_ind     (unsigned char destination_address[2], unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char nsdu[]);
void N_Data_Broadcast_ind (unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char source_address[2], unsigned char nsdu[]);

//confirmations

void N_Data_con(unsigned char n_status);
void N_Data_Individual_con();

void N_Data_Group_con      (unsigned char ack_request, unsigned char destination_address[2], unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char nsdu[], unsigned char n_status);
void N_Data_Individual_con (unsigned char ack_request, unsigned char destination_address[2], unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char nsdu[], unsigned char n_status);

#endif //TRANSPORT_LAYER_H
