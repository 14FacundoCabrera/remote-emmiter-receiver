#include "IRremote.hpp"
#include <vector>

#define IR_RECEIVE_PIN 14 //D5
#define IR_SENDER_PIN 4 //D2
#define SET_IRSIGNALS_PIN 5 //D1
#define BUTTON_1 0 //D3
#define GLOBAL_DELAY 125
bool setIrSignalsLastState = HIGH;
bool shouldSendSignals = true;
bool shouldExit = false; // to exit the reassign

int pinButtons[] = { BUTTON_1 }; // todo: add more buttons

struct IrData { // It has the necessary data to send an IR signal
  decode_type_t protocol;
  uint16_t address;
  uint8_t command;
};

struct IrDataButton { // The struct that holds the IrData, and assigns the data to a button pin
  int buttonPin;
  std::vector<IrData> data;
};

IrData button1Signals[] = {
  {NEC, 0x820, 0x1C},
  {NEC, 0x820, 0x45},
  {NEC, 0x820, 0x45}
};

std::vector<IrDataButton> buttons;
// IrDataButton buttons[] = {
//   {BUTTON_1, button1Signals, 3}
// };

// IrData IrSignals[] = {
//   {0x820, 0x1C},
//   {0x820, 0x1C},
//   {0x820, 0x45},
//   {0x820, 0x45}
// };

void buttonsSetup(){
  for(int pin:pinButtons){
    buttons.push_back({pin, {}});
  }
}

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);  
  IrSender.begin(IR_SENDER_PIN);
  pinMode(SET_IRSIGNALS_PIN, INPUT_PULLUP);
  buttonsSetup();
}

void setIrSignals(int pin){
  uint16_t address = IrReceiver.decodedIRData.address;
  uint8_t command = IrReceiver.decodedIRData.command;
  auto protocol = IrReceiver.decodedIRData.protocol;
  if (protocol != UNKNOWN){
    delay(50);
    IrReceiver.printIRResultShort(&Serial);
    Serial.print("Address: ");
    Serial.println(address);
    Serial.print("Command: ");
    Serial.println(command);
    for(int i = 0; i < buttons.size(); i++){
      if(pin == buttons[i].buttonPin){
        buttons[i].data.push_back({protocol, address, command});
        break;
      }
    }
  }
}

void sendIrSignals(IrData irData){
  Serial.println("Data: ---------");
  Serial.print("Address: ");
  Serial.println(irData.address);
  Serial.print("Command: ");
  Serial.println(irData.command);
  switch(irData.protocol){ // case on IrData protocol
    case NEC:
      IrSender.sendNEC(irData.address, irData.command, 0);
      break;
    case KASEIKYO_DENON:
      IrSender.sendKaseikyo_Denon(irData.address, irData.command, 0);
      break;
  }
}

void deleteButtonData(int pin){
  for(int i = 0; i < buttons.size(); i++){
    if(pin == buttons[i].buttonPin){
      buttons[i].data.clear(); // access the button data, then clears
      Serial.println("Data deleted");
      break;
    }
  }
}

void buttonRegisterListener(int pin){
  if(digitalRead(pin) == LOW){ // to assign button pressed
    delay(GLOBAL_DELAY);
    Serial.print("Entering button listener of pin: ");
    Serial.println(pin);
    deleteButtonData(pin);
    while (IrReceiver.decode()) {
      IrReceiver.resume();
    }
    while(!shouldExit){
      yield();
      if (digitalRead(SET_IRSIGNALS_PIN) == LOW) { // Exit if pressed the main button
        Serial.print("Exiting button listener of pin: ");
        Serial.println(pin);
        delay(GLOBAL_DELAY);
        shouldExit = true;
        shouldSendSignals = true;
      } else {
        if(IrReceiver.decode()){
          setIrSignals(pin);
          IrReceiver.resume(); // resume IrReceiver to listen
        }
      }
    }
  }
}

void buttonSenderListener(int pin){
  if (digitalRead(pin) == LOW){
    if(shouldSendSignals){
      Serial.print("Sending data from pin: ");
      Serial.println(pin);
      delay(GLOBAL_DELAY);
      for(IrDataButton &button : buttons){
        if(button.buttonPin == pin){
          for(int i =0; i < button.data.size(); i++){
            sendIrSignals(button.data[i]);
            delay(GLOBAL_DELAY); // todo: increase or decrease delay || maybe include a global delay var
          }
        }
      }
      Serial.println("Done sending data");
    }
  }
}

void loop() {
  if (digitalRead(SET_IRSIGNALS_PIN) == LOW){ // If main button is pressed
    delay(GLOBAL_DELAY);
    setIrSignalsLastState = LOW;
    shouldSendSignals = false;
    shouldExit = false; // reasurring the shouldExit is false
    Serial.println("Entering while on loop");
    while(!shouldExit){
      yield();
      if(digitalRead(SET_IRSIGNALS_PIN) == HIGH){
        setIrSignalsLastState = HIGH;
        delay(GLOBAL_DELAY);
      }
      
      // listens to all the buttons 
      for(int pin : pinButtons){
        buttonRegisterListener(pin);
      } 
      if(digitalRead(SET_IRSIGNALS_PIN) == LOW && setIrSignalsLastState == HIGH){ // exit main loop when pressing the main button
        delay(GLOBAL_DELAY);
        shouldExit = true;
        shouldSendSignals = true;
        Serial.println("Exiting while on loop");
      }
    }
  }
  if(shouldSendSignals == true){
    for(int pin : pinButtons){
      buttonSenderListener(pin);
    }
  }
}
// Power Off A: 0x820, C:0x0
// One: A:0x820, C:0x1C
// Eight: 0x820, C:0x45
// Vol. up: C: 0x2
// Vol. down: C:0x3