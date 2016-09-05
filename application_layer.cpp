#include "application_layer.h"
#include <string.h>


unsigned char groat_index = 0;

unsigned char getNextTsap(unsigned char asap) {
  unsigned char association_number, asap2;
  EEPROM.get(GROAT_ADDR, association_number);
  for(; groat_index < association_number; groat_index++) { //TODO count downwards is smaller code size????
    asap2 = EEPROM.read(GROAT_ADDR + 2 + 2*groat_index);
    if(asap2 == asap) {
      return EEPROM.read(GROAT_ADDR + 1 + 2*groat_index);
    }
  }
  return 0xff;
}

unsigned char getNextAsap(unsigned char tsap) {
  unsigned char association_number, tsap2;
  EEPROM.get(GROAT_ADDR, association_number);
  for(; groat_index < association_number; groat_index++) { //TODO count downwards is smaller code size????
    tsap2 = EEPROM.read(GROAT_ADDR + 1 + 2*groat_index);
    if(tsap2 == tsap) {
      return EEPROM.read(GROAT_ADDR + 2 + 2*groat_index++);
    }
  }
  return 0xff;
}

unsigned char getObjectLength(unsigned char type_octet) {
  if(type_octet < 6) {
    return 1;
  } else if(type_octet < 8) {
    return 2;
  } else if(type_octet < 10) {
    return (type_octet - 5);
  } else if(type_octet < 14) {
    return (type_octet - 10) * 2 + 5;
  } else if(type_octet == 14) {
    return 15;
  }
}

//services

bool A_GroupValue_Read_req(bool ack_request, unsigned char asap, unsigned char priority, unsigned char hop_count_type) {
  unsigned char tsap[2];
  unsigned char asap2;
  unsigned char association_number;
  EEPROM.get(GROAT_ADDR, association_number);
  for(unsigned char i = 0; i < association_number; i++) {
    asap2 = EEPROM.read(GROAT_ADDR + 2 + 2*i);
    if(asap2 == asap) {
      unsigned char tsap2 = EEPROM.read(GROAT_ADDR + 1 + 2*i);
      EEPROM.get(GRAT_ADDR + 3 + 2 * tsap2, tsap);
    }
  }

  unsigned char tsdu[3] = {0x00, A_GROUPVALUE_READ_PDU_L, A_GROUPVALUE_READ_PDU_H};
  unsigned char octet_count = 1;
  return T_Data_Group_req(ack_request, hop_count_type, octet_count, priority, tsdu, tsap);
}



bool A_GroupValue_Read_res(bool ack_request, unsigned char asap, unsigned char priority, unsigned char hop_count_type, unsigned char data[]) {
  groat_index = 0;
  unsigned char tsap = getNextTsap(asap);
  unsigned char destination_address[2];
  EEPROM.get(GRAT_ADDR + 3 + 2*tsap, destination_address);

  data[1] = A_GROUPVALUE_RESPONSE_PDU_H;
  data[2] = A_GROUPVALUE_RESPONSE_PDU_L; 

  unsigned char octet_count = EEPROM.read(GROT_ADDR + 2 + asap * 3 + TYPE_OCTET); //TODO unnecesary, get the data already in the application_interface_layer???
  octet_count = getObjectLength(octet_count);
  if(octet_count == 1) {//optimized A_GroupValue_Response-PDU format
    data[2] |= data[3] & DATA_MASK; 
  }

  return T_Data_Group_req(ack_request, hop_count_type, octet_count, priority, data, destination_address);
}


bool A_GroupValue_Write_req(bool ack_request, unsigned char asap, unsigned char priority, unsigned char hop_count_type, unsigned char data[]) {
  groat_index = 0;
  unsigned char tsap = getNextTsap(asap);
  unsigned char destination_address[2];
  EEPROM.get(GRAT_ADDR + 3 + 2*tsap, destination_address);
  
  data[1] = A_GROUPVALUE_WRITE_PDU_H;
  data[2] = A_GROUPVALUE_WRITE_PDU_L; 

  unsigned char octet_count = EEPROM.read(GROT_ADDR + 2 + asap * 3 + TYPE_OCTET); //TODO unnecesary, get the data already in the application_interface_layer???
  octet_count = getObjectLength(octet_count);
  if(octet_count == 1) {//optimized A_GroupValue_Response-PDU format
    data[2] |= data[3] & DATA_MASK; 
  }

  return T_Data_Group_req(ack_request, hop_count_type, octet_count, priority, data, destination_address);
}


