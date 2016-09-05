#include "transport_layer.h"
#include <string.h>

//variables
unsigned char tx_ongoing = false;

union address connection_address;
unsigned char seqNoSend;
unsigned char seqNoRcv;
unsigned char timeout_enabled;
unsigned char connection_timeout_timer;
unsigned char acknowledgment_timeout_timer;
unsigned char rep_count;
unsigned char t_state = T_CLOSED; //state of the transport layer state machine


struct Data_Connected {
  unsigned char waiting;
  unsigned char octet_count;//octet_count == 0 => Data_Connected is empty
  unsigned char priority;
  unsigned char tsap[2];
  unsigned char tsdu[17];
};

Data_Connected data_connected;

void N_Data_Individual_wait(unsigned char tsap[2], unsigned char octet_count, unsigned char priority, unsigned char tsdu[]) {
  data_connected.octet_count = octet_count;
  data_connected.priority = priority;
  memcpy(data_connected.tsdu, tsdu, octet_count + 2);
  memcpy(data_connected.tsap, tsap, 2);
  data_connected.waiting = 1;
}

//actions

void action6(unsigned char source_address[2]) {
  unsigned char nsdu2[2] = {0x00, T_DISCONNECT_REQ_PDU};
  unsigned char octet_count = 0;
  if(!tx_ongoing) {
    N_Data_Individual_req(true, connection_address.address, N_HOP_COUNT_TYPE, octet_count, PRIO_SYSTEM, nsdu2);
  } else {
    N_Data_Individual_wait(connection_address.address, octet_count, PRIO_SYSTEM, nsdu2);
  }
  T_Disconnect_ind(source_address);
  timeout_enabled = 0; //stop the acknowledge and connection timeout timer
}

void action10(unsigned char source_address[2]) {
  unsigned char nsdu2[2] = {0x00, T_DISCONNECT_REQ_PDU};
  unsigned char octet_count = 0;
  if(!tx_ongoing) {
    N_Data_Individual_req(true, source_address, N_HOP_COUNT_TYPE, octet_count, PRIO_SYSTEM, nsdu2);
  } else {
    N_Data_Individual_wait(source_address, octet_count, PRIO_SYSTEM, nsdu2);
  }
}


void check_transport_layer(void) {
  if(data_connected.waiting && !tx_ongoing) {
    unsigned char tsdu[data_connected.octet_count + 2];
    memcpy(tsdu, data_connected.tsdu, data_connected.octet_count + 2);
    unsigned char tsap[2];
    memcpy(tsap, data_connected.tsap, 2);
    data_connected.waiting = 0;
    N_Data_Individual_req(true, tsap, N_HOP_COUNT_TYPE, data_connected.octet_count, data_connected.priority, tsdu);
  }

  //TODO timeout_check;
  if(timeout_enabled & CONNECTION_TIMEOUT_TIMER) {
    if((unsigned char)(get_knx_timer_ticks() - KNX_TIMEOUT_6S) == connection_timeout_timer) {
      //E16
      if(T_CLOSED == t_state) {
        //A0
      } else {
        //A6
        action6(connection_address.address);
      }
      t_state = T_CLOSED;
    }
  }
}


//services

bool T_Data_Group_req(bool ack_request, unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[], unsigned char tsap[2]) {
  tsdu[1] |= T_DATA_GROUP_REQ_PDU;
  return N_Data_Group_req(ack_request, tsap, hop_count_type, octet_count, priority, tsdu);
}

bool T_Data_Tag_Group_req(bool ack_request, unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[], unsigned char destination_address[2], unsigned char frame_format) {
  tsdu[1] |= T_DATA_TAG_GROUP_REQ_PDU & TPDU_MASK;
  //TODO frame_format
  return N_Data_Group_req(ack_request, destination_address, hop_count_type, octet_count, priority, tsdu);
}

bool T_Data_Broadcast_req(bool ack_request, unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[]) {
  tsdu[1] |= T_DATA_BROADCAST_REQ_PDU;
  return N_Data_Broadcast_req(ack_request, hop_count_type, octet_count, priority, tsdu);
}

bool T_Data_SystemBroadcast_req(bool ack_request, unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[]) {

}

bool T_Data_Individual_req(bool ack_request, unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char tsdu[], unsigned char tsap[2]) {
  tsdu[1] |= T_DATA_INDIVIDUAL_REQ_PDU;
  return N_Data_Individual_req(ack_request, tsap, hop_count_type, octet_count, priority, tsdu);
}

bool T_Connect_req(unsigned char priority, unsigned char destination_address[2]) {
  unsigned char tsdu[] = {0x00, T_CONNECT_REQ_PDU};
  return N_Data_Individual_req(true, destination_address, N_HOP_COUNT_TYPE, 6, PRIO_SYSTEM, tsdu);
}

