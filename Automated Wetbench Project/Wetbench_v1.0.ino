#include <PinChangeInterrupt.h>
#include <PinChangeInterruptBoards.h>
#include <PinChangeInterruptPins.h>
#include <PinChangeInterruptSettings.h>

#define STEP_DUTY_X     200
#define STEP_DUTY_Y     1
#define DEBOUNCE_TIME   90
#define PUMP_DUTY       50

// Pin Assignments
const int Dir_x         = 15;
const int Step_x        = 14;
const int Dir_y         = 4;
const int Step_y        = 5;
const int Gripper_A     = 6;
const int Gripper_B     = 7;
const int test_led      = A0;
const int IR_Tank_A     = 18;
const int IR_Tank_B     = 19;
const int IR_Tank_C     = 20;
const int Micro_sw_down = 3;
const int Micro_sw_up   = 2;
const int Pin_1         = A1;
const int Pump_1        = 8;
const int Pump_2        = 9;
const int Pump_3        = 10;
const int Pump_4        = 11;
const int Pump_5        = 12;
const int Pump_6        = 13;


volatile int flag_0     = 1;
volatile int flag_1     = 0;

int state = 0;

volatile int test_program = 0;

// Horizontal Stepper Motor Functions
void forward_x(){
  digitalWrite(Dir_x, HIGH);
  analogWrite(Step_x, STEP_DUTY_X); 
}

void backward_x(){
  digitalWrite(Dir_x, LOW);
  analogWrite(Step_x, STEP_DUTY_X);
}

void stop_x(){
  digitalWrite(Dir_x, LOW);
  analogWrite(Step_x, 0);
}

// Vertical Stepper Motor Functions
void forward_y(){
  digitalWrite(Dir_y, HIGH);
  analogWrite(Step_y, STEP_DUTY_Y); 
}

void backward_y(){
  digitalWrite(Dir_y, LOW);
  analogWrite(Step_y, STEP_DUTY_Y);
}

void stop_y(){
  digitalWrite(Dir_y, LOW);
  analogWrite(Step_y, 0);
}

void stop_all(){
  analogWrite(Step_y, 0);
  analogWrite(Step_x, 0);
}

void gripper_open(){
  digitalWrite(Gripper_A, HIGH);
  delay(3000);
  digitalWrite(Gripper_A, LOW);
}

void gripper_close(){
  digitalWrite(Gripper_B, HIGH);
  delay(5000);
  digitalWrite(Gripper_B, LOW);
}

// Micro Switch ISR Functions
void Micro_sw_up_isr(){
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > DEBOUNCE_TIME){
    Serial.println("Micro Up");
    if(test_program == 1){
      flag_1 = 1;
      flag_0 = 0;
    }
    if(test_program == 3){
      switch(state){
        case 1:
          state = 2;
          break;
        case 7:
          state = 8;
          break;
      }
    }
    digitalWrite(test_led, HIGH);
  }
  last_interrupt_time = interrupt_time;
}
void Micro_sw_down_isr(){
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > DEBOUNCE_TIME){
    Serial.println("Micro Down");
    if(test_program == 1){
      flag_0 = 1;
      flag_1 = 0;
    }
    if(test_program == 3){
      switch(state){
        case 5:
          state = 6;
          break;
        case 11:
          state = 12;
          break;
      }
    }
    digitalWrite(test_led, LOW);
  }
  last_interrupt_time = interrupt_time;
}

// IR Sensor ISR Functions
void IR_Tank_A_isr(){
  if(test_program == 2){
    flag_0 = 1;
    flag_1 = 0;
    Serial.print("Flag 0: ");
    Serial.println(flag_0);
    Serial.print("Flag 1: ");
    Serial.println(flag_1);
  }
  if(test_program == 3){
    switch(state){
      case 3:
        state = 4;
        break;
    }
  }
}
void IR_Tank_B_isr(){
  
}
void IR_Tank_C_isr(){
  if(test_program == 2){
    flag_1 = 1;
    flag_0 = 0;
    Serial.print("Flag 0: ");
    Serial.println(flag_0);
    Serial.print("Flag 1: ");
    Serial.println(flag_1);
  }
  if(test_program == 3){
    switch(state){
      case 9:
        state = 10;
        break;
    }
  }
}