void A_Memory_Read_res(bool ack_request, unsigned char asap[2], unsigned char priority, unsigned char number, unsigned char memory_address[2], unsigned char data[]) {
  unsigned char octet_count = number + 3; //number + APCI(2) + memory_address(2)
  memcpy(data+3, memory_address, 2);
  data[2] = A_MEMORY_RESPONSE_PDU_L | number;
  data[1] = A_MEMORY_RESPONSE_PDU_H;
  T_Data_Connected_req(priority, asap, octet_count, data);
}

void A_DeviceDescriptor_Read_res(bool ack_request, unsigned char asap[2], unsigned char priority, unsigned char hop_count_type, unsigned char descriptor_type, unsigned char device_descriptor[2]) {
  unsigned char octet_count = 3;
  unsigned char tsdu[octet_count + 2];
  tsdu[1] = A_DEVICEDESCRIPTOR_RESPONSE_PDU_H;
  tsdu[2] = A_DEVICEDESCRIPTOR_RESPONSE_PDU_L | descriptor_type;
  tsdu[3] = device_descriptor[0]; 
  tsdu[4] = device_descriptor[1]; 
  T_Data_Individual_req(ack_request, hop_count_type, octet_count, priority, tsdu, asap);
}

//indications

void T_Data_Group_ind(unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[], unsigned char tsap) {
  unsigned char asap;
  if(((tsdu[1] & APDU_MASK_H) == A_GROUPVALUE_READ_PDU_H) && ((tsdu[2] & APDU_MASK_L_SHORT) == A_GROUPVALUE_READ_PDU_L)) {
    groat_index = 0;
    asap = getNextAsap(tsap);
    //TODO check readable????
    //while(!readable) {
    //  asap = getNextAsap()
    //}
    A_GroupValue_Read_ind(asap, priority, hop_count_type);
  } else if(((tsdu[1] & APDU_MASK_H) == A_GROUPVALUE_WRITE_PDU_H) && ((tsdu[2] & APDU_MASK_L_SHORT) == A_GROUPVALUE_WRITE_PDU_L)) {
    tsdu[2] &= DATA_MASK; //remove APDU from data
    groat_index = 0;
    while(true) { //for each asap
      asap = getNextAsap(tsap);
      if(asap == 0xFF) {
        break;
      }
      A_GroupValue_Write_ind(asap, priority, hop_count_type, tsdu);
    }
  } else if(((tsdu[1] & APDU_MASK_H) == A_GROUPVALUE_RESPONSE_PDU_H) && ((tsdu[2] & APDU_MASK_L_SHORT) == A_GROUPVALUE_RESPONSE_PDU_L)) {
    tsdu[2] &= DATA_MASK; //remove APDU from data
    groat_index = 0;
    while(true) { //for each asap
      asap = getNextAsap(tsap);
      if(asap == 0xFF) {
        break;
      }
      A_GroupValue_Read_Acon(asap, priority, hop_count_type, tsdu);    
    }
  }
}

void T_Data_Tag_Group_ind(unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[], unsigned char source_address[2], unsigned char destinationAddress[2], unsigned char frameFormat) {

}

void T_Data_Broadcast_ind(unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[], unsigned char source_address[2]) {
  if(A_INDIVIDUAL_ADDRESS_WRITE_PDU_H == (tsdu[1] & APDU_MASK_H) && A_INDIVIDUAL_ADDRESS_WRITE_PDU_L == (tsdu[2] & APDU_MASK_L_SHORT)) {
    A_IndividualAddress_Write_ind(priority, hop_count_type, &tsdu[3]);
  }  
}

