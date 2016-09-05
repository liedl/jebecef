#ifndef KNX_TRANSCEIVER_H
#define KNX_TRANSCEIVER_H

#define STANDARD_FRAME_MASK     0b11010011
#define STANDARD_FRAME_PATTERN  0b10010000
#define EXTENDED_FRAME_MASK     STANDARD_FRAME_MASK
#define EXTENDED_FRAME_PATTERN  0b00010000
#define POLL_FRAME_PATTERN      0b11110000
#define ACKN_MASK               0b00110011
#define ACKN_PATTERN            0b00000000
#define CONFIGURE_MASK          0b10000011
#define CONFIGURE_PATTERN       0b00000001
#define STATE_MASK              0b00000111
#define STATE_PATTERN           0b00000111
#define FRAMESTATE_MASK         0b00010111
#define FRAMESTATE_PATTERN      0b00010011
#define DATA_CON_MASK           0b01111111
#define DATA_CON_PATTERN        0b00001011
#define RESET_PATTERN           0b00000011
#define STOPMODE_PATTERN        0b00101011
#define SYSTEMSTATE_PATTERN     0b01001011
#define FRAMEEND_PATTERN        0b11001011



#endif // KNX_TRANSCEIVER_H
