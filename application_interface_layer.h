#ifndef APPLICATION_INTERFACE_LAYER_H
#define APPLICATION_INTERFACE_LAYER_H

#include "knx.h"
#include "eeprom.h"
#include "application_layer.h"

#ifndef  EXTERN
#define  EXTERN  extern
#endif

void handleAIL(void);
bool send_read_request(unsigned char asap);
bool send_write_request(unsigned char asap);

EXTERN char ramFlagsTable[(GROUP_OBJECT_NUMBER+1)/2];
void setRamFlags(unsigned char asap, unsigned char flags);
void clearRamFlags(unsigned char asap, unsigned char flags);
bool checkRamFlags(unsigned char asap, unsigned char flags);

void setUpdateFlag(unsigned char asap);
void clearUpdateFlag(unsigned char asap);
void setDataRequestFlag(unsigned char asap);
void clearDataRequestFlag(unsigned char asap);
void setOKFlag(unsigned char asap);
void setErrorFlag(unsigned char asap);
void setTransmitRequestFlag(unsigned char asap);
void setTransmittingFlag(unsigned char asap);

//request from application

//request the current value from the bus
void read_telegram        (unsigned char asap);

//send a new value to the bus
void write_telegram       (unsigned char asap, unsigned char data[]);

//read the current local value from EEPROM
void get_value            (unsigned char asap, unsigned char data[]);

//read parameters from EEPROM
unsigned char get_parameter(unsigned char address); 

//get the next asap witch was updated from the bus
unsigned char checkUpdate(void);

//set the device in the programming mode
void set_programming_mode(void);

//get the programming_mode 
unsigned char get_programming_mode(void);


//indication

void A_GroupValue_Read_ind  (unsigned char asap, unsigned char priority, unsigned char hop_count_type);
void A_GroupValue_Read_Acon (unsigned char asap, unsigned char priority, unsigned char hop_count_type, unsigned char data[]);
void A_GroupValue_Write_ind (unsigned char asap, unsigned char priority, unsigned char hop_count_type, unsigned char data[]);

void A_DeviceDescriptor_Read_ind(unsigned char asap[2], unsigned char priority, unsigned char hop_count_type, unsigned char descriptor_type);
void A_Memory_Read_ind          (unsigned char asap[2], unsigned char priority, unsigned char number, unsigned char memory_address[2]);
void A_Memory_Write_ind         (unsigned char asap[2], unsigned char priority, unsigned char number, unsigned char memory_address[2], unsigned char data[]);

void A_IndividualAddress_Write_ind(unsigned char priority, unsigned char hop_count_type, unsigned char newaddress[2]);

//confirmations

void A_Data_con(unsigned char a_status);
void A_GroupValue_Read_Lcon (unsigned char ack_request, unsigned char asap, unsigned char priority, unsigned char hop_count_type, unsigned char a_status);
void A_GroupValue_Read_Rcon (unsigned char ack_request, unsigned char asap, unsigned char priority, unsigned char hop_count_type, unsigned char a_status, unsigned char data[]);
void A_GroupValue_Write_Lcon(unsigned char ack_request, unsigned char asap, unsigned char priority, unsigned char hop_count_type, unsigned char a_status, unsigned char data[]);

#endif //TRANSPORT_INTERFACE_LAYER_H
