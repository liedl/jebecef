#define EXTERN
#include "application_interface_layer.h"


#define ACTION_GROUPVALUE_WRITE     0
#define ACTION_GROUPVALUE_READ      1
#define ACTION_GROUPVALUE_RESPONSE  2

extern unsigned char tx_ongoing;
unsigned char programming_mode = 0;

struct Response {
  unsigned char asap = 0xFF;
  unsigned char priority;
};

Response response_waiting;

void handleAIL(void) {
  if(!tx_ongoing) {
    if(response_waiting.asap != 0xFF) {
      //A_GroupValue_Read_ind(response_waiting.asap, response_waiting.priority, N_HOP_COUNT_TYPE);
      response_waiting.asap = 0xFF;
      return;
    }
    for(unsigned char i = 0; i < (GROUP_OBJECT_NUMBER+1)/2; i++) {
      if(ramFlagsTable[i] & 0b01110111) {
        unsigned char asap;
        if((ramFlagsTable[i] & 0x07)) {
          asap = 2*i;
        } else {
          asap = 2*i+1;
        }

        if       ((ramFlagsTable[i] & 0b00000011) == 0b00000011) { //transmitting low
          ramFlagsTable[i] &= 0b11110010; //change transmit_request to transmitting flag; read a value after writing it is nonsense
          if(!send_write_request(asap)) { //not allowed to send write request (T and C flag must set)
            ramFlagsTable[i] &= 0b11110000; //set status OK
            continue;
          }
        } else if((ramFlagsTable[i] & 0b00110000) == 0b00110000) { //transmitting high
          ramFlagsTable[i] &= 0b00101111; //change transmit_request to transmitting flag; read a value after writing it is nonsense
          if(!send_write_request(asap)) { //not allowed to send write request (T and C flag must set)
            ramFlagsTable[i] &= 0b00001111; //set status OK
            continue;
          }
        } else if((ramFlagsTable[i] & 0b00000100) == 0b00000100) { //data request low
          ramFlagsTable[i] |= 0b00000010; // set transmitting flag
          ramFlagsTable[i] &= 0b11111011; // clear data request flag 
          if(!send_read_request(asap)) { //not allowed to send read request (C flag must set)
            ramFlagsTable[i] &= 0b11110000; //set status OK
            continue;
          }
        } else if((ramFlagsTable[i] & 0b01000000) == 0b01000000) { //data request low
          ramFlagsTable[i] |= 0b00100000; // set transmitting flag
          ramFlagsTable[i] &= 0b10111111; // clear data request flag 
          if(!send_read_request(asap)) { //not allowed to send read request (C flag must set)
            ramFlagsTable[i] &= 0b00001111; //set status OK
            continue;
          }
        }
        break;
      }
    }
  }
}
unsigned char checkUpdate(void) {
  for(unsigned char i = 0; i < (GROUP_OBJECT_NUMBER+1)/2; i++) {
    if((ramFlagsTable[i] & 0b1000) == UPDATE) {
      ramFlagsTable[i] &= ~(UPDATE); //clear update flag
      return 2*i;
    }
    if((ramFlagsTable[i] & (UPDATE << 4)) == (UPDATE << 4)) {
      ramFlagsTable[i] &= ~(UPDATE << 4); //clear update flag
      return 2*i+1;
    }
  }
  return 0xFF;
}


void setRamFlags(unsigned char asap, unsigned char flags) {
  if(asap & 0b1) { //if odd
    unsigned char temp = flags >> 4;
    flags = flags << 4;
    flags |= temp;
  }
  ramFlagsTable[asap/2] |= flags;
}

void clearRamFlags(unsigned char asap, unsigned char flags) {
  if(asap & 0b1) { //if odd
    unsigned char temp = flags >> 4;
    flags = flags << 4;
    flags |= temp;
  } 
  ramFlagsTable[asap/2] &= ~(flags);
}

bool checkRamFlags(unsigned char asap, unsigned char maskflags) { //maskflags = 0bMMMMFFFF
  unsigned char mask = maskflags >> 4;     //0b0000MMMM M=mask
  if(asap & 0b1) { //if odd
    mask = mask << 4;            //0bMMMM0000
    maskflags = maskflags << 4;  //0bFFFF0000
  }
  unsigned char ramFlags = ramFlagsTable[asap/2];
  return (ramFlags & mask) == maskflags;
}

void read_telegram(unsigned char asap) {
  setRamFlags(asap, DATA_REQUEST);
} 

void write_telegram(unsigned char asap, unsigned char data[]) {
  unsigned char groupObject[3];
  EEPROM.get(GROT_ADDR + 2 + asap * 3, groupObject);
  unsigned char length = getObjectLength(groupObject[TYPE_OCTET]);
  length = (length == 1 ? 2 : length) - 1;
  for(unsigned char i = 0; i < length; i++) {
    EEPROM.update(OBJECT_DATA_ADDR + groupObject[DATA_POINTER] + i, data[i]);
  }
  setRamFlags(asap, TRANSMIT_REQUEST_S);
}

