#include <jebecef.h>

#include <SoftwareSerial.h>

//communication objects
#define LOGIC_MODULE_1      0
#define LOGIC_MODULE_1_END  (LOGIC_MODULE_1 + OBJ_PER_LOGIC_MODULE)

#define LOGIC_MODULE_2      LOGIC_MODULE_1_END
#define LOGIC_MODULE_2_END  (LOGIC_MODULE_2 + OBJ_PER_LOGIC_MODULE)

#define LOGIC_MODULE_3      LOGIC_MODULE_2_END
#define LOGIC_MODULE_3_END  (LOGIC_MODULE_3 + OBJ_PER_LOGIC_MODULE)

#define LOGIC_MODULE_4      LOGIC_MODULE_3_END
#define LOGIC_MODULE_4_END  (LOGIC_MODULE_4 + OBJ_PER_LOGIC_MODULE)

//communication objects for logic module
#define LOGIC_MODULE_INPUT_1  0
#define LOGIC_MODULE_INPUT_2  1
#define LOGIC_MODULE_INPUT_3  2
#define LOGIC_MODULE_INPUT_4  3
#define LOGIC_MODULE_OUTPUT   4

#define OBJ_PER_LOGIC_MODULE 5

//parameter
#define PARAM_LOGIC_MODULE_1          0
#define PARAM_LOGIC_MODULE_2          (PARAM_LOGIC_MODULE_1 + PARAM_PER_LOGIC_MODULE)
#define PARAM_LOGIC_MODULE_3          (PARAM_LOGIC_MODULE_2 + PARAM_PER_LOGIC_MODULE)
#define PARAM_LOGIC_MODULE_4          (PARAM_LOGIC_MODULE_3 + PARAM_PER_LOGIC_MODULE)

//parameters for logic module
#define PARAM_LOGIC_MODULE_TYPE     0
#define PARAM_LOGIC_MODULE_ENABLED  1

#define PARAM_PER_LOGIC_MODULE 2


enum module_type{AND, OR, XOR};



SoftwareSerial SoftSerial(11, 10); //RX, TX


#ifdef DEBUG
void data_ind(const char c[]) {
  SoftSerial.print(c);
}

void setCB() {
  setDebugCallback(data_ind);
}
#endif


void setup() {
  init_jebecef();
  #ifdef DEBUG
  setCB();
  #endif
  read_telegram(0);
  
  SoftSerial.begin(19200);

  pinMode(2, INPUT); //Progbutton
  pinMode(8, OUTPUT);//ProgLED
  digitalWrite(8, LOW); 
  //debug("test");
  DEBUGGING("start");
}

void logicModule(unsigned char first_com_obj, unsigned char first_param, unsigned char com_obj) {
  unsigned char module_param = get_parameter(first_param + PARAM_LOGIC_MODULE_TYPE);
  unsigned char module_type = module_param & 0b11;
  unsigned char module_enabled = get_parameter(first_param + PARAM_LOGIC_MODULE_ENABLED);
  if((1 << (com_obj)) & module_enabled) {
    unsigned char values[4];
    get_value(LOGIC_MODULE_INPUT_1 + first_com_obj, &values[0]);
    get_value(LOGIC_MODULE_INPUT_2 + first_com_obj, &values[1]);
    get_value(LOGIC_MODULE_INPUT_3 + first_com_obj, &values[2]);
    get_value(LOGIC_MODULE_INPUT_4 + first_com_obj, &values[3]);
    unsigned char output;
    if(module_type == AND) {
      output = 1;
      for(unsigned char i = 0; i < 4; i++) {
        if(module_enabled & (1 << i)) {
          output &= values[i];
        }
      }
      
    } else if(module_type == OR) {
      output = 0;
      for(unsigned char i = 0; i < 4; i++) {
        if(module_enabled & (1 << i)) {
          output |= values[i];
        }
      }
      
    } else if(module_type == XOR) {
      output = 0;
      for(unsigned char i = 0; i < 4; i++) {
        if(module_enabled & (1 << i)) {
          if(output && values[i]) {
            output = 0;
            break;
          } else {
            output ^= values[i];
          }
        }
      }
    }
    write_telegram(LOGIC_MODULE_OUTPUT + first_com_obj, &output);
  } 
}

void loop() {
  handle_jebecef();
  if(digitalRead(2) == LOW) {
    set_programming_mode();
    digitalWrite(8, HIGH);
  }

  if(!get_programming_mode()) {
    digitalWrite(8, LOW);
  }


  unsigned char co = checkUpdate();
  if(co != 0xFF) { // co = communication object
    if(co < LOGIC_MODULE_1_END) {
      logicModule(LOGIC_MODULE_1, PARAM_LOGIC_MODULE_1, co - LOGIC_MODULE_1);
      
    } else if(co < LOGIC_MODULE_2_END) {
      logicModule(LOGIC_MODULE_2, PARAM_LOGIC_MODULE_2, co - LOGIC_MODULE_2);
      
    } else if(co < LOGIC_MODULE_3_END) {
      logicModule(LOGIC_MODULE_3, PARAM_LOGIC_MODULE_3, co - LOGIC_MODULE_3);
      
    } else if(co < LOGIC_MODULE_4_END) {
      logicModule(LOGIC_MODULE_4, PARAM_LOGIC_MODULE_4, co - LOGIC_MODULE_4);
    }
  }
}
