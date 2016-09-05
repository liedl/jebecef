#ifndef KNX_H
#define KNX_H

#define DEBUG

#ifdef DEBUG

#define DEBUGGING debug

#else

#define DEBUGGING(data) ((void)0)

#endif //DEBUG

//ramFlags
#define UPDATE                  0b1000
#define DATA_REQUEST            0b0100
#define TRANSMISSION_OK_C       0b0011 //this bit must cleared 
#define TRANSMISSION_ERROR_S    0b0001 //this bit must set
#define TRANSMISSION_ERROR_C    0b0010 //this bit must cleared
#define TRANSMITTING_S          0b0010
#define TRANSMITTING            0b0010
#define TRANSMITTING_C          0b0001
#define TRANSMIT_REQUEST_S      0b0011

//ramFlagMasks
#define UPDATE_MASK             0b10000000
#define DATA_REQUEST_MASK       0b01000000
#define TRANSMISSION_MASK       0b00110000

//address type
#define INDIVIDUAL 0
#define MULTICAST 1

#define GROUP_ADDR 1
#define INDIVIDUAL_ADDR 0

#define BROADCAST_ADDR 0


//priorities
#define PRIO_SYSTEM 0b00
#define PRIO_URGENT 0b10
#define PRIO_NORMAL 0b01
#define PRIO_LOW    0b11
#define PRIO_MASK   0b00001100  //mask for controlfield

//frame format
#define STANDARD_FRAME 0
#define EXTENDED_FRAME 1

//masks
#define LENGTH_MASK 0x0F


//statuses
#define OK     1
#define NOT_OK 0

//transport control field

#define TPDU_MASK                       0b11111100
#define T_DATA_BROADCAST_REQ_PDU        0x00
#define T_DATA_GROUP_REQ_PDU            0x00
#define T_DATA_TAG_GROUP_REQ_PDU        0x04
#define T_DATA_INDIVIDUAL_REQ_PDU       0x00
#define T_DATA_CONNECTED_REQ_PDU        0x40
#define T_DATA_CONNECTED_REQ_PDU_MASK   0b11000000
#define T_CONNECT_REQ_PDU               0x80
#define T_DISCONNECT_REQ_PDU            0x81
#define T_ACK_PDU                       0xC2
#define T_ACK_PDU_MASK                  0b11000011
#define T_NAK_PDU                       0xC3
#define T_NAK_PDU_MASK                  0b11000011
#define SEQ_NO_MASK                     0b00111100
#define SEQ_NO_MASK2                    0b00001111

//transport layer state machine states

#define T_CLOSED    0
#define T_OPEN_IDLE 1
#define T_OPEN_WAIT 2

//application control field

#define APDU_MASK_H                 0b00000011
#define APDU_MASK_L_SHORT           0b11000000
#define APDU_MASK_L_LONG            0b11111111
#define DATA_MASK                   0b00111111
#define DESCRIPTOR_TYPE_MASK        0b00111111
//multicast communication
#define A_GROUPVALUE_READ_PDU_H     0x00
#define A_GROUPVALUE_READ_PDU_L     0x00
#define A_GROUPVALUE_RESPONSE_PDU_H 0x00
#define A_GROUPVALUE_RESPONSE_PDU_L 0x40
#define A_GROUPVALUE_WRITE_PDU_H    0x00
#define A_GROUPVALUE_WRITE_PDU_L    0x80
//broadcast communication
#define A_INDIVIDUAL_ADDRESS_WRITE_PDU_H 0x00
#define A_INDIVIDUAL_ADDRESS_WRITE_PDU_L 0xC0
//point-to-point connectionless communication
#define A_DEVICEDESCRIPTOR_READ_PDU_H     0x03
#define A_DEVICEDESCRIPTOR_READ_PDU_L     0x00
#define A_DEVICEDESCRIPTOR_RESPONSE_PDU_H 0x03
#define A_DEVICEDESCRIPTOR_RESPONSE_PDU_L 0x01
//point-to-point connection-oriented communication
#define NUMBER_MASK             0b00111111
#define A_MEMORY_READ_PDU_H     0x02
#define A_MEMORY_READ_PDU_L     0x00
#define A_MEMORY_RESPONSE_PDU_H 0x02
#define A_MEMORY_RESPONSE_PDU_L 0x40
#define A_MEMORY_WRITE_PDU_H    0x02
#define A_MEMORY_WRITE_PDU_L    0x80

