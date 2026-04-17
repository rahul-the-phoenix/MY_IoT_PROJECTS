#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#define TFT_DC 2
#define TFT_CS 5
#define TFT_MOSI 23
#define TFT_CLK 18
#define TFT_RST 4
#define TFT_MISO 19

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

// colour define
#define SAFFRON 0xFD20
#define WHITE   0xFFFF
#define GREEN   0x04A8
#define BLACK   0x0000
#define YELLOW  0xFFE0
#define CYAN    0x07FF  // MAA KE এর জন্য নতুন রঙ

void setup() {
  tft.begin();
  tft.setRotation(2); 
  tft.fillScreen(BLACK);
  
  showSecretMessage(); 
  realTimeCountdown(10);
  showGo();
  finalAnimation();
}

void showSecretMessage() {
  tft.fillScreen(BLACK);
  
  tft.setTextSize(3);
  tft.setTextColor(SAFFRON);
  tft.setCursor(50, 60);
  tft.print("I HAVE A");
  
  tft.setTextSize(5);
  tft.setTextColor(WHITE);
  tft.setCursor(40, 100);
  tft.print("SECRET");
  
  tft.setTextSize(3);
  tft.setTextColor(GREEN);
  tft.setCursor(55, 150);
  tft.print("FOR YOU!");
 delay(1000);
  
  int ex = 120; // Emoji Center X
  int ey = 220; // Emoji Center Y
  int radius = 22; 
  
  tft.fillCircle(ex, ey, radius, YELLOW); // mouth
  tft.fillCircle(ex - 7, ey - 7, 2, BLACK); // left eye
  tft.fillCircle(ex + 7, ey - 7, 2, BLACK); // right eye
  tft.drawFastHLine(ex - 4, ey + 8, 8, BLACK); // mouth
  
  // small finger
  tft.fillRoundRect(ex - 3, ey + 3, 6, 20, 3, 0xFD20); 
  
  delay(3000);
  tft.fillScreen(BLACK);
}

void realTimeCountdown(int seconds) {
  tft.setTextSize(8); 
  for (int i = seconds; i >= 0; i--) {
    unsigned long startTime = millis();
    tft.fillRect(60, 110, 120, 80, BLACK); 
    tft.setTextColor(WHITE);
    if (i >= 10) tft.setCursor(75, 120);
    else tft.setCursor(100, 120);
    tft.print(i);
    while (millis() - startTime < 1000); 
  }
}

void showGo() {
  tft.fillScreen(BLACK);
  tft.setTextSize(6);
  tft.setTextColor(SAFFRON);
  tft.setCursor(70, 120);
  tft.print("GO!");
  delay(800);
  tft.fillScreen(BLACK);
}

void finalAnimation() {
  String lines[] = {"JE LORCHHE", "SOBAR DAKE", "SEI JETABE"};
  uint16_t colors[] = {SAFFRON, WHITE, WHITE};
  int yPos[] = {40, 90, 140};
  
  tft.setTextSize(3);
  for (int i = 0; i < 3; i++) {
    tft.setTextColor(colors[i]);
    tft.setCursor(35, yPos[i]);
    for (int j = 0; j < lines[i].length(); j++) {
      tft.print(lines[i][j]);
      delay(100);
    }
    delay(300);
  }

  // BANGLA (Size 5 - Green)
  delay(500);
  tft.setTextSize(5); 
  tft.setTextColor(GREEN);
  tft.setCursor(30, 200);
  String b = "BANGLA";
  for (int j = 0; j < b.length(); j++) {
    tft.print(b[j]);
    delay(120);
  }

  // MAA KE (Size 3 - Cyan/Blue Tone)
  tft.setTextSize(3);
  tft.setTextColor(CYAN);
  tft.setCursor(65, 260); 
  String m = "MAA KE";
  for (int j = 0; j < m.length(); j++) {
    tft.print(m[j]);
    delay(100);
  }
  
  tft.drawRect(10, 10, 220, 300, CYAN);
}

void loop() {}