void get_value(unsigned char asap, unsigned char data[]) {
  unsigned char groupObject[3];
  EEPROM.get(GROT_ADDR + 2 + asap * 3, groupObject);
  unsigned char length = getObjectLength(groupObject[TYPE_OCTET]);
  length = (length == 1 ? 2 : length) - 1;
  for(unsigned char i = 0; i < length; i++) {
    data[i] = EEPROM.read(OBJECT_DATA_ADDR + groupObject[DATA_POINTER] + i);
  }
}

unsigned char get_parameter(unsigned char address) {
  return EEPROM.read(PARAMETER_ADDR + address);
}


void set_programming_mode(void) {
  programming_mode = 1;
}

unsigned char get_programming_mode(void) {
  return programming_mode;
}

//request to the bus
bool send_read_request(unsigned char asap) {
  unsigned char groupObject[3];
  EEPROM.get(GROT_ADDR + 2 + asap * 3, groupObject);
  unsigned char priority = groupObject[CONFIG_OCTET] & FLAG_PRIO_MASK; 
  return A_GroupValue_Read_req(true, asap, priority, N_HOP_COUNT_TYPE);
}

bool send_write_request(unsigned char asap) {
  unsigned char groupObject[3];
  EEPROM.get(GROT_ADDR + 2 + asap * 3, groupObject);
  if((groupObject[CONFIG_OCTET] & (FLAG_C | FLAG_T)) == (FLAG_C | FLAG_T)) { //is communication and transmit flag set?
    unsigned char priority = groupObject[CONFIG_OCTET] & FLAG_PRIO_MASK; 
  
    unsigned char length = getObjectLength(groupObject[TYPE_OCTET]);
    length = (length == 1 ? 2 : length) - 1;
    
    unsigned char data[length + 3]; //3 octet offset, because the overhead of the lower layers
    for(unsigned char i = 0; i < length; i++) {
      data[i + 3] =  EEPROM.read(OBJECT_DATA_ADDR + groupObject[DATA_POINTER] + i);
    }
    
    return A_GroupValue_Write_req(true, asap, priority, N_HOP_COUNT_TYPE, data);
  } else {
    return false;
  }
}


//indications


void A_GroupValue_Read_ind(unsigned char asap, unsigned char priority, unsigned char hop_count_type) {
  unsigned char groupObject[3];
  EEPROM.get(GROT_ADDR + 2 + asap * 3, groupObject);
  if((groupObject[CONFIG_OCTET] & (FLAG_C | FLAG_R)) == (FLAG_C | FLAG_R)) { //is communication and read flag set?
    if(tx_ongoing) {
      response_waiting.asap = asap;
      response_waiting.priority = priority;
    } else {
      unsigned char length = getObjectLength(groupObject[TYPE_OCTET]);
      length = (length == 1 ? 2 : length) - 1;
      unsigned char data[length + 3]; //3 octet offset, because the overhead of the lower layer
      for(unsigned char i = 0; i < length; i++) {
        data[i + 3] = EEPROM.read(OBJECT_DATA_ADDR + groupObject[DATA_POINTER] + i);
      }
      A_GroupValue_Read_res(true, asap, priority, N_HOP_COUNT_TYPE, data);
    }
  }
}

void A_GroupValue_Read_Acon (unsigned char asap, unsigned char priority, unsigned char hop_count_type, unsigned char data[]) {
  unsigned char groupObject[3];
  EEPROM.get(GROT_ADDR + 2 + asap * 3, groupObject);
  if((groupObject[CONFIG_OCTET] & (FLAG_C | FLAG_U)) == (FLAG_C | FLAG_U)) { //is communication and update flag set?
    if(groupObject[TYPE_OCTET] < 6) { //optimized format, lesser than 7 bits
      data[3] = data[2];  
    }
    unsigned char length = getObjectLength(groupObject[TYPE_OCTET]);
    unsigned char eeprom_data[length];
    bool equal = true;
    for(unsigned char i = 0; i < length; i++) {
      eeprom_data[i] = EEPROM.read(OBJECT_DATA_ADDR + groupObject[DATA_POINTER] + i);
      if(data[3 + i] != eeprom_data[i]) {
        equal = false;
        break;
      }
    }
    if(equal) {
      //do nothing
    } else {
      for(unsigned char i = 0; i < length; i++) {
        EEPROM.update(OBJECT_DATA_ADDR + groupObject[DATA_POINTER] + i, data[3 + i]);
        setRamFlags(asap, UPDATE);
      }
    }
  }
}