//network_layer parameter
#define N_HOP_COUNT_TYPE  6

//union address {
//  short address;
//  struct {char high, low;};
//};
//
//union addresses {
//  struct {union address src_address, dest_address;};
//  char array[4];
//};

//GroupAddressTable

#define GROUP_ADDRESSES_NUMBER 50
#define GRAT_ADDR 0   //start address of GroupAddressTable in EEPROM
#define GRAT_INDIVIDUALADDR_ADDR 1
struct GroupAddressTable {
  unsigned char length;
  union {
    struct {
      unsigned char individualAddr[2]; 
      unsigned char groupAddresses[GROUP_ADDRESSES_NUMBER][2];
    };
    unsigned char addresses[GROUP_ADDRESSES_NUMBER + 1][2];
  };
};

//GroupObjectAssociationTable

#define ASSOCIATION_NUMBER 30
#define GROAT_ADDR (GRAT_ADDR + 2 * GROUP_ADDRESSES_NUMBER + 3) //start address of GroupObjectAssociationTable in EEPROM

//indexes for GrOAT
#define TSAP 0
#define ASAP 1

struct GroupObjectAssociationTable {
  unsigned char size;
  unsigned char association[ASSOCIATION_NUMBER][2];
};

//GroupObjectTable

#define GROUP_OBJECT_NUMBER 20
#define GROT_ADDR (GROAT_ADDR + 2 * ASSOCIATION_NUMBER + 1) //start address of GroupObjectTable in EEPROM
#define OBJECT_DATA_ADDR 0x100 //start address of object data

#define PARAMETER_ADDR 0x200 //start address of parameters

//indexes for GrOT
#define DATA_POINTER 0
#define CONFIG_OCTET 1
#define TYPE_OCTET   2

//config octet masks

#define FLAG_PRIO_MASK  0b00000011
#define FLAG_C     0b00000100
#define FLAG_R     0b00001000
#define FLAG_W     0b00010000
#define MEMORY_SEG 0b00100000
#define FLAG_T     0b01000000
#define FLAG_U     0b10000000



struct GroupObjectTable {
  unsigned char size;
  unsigned char ramFlagsTablePointer;
  unsigned char groupObjectDescriptor[GROUP_OBJECT_NUMBER][3];
};

//type_octet length
#define BIT1    0
#define BIT2    1
#define BIT3    2
#define BIT4    3
#define BIT5    4
#define BIT6    5
#define BIT7    6
#define OCTET1  7
#define OCTET2  8
#define OCTET3  9
#define OCTET4  10
#define OCTET6  11
#define OCTET8  12
#define OCTET10 13
#define OCTET14 14 

void init_knx(void);

#define NOT_REPEAT        5

//Address high/low Byte
#define ADDRHIGH          0
#define ADDRLOW           1

#define FRAMEEND      0xCB

#define NOT_ADDRESSED 0b00010000
#define ADDRESSED     0b00010001
#define BUSY          0b00010010
#define NACK          0b00010100

//Services from KNX-Controller
#define RESET         0x01
#define STATE         0x02
#define SETBUSY       0x03
#define QUITBUSY      0x04
#define BUSMON        0x05
#define SETADDRESS    0xF1
#define SETREPITION   0xF2
#define L_DATA_OFFSET 0x08
#define SYSTEMSTATE   0x0D
#define STOPMODE      0x0E
#define EXITSTOPMODE  0x0F
#define ACKN          0x10
#define CONFIGURE     0x18
#define INTREGWR      0x28
#define INTREGRD      0x38
#define POLLINGSTATE  0xE0
#define L_DATASTART   0x80
#define L_DATACON     0x80
#define L_DATAEND     0x40

//Configure Flags
#define END_MARKER    0
#define CRC_CITT      1
#define AUTO_POLLING  2


#define CALC_GROUP_ADDR(X, Y, Z) ((X*8 + Y)*256 + Z)


//addresses
union address {
  unsigned char address[2];
  struct {unsigned char high, low;};
};

union addresses {
  struct {unsigned short src_addr1, dest_addr1;};
  struct {unsigned char src_addr[2], dest_addr[2];};
  struct {unsigned char src_addr_high, src_addr_low, dest_addr_high, dest_addr_low;};
  unsigned char array[4];
};

#ifdef DEBUG
extern void (*debug)(const char[]);
void setDebugCallback(void (*cb)(const char[]));
#endif


#endif //KNX_H
