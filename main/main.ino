#include "IRremote.hpp"

#define IR_RECEIVE_PIN 14 //D5
#define IR_SENDER_PIN 4 //D2
#define SET_IRSIGNALS_PIN 5 //D1
#define BUTTON_1 0 //D3
bool shouldExit = false; // to exit the reasign

int pinButtons[] = { BUTTON_1 }; // todo: add more buttons

struct IrData { // It has the necessary data to send an IR signal
  decode_type_t protocol;
  uint16_t address;
  uint8_t command;
};

struct IrDataButton { // The struct that holds the IrData, and assigns the data to a button pin
  int buttonPin;
  IrData* data;
  int length;
};

IrData button1Signals[] = {
  {NEC, 0x820, 0x1C},
  {NEC, 0x820, 0x45},
  {NEC, 0x820, 0x45}
};

IrDataButton buttons[] = {
  {BUTTON_1, button1Signals, 3}
};

// IrData IrSignals[] = {
//   {0x820, 0x1C},
//   {0x820, 0x1C},
//   {0x820, 0x45},
//   {0x820, 0x45}
// };

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);  
  IrSender.begin(IR_SENDER_PIN);
  pinMode(SET_IRSIGNALS_PIN, INPUT_PULLUP);
}

void setIrSignals(){
  uint16_t address = IrReceiver.decodedIRData.address;
  uint8_t command = IrReceiver.decodedIRData.command;
  auto protocol = IrReceiver.decodedIRData.protocol;
  if (protocol != UNKNOWN){
    delay(50);
    IrReceiver.printIRResultShort(&Serial);
    Serial.println(address);
    Serial.println(command);
  }
}

void sendIrSignals(IrData irData){
  switch(irData.protocol){
    case NEC:
      IrSender.sendNEC(irData.address, irData.command, 0);
      break;
    case KASEIKYO_DENON:
      IrSender.sendKaseikyo_Denon(irData.address, irData.command, 0);
      break;
  }
}

void buttonRegisterListener(int pin){
  if(digitalRead(pin) == LOW){ // to assign button pressed
  // todo: delete already existing button pin data if exists
    while(!shouldExit){
      if (digitalRead(SET_IRSIGNALS_PIN) == LOW) { // Exit if pressed the main button
        shouldExit = true;
      } else {
        if(IrReceiver.decode()){
          setIrSignals(); // todo: Set an IrData to the IrDataButton
          IrReceiver.resume();
        }
      }
    }
  }
}

void buttonSenderListener(int pin){
  if (digitalRead(pin) == LOW){
    for(IrDataButton button : buttons){
      if(button.buttonPin == pin){
        for(int i =0; i < button.length; i++){
          sendIrSignals(button.data[i]);
          delay(100); // todo: increase or decrease delay || maybe include a global delay var
        }
      }
    }
  }
}

void loop() {
  if (digitalRead(SET_IRSIGNALS_PIN) == LOW){ // If main button is pressed
    shouldExit = false; // reasurring the shouldExit is false
    while(!shouldExit){
      // listens to all the buttons 
      for(int pin : pinButtons){
        buttonRegisterListener(pin);
      } 
      if(digitalRead(SET_IRSIGNALS_PIN) == LOW){ // exit main loop when pressing the main button
        shouldExit = true;
        Serial.println("Exiting while on loop");
      }
    }
  }
   for(int pin : pinButtons){
    buttonSenderListener(pin);
  } 
  // if (IrReceiver.decode()) {
  //   uint16_t address = IrReceiver.decodedIRData.address;
  //   uint8_t command = IrReceiver.decodedIRData.command;
  //   IrReceiver.printIRResultShort(&Serial);
  //   Serial.println(address);
  //   Serial.println(command);
  //   IrSender.sendNEC(address, command, 3);
  //   // for (int i = 0; i < 4; i++){
  //   //   IrSender.sendNEC(IrSignals[i].address, IrSignals[i].command, 3);
  //   //   delay(500); // todo: consider lower cooldown
  //   // }
  //   IrReceiver.resume();   
  //}
}
// Power Off A: 0x820, C:0x0
// One: A:0x820, C:0x1C
// Eight: 0x820, C:0x45
// Vol. up: C: 0x2
// Vol. down: C:0x3