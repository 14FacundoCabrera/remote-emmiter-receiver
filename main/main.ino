#include "IRremote.hpp"

#define IR_RECEIVE_PIN 14 //D5
#define IR_SENDER_PIN 4 //D2
//#define SET_IRSIGNALS_PIN
//todo: if the setIrSignals pin is on, it starts to record an IrSignals, then press the button where you'd like the IrSignals to be stored

struct IrData {
  uint16_t address;
  uint8_t command;
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
}

void loop() {
  if (IrReceiver.decode()) {
    uint16_t address = IrReceiver.decodedIRData.address;
    uint8_t command = IrReceiver.decodedIRData.command;
    IrReceiver.printIRResultShort(&Serial);
    Serial.println(address);
    Serial.println(command);
    IrSender.sendNEC(address, command, 3);
    // for (int i = 0; i < 4; i++){
    //   IrSender.sendNEC(IrSignals[i].address, IrSignals[i].command, 3);
    //   delay(500);
    // }
    IrReceiver.resume();   
  }
}
// Power Off A: 0x820, C:0x0
// One: A:0x820, C:0x1C
// Eight: 0x820, C:0x45
// Vol. up: C: 0x2
// Vol. down: C:0x3