void T_Data_SystemBroadcast_ind(unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[], unsigned char source_address[2]) {

}

void T_Data_Individual_ind(unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[], unsigned char tsap[2]) {
  if(A_DEVICEDESCRIPTOR_READ_PDU_H == (tsdu[1] & APDU_MASK_H) && A_DEVICEDESCRIPTOR_READ_PDU_L == (tsdu[2] & APDU_MASK_L_SHORT)) {
    A_DeviceDescriptor_Read_ind(tsap, priority, hop_count_type, tsdu[2] & DESCRIPTOR_TYPE_MASK);
  }
}

void T_Connect_ind(unsigned char tsap[2]) {


}

void T_Disconnect_ind(unsigned char tsap[2]) {

}

void T_Data_Connected_ind(unsigned char tsap[2], unsigned char octet_count, unsigned char priority, unsigned char tsdu[]) {
  if((A_MEMORY_READ_PDU_H == (tsdu[1] & APDU_MASK_H)) && (A_MEMORY_READ_PDU_L == (tsdu[2] & APDU_MASK_L_SHORT))) {
    DEBUGGING("AMR");
    unsigned char number = tsdu[2]; //no mask because the last two bits of the APCI are 0; 0baannnnnn : a = apci, n = number
    unsigned char memory_address[2];
    memcpy(memory_address, tsdu+3, 2);
    A_Memory_Read_ind(tsap, priority, number, memory_address);
  } else if((A_MEMORY_WRITE_PDU_H == (tsdu[1] & APDU_MASK_H)) && (A_MEMORY_WRITE_PDU_L == (tsdu[2] & APDU_MASK_L_SHORT))) {
    DEBUGGING("AMW");
    unsigned char number = tsdu[2] & NUMBER_MASK; 
    unsigned char memory_address[2];
    memcpy(memory_address, tsdu+3, 2);
    A_Memory_Write_ind(tsap, priority, number, memory_address, &tsdu[5]);
  }
}


//confirmations

void T_Data_Group_con(unsigned char ack_request, unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsap, unsigned char tsdu[], unsigned char t_status) {
  unsigned char asap;
  groat_index = 0;
  if(((tsdu[1] & APDU_MASK_H) == A_GROUPVALUE_READ_PDU_H) && ((tsdu[2] & APDU_MASK_L_SHORT) == A_GROUPVALUE_READ_PDU_L)) {
    //inform all ASAPs, if ramFlags is transmitting then set ramFlags OK else readPDU
    unsigned char readable = 0;
    while((asap = getNextAsap(tsap)) != 0xFF) {
      //clear transmitting flag
      A_GroupValue_Read_Lcon(ack_request, asap, priority, hop_count_type, t_status);
      //until one readable comObj, do A_GroupValue_Read_ind
      if(!readable) { //TODO don't request the requesting comObj
        readable = 1; //TODO handle readable
        A_GroupValue_Read_ind(asap, priority, hop_count_type);
      }
    }
  } else if(((tsdu[1] & APDU_MASK_H) == A_GROUPVALUE_WRITE_PDU_H) && ((tsdu[2] & APDU_MASK_L_SHORT) == A_GROUPVALUE_WRITE_PDU_L)) {
    //inform all ASAPs, if ramFlags is transmitting then set ramFlags OK else writePDU
    while((asap = getNextAsap(tsap)) != 0xFF) {
      A_GroupValue_Write_Lcon(ack_request, asap, priority, hop_count_type, t_status, tsdu);
    }
  } else if(((tsdu[1] & APDU_MASK_H) == A_GROUPVALUE_RESPONSE_PDU_H) && ((tsdu[2] & APDU_MASK_L_SHORT) == A_GROUPVALUE_RESPONSE_PDU_L)) {
    //inform all ASAPs, if ramFlags is transmitting then set ramFlags OK else responsePDU
    while((asap = getNextAsap(tsap)) != 0xFF) {
      A_GroupValue_Read_Rcon(ack_request, asap, priority, hop_count_type, t_status, tsdu);
    }
  }
}

void T_Data_Connected_con(unsigned char tsap[2]) {

}