void A_GroupValue_Write_ind(unsigned char asap, unsigned char priority, unsigned char hop_count_type, unsigned char data[]) {
  unsigned char groupObject[3];
  EEPROM.get(GROT_ADDR + 2 + asap * 3, groupObject);
  if((groupObject[CONFIG_OCTET] & (FLAG_C | FLAG_W)) == (FLAG_C | FLAG_W)) { //is communication and write flag set?
    if(groupObject[TYPE_OCTET] < 6) { //optimized format, lesser than 7 bits
      data[3] = data[2];  
    }
    unsigned char length = getObjectLength(groupObject[TYPE_OCTET]);
    unsigned char eeprom_data;
    bool equal = true;
    for(unsigned char i = 0; i < length; i++) {
      eeprom_data = EEPROM.read(OBJECT_DATA_ADDR + groupObject[DATA_POINTER] + i);
      if(data[3 + i] != eeprom_data) {
        equal = false;
        break;
      }
    }
    if(equal) {
      //do nothing
    } else {
      for(unsigned char i = 0; i < length; i++) {
        EEPROM.update(OBJECT_DATA_ADDR + groupObject[DATA_POINTER] + i, data[3 + i]);
      }
      setRamFlags(asap, UPDATE);
    }
  }
}


void A_DeviceDescriptor_Read_ind(unsigned char asap[2], unsigned char priority, unsigned char hop_count_type, unsigned char descriptor_type) {
  descriptor_type = 0;
  unsigned char device_descriptor[2] = {0x07, 0x01};
  A_DeviceDescriptor_Read_res(true, asap, priority, hop_count_type, descriptor_type, device_descriptor);
}

void A_Memory_Read_ind(unsigned char asap[2], unsigned char priority, unsigned char number, unsigned char memory_address[2]) {
  bool ack_request = true;
  unsigned char data[number + 5];  
  for(unsigned char i = 0; i < number; i++) {
    data[i + 5] = EEPROM.read((memory_address[0] << 8 | memory_address[1]) + i);
  }
  A_Memory_Read_res(ack_request, asap, priority, number, memory_address, data);
}

void A_Memory_Write_ind(unsigned char asap[2], unsigned char priority, unsigned char number, unsigned char memory_address[2], unsigned char data[]) {
  //TODO verify mode
  DEBUGGING("IMW");
  for(unsigned char i = 0; i < number; i++) {
    EEPROM.update((memory_address[0] << 8 | memory_address[1]) + i, data[i]);
  }
}


void A_IndividualAddress_Write_ind(unsigned char priority, unsigned char hop_count_type, unsigned char newaddress[2]) {
  if(programming_mode) {
    for(unsigned char i = 0; i < 2; i++) {
      EEPROM.update(GRAT_ADDR + GRAT_INDIVIDUALADDR_ADDR + i, newaddress[i]);
    } 
    programming_mode = 0;
  }
}

//confirmations

void A_GroupValue_Read_Lcon(unsigned char ack_request, unsigned char asap, unsigned char priority, unsigned char hop_count_type, unsigned char a_status) {
  //unsigned char transmitting = TRANSMITTING;
  unsigned char clear = 0xF0;
  //if(!(asap & 0b1)) {
    //transmitting = transmitting << 4;
    //clear = clear >> 4;
  if((ramFlagsTable[asap/2] & TRANSMITTING) == TRANSMITTING) {
    ramFlagsTable[asap/2] &= clear; //clear transmitting flag, set OK
  } else if((ramFlagsTable[asap/2] & (TRANSMITTING << 4)) == (TRANSMITTING << 4)) {
    ramFlagsTable[asap/2] &= (clear >> 4); //clear transmitting flag, set OK
  } else {
    A_GroupValue_Read_ind(asap, priority, hop_count_type);
  } 
  //}
  //if((ramFlagsTable[asap/2] & transmitting) == transmitting) {
  //  ramFlagsTable[asap/2] &= clear; //clear transmitting flag, set OK
  //} else {
  //  A_GroupValue_Read_Acon(asap, priority, hop_count_type, data);
  //}
}

void A_GroupValue_Read_Rcon(unsigned char ack_request, unsigned char asap, unsigned char priority, unsigned char hop_count_type, unsigned char a_status, unsigned char data[]) {
  unsigned char transmitting = TRANSMITTING;
  unsigned char clear = 0xF0;
  if(!(asap & 0b1)) {
    transmitting = transmitting << 4;
    clear = clear >> 4;
  }
  if((ramFlagsTable[asap/2] & transmitting) == transmitting) {
    ramFlagsTable[asap/2] &= clear; //clear transmitting flag, set OK
  } else {
    A_GroupValue_Read_Acon(asap, priority, hop_count_type, data);
  }
}

void A_GroupValue_Write_Lcon(unsigned char ack_request, unsigned char asap, unsigned char priority, unsigned char hop_count_type, unsigned char a_status, unsigned char data[]) {
  unsigned char transmitting = TRANSMITTING;
  unsigned char clear = 0xF0;
  if(asap & 0b1) {
    transmitting = transmitting << 4;
    clear = clear >> 4;
  }
  if((ramFlagsTable[asap/2] & transmitting) == transmitting) {
    ramFlagsTable[asap/2] &= clear; //clear transmitting flag, set OK
  } else {
    A_GroupValue_Write_ind(asap, priority, hop_count_type, data);



    DEBUGGING("WRite");
  }
}
