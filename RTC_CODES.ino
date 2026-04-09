/*
╔══════════════════════════════════════════════════════════════════╗
║              DS3231 RTC PERMANENT TIME SETTER                    ║
║                     Author: RAHUL MANNA                          ║
║                      Version: 1.0.0                              ║
╚══════════════════════════════════════════════════════════════════╝

📌 DESCRIPTION
───────────────────────────────────────────────────────────────────
DS3231 Real-Time Clock configuration tool with permanent save feature.
Supports manual time setting, compiler timestamp, and OSF flag management.

 HARDWARE
───────────────────────────────────────────────────────────────────
• ESP32 | SDA=21, SCL=22 | LED=GPIO2
• DS3231 RTC Module (I2C Address: 0x68)

 LIBRARIES
───────────────────────────────────────────────────────────────────
• RTClib by Adafruit
• Wire (Built-in)

 MENU OPTIONS
───────────────────────────────────────────────────────────────────
[1] Set Manually     Format: YYYY-MM-DD HH:MM:SS
[2] Set Current Time Uses compiler timestamp
[3] Check Time       10-second verification
[4] Save & Lock      Permanent save + halt

 FEATURES
───────────────────────────────────────────────────────────────────
✓ Manual time input with validation
✓ Automatic time from PC
✓ OSF flag management
✓ Control register reset
✓ LED status indicator

 LICENSE
───────────────────────────────────────────────────────────────────
MIT License - Free to use, modify, and distribute.
Copyright (c) 2024 RAHUL MANNA

═══════════════════════════════════════════════════════════════════
*/

#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n⚡ DS3231 PERMANENT TIME SETTER ⚡");
  Serial.println("-----------------------------------");
  
  Wire.begin(21, 22);
  
  if (!rtc.begin()) {
    Serial.println("RTC NOT FOUND!");
    while(1);
  }
  
  showMenu();
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input == "1") {
      setTimeFromSerial();
    } else if (input == "2") {
      setTimeToNow();
    } else if (input == "3") {
      showCurrentTime();
    } else if (input == "4") {
      saveAndLock();
    }
  }
}

void showMenu() {
  Serial.println("\nMENU:");
  Serial.println("1. SET MANUALLY");
  Serial.println("2. SET CURRENT TIME");
  Serial.println("3. CHEAK CURRENT TIME");
  Serial.println("4. SAVE AND LOCK");
  Serial.print("YOUR CHOICE: ");
}

void setTimeFromSerial() {
  Serial.println("\n📅 GIVE INPUT DATE AND TIME:");
  Serial.println("FORMAT : YYYY-MM-DD HH:MM:SS");
  Serial.println("EXAMPLE: 2024-01-15 16:15:00");
  Serial.print("INPUT: ");
  
  while(!Serial.available());
  String input = Serial.readStringUntil('\n');
  
  int year = input.substring(0,4).toInt();
  int month = input.substring(5,7).toInt();
  int day = input.substring(8,10).toInt();
  int hour = input.substring(11,13).toInt();
  int minute = input.substring(14,16).toInt();
  int second = input.substring(17,19).toInt();
  
  if(year<2000 || year>2100) {
    Serial.println("❌ INVALID YEAR");
    return;
  }
  
  DateTime dt(year, month, day, hour, minute, second);
  rtc.adjust(dt);
  clearOSFFlag();
  
  Serial.print("✅ TIME SET: ");
  printTime(dt);
}

void setTimeToNow() {
  Serial.println("\n⏰ CURRENT TIME IS SETTING ...");
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  clearOSFFlag();
  
  DateTime now = rtc.now();
  Serial.print("✅ SETTING DONE: ");
  printTime(now);
}

void showCurrentTime() {
  DateTime now = rtc.now();
  Serial.print("\n🕒  CURRENT RTC TIME: ");
  printTime(now);
  
  
  for(int i=0; i<10; i++) {
    now = rtc.now();
    Serial.print("Check ");
    Serial.print(i+1);
    Serial.print(": ");
    printTime(now);
    delay(1000);
  }
}

void saveAndLock() {
  Serial.println("\n🔒 SET PERMANANTLY ...");
  
  // 1. OSF FLAG CLEAR
  clearOSFFlag();
  
  // 2. Control register RESET
  Wire.beginTransmission(0x68);
  Wire.write(0x0E); // Control register
  Wire.write(0x00); // Default value
  Wire.endTransmission();
  

  Serial.println("✅ PERMANANTLY SAVED ");

  

  pinMode(2, OUTPUT);
  for(int i=0; i<3; i++) {
    digitalWrite(2, HIGH);
    delay(500);
    digitalWrite(2, LOW);
    delay(500);
  }
  
  while(1); 
}

void clearOSFFlag() {
  Wire.beginTransmission(0x68);
  Wire.write(0x0F);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 1);
  
  if(Wire.available()) {
    uint8_t status = Wire.read();
    if(status & 0x80) {
      Wire.beginTransmission(0x68);
      Wire.write(0x0F);
      Wire.write(status & 0x7F);
      Wire.endTransmission();
    }
  }
}

void printTime(DateTime dt) {
  Serial.print(dt.hour());
  Serial.print(":");
  Serial.print(dt.minute());
  Serial.print(":");
  Serial.print(dt.second());
  Serial.print(" ");
  Serial.print(dt.day());
  Serial.print("/");
  Serial.print(dt.month());
  Serial.print("/");
  Serial.println(dt.year());
}
