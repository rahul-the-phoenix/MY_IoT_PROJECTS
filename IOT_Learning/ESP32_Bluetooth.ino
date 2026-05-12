
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

//===========================================DAY 2 OF LEARNING MID LEVEL IOT (BLUETOOTH fastest )===========================================

#include "BluetoothSerial.h"

BluetoothSerial SerialBT;  // Create Bluetooth object
int ledPin = 2;            
String command = "";       // Buffer to store incoming command characters

void setup() {
  pinMode(ledPin, OUTPUT);     
  digitalWrite(ledPin, LOW);    
  SerialBT.begin("ESP32_LED");  // Start Bluetooth with device name
}

void loop() {
  while (SerialBT.available()) {
    
    // IMPORTANT: Read ONE character at a time (fastest method)
    // No timeout delay like readString() or readStringUntil()
    char c = SerialBT.read();
    
    // IMPORTANT: Check for end-of-line markers
    // '\n' = Newline (LF) - sent when user presses Enter
    // '\r' = Carriage Return (CR) - sometimes sent with Enter
    if (c == '\n' || c == '\r') {
      
      // IMPORTANT: Remove whitespace from start and end
      // This removes any accidental spaces or line endings
      command.trim();
      
      // IMPORTANT: Convert to lowercase for case-insensitive comparison
      // Makes "ON", "On", "oN" all work like "on"
      command.toLowerCase();
      
      // IMPORTANT: Compare and execute command
      if (command == "on") {
        digitalWrite(ledPin, HIGH);     // Turn LED ON
        SerialBT.println("LED IS ON");  // Send confirmation
      }
      else if (command == "off") {
        digitalWrite(ledPin, LOW);      // Turn LED OFF
        SerialBT.println("LED IS OFF"); // Send confirmation
      }
      else if (command != "") {
        SerialBT.println("WRONG COMMAND");  // Invalid command feedback
      }
      
      // IMPORTANT: Clear buffer for next command
      command = "";
    }
    else {
      // IMPORTANT: Build command character by character
      // Each character is appended until Enter is pressed
      command += c;
    }
  }
}
