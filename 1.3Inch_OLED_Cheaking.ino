#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// I2C pins - adjust these based on your wiring
// Common ESP32 DevKit uses: SDA=21, SCL=22
#define OLED_SDA 21
#define OLED_SCL 22

// SH1106 I2C address (typically 0x3C or 0x3D)
#define OLED_ADDR 0x3C

// Display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Create display object
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void scanI2C() {
  Serial.println("Scanning I2C bus...");
  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      Serial.println(address, HEX);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\nESP32 1.3\" OLED Test Starting...");
  
  // Initialize I2C with custom pins
  Wire.begin(OLED_SDA, OLED_SCL);
  
  // Scan for I2C devices (helps debug connection issues)
  scanI2C();
  
  // Initialize display
  Serial.println("Initializing SH1106 display...");
  if (!display.begin(OLED_ADDR, true)) {
    Serial.println("SH1106 initialization failed!");
    while (1) {
      delay(1000);
      Serial.print(".");
    }
  }
  
  Serial.println("Display initialized successfully!");
  
  // Basic display settings
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  
  // Show "Hello World"
  display.println("Hello World!");
  display.println("1.3\" OLED");
  display.println("ESP32 Ready!");
  display.display();
  
  delay(2000);
}

void loop() {
  display.clearDisplay();
  display.setCursor(0, 0);
  
  // Display a simple counter
  static int counter = 0;
  
  display.setTextSize(2);
  display.println("ESP32 OLED");
  display.setTextSize(1);
  display.println("");
  display.print("Counter: ");
  display.println(counter++);
  display.println("");
  display.println("I2C: SDA=21, SCL=22");
  display.println("SH1106 1.3\"");
  
  display.display();
  delay(1000);
  
  // Blink pattern - just for fun
  if (counter % 2 == 0) {
    display.invertDisplay(true);
  } else {
    display.invertDisplay(false);
  }
}
