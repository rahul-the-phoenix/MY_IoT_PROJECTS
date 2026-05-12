
//===========================================DAY 1 OF LEARNING MID LEVEL IOT (BLUETOOTH)===========================================

#include "BluetoothSerial.h"
BluetoothSerial SerialBT;  //bluetoothserial is a class and serialBT is a object we made a bluettoth connection name SerialBT
int ledPin = 2;

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  SerialBT.begin("ESP32_LED");  // Bluetooth name
}

void loop() {
  if (SerialBT.available()) {             //cheaking available or not 
    String command = SerialBT.readString();     //reading   ### make delay usually 1 second 
    command.trim();       //for remove gaps , newline etc 
    command.toLowerCase();
    
    if (command == "on") {
      digitalWrite(ledPin, HIGH);
      SerialBT.println("LED IS ON");
    }
    else if (command == "off") {
      digitalWrite(ledPin, LOW);
      SerialBT.println("LED IS OFF");
    }
    else{
      SerialBT.println("WRONG COMMAND");
    }
  }
}
