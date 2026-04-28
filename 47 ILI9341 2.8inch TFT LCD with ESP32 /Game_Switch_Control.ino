#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

// Button pins - তোমার নতুন configuration
#define BTN_UP     13
#define BTN_DOWN   12
#define BTN_LEFT   27
#define BTN_RIGHT  26
#define BTN_A      14
#define BTN_B      33
#define BTN_START  32
#define BTN_SELECT 25

void setup() {
  Serial.begin(115200);
  
  // Display setup
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  
  // Button setup - INPUT_PULLUP mode
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  
  tft.setCursor(10, 10);
  tft.println("Button Test");
  tft.println("Press buttons...");
  
  Serial.println("Button test ready!");
}

void loop() {
  tft.fillRect(10, 60, 300, 30, TFT_BLACK);
  tft.setCursor(10, 60);
  tft.setTextSize(3);
  
  // Check each button
  if(!digitalRead(BTN_UP)) {
    tft.setTextColor(TFT_GREEN);
    tft.println("UP");
    Serial.println("UP pressed");
  }
  else if(!digitalRead(BTN_DOWN)) {
    tft.setTextColor(TFT_GREEN);
    tft.println("DOWN");
    Serial.println("DOWN pressed");
  }
  else if(!digitalRead(BTN_LEFT)) {
    tft.setTextColor(TFT_GREEN);
    tft.println("LEFT");
    Serial.println("LEFT pressed");
  }
  else if(!digitalRead(BTN_RIGHT)) {
    tft.setTextColor(TFT_GREEN);
    tft.println("RIGHT");
    Serial.println("RIGHT pressed");
  }
  else if(!digitalRead(BTN_A)) {
    tft.setTextColor(TFT_YELLOW);
    tft.println("A");
    Serial.println("A pressed");
  }
  else if(!digitalRead(BTN_B)) {
    tft.setTextColor(TFT_YELLOW);
    tft.println("B");
    Serial.println("B pressed");
  }
  else if(!digitalRead(BTN_START)) {
    tft.setTextColor(TFT_CYAN);
    tft.println("START");
    Serial.println("START pressed");
  }
  else if(!digitalRead(BTN_SELECT)) {
    tft.setTextColor(TFT_CYAN);
    tft.println("SELECT");
    Serial.println("SELECT pressed");
  }
  else {
    tft.setTextColor(TFT_WHITE);
    tft.println("---");
  }
  
  delay(50);
}