bool T_Disconnect_req(unsigned char priority, unsigned char tsap[2]) {
  unsigned char tsdu[] = {0x00, T_DISCONNECT_REQ_PDU};
  return N_Data_Individual_req(true, tsap, N_HOP_COUNT_TYPE, 6, PRIO_SYSTEM, tsdu);

}

bool T_Data_Connected_req(unsigned char priority, unsigned char tsap[2], unsigned char octet_count, unsigned char tsdu[]) {
  //E15
  if(T_CLOSED == t_state) {
    //A5
    T_Disconnect_ind(tsap);
    timeout_enabled = 0; //stop the acknowledge and connection timeout timer
  } else if(T_OPEN_IDLE == t_state) {
    t_state = T_OPEN_WAIT;
    //A7
    tsdu[1] |= T_DATA_CONNECTED_REQ_PDU | (seqNoSend << 2);
    N_Data_Individual_wait(tsap, octet_count, priority, tsdu);
    if(!tx_ongoing) {
      N_Data_Individual_req(true, connection_address.address, N_HOP_COUNT_TYPE, octet_count, priority, tsdu);
      data_connected.waiting = 0; //clear, because set above
    } else {
      //already stored
    }
    rep_count = 0;
    connection_timeout_timer = get_knx_timer_ticks(); //restart the connection timeout timer
    acknowledgment_timeout_timer = get_knx_timer_ticks();
    timeout_enabled |= ACKNOWLEDGMENT_TIMEOUT_TIMER; //start the acknowledge timeout timer
    return true;
  } else {
    //A11 //TODO
    return false;
  }
}

//indication

 
void N_Data_Individual_ind(unsigned char destination_address[2], unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char source_address[2], unsigned char nsdu[]) {
  unsigned char seqNoPdu = (lsdu[1] & SEQ_NO_MASK) >> 2;
  if(T_CONNECT_REQ_PDU == lsdu[1]) {
    if((connection_address.high == source_address[ADDRHIGH])
        && (connection_address.low == source_address[ADDRLOW])) {
    //E00
      if(T_CLOSED == t_state) {
        t_state = T_OPEN_IDLE;
        //A1
        connection_address.high = source_address[ADDRHIGH];
        connection_address.low  = source_address[ADDRLOW];
        T_Connect_ind(source_address);
        seqNoSend = 0;
        seqNoRcv  = 0;
        connection_timeout_timer = get_knx_timer_ticks(); //start the connection timeout timer
        timeout_enabled |= CONNECTION_TIMEOUT_TIMER;
      } else {
        t_state = T_CLOSED;
        //A6
        action6(source_address);
      }
    } else {
    //E01
      if(T_CLOSED == t_state) {
        t_state = T_OPEN_IDLE;
        //A1
        connection_address.high = source_address[ADDRHIGH];
        connection_address.low  = source_address[ADDRLOW];
        T_Connect_ind(source_address);
        seqNoSend = 0;
        seqNoRcv  = 0;
        connection_timeout_timer = get_knx_timer_ticks(); //start the connection timeout timer
        timeout_enabled |= CONNECTION_TIMEOUT_TIMER;
      } else {
        //A10
        action10(source_address);
      }
    }

  } else if(T_DISCONNECT_REQ_PDU == lsdu[1]) {
    if(connection_address.high == source_address[ADDRHIGH]
        && connection_address.low == source_address[ADDRLOW]) {
    //E02
      if(T_CLOSED == t_state) {
        //A0
      } else {
        //A5
        T_Disconnect_ind(source_address);
        timeout_enabled = 0; //stop the acknowledge and connection timeout timer
      }
      t_state = T_CLOSED;
    } else {
    //E03
      //A0
    }

  } else if(T_DATA_CONNECTED_REQ_PDU == (lsdu[1] & T_DATA_CONNECTED_REQ_PDU_MASK)) {
    DEBUGGING("TDC");
    if(connection_address.high == source_address[ADDRHIGH]
        && connection_address.low == source_address[ADDRLOW]) {
      if(seqNoPdu == seqNoRcv) {
      //E04
        if(T_CLOSED == t_state) {
          //A10
          action10(source_address);
        } else {
          //A2
          unsigned char nsdu2[2] = {0x00, (unsigned char) (T_ACK_PDU | (seqNoRcv << 2))};
          unsigned char octet_count = 0;
          if(!tx_ongoing) {
            N_Data_Individual_req(true, connection_address.address, N_HOP_COUNT_TYPE, octet_count, PRIO_SYSTEM, nsdu2);
          } else {
            N_Data_Individual_wait(connection_address.address, octet_count, PRIO_SYSTEM, nsdu2);
          }
          seqNoRcv = (++seqNoRcv & SEQ_NO_MASK2);
          T_Data_Connected_ind(source_address, octet_count, priority, nsdu);
          connection_timeout_timer = get_knx_timer_ticks(); //restart the connection timeout timer
        }
      } else if(seqNoPdu == ((seqNoRcv - 1) & 0x0F)) {
      //E05
        if(T_CLOSED == t_state) {
          //A10
          action10(source_address);
        } else {
          //A3
          unsigned char nsdu2[2] = {0x00, (unsigned char) (T_ACK_PDU |(seqNoPdu << 2))};
          unsigned char octet_count = 0;
          if(!tx_ongoing) {
            N_Data_Individual_req(true, connection_address.address, N_HOP_COUNT_TYPE, octet_count, PRIO_SYSTEM, nsdu2);
          } else {
            N_Data_Individual_wait(connection_address.address, octet_count, PRIO_SYSTEM, nsdu2);
          }
          connection_timeout_timer = get_knx_timer_ticks(); //restart the connection timeout timer
        }
      } else {
      //E06
        if(T_CLOSED == t_state) {
          //A10
          action10(source_address);
        } else {
          //A6
          action6(source_address);
        }
        t_state = T_CLOSED;
      }
    } else {
    //E07
      //A10
      action10(source_address);
    }

  } else if(T_ACK_PDU == (lsdu[1] & T_ACK_PDU_MASK)) {
    if(connection_address.high == source_address[ADDRHIGH]
        && connection_address.low == source_address[ADDRLOW]) {
      if(seqNoPdu == seqNoSend) {
      //E08
        if(T_CLOSED == t_state) {
          //A10
          action10(source_address);
        } else if(T_OPEN_IDLE == t_state) {
          t_state = T_CLOSED;
          //A6
          action6(source_address);
        } else {
          t_state = T_OPEN_IDLE;
          //A8
          timeout_enabled &= ~ACKNOWLEDGMENT_TIMEOUT_TIMER; //stop the acknowledge timeout timer
          seqNoSend = (++seqNoSend & SEQ_NO_MASK2);
          T_Data_Connected_con(data_connected.tsap);
          connection_timeout_timer = get_knx_timer_ticks(); //restart the connection timeout timer
        }
        
      } else {
      //E09
        if(T_CLOSED == t_state) {
          //A10
          action10(source_address);
        } else {
          //A6
          action6(source_address);
        }
        t_state = T_CLOSED;
      }
    } else {
    //E10
      //A10 
      action10(source_address);
    }

  } else if(T_NAK_PDU == (lsdu[1] & T_NAK_PDU_MASK)) {
    if(connection_address.high == source_address[ADDRHIGH]
        && connection_address.low == source_address[ADDRLOW]) {
    //E11b
      if(T_CLOSED == t_state) {
        //A10
        action10(source_address);
      } else {
        //A6
        action6(source_address);
      }
      t_state = T_CLOSED;
      
    } else {
    //E14
      //A10
      action10(source_address);
    }

  } else if(T_DATA_INDIVIDUAL_REQ_PDU == (lsdu[1] & TPDU_MASK)) {
    T_Data_Individual_ind(hop_count_type, octet_count, priority, nsdu, source_address); 
  }
}

