#include "network_layer.h"


//requests

bool N_Data_Individual_req(bool ack_request, unsigned char destination_address[2], unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char nsdu[]) {
  nsdu[0] = hop_count_type;
  unsigned char individualAddr[2];
  EEPROM.get(GRAT_ADDR + 1, individualAddr);

  return L_Data_req(ack_request, INDIVIDUAL, destination_address, STANDARD_FRAME, nsdu, octet_count, priority, individualAddr);
}

bool N_Data_Group_req(bool ack_request, unsigned char destination_address[2], unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char nsdu[]) {
  nsdu[0] = hop_count_type;
  unsigned char individualAddr[2];
  EEPROM.get(GRAT_ADDR + 1, individualAddr);

  return L_Data_req(ack_request, MULTICAST, destination_address, STANDARD_FRAME, nsdu, octet_count, priority, individualAddr);
}

bool N_Data_Broadcast_req(bool ack_request, unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char nsdu[]) {
  nsdu[0] = hop_count_type;
  unsigned char individualAddr[2];
  EEPROM.get(GRAT_ADDR + 1, individualAddr);

  return L_Data_req(ack_request, MULTICAST, BROADCAST_ADDR, STANDARD_FRAME, nsdu, octet_count, priority, individualAddr);
}

//void N_Data_SystemBroadcast_req(bool ack_request, unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char nsdu[]) {
//  nsdu[0] = hop_count_type;
//
//  L_SystemBroadcast_req(ack_request, MULTICAST, BROADCAST_ADDR, STANDARD_FRAME, nsdu, octet_count, priority);
//}

//indications
void L_Data_ind(bool ack_request, unsigned char address_type, unsigned char destination_address[2], unsigned char frame_format, unsigned char lsdu[], unsigned char octet_count, unsigned char priority, unsigned char source_address[2]) {
  unsigned char hop_count_type = lsdu[0];
  if(address_type == MULTICAST && ((destination_address[0] | destination_address[1]) != 0)) {
    N_Data_Group_ind(destination_address, hop_count_type, octet_count, priority, lsdu);

  } else if(address_type == INDIVIDUAL) {
    N_Data_Individual_ind(destination_address, hop_count_type, octet_count, priority, source_address, lsdu);

  } else if(address_type == MULTICAST && ((destination_address[0] | destination_address[1]) == 0)) {
    N_Data_Broadcast_ind(hop_count_type, octet_count, priority, source_address, lsdu);
  }
}

void L_SystemBroadcast_ind(bool ack_request, unsigned char address_type, unsigned char destination_address[2], unsigned char lsdu[], unsigned char octet_count, unsigned char priority, unsigned char source_address[2]) {

}

//confirmations

void L_Data_con(unsigned char address_type, unsigned char destination_address[2], unsigned char frame_format, unsigned char octet_count, unsigned char priority, unsigned char source_address[2], unsigned char lsdu[], unsigned char l_status) {
  unsigned char ack_request = true;//TODO
  unsigned char hop_count_type = lsdu[0];
  if(address_type == MULTICAST && (destination_address[0] | destination_address[1] != 0)) {
    N_Data_Group_con(ack_request, destination_address, hop_count_type, octet_count, priority, lsdu, l_status);
  } else if(address_type == INDIVIDUAL) {
    N_Data_Individual_con(ack_request, destination_address, hop_count_type, octet_count, priority, lsdu, l_status);
  }
}
