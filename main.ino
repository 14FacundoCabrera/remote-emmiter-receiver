#include "IRremote.hpp"

#define IR_RECEIVE_PIN 14 //D5
#define IR_SENDER_PIN 4 //D2

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);  
  IrSender.begin(IR_SENDER_PIN);
}

void loop() {
  if (IrReceiver.decode()) {
    uint16_t address = IrReceiver.decodedIRData.address;
    uint8_t command = IrReceiver.decodedIRData.command;
    IrReceiver.printIRResultShort(&Serial);
    Serial.println(address);
    Serial.println(command);
    IrSender.sendNEC(address, command, 3);
    IrReceiver.resume();   
  }
}