void Pump_1_open(int _open){
  if(_open==1)
    analogWrite(Pump_1, PUMP_DUTY);
  else
    analogWrite(Pump_1, 0);
}
void Pump_2_open(int _open){
  if(_open==1)
    analogWrite(Pump_2, PUMP_DUTY);
  else
    analogWrite(Pump_2, 0);
}
void Pump_3_open(int _open){
  if(_open==1)
    analogWrite(Pump_3, PUMP_DUTY);
  else
    analogWrite(Pump_3, 0);
}
void Pump_4_open(int _open){
  if(_open==1)
    analogWrite(Pump_4, PUMP_DUTY);
  else
    analogWrite(Pump_4, 0);
}
void Pump_5_open(int _open){
  if(_open==1)
    analogWrite(Pump_5, PUMP_DUTY);
  else
    analogWrite(Pump_5, 0);
}
void Pump_6_open(int _open){
  if(_open==1)
    analogWrite(Pump_6, PUMP_DUTY);
  else
    analogWrite(Pump_6, 0);
}

// Test Programs
void test_program_1(){
  test_program = 1;
  if(flag_0){
    forward_y();
  }
  else if(flag_1){
    backward_y();
  }
}
void test_program_2(){
  test_program = 2;
  if(flag_0){
    forward_x();
  }
  else if(flag_1){
    backward_x();
  }
}

void test_program_3(){
  test_program = 3;
  switch(state){
    case 0:
      gripper_close();
      state = 1;
      break;
    case 1:
      forward_y();
      break;
    case 2:
      stop_all();
      delay(3000);
      state = 3;
    case 3:
      backward_x();
      break;
    case 4:
      stop_all();
      delay(3000);
      state = 5;
      break;
    case 5:
      backward_y();
      break;
    case 6:
      stop_all();
      gripper_open();
      state = 7;
      break;
    case 7:
      forward_y();
      break;
    case 8:
      stop_all();
      delay(3000);
      state = 9;
      break;
    case 9:
      forward_x();
      break;
    case 10:
      stop_all();
      delay(3000);
      state = 11;
      break;
    case 11:
      backward_y();
      break;
    case 12:
      stop_all();
      break;
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(Dir_x,  OUTPUT);
  pinMode(Step_x, OUTPUT);
  pinMode(Dir_y,  OUTPUT);
  pinMode(Step_y, OUTPUT);

  pinMode(test_led, OUTPUT);
  digitalWrite(test_led,LOW);
  
  pinMode(Gripper_A, OUTPUT);
  digitalWrite(Gripper_A, LOW);
  pinMode(Gripper_B, OUTPUT);
  digitalWrite(Gripper_B, LOW);

  attachInterrupt(digitalPinToInterrupt(IR_Tank_A),IR_Tank_A_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(IR_Tank_B),IR_Tank_B_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(IR_Tank_C),IR_Tank_C_isr, RISING);

  attachInterrupt(digitalPinToInterrupt(Micro_sw_up),   Micro_sw_up_isr,    RISING);
  attachInterrupt(digitalPinToInterrupt(Micro_sw_down), Micro_sw_down_isr,  RISING);

  pinMode(Pin_1, INPUT);

  pinMode(Pump_1, OUTPUT);
  pinMode(Pump_2, OUTPUT);
  pinMode(Pump_3, OUTPUT);
  pinMode(Pump_4, OUTPUT);
  pinMode(Pump_5, OUTPUT);
  pinMode(Pump_6, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  //test_program_1();
  //test_program_2();
  //test_program_3();
  //Serial.println(state);
  //forward_y();

  /*Pump_1_open(1);
  Serial.println("Pump 1");
  delay(5000);
  Pump_1_open(0);
  delay(2000);
  
  Pump_2_open(1);
  Serial.println("Pump 2");
  delay(5000);
  Pump_2_open(0);
  delay(2000);
  
  Pump_3_open(1);
  Serial.println("Pump 3");
  delay(5000);
  Pump_3_open(0);
  delay(2000);
  
  Pump_4_open(1);
  Serial.println("Pump 4");
  delay(5000);
  Pump_4_open(0);
  delay(2000);
  
  Pump_5_open(1);
  Serial.println("Pump 5");
  delay(5000);
  Pump_5_open(0);
  delay(2000);
  
  Pump_6_open(1);
  Serial.println("Pump 6");  
  delay(5000);
  Pump_6_open(0);
  delay(2000);*/

  Pump_6_open(1);
}