unsigned char get_tsap_from_dest_addr(unsigned char destination_address[2]) {
  //TODO optimize search algorythm
  GroupAddressTable grat;
  EEPROM.get(GRAT_ADDR, grat);
  for(unsigned char i = 1; i < grat.length; i++) { // i=0 is individual address
    if(grat.addresses[i][ADDRHIGH] == destination_address[0]
        && grat.addresses[i][ADDRLOW] == destination_address[1]) {
      return i-1;
    }
  }
  return 0xFF;
}

void N_Data_Group_ind(unsigned char destination_address[2], unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char nsdu[]) {
  if((nsdu[1] & TPDU_MASK) == T_DATA_GROUP_REQ_PDU)  {
    unsigned char tsap = get_tsap_from_dest_addr(destination_address);
    if(0xFF == tsap) {
      return;
    }
    T_Data_Group_ind(hop_count_type, octet_count, priority, nsdu, tsap); 
  }
}

void N_Data_Broadcast_ind (unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char source_address[2], unsigned char nsdu[]) {
  T_Data_Broadcast_ind(hop_count_type, octet_count, priority, nsdu, source_address);
}

//confirmations

void N_Data_Group_con(unsigned char ack_request, unsigned char destination_address[2], unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char nsdu[], unsigned char n_status) {
  unsigned char tsap = get_tsap_from_dest_addr(destination_address);
  T_Data_Group_con(ack_request, hop_count_type, octet_count, priority, tsap, nsdu, n_status);
}

void N_Data_Individual_con (unsigned char ack_request, unsigned char destination_address[2], unsigned char hop_count_type, unsigned char octet_count, unsigned char priority, unsigned char nsdu[], unsigned char n_status) {
  if(T_DISCONNECT_REQ_PDU == lsdu[1]) {
  //E21
    //A0
  } else if(T_DATA_CONNECTED_REQ_PDU == lsdu[1]) {
  //E22
    //A0
  } else if(T_ACK_PDU == lsdu[1]) {
  //E23
    //A0
  } else if(T_NAK_PDU == lsdu[1]) {
  //E24
  }
}
