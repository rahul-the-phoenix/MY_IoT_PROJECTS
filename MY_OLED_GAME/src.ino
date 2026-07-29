// ============================================================
// COMPLETE GAME CONSOLE FOR ESP32-C3
// 27 Games Total with Main Menu, Music Player, and Settings
// ============================================================

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <EEPROM.h>
#include <string.h> 

#if !defined(ARDUINO_ARCH_ESP32)
#error This sketch is built for ESP32. In Arduino IDE choose Tools -> Board -> ESP32 Arduino -> ESP32 Dev Module.
#endif

#include <esp_random.h>

// ── Display Pins for ESP32-C3 ─────────
#define OLED_SDA 8
#define OLED_SCL 9

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);

#define SCREEN_W 128
#define SCREEN_H 64

// ── Buttons for ESP32-C3 ──────────────
#define BTN_UP 6
#define BTN_DOWN 7
#define BTN_LEFT 4    
#define BTN_RIGHT 5   
#define BTN_MENU 2    // Back
#define BTN_PAUSE 1   // Pause/Resume
#define BTN_ENTER 0   // Enter/Select button

// ── Buzzer ─────────────────────────────
#define BUZZER_PIN 10

// ── EEPROM for high scores ────────────
#define EEPROM_SIZE 512
#define GAME_COUNT 28
#define MAX_FAVORITES 10
#define MUSIC_COUNT 30

#define NOTE_REST 0

// ── Love Heart Bitmap (8x8) ───────────
static const unsigned char PROGMEM heart_bmp[] = {
  0b00000000,
  0b01100110,
  0b11111111,
  0b11111111,
  0b11111111,
  0b01111110,
  0b00111100,
  0b00011000
};

// ── RAHUL Bitmap (128x64) ─────────────
static const uint8_t PROGMEM RahulBitmap[] = {
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x72, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xc0, 0x00, 0x19, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xb8, 0x00, 0x00, 0x00, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0x72, 0x00, 0x40, 0x00, 0x00, 0x18, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xfe, 0x20, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xfe, 0x40, 0x00, 0x00, 0x00, 0x00, 0x02, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xfc, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xc4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x6f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xc0, 0x00, 0x1f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xde, 0x0f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xfd, 0x06, 0xcf, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0x43, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xff, 0xff, 0x80, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0x0f, 0xc0, 0x00, 0x03, 0xff, 0x80, 0x00, 0xf0, 0x0f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x7f, 0x80, 0x07, 0x0c, 0x1f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0xff, 0x80, 0x1c, 0xe6, 0x1f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0x80, 0xc0, 0x00, 0x01, 0xff, 0xc0, 0xf9, 0xe6, 0x1f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xe1, 0xc0, 0x00, 0x07, 0xff, 0xc1, 0x81, 0xe6, 0x1f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xf3, 0xc0, 0x00, 0x3f, 0xff, 0xc1, 0xc0, 0x64, 0x3f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xfb, 0x0f, 0xff, 0xff, 0x83, 0xe2, 0x48, 0x3f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x03, 0x01, 0x90, 0x7f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xf9, 0xff, 0xff, 0xfe, 0x02, 0x1f, 0x30, 0x7f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xc3, 0xf8, 0xff, 0xff, 0xfc, 0x02, 0x6b, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xe0, 0x0c, 0xff, 0xc7, 0xfc, 0x00, 0x7f, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xf0, 0x01, 0xff, 0x80, 0x40, 0x00, 0x10, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xe0, 0x03, 0xcc, 0x40, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xe0, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xe0, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xf8, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xfe, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x01, 0xc1, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x0e, 0x01, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x70, 0x00, 0x0f, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x07, 0xc0, 0x71, 0x07, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x08, 0x1f, 0xff, 0xfc, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xb0, 0x3f, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xce, 0x00, 0x00, 0x06, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff
};

// ── Note Definitions ──────────────────
#define NOTE_G2   98
#define NOTE_AS2  117
#define NOTE_C3   131
#define NOTE_CS3  139
#define NOTE_D3   147
#define NOTE_DS3  156
#define NOTE_E3   165
#define NOTE_F3   175
#define NOTE_FS3  185
#define NOTE_G3   196
#define NOTE_GS3  208
#define NOTE_A3   220
#define NOTE_AS3  233
#define NOTE_B3   247
#define NOTE_C4   262
#define NOTE_CS4  277
#define NOTE_D4   294
#define NOTE_DS4  311
#define NOTE_E4   330
#define NOTE_F4   349
#define NOTE_FS4  370
#define NOTE_G4   392
#define NOTE_GS4  415
#define NOTE_A4   440
#define NOTE_AS4  466
#define NOTE_B4   494
#define NOTE_C5   523
#define NOTE_CS5  554
#define NOTE_D5   587
#define NOTE_DS5  622
#define NOTE_E5   659
#define NOTE_F5   698
#define NOTE_FS5  740
#define NOTE_G5   784
#define NOTE_GS5  831
#define NOTE_A5   880
#define NOTE_AS5  932
#define NOTE_B5   988
#define NOTE_C6   1047
#define NOTE_CS6  1109
#define NOTE_D6   1175
#define NOTE_DS6  1245
#define NOTE_E6   1319
#define NOTE_F6   1397
#define NOTE_FS6  1480
#define NOTE_G6   1568
#define NOTE_GS6  1661
#define NOTE_A6   1760
#define NOTE_AS6  1865
#define NOTE_B6   1976
#define NOTE_C7   2093

// ── Music Duration ────────────────────
#define D_QTR   500
#define D_WHL   (D_QTR * 4)
#define D_HLF   (D_QTR * 2)
#define D_EIT   (D_QTR / 2)
#define D_SXT   (D_QTR / 4)
#define D_QTRD  (D_QTR + D_QTR / 2)

struct MelodyNote {
  uint16_t freq;
  uint16_t duration;
};

// ── Song Data ──────────────────────────
const MelodyNote SONG_01[] = {
  {NOTE_C5,D_EIT},{NOTE_E5,D_EIT},{NOTE_G5,D_EIT},{NOTE_C6,D_EIT},
  {NOTE_G5,D_EIT},{NOTE_E5,D_EIT},{NOTE_C5,D_QTR},{NOTE_REST,D_EIT},
  {NOTE_D5,D_EIT},{NOTE_F5,D_EIT},{NOTE_A5,D_EIT},{NOTE_D6,D_EIT},
  {NOTE_A5,D_EIT},{NOTE_F5,D_EIT},{NOTE_D5,D_QTR},{NOTE_REST,D_EIT},
  {NOTE_E5,D_EIT},{NOTE_G5,D_EIT},{NOTE_B4,D_EIT},{NOTE_E5,D_EIT},
  {NOTE_B4,D_EIT},{NOTE_G5,D_EIT},{NOTE_E5,D_QTR},{NOTE_REST,D_EIT},
  {NOTE_F5,D_EIT},{NOTE_A5,D_EIT},{NOTE_C6,D_EIT},{NOTE_F6,D_EIT},
  {NOTE_C6,D_EIT},{NOTE_A5,D_EIT},{NOTE_F5,D_QTR},{NOTE_REST,D_EIT},
  {NOTE_C5,D_EIT},{NOTE_E5,D_EIT},{NOTE_G5,D_EIT},{NOTE_C6,D_QTR},
  {NOTE_G5,D_QTR},{NOTE_C6,D_HLF},
};

const MelodyNote SONG_02[] = {
  {NOTE_E5,D_QTR},{NOTE_B4,D_EIT},{NOTE_C5,D_EIT},{NOTE_D5,D_QTR},{NOTE_C5,D_EIT},{NOTE_B4,D_EIT},
  {NOTE_A4,D_QTR},{NOTE_A4,D_EIT},{NOTE_C5,D_EIT},{NOTE_E5,D_QTR},{NOTE_D5,D_EIT},{NOTE_C5,D_EIT},
  {NOTE_B4,D_QTRD},{NOTE_C5,D_EIT},{NOTE_D5,D_QTR},{NOTE_E5,D_QTR},
  {NOTE_C5,D_QTR},{NOTE_A4,D_QTR},{NOTE_A4,D_QTR},{NOTE_REST,D_EIT},
  {NOTE_D5,D_QTRD},{NOTE_F5,D_EIT},{NOTE_A5,D_QTR},{NOTE_G5,D_EIT},{NOTE_F5,D_EIT},
  {NOTE_E5,D_QTRD},{NOTE_C5,D_EIT},{NOTE_E5,D_QTR},{NOTE_D5,D_EIT},{NOTE_C5,D_EIT},
  {NOTE_B4,D_QTR},{NOTE_B4,D_EIT},{NOTE_C5,D_EIT},{NOTE_D5,D_QTR},{NOTE_E5,D_QTR},
  {NOTE_C5,D_QTR},{NOTE_A4,D_QTR},{NOTE_A4,D_HLF},
  {NOTE_REST,D_EIT},
  {NOTE_E5,D_QTR},{NOTE_C5,D_QTR},{NOTE_D5,D_QTR},{NOTE_B4,D_QTR},
  {NOTE_C5,D_QTR},{NOTE_A4,D_QTR},{NOTE_GS4,D_QTR},{NOTE_B4,D_QTRD},
  {NOTE_E5,D_QTR},{NOTE_C5,D_QTR},{NOTE_D5,D_QTR},{NOTE_B4,D_QTR},
  {NOTE_C5,D_QTR},{NOTE_E5,D_QTR},{NOTE_A5,D_HLF},
  {NOTE_GS5,D_QTR},{NOTE_A5,D_WHL},
};

const MelodyNote SONG_03[] = {
  {NOTE_G4,D_EIT},{NOTE_G4,D_EIT},{NOTE_A4,D_QTR},{NOTE_G4,D_QTR},{NOTE_C5,D_QTR},{NOTE_B4,D_HLF},
  {NOTE_G4,D_EIT},{NOTE_G4,D_EIT},{NOTE_A4,D_QTR},{NOTE_G4,D_QTR},{NOTE_D5,D_QTR},{NOTE_C5,D_HLF},
  {NOTE_G4,D_EIT},{NOTE_G4,D_EIT},{NOTE_G5,D_QTR},{NOTE_E5,D_QTR},{NOTE_C5,D_QTR},{NOTE_B4,D_QTR},{NOTE_A4,D_HLF},
  {NOTE_F5,D_EIT},{NOTE_F5,D_EIT},{NOTE_E5,D_QTR},{NOTE_C5,D_QTR},{NOTE_D5,D_QTR},{NOTE_C5,D_WHL},
  {NOTE_REST,D_QTR},
  {NOTE_G5,D_EIT},{NOTE_G5,D_EIT},{NOTE_A5,D_QTR},{NOTE_G5,D_QTR},{NOTE_C6,D_QTR},{NOTE_B5,D_HLF},
  {NOTE_A5,D_QTR},{NOTE_F5,D_QTR},{NOTE_D5,D_QTR},{NOTE_C5,D_WHL},
};

const MelodyNote SONG_04[] = {
  {NOTE_C5,D_QTR},{NOTE_D5,D_QTR},{NOTE_E5,D_QTR},{NOTE_C5,D_QTR},
  {NOTE_E5,D_QTR},{NOTE_F5,D_QTR},{NOTE_G5,D_HLF},
  {NOTE_G5,D_EIT},{NOTE_A5,D_EIT},{NOTE_G5,D_EIT},{NOTE_F5,D_EIT},{NOTE_E5,D_QTR},{NOTE_C5,D_QTR},
  {NOTE_D5,D_QTR},{NOTE_C5,D_HLF},
  {NOTE_E5,D_QTR},{NOTE_F5,D_QTR},{NOTE_G5,D_QTR},{NOTE_E5,D_QTR},
  {NOTE_G5,D_QTR},{NOTE_A5,D_QTR},{NOTE_C6,D_HLF},
  {NOTE_B5,D_EIT},{NOTE_C6,D_EIT},{NOTE_B5,D_EIT},{NOTE_A5,D_EIT},{NOTE_G5,D_QTR},{NOTE_E5,D_QTR},
  {NOTE_F5,D_QTR},{NOTE_D5,D_QTR},{NOTE_C5,D_WHL},
};

const MelodyNote SONG_05[] = {
  {NOTE_A4,D_SXT},{NOTE_C5,D_SXT},{NOTE_E5,D_SXT},{NOTE_A5,D_SXT},
  {NOTE_G4,D_SXT},{NOTE_B4,D_SXT},{NOTE_D5,D_SXT},{NOTE_G5,D_SXT},
  {NOTE_F4,D_SXT},{NOTE_A4,D_SXT},{NOTE_C5,D_SXT},{NOTE_F5,D_SXT},
  {NOTE_E4,D_SXT},{NOTE_G4,D_SXT},{NOTE_C5,D_SXT},{NOTE_E5,D_QTRD},
  {NOTE_A4,D_HLF},
  {NOTE_D5,D_SXT},{NOTE_F5,D_SXT},{NOTE_A5,D_SXT},{NOTE_D6,D_SXT},
  {NOTE_C5,D_SXT},{NOTE_E5,D_SXT},{NOTE_G5,D_SXT},{NOTE_C6,D_SXT},
  {NOTE_B4,D_SXT},{NOTE_D5,D_SXT},{NOTE_F5,D_SXT},{NOTE_B5,D_SXT},
  {NOTE_A4,D_SXT},{NOTE_C5,D_SXT},{NOTE_F5,D_SXT},{NOTE_A5,D_QTRD},
  {NOTE_D5,D_QTR},{NOTE_A4,D_WHL},
};

const MelodyNote SONG_06[] = {
  {NOTE_C3,D_QTR},{NOTE_C3,D_EIT},{NOTE_G3,D_EIT},{NOTE_C3,D_QTR},
  {NOTE_A3,D_QTR},{NOTE_A3,D_EIT},{NOTE_F3,D_EIT},{NOTE_G3,D_QTR},
  {NOTE_C3,D_QTR},{NOTE_C3,D_EIT},{NOTE_G3,D_EIT},{NOTE_C3,D_QTRD},
  {NOTE_D3,D_QTR},{NOTE_G3,D_HLF},
  {NOTE_F3,D_QTR},{NOTE_F3,D_EIT},{NOTE_C3,D_EIT},{NOTE_F3,D_QTR},
  {NOTE_D3,D_QTR},{NOTE_D3,D_EIT},{NOTE_AS2,D_EIT},{NOTE_C3,D_QTR},
  {NOTE_G3,D_QTR},{NOTE_E3,D_QTR},{NOTE_C3,D_QTRD},{NOTE_G2,D_QTR},
  {NOTE_C3,D_WHL},
};

const MelodyNote SONG_07[] = {
  {NOTE_E4,D_EIT},{NOTE_REST,D_SXT},{NOTE_G4,D_EIT},{NOTE_A4,D_SXT},
  {NOTE_E4,D_EIT},{NOTE_REST,D_SXT},{NOTE_G4,D_EIT},{NOTE_C5,D_SXT},
  {NOTE_B4,D_EIT},{NOTE_REST,D_SXT},{NOTE_G4,D_EIT},{NOTE_A4,D_SXT},
  {NOTE_E4,D_QTR},{NOTE_REST,D_EIT},{NOTE_D4,D_EIT},{NOTE_E4,D_QTRD},
  {NOTE_REST,D_SXT},{NOTE_G4,D_SXT},{NOTE_REST,D_SXT},{NOTE_A4,D_SXT},{NOTE_REST,D_SXT},{NOTE_C5,D_SXT},
  {NOTE_B4,D_EIT},{NOTE_G4,D_EIT},{NOTE_E4,D_EIT},{NOTE_D4,D_EIT},
  {NOTE_E4,D_EIT},{NOTE_REST,D_SXT},{NOTE_G4,D_EIT},{NOTE_A4,D_SXT},
  {NOTE_B4,D_QTR},{NOTE_E4,D_HLF},
};

const MelodyNote SONG_08[] = {
  {NOTE_C5,D_QTR},{NOTE_C5,D_EIT},{NOTE_C5,D_EIT},{NOTE_G4,D_QTR},{NOTE_A4,D_QTR},
  {NOTE_C5,D_QTR},{NOTE_C5,D_EIT},{NOTE_C5,D_EIT},{NOTE_D5,D_QTR},{NOTE_C5,D_QTR},
  {NOTE_B4,D_QTR},{NOTE_C5,D_QTR},{NOTE_D5,D_QTR},{NOTE_E5,D_HLF},
  {NOTE_E5,D_QTR},{NOTE_E5,D_EIT},{NOTE_E5,D_EIT},{NOTE_D5,D_QTR},{NOTE_C5,D_QTR},
  {NOTE_D5,D_QTR},{NOTE_E5,D_QTR},{NOTE_F5,D_QTR},{NOTE_E5,D_QTR},
  {NOTE_D5,D_QTR},{NOTE_C5,D_QTR},{NOTE_G4,D_QTR},{NOTE_C5,D_WHL},
};

const MelodyNote SONG_09[] = {
  {NOTE_G5,D_SXT},{NOTE_E5,D_SXT},{NOTE_C5,D_SXT},{NOTE_E5,D_SXT},
  {NOTE_G5,D_SXT},{NOTE_E5,D_SXT},{NOTE_C5,D_SXT},{NOTE_E5,D_SXT},
  {NOTE_A5,D_SXT},{NOTE_F5,D_SXT},{NOTE_D5,D_SXT},{NOTE_F5,D_SXT},
  {NOTE_A5,D_SXT},{NOTE_F5,D_SXT},{NOTE_D5,D_SXT},{NOTE_F5,D_SXT},
  {NOTE_G5,D_QTRD},{NOTE_C6,D_HLF},
  {NOTE_B5,D_SXT},{NOTE_G5,D_SXT},{NOTE_D5,D_SXT},{NOTE_G5,D_SXT},
  {NOTE_C6,D_SXT},{NOTE_G5,D_SXT},{NOTE_E5,D_SXT},{NOTE_G5,D_SXT},
  {NOTE_A5,D_SXT},{NOTE_G5,D_SXT},{NOTE_F5,D_SXT},{NOTE_E5,D_SXT},
  {NOTE_D5,D_SXT},{NOTE_E5,D_SXT},{NOTE_F5,D_SXT},{NOTE_G5,D_SXT},
  {NOTE_C6,D_QTRD},{NOTE_G5,D_EIT},{NOTE_C6,D_WHL},
};

const MelodyNote SONG_10[] = {
  {NOTE_A4,D_EIT},{NOTE_GS4,D_EIT},{NOTE_G4,D_EIT},{NOTE_FS4,D_EIT},
  {NOTE_F4,D_EIT},{NOTE_E4,D_EIT},{NOTE_DS4,D_EIT},{NOTE_D4,D_QTR},
  {NOTE_REST,D_EIT},
  {NOTE_A3,D_EIT},{NOTE_GS3,D_EIT},{NOTE_G3,D_EIT},{NOTE_FS3,D_EIT},
  {NOTE_F3,D_HLF},
  {NOTE_REST,D_EIT},
  {NOTE_D4,D_EIT},{NOTE_CS4,D_EIT},{NOTE_C4,D_EIT},{NOTE_B3,D_EIT},
  {NOTE_AS3,D_EIT},{NOTE_A3,D_EIT},{NOTE_GS3,D_EIT},{NOTE_G3,D_QTR},
  {NOTE_REST,D_EIT},
  {NOTE_C3,D_EIT},{NOTE_G3,D_EIT},{NOTE_C4,D_EIT},{NOTE_G3,D_QTRD},
  {NOTE_C3,D_HLF},
};

const MelodyNote SONG_11[] = {
  {NOTE_A4,D_QTR},{NOTE_A4,D_EIT},{NOTE_B4,D_EIT},{NOTE_C5,D_QTR},{NOTE_C5,D_EIT},{NOTE_B4,D_EIT},
  {NOTE_A4,D_QTR},{NOTE_G4,D_EIT},{NOTE_A4,D_EIT},{NOTE_A4,D_HLF},
  {NOTE_E5,D_SXT},{NOTE_E5,D_SXT},{NOTE_E5,D_SXT},{NOTE_F5,D_SXT},{NOTE_G5,D_SXT},{NOTE_F5,D_SXT},{NOTE_E5,D_SXT},{NOTE_D5,D_SXT},
  {NOTE_C5,D_QTR},{NOTE_A4,D_QTR},{NOTE_A4,D_HLF},
  {NOTE_A5,D_SXT},{NOTE_G5,D_SXT},{NOTE_F5,D_SXT},{NOTE_E5,D_SXT},{NOTE_D5,D_SXT},{NOTE_C5,D_SXT},{NOTE_B4,D_SXT},{NOTE_A4,D_SXT},
  {NOTE_G4,D_SXT},{NOTE_A4,D_SXT},{NOTE_B4,D_SXT},{NOTE_C5,D_SXT},{NOTE_D5,D_SXT},{NOTE_E5,D_SXT},{NOTE_F5,D_SXT},{NOTE_G5,D_SXT},
  {NOTE_A5,D_QTR},{NOTE_A4,D_WHL},
};

const MelodyNote SONG_12[] = {
  {NOTE_D4,D_QTR},{NOTE_F4,D_EIT},{NOTE_GS4,D_QTR},{NOTE_REST,D_EIT},
  {NOTE_G4,D_QTR},{NOTE_D4,D_EIT},{NOTE_AS3,D_QTRD},
  {NOTE_C4,D_QTR},{NOTE_D4,D_EIT},{NOTE_F4,D_QTR},{NOTE_REST,D_EIT},
  {NOTE_E4,D_HLF},
  {NOTE_REST,D_QTR},
  {NOTE_CS4,D_QTR},{NOTE_E4,D_EIT},{NOTE_GS4,D_QTR},{NOTE_REST,D_EIT},
  {NOTE_A4,D_QTR},{NOTE_FS4,D_EIT},{NOTE_D4,D_QTRD},
  {NOTE_C4,D_EIT},{NOTE_CS4,D_EIT},{NOTE_D4,D_QTR},{NOTE_REST,D_EIT},
  {NOTE_GS3,D_WHL},
};

const MelodyNote SONG_13[] = {
  {NOTE_C4,D_EIT},{NOTE_C4,D_EIT},{NOTE_C4,D_EIT},{NOTE_F4,D_QTRD},
  {NOTE_C4,D_EIT},{NOTE_C4,D_EIT},{NOTE_C4,D_EIT},{NOTE_G4,D_QTRD},
  {NOTE_F4,D_EIT},{NOTE_E4,D_EIT},{NOTE_D4,D_EIT},{NOTE_C4,D_QTR},{NOTE_REST,D_EIT},
  {NOTE_G3,D_QTR},{NOTE_C4,D_HLF},
  {NOTE_E4,D_EIT},{NOTE_E4,D_EIT},{NOTE_E4,D_EIT},{NOTE_A4,D_QTRD},
  {NOTE_E4,D_EIT},{NOTE_E4,D_EIT},{NOTE_E4,D_EIT},{NOTE_C5,D_QTRD},
  {NOTE_A4,D_EIT},{NOTE_G4,D_EIT},{NOTE_F4,D_EIT},{NOTE_E4,D_QTR},{NOTE_REST,D_EIT},
  {NOTE_C4,D_QTR},{NOTE_G3,D_QTR},{NOTE_C4,D_WHL},
};

const MelodyNote SONG_14[] = {
  {NOTE_C5,D_SXT},{NOTE_D5,D_SXT},{NOTE_E5,D_SXT},{NOTE_F5,D_SXT},
  {NOTE_G5,D_SXT},{NOTE_A5,D_SXT},{NOTE_B5,D_SXT},{NOTE_C6,D_SXT},
  {NOTE_B5,D_SXT},{NOTE_A5,D_SXT},{NOTE_G5,D_SXT},{NOTE_F5,D_SXT},
  {NOTE_E5,D_SXT},{NOTE_D5,D_SXT},{NOTE_C5,D_QTRD},
  {NOTE_G5,D_QTR},{NOTE_C6,D_HLF},
  {NOTE_D5,D_SXT},{NOTE_F5,D_SXT},{NOTE_A5,D_SXT},{NOTE_C6,D_SXT},
  {NOTE_D6,D_SXT},{NOTE_C6,D_SXT},{NOTE_A5,D_SXT},{NOTE_F5,D_SXT},
  {NOTE_G5,D_SXT},{NOTE_B5,D_SXT},{NOTE_D6,D_SXT},{NOTE_G6,D_SXT},
  {NOTE_F6,D_SXT},{NOTE_D6,D_SXT},{NOTE_B5,D_SXT},{NOTE_G5,D_QTRD},
  {NOTE_C6,D_QTR},{NOTE_G5,D_WHL},
};

const MelodyNote SONG_15[] = {
  {NOTE_C5,D_EIT},{NOTE_E5,D_EIT},{NOTE_G5,D_EIT},{NOTE_C6,D_QTR},
  {NOTE_G5,D_EIT},{NOTE_C6,D_QTRD},
  {NOTE_A5,D_EIT},{NOTE_C6,D_EIT},{NOTE_F6,D_QTR},{NOTE_E6,D_HLF},
  {NOTE_REST,D_EIT},
  {NOTE_G5,D_EIT},{NOTE_C6,D_EIT},{NOTE_E6,D_QTR},
  {NOTE_D6,D_EIT},{NOTE_C6,D_EIT},{NOTE_B5,D_EIT},{NOTE_C6,D_QTRD},
  {NOTE_G5,D_EIT},{NOTE_C6,D_EIT},{NOTE_E6,D_EIT},{NOTE_G6,D_QTR},
  {NOTE_C7,D_WHL},
};

const MelodyNote SONG_16[] = {
  {NOTE_A5,D_QTRD},{NOTE_REST,D_SXT},{NOTE_A5,D_QTRD},{NOTE_REST,D_QTRD},
  {NOTE_A5,D_QTRD},{NOTE_REST,D_SXT},{NOTE_A5,D_QTRD},{NOTE_REST,D_HLF},
  {NOTE_A5,D_QTRD},{NOTE_REST,D_SXT},{NOTE_A5,D_QTRD},{NOTE_REST,D_QTRD},
  {NOTE_A5,D_QTRD},{NOTE_REST,D_SXT},{NOTE_A5,D_QTRD},
  {NOTE_REST,D_HLF},
  {NOTE_A5,D_QTRD},{NOTE_REST,D_SXT},{NOTE_A5,D_QTRD},{NOTE_REST,D_QTRD},
  {NOTE_A5,D_QTRD},{NOTE_REST,D_SXT},{NOTE_A5,D_QTRD},{NOTE_REST,D_HLF},
  {NOTE_A5,D_QTRD},{NOTE_REST,D_SXT},{NOTE_A5,D_QTRD},{NOTE_REST,D_QTRD},
  {NOTE_A5,D_QTRD},{NOTE_REST,D_SXT},{NOTE_A5,D_QTRD},
};

const MelodyNote SONG_17[] = {
  {NOTE_G5,D_HLF},{NOTE_REST,D_QTR},{NOTE_G5,D_HLF},{NOTE_REST,D_HLF},
  {NOTE_G5,D_HLF},{NOTE_REST,D_QTR},{NOTE_G5,D_HLF},
  {NOTE_REST,D_HLF},
  {NOTE_G5,D_HLF},{NOTE_REST,D_QTR},{NOTE_G5,D_HLF},{NOTE_REST,D_HLF},
  {NOTE_G5,D_HLF},{NOTE_REST,D_QTR},{NOTE_G5,D_HLF},
};

const MelodyNote SONG_18[] = {
  {NOTE_E5,D_EIT},{NOTE_DS5,D_EIT},{NOTE_E5,D_EIT},{NOTE_DS5,D_EIT},{NOTE_E5,D_EIT},{NOTE_B4,D_EIT},{NOTE_D5,D_EIT},{NOTE_C5,D_EIT},
  {NOTE_A4,D_QTR},{NOTE_REST,D_EIT},{NOTE_C4,D_EIT},{NOTE_E4,D_EIT},{NOTE_A4,D_EIT},
  {NOTE_B4,D_QTR},{NOTE_REST,D_EIT},{NOTE_E4,D_EIT},{NOTE_GS4,D_EIT},{NOTE_B4,D_EIT},
  {NOTE_C5,D_HLF},
  {NOTE_REST,D_EIT},
  {NOTE_A5,D_EIT},{NOTE_GS5,D_EIT},{NOTE_A5,D_EIT},{NOTE_GS5,D_EIT},{NOTE_A5,D_EIT},{NOTE_E5,D_EIT},{NOTE_G5,D_EIT},{NOTE_F5,D_EIT},
  {NOTE_D5,D_QTR},{NOTE_REST,D_EIT},{NOTE_F4,D_EIT},{NOTE_A4,D_EIT},{NOTE_D5,D_EIT},
  {NOTE_E5,D_QTR},{NOTE_REST,D_EIT},{NOTE_A4,D_EIT},{NOTE_CS5,D_EIT},{NOTE_E5,D_EIT},
  {NOTE_A4,D_WHL},
};

const MelodyNote SONG_19[] = {
  {NOTE_E4,D_EIT},{NOTE_E4,D_EIT},{NOTE_G4,D_EIT},{NOTE_E4,D_EIT},
  {NOTE_D4,D_EIT},{NOTE_D4,D_EIT},{NOTE_E4,D_QTR},
  {NOTE_E4,D_EIT},{NOTE_E4,D_EIT},{NOTE_G4,D_EIT},{NOTE_A4,D_EIT},
  {NOTE_B4,D_QTRD},{NOTE_G4,D_QTR},{NOTE_E4,D_HLF},
  {NOTE_A4,D_EIT},{NOTE_A4,D_EIT},{NOTE_C5,D_EIT},{NOTE_A4,D_EIT},
  {NOTE_G4,D_EIT},{NOTE_G4,D_EIT},{NOTE_A4,D_QTR},
  {NOTE_E5,D_SXT},{NOTE_D5,D_SXT},{NOTE_C5,D_SXT},{NOTE_B4,D_SXT},{NOTE_A4,D_QTR},{NOTE_E4,D_QTR},
  {NOTE_G4,D_QTRD},{NOTE_E4,D_QTR},{NOTE_E4,D_WHL},
};

const MelodyNote SONG_20[] = {
  {NOTE_C4,D_EIT},{NOTE_E4,D_EIT},{NOTE_G4,D_EIT},{NOTE_AS4,D_EIT},
  {NOTE_A4,D_EIT},{NOTE_F4,D_EIT},{NOTE_D4,D_EIT},{NOTE_G4,D_QTR},
  {NOTE_E4,D_EIT},{NOTE_G4,D_EIT},{NOTE_AS4,D_EIT},{NOTE_D5,D_EIT},
  {NOTE_C5,D_HLF},
  {NOTE_REST,D_EIT},
  {NOTE_D4,D_EIT},{NOTE_F4,D_EIT},{NOTE_A4,D_EIT},{NOTE_C5,D_EIT},
  {NOTE_G4,D_EIT},{NOTE_B4,D_EIT},{NOTE_D5,D_EIT},{NOTE_F5,D_QTR},
  {NOTE_E5,D_EIT},{NOTE_C5,D_EIT},{NOTE_G4,D_EIT},{NOTE_E4,D_EIT},
  {NOTE_C4,D_WHL},
};

const MelodyNote SONG_21[] = {
  {NOTE_E5,D_QTR},{NOTE_E5,D_QTR},{NOTE_E5,D_HLF},
  {NOTE_E5,D_QTR},{NOTE_E5,D_QTR},{NOTE_E5,D_HLF},
  {NOTE_E5,D_QTR},{NOTE_G5,D_QTR},{NOTE_C5,D_QTR},{NOTE_D5,D_EIT},{NOTE_E5,D_WHL},
  {NOTE_F5,D_QTR},{NOTE_F5,D_QTR},{NOTE_F5,D_QTRD},{NOTE_F5,D_EIT},
  {NOTE_F5,D_QTR},{NOTE_E5,D_QTR},{NOTE_E5,D_EIT},{NOTE_E5,D_EIT},
  {NOTE_E5,D_QTR},{NOTE_D5,D_QTR},{NOTE_D5,D_QTR},{NOTE_E5,D_QTR},{NOTE_D5,D_HLF},{NOTE_G5,D_HLF},
  {NOTE_G5,D_QTR},{NOTE_F5,D_QTR},{NOTE_E5,D_QTR},{NOTE_D5,D_QTR},
  {NOTE_C5,D_HLF},{NOTE_REST,D_QTR},
  {NOTE_E5,D_QTR},{NOTE_E5,D_QTR},{NOTE_E5,D_HLF},
  {NOTE_E5,D_QTR},{NOTE_G5,D_QTR},{NOTE_C5,D_QTR},{NOTE_D5,D_EIT},{NOTE_E5,D_WHL},
};

const MelodyNote SONG_22[] = {
  {NOTE_REST,D_EIT},{NOTE_C4,D_EIT},{NOTE_REST,D_EIT},{NOTE_C4,D_EIT},
  {NOTE_REST,D_EIT},{NOTE_F4,D_EIT},{NOTE_REST,D_EIT},{NOTE_F4,D_EIT},
  {NOTE_REST,D_EIT},{NOTE_G4,D_EIT},{NOTE_REST,D_EIT},{NOTE_G4,D_EIT},
  {NOTE_REST,D_EIT},{NOTE_F4,D_EIT},{NOTE_C4,D_QTRD},
  {NOTE_REST,D_EIT},{NOTE_D4,D_EIT},{NOTE_REST,D_EIT},{NOTE_D4,D_EIT},
  {NOTE_REST,D_EIT},{NOTE_G4,D_EIT},{NOTE_REST,D_EIT},{NOTE_G4,D_EIT},
  {NOTE_REST,D_EIT},{NOTE_A4,D_EIT},{NOTE_REST,D_EIT},{NOTE_A4,D_EIT},
  {NOTE_REST,D_EIT},{NOTE_G4,D_EIT},{NOTE_D4,D_QTRD},
  {NOTE_C4,D_WHL},
};

const MelodyNote SONG_23[] = {
  {NOTE_A4,D_SXT},{NOTE_A5,D_SXT},{NOTE_A4,D_SXT},{NOTE_A5,D_SXT},
  {NOTE_C5,D_SXT},{NOTE_C6,D_SXT},{NOTE_C5,D_SXT},{NOTE_C6,D_SXT},
  {NOTE_E5,D_SXT},{NOTE_E6,D_SXT},{NOTE_E5,D_SXT},{NOTE_E6,D_SXT},
  {NOTE_A5,D_QTRD},{NOTE_REST,D_EIT},{NOTE_A5,D_QTRD},
  {NOTE_G5,D_SXT},{NOTE_G6,D_SXT},{NOTE_G5,D_SXT},{NOTE_G6,D_SXT},
  {NOTE_F5,D_SXT},{NOTE_F6,D_SXT},{NOTE_F5,D_SXT},{NOTE_F6,D_SXT},
  {NOTE_E5,D_SXT},{NOTE_E6,D_SXT},{NOTE_D5,D_SXT},{NOTE_D6,D_SXT},
  {NOTE_C5,D_QTRD},{NOTE_REST,D_EIT},{NOTE_C6,D_QTRD},
  {NOTE_A5,D_QTR},{NOTE_A5,D_WHL},
};

const MelodyNote SONG_24[] = {
  {NOTE_C5,D_EIT},{NOTE_D5,D_EIT},{NOTE_E5,D_QTR},{NOTE_G4,D_QTR},{NOTE_G4,D_QTRD},
  {NOTE_E4,D_EIT},{NOTE_G4,D_QTR},{NOTE_A4,D_QTR},{NOTE_A4,D_QTR},{NOTE_G4,D_QTRD},
  {NOTE_E4,D_EIT},{NOTE_D4,D_QTR},{NOTE_C4,D_QTR},{NOTE_C4,D_QTR},{NOTE_D4,D_QTR},{NOTE_E4,D_QTR},
  {NOTE_C4,D_HLF},
  {NOTE_E4,D_EIT},{NOTE_F4,D_EIT},{NOTE_G4,D_QTR},{NOTE_A4,D_QTR},{NOTE_A4,D_QTRD},
  {NOTE_G4,D_EIT},{NOTE_E4,D_QTR},{NOTE_D4,D_QTR},{NOTE_C4,D_QTR},{NOTE_D4,D_QTRD},
  {NOTE_E4,D_EIT},{NOTE_G4,D_QTR},{NOTE_E4,D_QTR},{NOTE_D4,D_QTR},{NOTE_C4,D_QTR},
  {NOTE_C4,D_WHL},
};

const MelodyNote SONG_25[] = {
  {NOTE_C5,D_QTR},{NOTE_E5,D_QTR},{NOTE_G5,D_QTR},{NOTE_C6,D_QTRD},
  {NOTE_B5,D_EIT},{NOTE_A5,D_EIT},{NOTE_G5,D_QTR},{NOTE_E5,D_QTR},
  {NOTE_F5,D_QTR},{NOTE_A5,D_QTR},{NOTE_C6,D_HLF},
  {NOTE_D5,D_QTR},{NOTE_F5,D_QTR},{NOTE_A5,D_QTR},{NOTE_D6,D_QTRD},
  {NOTE_C6,D_EIT},{NOTE_B5,D_EIT},{NOTE_A5,D_QTR},{NOTE_F5,D_QTR},
  {NOTE_G5,D_QTR},{NOTE_C6,D_QTR},{NOTE_E6,D_HLF},
  {NOTE_C6,D_QTR},{NOTE_G5,D_QTR},{NOTE_C6,D_WHL},
};

const MelodyNote SONG_26[] = {
  {NOTE_C4,D_HLF},{NOTE_G4,D_HLF},{NOTE_C5,D_QTRD},
  {NOTE_REST,D_EIT},{NOTE_D5,D_SXT},{NOTE_C5,D_QTRD},
  {NOTE_REST,D_QTR},{NOTE_G3,D_WHL},
  {NOTE_REST,D_QTR},
  {NOTE_E4,D_HLF},{NOTE_B4,D_HLF},{NOTE_E5,D_QTRD},
  {NOTE_REST,D_EIT},{NOTE_FS5,D_SXT},{NOTE_E5,D_QTRD},
  {NOTE_REST,D_QTR},{NOTE_C4,D_WHL},
};

const MelodyNote SONG_27[] = {
  {NOTE_G4,D_SXT},{NOTE_C5,D_SXT},{NOTE_E5,D_SXT},{NOTE_G5,D_SXT},
  {NOTE_E5,D_SXT},{NOTE_C5,D_SXT},{NOTE_G4,D_QTR},
  {NOTE_A4,D_SXT},{NOTE_D5,D_SXT},{NOTE_F5,D_SXT},{NOTE_A5,D_SXT},
  {NOTE_F5,D_SXT},{NOTE_D5,D_SXT},{NOTE_A4,D_QTR},
  {NOTE_G4,D_EIT},{NOTE_G4,D_EIT},{NOTE_G4,D_QTR},{NOTE_G4,D_HLF},
  {NOTE_C5,D_SXT},{NOTE_E5,D_SXT},{NOTE_G5,D_SXT},{NOTE_C6,D_SXT},
  {NOTE_G5,D_SXT},{NOTE_E5,D_SXT},{NOTE_C5,D_QTR},
  {NOTE_D5,D_EIT},{NOTE_E5,D_EIT},{NOTE_F5,D_EIT},{NOTE_G5,D_EIT},
  {NOTE_G5,D_QTR},{NOTE_G4,D_WHL},
};

const MelodyNote SONG_28[] = {
  {NOTE_D5,D_QTR},{NOTE_F5,D_EIT},{NOTE_G5,D_EIT},{NOTE_A5,D_QTRD},
  {NOTE_AS5,D_EIT},{NOTE_A5,D_QTR},{NOTE_G5,D_EIT},{NOTE_F5,D_QTRD},
  {NOTE_D5,D_EIT},{NOTE_D5,D_QTR},{NOTE_F5,D_QTR},{NOTE_A5,D_QTRD},
  {NOTE_G5,D_EIT},{NOTE_F5,D_QTR},{NOTE_E5,D_HLF},
  {NOTE_A5,D_QTR},{NOTE_G5,D_EIT},{NOTE_F5,D_EIT},{NOTE_E5,D_QTRD},
  {NOTE_D5,D_EIT},{NOTE_E5,D_QTR},{NOTE_F5,D_EIT},{NOTE_G5,D_QTRD},
  {NOTE_A5,D_EIT},{NOTE_A5,D_QTR},{NOTE_G5,D_QTR},{NOTE_F5,D_QTRD},
  {NOTE_E5,D_EIT},{NOTE_D5,D_QTR},{NOTE_D5,D_WHL},
};

const MelodyNote SONG_29[] = {
  {NOTE_C4,D_HLF},{NOTE_E4,D_QTR},{NOTE_G4,D_QTRD},
  {NOTE_C5,D_HLF},{NOTE_G4,D_QTR},{NOTE_E4,D_QTRD},
  {NOTE_F4,D_QTR},{NOTE_A4,D_QTR},{NOTE_C5,D_WHL},
  {NOTE_REST,D_QTR},
  {NOTE_F4,D_HLF},{NOTE_A4,D_QTR},{NOTE_C5,D_QTRD},
  {NOTE_F5,D_HLF},{NOTE_C5,D_QTR},{NOTE_A4,D_QTRD},
  {NOTE_G4,D_QTR},{NOTE_C5,D_QTR},{NOTE_E5,D_QTR},{NOTE_G5,D_WHL},
};

const MelodyNote SONG_30[] = {
  {NOTE_G4,D_EIT},{NOTE_C5,D_EIT},{NOTE_E5,D_EIT},{NOTE_G5,D_QTR},
  {NOTE_E5,D_EIT},{NOTE_G5,D_QTRD},
  {NOTE_REST,D_EIT},
  {NOTE_G4,D_EIT},{NOTE_C5,D_EIT},{NOTE_E5,D_EIT},{NOTE_C6,D_HLF},
  {NOTE_REST,D_EIT},
  {NOTE_A4,D_EIT},{NOTE_D5,D_EIT},{NOTE_F5,D_EIT},{NOTE_A5,D_QTR},
  {NOTE_F5,D_EIT},{NOTE_A5,D_QTRD},
  {NOTE_REST,D_EIT},
  {NOTE_G4,D_EIT},{NOTE_C5,D_EIT},{NOTE_E5,D_EIT},{NOTE_G5,D_EIT},{NOTE_C6,D_EIT},{NOTE_E6,D_QTR},
  {NOTE_C6,D_WHL},
};

// ── Music Arrays ──────────────────────
const MelodyNote *const MUSIC_SONGS[MUSIC_COUNT] = {
  SONG_01, SONG_02, SONG_03, SONG_04, SONG_05, SONG_06, SONG_07, SONG_08,
  SONG_09, SONG_10, SONG_11, SONG_12, SONG_13, SONG_14, SONG_15, SONG_16,
  SONG_17, SONG_18, SONG_19, SONG_20, SONG_21, SONG_22, SONG_23, SONG_24,
  SONG_25, SONG_26, SONG_27, SONG_28, SONG_29, SONG_30,
};

const uint16_t SONG_LENGTHS[MUSIC_COUNT] = {
  sizeof(SONG_01)/sizeof(MelodyNote),
  sizeof(SONG_02)/sizeof(MelodyNote),
  sizeof(SONG_03)/sizeof(MelodyNote),
  sizeof(SONG_04)/sizeof(MelodyNote),
  sizeof(SONG_05)/sizeof(MelodyNote),
  sizeof(SONG_06)/sizeof(MelodyNote),
  sizeof(SONG_07)/sizeof(MelodyNote),
  sizeof(SONG_08)/sizeof(MelodyNote),
  sizeof(SONG_09)/sizeof(MelodyNote),
  sizeof(SONG_10)/sizeof(MelodyNote),
  sizeof(SONG_11)/sizeof(MelodyNote),
  sizeof(SONG_12)/sizeof(MelodyNote),
  sizeof(SONG_13)/sizeof(MelodyNote),
  sizeof(SONG_14)/sizeof(MelodyNote),
  sizeof(SONG_15)/sizeof(MelodyNote),
  sizeof(SONG_16)/sizeof(MelodyNote),
  sizeof(SONG_17)/sizeof(MelodyNote),
  sizeof(SONG_18)/sizeof(MelodyNote),
  sizeof(SONG_19)/sizeof(MelodyNote),
  sizeof(SONG_20)/sizeof(MelodyNote),
  sizeof(SONG_21)/sizeof(MelodyNote),
  sizeof(SONG_22)/sizeof(MelodyNote),
  sizeof(SONG_23)/sizeof(MelodyNote),
  sizeof(SONG_24)/sizeof(MelodyNote),
  sizeof(SONG_25)/sizeof(MelodyNote),
  sizeof(SONG_26)/sizeof(MelodyNote),
  sizeof(SONG_27)/sizeof(MelodyNote),
  sizeof(SONG_28)/sizeof(MelodyNote),
  sizeof(SONG_29)/sizeof(MelodyNote),
  sizeof(SONG_30)/sizeof(MelodyNote),
};

const char* MUSIC_NAMES[MUSIC_COUNT] = {
  "1. Classic Arcade", "2. Tetris Theme", "3. Happy Birthday",
  "4. Joyful Melody", "5. Retro Wave", "6. Deep Bass",
  "7. Funky Beat", "8. Sporty Anthem", "9. Chiptune Classic",
  "10. Alien Invasion", "11. Russian Dance", "12. Mysterious",
  "13. Boxing Fight", "14. Racing Speed", "15. Epic Victory",
  "16. Old Phone Ring 1", "17. Old Phone Ring 2", "18. Classical Piano",
  "19. Rock Guitar", "20. Jazz Club", "21. Christmas Bell",
  "22. Reggae Beat", "23. Electronic Dance", "24. Folk Tune",
  "25. 8-Bit Adventure", "26. Space Odyssey", "27. Circus Theme",
  "28. Medieval Dance", "29. Cinematic Score", "30. Victory Fanfare"
};

// ── Game Variables ────────────────────
uint16_t highScores[GAME_COUNT];
uint16_t totalGamesPlayed[GAME_COUNT];
uint16_t rpsWins[GAME_COUNT];
uint8_t favoriteGames[MAX_FAVORITES];
uint8_t favoriteCount = 0;
uint8_t brightnessLevel = 4;
uint8_t soundLevel = 3;
bool soundEnabled = true;
bool musicPlaying = false;
bool musicPaused = false;
int musicNoteIndex = 0;
uint32_t musicNoteStartTime = 0;
bool musicNotePlaying = false;
int currentMusicVolume = 3;

// ── Timer & Stopwatch Variables ──────
uint32_t timerSetHours = 0;
uint32_t timerSetMinutes = 0;
uint32_t timerSetSeconds = 0;
uint32_t timerRemainingSeconds = 0;
uint32_t timerStartTime = 0;
bool timerRunning = false;
bool timerPaused = false;
bool timerInputMode = false;
int timerInputPos = 0;

uint32_t stopwatchStartTime = 0;
uint32_t stopwatchElapsedMs = 0;
bool stopwatchRunning = false;
bool stopwatchPaused = false;
uint32_t lastStopwatchUpdate = 0;
uint32_t lastBeepTime = 0;

// ── Pomodoro Variables ────────────────
uint32_t pomodoroWorkTime = 25 * 60;
uint32_t pomodoroBreakTime = 5 * 60;
uint32_t pomodoroRemaining = 0;
bool pomodoroRunning = false;
bool pomodoroPaused = false;
bool pomodoroIsWork = true;
uint32_t pomodoroStartTime = 0;
int pomodoroSessions = 0;
bool pomodoroInputMode = false;
int pomodoroInputPos = 0;

// Menu navigation state
int lastGameIndex = 0;

// ── Forward Declarations ──────────────
typedef void (*GameFunction)();

bool btnPressed(uint8_t pin);
bool btnHeld(uint8_t pin);
bool btnLongPressed(uint8_t pin, uint16_t holdTime = 200);
void beep(uint16_t freq, uint16_t ms, uint8_t volume = 3);
void beepNonBlocking(uint16_t freq, uint16_t ms, uint8_t volume = 3);
void playMenuButtonSound();
void waitRelease();
void centreStr(const char *s, uint8_t y);
void drawHeart(int x, int y);
void drawRahulBitmap();
void playStartMusic();
void playGameOverMusic();
void playPauseSound();
void playResumeSound();
void loadHighScores();
void saveHighScore(int gameIndex, uint16_t score);
void saveTotalGames(int gameIndex);
void saveRPSWin(int gameIndex);
bool checkPause(const char* gameName);
bool checkMenuAndReturn();
void gameOverScreen(uint16_t score, int gameIndex, bool isWin);
void showGameSubMenu(const char* gameName, int gameIndex);
int menuSelect();
void showSplash();
void showMainGridMenu();
void showSetupMenu();
void showFavoritesMenu();
void showSettingsMenu();
void showMusicMenu();
void showMediaMenu();
void showImageMenu();
void showVideoMenu();
void toggleFavorite(int gameIndex);
bool isFavorite(int gameIndex);
void saveFavorites();
void loadFavorites();
void saveBrightness();
void loadBrightness();
void saveSoundSetting();
void loadSoundSetting();
void playMusicSong(int songIndex);
void stopMusicPlayer();
void resetDevice();
void showCountdown();
void showPomodoroMenu();
void showDeviceInfo();
void playHappyBirthdayMusic();
void showTimerMenu();
void showStopwatchMenu();
void centreStrBox(const char *s, int boxX, int boxW, int y);

// Game functions
void game_asteroids();
void game_breakout();
void game_dino();
void game_flappy();
void game_snake1();
void game_snake2();
void game_pong();
void game_pacman();
void game_spaceinvaders();
void game_tetris();
void game_tank();
void game_maze();
void game_rps();
void game_car();
void game_car_2lane();
void game_trex();
void game_trex2();
void game_meteor_defenders();
void game_death_star();
void game_tictactoe();
void game_memory_match();
void game_whack_a_mole();
void game_lunar_lander();
void game_color_matching();
void game_ninja_spike();
void game_sperm_race();
void game_frogger();
void game_frogger2(); 
// Tetris helper functions
void loadPiece(struct TetPiece &p, uint8_t t);
bool ttFits(struct TetPiece &p, int dx, int dy);
void ttRotate(struct TetPiece &p);
void showDeviceInfo();
// ============================================================
// UTILITY FUNCTIONS
// ============================================================

bool btnPressed(uint8_t pin) {
  static uint32_t lastTime[8] = {0};
  static bool lastSt[8] = {true, true, true, true, true, true, true, true};
  uint8_t idx;
  if (pin == BTN_UP) idx = 0;
  else if (pin == BTN_DOWN) idx = 1;
  else if (pin == BTN_LEFT) idx = 2;
  else if (pin == BTN_RIGHT) idx = 3;
  else if (pin == BTN_MENU) idx = 4;
  else if (pin == BTN_PAUSE) idx = 5;
  else if (pin == BTN_ENTER) idx = 6;
  else return false;

  bool cur = (digitalRead(pin) == LOW);
  bool edge = cur && !lastSt[idx] && (millis() - lastTime[idx] > 80);   //was 80 
  if (cur != lastSt[idx]) lastTime[idx] = millis();
  lastSt[idx] = cur;
  return edge;
}

bool btnHeld(uint8_t pin) { 
  return digitalRead(pin) == LOW; 
}

bool btnLongPressed(uint8_t pin, uint16_t holdTime) {
  static uint32_t pressStart[8] = {0};
  static bool wasPressed[8] = {false};
  uint8_t idx;
  if (pin == BTN_UP) idx = 0;
  else if (pin == BTN_DOWN) idx = 1;
  else if (pin == BTN_LEFT) idx = 2;
  else if (pin == BTN_RIGHT) idx = 3;
  else if (pin == BTN_MENU) idx = 4;
  else if (pin == BTN_PAUSE) idx = 5;
  else if (pin == BTN_ENTER) idx = 6;
  else return false;

  bool cur = (digitalRead(pin) == LOW);
  
  if (cur && !wasPressed[idx]) {
    pressStart[idx] = millis();
    wasPressed[idx] = true;
  }
  
  if (!cur) {
    wasPressed[idx] = false;
    return false;
  }
  
  if (cur && wasPressed[idx] && (millis() - pressStart[idx] > holdTime)) {
    wasPressed[idx] = false;
    return true;
  }
  
  return false;
}

void drawHeart(int x, int y) {
  u8g2.drawXBMP(x, y, 8, 8, heart_bmp);
}

void drawRahulBitmap() {
  // Create a temporary buffer to hold reversed bitmap data
  static uint8_t reversedBitmap[1024]; // 128*64/8 = 1024 bytes
  static bool initialized = false;
  
  if (!initialized) {
    // Reverse the bits of each byte
    for (int i = 0; i < 1024; i++) {
      uint8_t byte = pgm_read_byte(&RahulBitmap[i]);
      uint8_t reversed = 0;
      for (int j = 0; j < 8; j++) {
        if (byte & (1 << j)) {
          reversed |= (1 << (7 - j));
        }
      }
      reversedBitmap[i] = reversed;
    }
    initialized = true;
  }
  
  // Draw the reversed bitmap
  u8g2.drawXBMP(0, 0, 128, 64, reversedBitmap);
}

void beep(uint16_t freq, uint16_t ms, uint8_t volume) {
  if (!soundEnabled || volume == 0 || freq == 0 || ms == 0) return;
  uint16_t actualMs = ms;
  if (volume == 1) actualMs = ms * 0.6;
  else if (volume == 2) actualMs = ms * 0.8;
  else actualMs = ms;
  
  tone(BUZZER_PIN, freq, actualMs);
  delay(actualMs);
  noTone(BUZZER_PIN);
}

void beepNonBlocking(uint16_t freq, uint16_t ms, uint8_t volume) {
  if (!soundEnabled || volume == 0 || freq == 0 || ms == 0) return;
  uint16_t actualMs = ms;
  if (volume == 1) actualMs = ms * 0.6;
  else if (volume == 2) actualMs = ms * 0.8;
  else actualMs = ms;
  
  uint8_t vol = (currentMusicVolume > 0) ? currentMusicVolume : volume;
  if (vol == 1) actualMs = ms * 0.6;
  else if (vol == 2) actualMs = ms * 0.8;
  else actualMs = ms;
  
  tone(BUZZER_PIN, freq, actualMs);
}

void playMenuButtonSound() {
  if (!soundEnabled) return;
  beep(600, 80, soundLevel);
  delay(80);
  beep(800, 80, soundLevel);
}

void waitRelease() {
  delay(30);
  while (btnHeld(BTN_UP) || btnHeld(BTN_DOWN) || btnHeld(BTN_LEFT) ||
         btnHeld(BTN_RIGHT) || btnHeld(BTN_MENU) || btnHeld(BTN_PAUSE) ||
         btnHeld(BTN_ENTER))
    delay(10);
}

void centreStr(const char *s, uint8_t y) {
  uint8_t w = u8g2.getStrWidth(s);
  u8g2.drawStr((SCREEN_W - w) / 2, y, s);
}

void centreStrBox(const char *s, int boxX, int boxW, int y) {
  uint8_t w = u8g2.getStrWidth(s);
  u8g2.drawStr(boxX + (boxW - w) / 2, y, s);
}

void playStartMusic() {
  if (!soundEnabled) return;
  beep(523, 100, soundLevel); delay(150);
  beep(659, 100, soundLevel); delay(150);
  beep(784, 100, soundLevel); delay(150);
  beep(523, 100, soundLevel); delay(150);
  beep(784, 100, soundLevel); delay(150);
  beep(1046, 200, soundLevel); delay(300);
  beep(880, 100, soundLevel); delay(150);
  beep(1046, 300, soundLevel);
}

void playGameOverMusic() {
  if (!soundEnabled) return;
  beep(392, 150, soundLevel); delay(120);
  beep(349, 150, soundLevel); delay(120);
  beep(329, 150, soundLevel); delay(120);
  beep(261, 300, soundLevel); delay(120);
  beep(196, 500, soundLevel);
}

void playPauseSound() {
  if (!soundEnabled) return;
  beep(500, 80, soundLevel);
  delay(100);
  beep(400, 80, soundLevel);
}

void playResumeSound() {
  if (!soundEnabled) return;
  beep(400, 80, soundLevel);
  delay(100);
  beep(500, 80, soundLevel);
}

void levelCompleteMusic() {
  if (!soundEnabled) return;
  beep(523, 100, soundLevel); delay(120);
  beep(659, 100, soundLevel); delay(120);
  beep(784, 100, soundLevel); delay(120);
  beep(1047, 200, soundLevel);
}

void uniqueGameStartSound(int gameIndex) {
  if (!soundEnabled) return;
  beep(500, 60, soundLevel); delay(60);
  beep(600, 60, soundLevel); delay(60);
  beep(700, 80, soundLevel); delay(60);
  beep(800, 60, soundLevel); delay(60);
  beep(900, 100, soundLevel);
}

// ── Countdown Timer ──
void showCountdown() {
  for (int i = 3; i >= 1; i--) {
    u8g2.setDrawColor(0);
    u8g2.drawBox(40, 20, 48, 30);
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_ncenB18_tr);
    char num[4];
    snprintf(num, sizeof(num), "%d", i);
    centreStr(num, 42);
    
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.setDrawColor(0);
    u8g2.drawBox(44, 50, 40, 10);
    u8g2.setDrawColor(1);
    centreStr("Resume...", 58);
    
    u8g2.sendBuffer();
    beep(800 + (3 - i) * 100, 60, soundLevel);
    delay(1000);
  }
  
  u8g2.setDrawColor(0);
  u8g2.drawBox(35, 15, 58, 35);
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_ncenB18_tr);
  centreStr("GO!", 38);
  u8g2.sendBuffer();
  beep(1200, 100, soundLevel);
  delay(300);
}

// ============================================================
// RESET DEVICE FUNCTION
// ============================================================

void resetDevice() {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < GAME_COUNT; i++) {
    highScores[i] = 0;
    totalGamesPlayed[i] = 0;
    rpsWins[i] = 0;
    EEPROM.put(i * sizeof(uint16_t), (uint16_t)0);
    EEPROM.put((i + GAME_COUNT) * sizeof(uint16_t), (uint16_t)0);
    EEPROM.put((i + GAME_COUNT*2) * sizeof(uint16_t), (uint16_t)0);
  }
  EEPROM.commit();
  EEPROM.end();
  
  favoriteCount = 0;
  for (int i = 0; i < MAX_FAVORITES; i++) {
    favoriteGames[i] = 0;
  }
  saveFavorites();
  
  brightnessLevel = 4;
  soundLevel = 3;
  soundEnabled = true;
  currentMusicVolume = 3;
  saveBrightness();
  saveSoundSetting();
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("RESET", 24);
  centreStr("COMPLETE!", 40);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("All data cleared", 55);
  u8g2.sendBuffer();
  delay(2000);
}

// ============================================================
// MUSIC PLAYER (WITH VOLUME CONTROL)
// ============================================================

void playMusicSong(int songIndex) {
  if (!soundEnabled || songIndex < 0 || songIndex >= MUSIC_COUNT) return;
  if (MUSIC_SONGS[songIndex] == nullptr) return;
  
  const MelodyNote* song = MUSIC_SONGS[songIndex];
  uint16_t songLen = SONG_LENGTHS[songIndex];
  
  uint32_t now = millis();
  
  if (musicNoteIndex >= songLen) {
    musicNoteIndex = 0;
    musicPlaying = false;
    musicNotePlaying = false;
    noTone(BUZZER_PIN);
    return;
  }
  
  if (musicNotePlaying) {
    uint16_t dur = song[musicNoteIndex].duration;
    if (now - musicNoteStartTime >= dur) {
      musicNotePlaying = false;
      musicNoteIndex++;
      noTone(BUZZER_PIN);
    }
    return;
  }
  
  if (musicNoteIndex < songLen) {
    uint16_t freq = song[musicNoteIndex].freq;
    uint16_t dur = song[musicNoteIndex].duration;
    
    if (freq != 0) {
      beepNonBlocking(freq, dur, currentMusicVolume);
    }
    musicNoteStartTime = now;
    musicNotePlaying = true;
    
    if (freq == 0) {
      musicNotePlaying = false;
      musicNoteIndex++;
    }
  }
}

void stopMusicPlayer() {
  musicPlaying = false;
  musicPaused = false;
  musicNoteIndex = 0;
  musicNotePlaying = false;
  noTone(BUZZER_PIN);
}

void playHappyBirthdayMusic() {
  const MelodyNote happyBirthday[] = {
    {NOTE_C4, D_QTR}, {NOTE_C4, D_QTR}, {NOTE_D4, D_QTR}, {NOTE_C4, D_QTR},
    {NOTE_F4, D_QTR}, {NOTE_E4, D_HLF},
    {NOTE_C4, D_QTR}, {NOTE_C4, D_QTR}, {NOTE_D4, D_QTR}, {NOTE_C4, D_QTR},
    {NOTE_G4, D_QTR}, {NOTE_F4, D_HLF},
    {NOTE_C4, D_QTR}, {NOTE_C4, D_QTR}, {NOTE_C5, D_QTR}, {NOTE_A4, D_QTR},
    {NOTE_F4, D_QTR}, {NOTE_E4, D_QTR}, {NOTE_D4, D_QTR},
    {NOTE_AS4, D_QTR}, {NOTE_AS4, D_QTR}, {NOTE_A4, D_QTR}, {NOTE_F4, D_QTR},
    {NOTE_G4, D_QTR}, {NOTE_F4, D_HLF}
  };
  
  const int songLength = sizeof(happyBirthday) / sizeof(MelodyNote);
  int noteIndex = 0;
  uint32_t noteStartTime = 0;
  bool notePlaying = false;
  
  musicPlaying = true;
  musicPaused = false;
  
  while (musicPlaying) {
    uint32_t now = millis();
    
    if (btnPressed(BTN_MENU)) {
      musicPlaying = false;
      noTone(BUZZER_PIN);
      break;
    }
    
    if (!notePlaying) {
      if (noteIndex >= songLength) {
        noteIndex = 0;
      }
      
      uint16_t freq = happyBirthday[noteIndex].freq;
      uint16_t dur = happyBirthday[noteIndex].duration;
      
      if (freq != 0 && soundEnabled) {
        tone(BUZZER_PIN, freq, dur);
        notePlaying = true;
        noteStartTime = now;
      } else {
        noteIndex++;
      }
    } else {
      uint16_t dur = happyBirthday[noteIndex].duration;
      if (now - noteStartTime >= dur) {
        notePlaying = false;
        noteIndex++;
        noTone(BUZZER_PIN);
      }
    }
    
    delay(10);
  }
  
  noTone(BUZZER_PIN);
}

// ============================================================
// TIMER FUNCTION
// ============================================================
void showTimerMenu() {
  timerSetHours = 0;
  timerSetMinutes = 0;
  timerSetSeconds = 0;
  timerRemainingSeconds = 0;
  timerRunning = false;
  timerPaused = false;
  timerInputMode = true;
  timerInputPos = 0;
  timerStartTime = 0;
  
  uint32_t lastHoldTime = 0;
  const uint32_t HOLD_DELAY = 100;
  
  // Music state tracking
  bool musicPlaying = false;
  
  while (true) {
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    
    if (timerInputMode) {
      // â”€â”€ Button Handling â”€â”€
      // FIX #3 (v2): (pos+1)%2 for both buttons gave identical results with only
      // 2 fields, so LEFT/RIGHT looked like they "did the same thing". Now LEFT
      // always jumps to HOUR and RIGHT always jumps to MIN â€” clearly distinct.
      if (btnPressed(BTN_LEFT)) {
        timerInputPos = 0; // HOUR
        beep(600, 10, soundLevel);
        lastHoldTime = 0;
      }
      if (btnPressed(BTN_RIGHT)) {
        timerInputPos = 1; // MIN
        beep(600, 10, soundLevel);
        lastHoldTime = 0;
      }
      
      if (btnHeld(BTN_UP)) {
        if (now - lastHoldTime >= HOLD_DELAY) {
          lastHoldTime = now;
          if (timerInputPos == 0) timerSetHours = (timerSetHours + 1) % 24;
          else if (timerInputPos == 1) timerSetMinutes = (timerSetMinutes + 1) % 60;
          beep(800, 10, soundLevel);
        }
      }
      
      if (btnHeld(BTN_DOWN)) {
        if (now - lastHoldTime >= HOLD_DELAY) {
          lastHoldTime = now;
          if (timerInputPos == 0) timerSetHours = (timerSetHours + 23) % 24;
          else if (timerInputPos == 1) timerSetMinutes = (timerSetMinutes + 59) % 60;
          beep(800, 10, soundLevel);
        }
      }
      
      if (!btnHeld(BTN_UP) && !btnHeld(BTN_DOWN)) {
        lastHoldTime = 0;
      }
      
      if (btnPressed(BTN_ENTER)) {
        if (timerSetHours > 0 || timerSetMinutes > 0) {
          timerRemainingSeconds = timerSetHours * 3600 + timerSetMinutes * 60;
          timerStartTime = now;
          timerRunning = true;
          timerPaused = false;
          timerInputMode = false;
          beep(1000, 30, soundLevel);
          waitRelease();
          delay(50);
          continue; // FIX #5: skip the "SET TIMER" redraw below this same frame,
                     // go straight to the running-mode branch next iteration.
        }
      }
      
      // â”€â”€ DRAW INPUT â”€â”€
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawBox(0, 0, SCREEN_W, 11);
      u8g2.setDrawColor(0);
      centreStr("SET TIMER", 9);
      u8g2.setDrawColor(1);
      
      u8g2.setFont(u8g2_font_6x10_tr);
      centreStr(" HOUR      MIN ", 26);

      u8g2.setFont(u8g2_font_ncenB24_tr);
      
      char hoursStr[3];
      char minutesStr[3];
      snprintf(hoursStr, sizeof(hoursStr), "%02d", timerSetHours);
      snprintf(minutesStr, sizeof(minutesStr), "%02d", timerSetMinutes);
      
      int hoursX = 20;
      int minutesX = 76;
      int yPos = 59;
      
      u8g2.drawStr(hoursX, yPos, hoursStr);
      u8g2.drawStr(hoursX + 32, yPos-3, " : ");
      u8g2.drawStr(minutesX, yPos, minutesStr);
      
      int cursorX = 0;
      int cursorY = 33;
      int cursorW = 40;
      int cursorH = 30;

      if (timerInputPos == 0) {
        cursorX = hoursX - 1;
      } else if (timerInputPos == 1) {
        cursorX = minutesX - 1;
      }
      
      if ((now / 300) % 2 == 0) {
        u8g2.drawFrame(cursorX, cursorY, cursorW, cursorH);
      }
      
      u8g2.sendBuffer();
    }
    else {
      // â”€â”€ RUNNING MODE â”€â”€
      if (timerRunning && !timerPaused) {
        uint32_t totalSeconds = timerSetHours * 3600 + timerSetMinutes * 60;
        uint32_t elapsed = (now - timerStartTime) / 1000;
        
        if (elapsed >= totalSeconds) {
          timerRunning = false;
          
          // FIX #6: draw the TIME'S UP screen FIRST, before starting the music.
          // playHappyBirthdayMusic() appears to block until the tune finishes,
          // so calling it first left the screen frozen on the old countdown
          // frame for the whole song â€” it only looked "fixed" once a button
          // press happened to line up with the music finishing.
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_ncenB18_tr);
          centreStr("TIME UP!", 32);
          u8g2.setFont(u8g2_font_6x10_tr);
          centreStr("Press MENU to stop", 55);
          u8g2.sendBuffer();
          
          // ðŸ”¥ Start music only if not already playing
          if (!musicPlaying) {
            playHappyBirthdayMusic();
            musicPlaying = true;
          }
          
          // ðŸ”¥ Show TIME'S UP! screen with music playing
          bool timeUpActive = true;
          while (timeUpActive) {
            // FIX #1: honor the global menu/long-press handler even inside
            // this nested loop, so the physical MENU-button long-press exit
            // still works instead of only the ad-hoc BTN_MENU check below.
            if (checkMenuAndReturn()) {
              stopMusicPlayer();
              musicPlaying = false;
              return;
            }

            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_ncenB18_tr);
            centreStr("TIME'S UP!", 32);
            u8g2.setFont(u8g2_font_6x10_tr);
            centreStr("Press MENU to stop", 55);
            u8g2.sendBuffer();
            
            // ðŸ”¥ Check for MENU button
            if (btnPressed(BTN_MENU)) {
              stopMusicPlayer();
              musicPlaying = false;
              timerRunning = false;
              playMenuButtonSound();
              timeUpActive = false;
              timerInputMode = true;
              // FIX #4: clear the previous set time so the next SET TIMER
              // screen starts fresh at 00:00 instead of showing stale values.
              timerSetHours = 0;
              timerSetMinutes = 0;
              waitRelease();
              break;
            }
            
            // ðŸ”¥ Check for ENTER button
            if (btnPressed(BTN_ENTER)) {
              stopMusicPlayer();
              musicPlaying = false;
              timerRunning = false;
              // FIX #2: give ENTER the same audio feedback as MENU for
              // consistent UX, since both dismiss the alarm.
              playMenuButtonSound();
              timeUpActive = false;
              timerInputMode = true;
              timerSetHours = 0;
              timerSetMinutes = 0;
              waitRelease();
              break;
            }
            
            delay(50);
          }
          continue;  // ðŸ”¥ Go back to main loop
        }
        
        timerRemainingSeconds = totalSeconds - elapsed;
        
        if (now - lastBeepTime >= 1000) {
          lastBeepTime = now;
          beep(800, 20, soundLevel);
        }
      }
      
      if (btnPressed(BTN_PAUSE) && timerRunning) {
        timerPaused = !timerPaused;
        if (timerPaused) {
          playPauseSound();
          timerRemainingSeconds = timerSetHours * 3600 + timerSetMinutes * 60 - (now - timerStartTime) / 1000;
        } else {
          playResumeSound();
          timerStartTime = now - (timerSetHours * 3600 + timerSetMinutes * 60 - timerRemainingSeconds) * 1000;
        }
        waitRelease();
      }
      
      // â”€â”€ DRAW RUNNING â”€â”€
      u8g2.clearBuffer();
      
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawBox(0, 0, SCREEN_W, 11);
      u8g2.setDrawColor(0);
      centreStr("TIMER", 9);
      u8g2.setDrawColor(1);
      
      uint32_t hours = timerRemainingSeconds / 3600;
      uint32_t minutes = (timerRemainingSeconds % 3600) / 60;
      uint32_t seconds = timerRemainingSeconds % 60;
      
      char remStr[20];
      snprintf(remStr, sizeof(remStr), "%02d:%02d:%02d", hours, minutes, seconds);
      
      u8g2.setFont(u8g2_font_ncenB24_tr);
      centreStr(remStr, 45);
      
      if (timerPaused) {
        u8g2.setFont(u8g2_font_6x10_tr);
        centreStr("PAUSED", 61);
      }
      
      u8g2.sendBuffer();
    }
    delay(50);
  }
}

// void showTimerMenu() {
//   timerSetHours = 0;
//   timerSetMinutes = 0;
//   timerSetSeconds = 0;
//   timerRemainingSeconds = 0;
//   timerRunning = false;
//   timerPaused = false;
//   timerInputMode = true;
//   timerInputPos = 0;
//   timerStartTime = 0;
  
//   // Hold detection
//   uint32_t lastHoldTime = 0;
//   const uint32_t HOLD_DELAY = 100;
  
//   while (true) {
//     if (checkMenuAndReturn()) return;
    
//     uint32_t now = millis();
    
//     if (timerInputMode) {
//       // ── Button Handling ──
//       if (btnPressed(BTN_LEFT)) {
//         timerInputPos = (timerInputPos + 1) % 2;
//         beep(600, 10, soundLevel);
//         lastHoldTime = 0;
//       }
//       if (btnPressed(BTN_RIGHT)) {
//         timerInputPos = (timerInputPos + 1) % 2;
//         beep(600, 10, soundLevel);
//         lastHoldTime = 0;
//       }
      
//       if (btnHeld(BTN_UP)) {
//         if (now - lastHoldTime >= HOLD_DELAY) {
//           lastHoldTime = now;
//           if (timerInputPos == 0) timerSetHours = (timerSetHours + 1) % 24;
//           else if (timerInputPos == 1) timerSetMinutes = (timerSetMinutes + 1) % 60;
//           beep(800, 10, soundLevel);
//         }
//       }
      
//       if (btnHeld(BTN_DOWN)) {
//         if (now - lastHoldTime >= HOLD_DELAY) {
//           lastHoldTime = now;
//           if (timerInputPos == 0) timerSetHours = (timerSetHours + 23) % 24;
//           else if (timerInputPos == 1) timerSetMinutes = (timerSetMinutes + 59) % 60;
//           beep(800, 10, soundLevel);
//         }
//       }
      
//       if (!btnHeld(BTN_UP) && !btnHeld(BTN_DOWN)) {
//         lastHoldTime = 0;
//       }
      
//       if (btnPressed(BTN_ENTER)) {
//         if (timerSetHours > 0 || timerSetMinutes > 0) {
//           timerRemainingSeconds = timerSetHours * 3600 + timerSetMinutes * 60;
//           timerStartTime = now;
//           timerRunning = true;
//           timerPaused = false;
//           timerInputMode = false;
//           beep(1000, 30, soundLevel);
//           waitRelease();
//         }
//       }
      
//       // ── DRAW INPUT ──
//       u8g2.clearBuffer();
//     u8g2.setFont(u8g2_font_ncenB08_tr);
//     u8g2.drawBox(0, 0, SCREEN_W, 11);
//     u8g2.setDrawColor(0);
//     centreStr("SET TIMER", 9);
//     u8g2.setDrawColor(1);
      
//       u8g2.setFont(u8g2_font_6x10_tr);
//       centreStr(" HOUR      MIN ", 26);


//       u8g2.setFont(u8g2_font_ncenB24_tr);
      
//       char hoursStr[3];
//       char minutesStr[3];
//       snprintf(hoursStr, sizeof(hoursStr), "%02d", timerSetHours);
//       snprintf(minutesStr, sizeof(minutesStr), "%02d", timerSetMinutes);
      
//       int hoursX = 20;
//       int minutesX = 76;
//       int yPos = 59;
      
//       u8g2.drawStr(hoursX, yPos, hoursStr);
//       u8g2.drawStr(hoursX + 32, yPos-3, " : ");
//       u8g2.drawStr(minutesX, yPos, minutesStr);

      
      
// int cursorX = 0;      // ফ্রেমের X পজিশন (বাম দিক থেকে)
// int cursorY = 33;     // ফ্রেমের Y পজিশন (উপর থেকে) - 2 পিক্সেল উপরে
// int cursorW = 40;     // প্রস্থ (width) - 34+2 = 36
// int cursorH = 30;

//       if (timerInputPos == 0) {
//         cursorX = hoursX - 1;
//       } else if (timerInputPos == 1) {
//         cursorX = minutesX - 1;
//       }
      
//       if ((now / 300) % 2 == 0) {
//         u8g2.drawFrame(cursorX, cursorY, cursorW, cursorH);
//       }
      
//       u8g2.sendBuffer();
//     }
//     else {
//       // ── RUNNING MODE ──
//       if (timerRunning && !timerPaused) {
//         // 🔥 FIXED: Calculate remaining time correctly
//         uint32_t totalSeconds = timerSetHours * 3600 + timerSetMinutes * 60;
//         uint32_t elapsed = (now - timerStartTime) / 1000;
        
//         // 🔥 FIXED: Check if time is up
//         if (elapsed >= totalSeconds) {
//           timerRunning = false;
//           playHappyBirthdayMusic();
//           u8g2.clearBuffer();
//           u8g2.setFont(u8g2_font_ncenB18_tr);
//           centreStr("TIME'S UP!", 32);
//           u8g2.setFont(u8g2_font_6x10_tr);
//           centreStr("Press MENU to stop", 55);
//           u8g2.sendBuffer();
          
//           while (true) {
//             if (btnPressed(BTN_MENU)) {
//               stopMusicPlayer();
//               timerRunning = false;
//               playMenuButtonSound();
//               break;
//             }
//             if (btnPressed(BTN_ENTER)) {
//               timerInputMode = true;
//               timerRunning = false;
//               stopMusicPlayer();
//               break;
//             }
//             delay(50);
//           }
//           waitRelease();
//           return;
//         }
        
//         // 🔥 FIXED: Calculate remaining time
//         timerRemainingSeconds = totalSeconds - elapsed;
        
//         if (now - lastBeepTime >= 1000) {
//           lastBeepTime = now;
//           beep(800, 20, soundLevel);
//         }
//       }
      
//       if (btnPressed(BTN_PAUSE) && timerRunning) {
//         timerPaused = !timerPaused;
//         if (timerPaused) {
//           playPauseSound();
//           // 🔥 FIXED: Store remaining time when paused
//           timerRemainingSeconds = timerSetHours * 3600 + timerSetMinutes * 60 - (now - timerStartTime) / 1000;
//         } else {
//           playResumeSound();
//           // 🔥 FIXED: Resume from remaining time
//           timerStartTime = now - (timerSetHours * 3600 + timerSetMinutes * 60 - timerRemainingSeconds) * 1000;
//         }
//         waitRelease();
//       }
      
//       // ── DRAW RUNNING ──
//       u8g2.clearBuffer();
      

//     u8g2.setFont(u8g2_font_ncenB08_tr);
//     u8g2.drawBox(0, 0, SCREEN_W, 11);
//     u8g2.setDrawColor(0);
//     centreStr("TIMER", 9);
//     u8g2.setDrawColor(1);
      
//       uint32_t hours = timerRemainingSeconds / 3600;
//       uint32_t minutes = (timerRemainingSeconds % 3600) / 60;
//       uint32_t seconds = timerRemainingSeconds % 60;
      
//       char remStr[20];
//       snprintf(remStr, sizeof(remStr), "%02d:%02d:%02d", hours, minutes, seconds);
      
//       u8g2.setFont(u8g2_font_ncenB24_tr);
//       centreStr(remStr, 43);
      
//       if (timerPaused) {
//         u8g2.setFont(u8g2_font_6x10_tr);
//         centreStr("PAUSED", 60);
//       }
      
//       u8g2.sendBuffer();
//     }
//     delay(50);
//   }
// }

// ============================================================
// STOPWATCH FUNCTION
// ============================================================

void showStopwatchMenu() {
  stopwatchStartTime = 0;
  stopwatchElapsedMs = 0;
  stopwatchRunning = false;
  stopwatchPaused = false;
  lastStopwatchUpdate = 0;
  lastBeepTime = 0;
  
  while (true) {
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    
    if (btnPressed(BTN_ENTER)) {
      if (!stopwatchRunning) {
        if (stopwatchPaused) {
          stopwatchStartTime = now - stopwatchElapsedMs;
          stopwatchPaused = false;
          stopwatchRunning = true;
          playResumeSound();
        } else {
          stopwatchElapsedMs = 0;
          stopwatchStartTime = now;
          stopwatchRunning = true;
          stopwatchPaused = false;
          beep(1700, 50, soundLevel);
        }
      } else {
        stopwatchRunning = false;
        stopwatchPaused = true;
        stopwatchElapsedMs = now - stopwatchStartTime;
        playPauseSound();
      }
      waitRelease();
    }
    
    if (btnLongPressed(BTN_MENU, 500)) {
      stopwatchRunning = false;
      stopwatchPaused = false;
      stopwatchElapsedMs = 0;
      playMenuButtonSound();
      waitRelease();
    }
    
    if (stopwatchRunning && !stopwatchPaused) {
      stopwatchElapsedMs = now - stopwatchStartTime;
      
      uint32_t currentSec = stopwatchElapsedMs / 1000;
      if (currentSec > 0 && (now - lastBeepTime) >= 1000) {
        lastBeepTime = now;
        beep(600, 15, soundLevel);
      }
    }
    
    uint32_t totalMs = stopwatchElapsedMs;
    uint32_t hours = totalMs / 3600000;
    uint32_t minutes = (totalMs % 3600000) / 60000;
    uint32_t seconds = (totalMs % 60000) / 1000;
    uint32_t millis = totalMs % 1000;
    
    u8g2.clearBuffer();
    
    u8g2.setFont(u8g2_font_ncenB08_tr);
    centreStr("STOPWATCH", 10);
    
    char timeStr[20];
    snprintf(timeStr, sizeof(timeStr), "%01d:%02d:%02d", hours, minutes, seconds);
    u8g2.setFont(u8g2_font_ncenB24_tr);
    centreStr(timeStr, 43);
    
    u8g2.setFont(u8g2_font_ncenB08_tr);
    char msStr[10];
    snprintf(msStr, sizeof(msStr), "0.%03dms", millis);
    int msWidth = u8g2.getStrWidth(msStr);
    u8g2.drawStr((SCREEN_W - msWidth) / 2 + 30, 58, msStr);
    
    u8g2.sendBuffer();
    delay(20);
  }
}

// ============================================================
// POMODORO TIMER
// ============================================================

void showPomodoroMenu() {
  pomodoroWorkTime = 25 * 60;
  pomodoroBreakTime = 5 * 60;
  pomodoroRemaining = pomodoroWorkTime;
  pomodoroRunning = false;
  pomodoroPaused = false;
  pomodoroIsWork = true;
  pomodoroStartTime = 0;
  pomodoroSessions = 0;
  pomodoroInputMode = true;
  pomodoroInputPos = 0;
  
  while (true) {
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    
    if (pomodoroInputMode) {
      if (btnPressed(BTN_LEFT)) {
        pomodoroInputPos = (pomodoroInputPos + 1) % 2;
        beep(600, 10, soundLevel);
      }
      if (btnPressed(BTN_RIGHT)) {
        pomodoroInputPos = (pomodoroInputPos + 1) % 2;
        beep(600, 10, soundLevel);
      }
      
      if (btnPressed(BTN_UP)) {
        if (pomodoroInputPos == 0) {
          pomodoroWorkTime = min((uint32_t)3600, pomodoroWorkTime + 60);
        } else {
          pomodoroBreakTime = min((uint32_t)1800, pomodoroBreakTime + 60);
        }
        beep(800, 10, soundLevel);
      }
      if (btnPressed(BTN_DOWN)) {
        if (pomodoroInputPos == 0) {
          pomodoroWorkTime = max((uint32_t)60, pomodoroWorkTime - 60);
        } else {
          pomodoroBreakTime = max((uint32_t)60, pomodoroBreakTime - 60);
        }
        beep(800, 10, soundLevel);
      }
      
      if (btnPressed(BTN_ENTER)) {
        if (pomodoroWorkTime > 0 && pomodoroBreakTime > 0) {
          pomodoroRemaining = pomodoroWorkTime;
          pomodoroIsWork = true;
          pomodoroRunning = true;
          pomodoroPaused = false;
          pomodoroStartTime = now;
          pomodoroInputMode = false;
          beep(1000, 30, soundLevel);
          waitRelease();
        }
      }
      
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);
      centreStr("POMODORO", 12);
      
      u8g2.setFont(u8g2_font_6x10_tr);
      char workStr[20];
      snprintf(workStr, sizeof(workStr), "Work: %02d:%02d", pomodoroWorkTime / 60, pomodoroWorkTime % 60);
      centreStr(workStr, 30);
      
      char breakStr[20];
      snprintf(breakStr, sizeof(breakStr), "Break: %02d:%02d", pomodoroBreakTime / 60, pomodoroBreakTime % 60);
      centreStr(breakStr, 42);
      
      int cursorX = (pomodoroInputPos == 0) ? 20 : 20;
      int cursorY = (pomodoroInputPos == 0) ? 22 : 34;
      if ((now / 300) % 2 == 0) {
        u8g2.drawFrame(cursorX, cursorY, 88, 10);
      }
      
      u8g2.setFont(u8g2_font_5x7_tr);
      centreStr("L/R=Select  UP/DOWN=Set", 56);
      centreStr("ENTER=Start  MENU=Back", 64);
      u8g2.sendBuffer();
    }
    else {
      if (pomodoroRunning && !pomodoroPaused) {
        uint32_t elapsed = (now - pomodoroStartTime);
        uint32_t elapsedSeconds = elapsed / 1000;
        
        if (elapsedSeconds >= pomodoroRemaining) {
          pomodoroRunning = false;
          beep(2000, 200, soundLevel);
          delay(200);
          beep(2000, 200, soundLevel);
          
          if (pomodoroIsWork) {
            pomodoroSessions++;
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_ncenB18_tr);
            centreStr("WORK DONE!", 28);
            u8g2.setFont(u8g2_font_6x10_tr);
            char sessStr[20];
            snprintf(sessStr, sizeof(sessStr), "Sessions: %d", pomodoroSessions);
            centreStr(sessStr, 44);
            u8g2.setFont(u8g2_font_5x7_tr);
            centreStr("Press ENTER for break", 56);
            u8g2.sendBuffer();
            
            while (true) {
              if (btnPressed(BTN_ENTER)) {
                pomodoroRemaining = pomodoroBreakTime;
                pomodoroIsWork = false;
                pomodoroRunning = true;
                pomodoroPaused = false;
                pomodoroStartTime = now;
                beep(1000, 50, soundLevel);
                break;
              }
              if (btnPressed(BTN_MENU)) {
                pomodoroRunning = false;
                pomodoroInputMode = true;
                playMenuButtonSound();
                break;
              }
              delay(50);
            }
          } else {
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_ncenB18_tr);
            centreStr("BREAK OVER!", 28);
            u8g2.setFont(u8g2_font_6x10_tr);
            centreStr("Ready to work again?", 44);
            u8g2.setFont(u8g2_font_5x7_tr);
            centreStr("Press ENTER to start", 56);
            u8g2.sendBuffer();
            
            while (true) {
              if (btnPressed(BTN_ENTER)) {
                pomodoroRemaining = pomodoroWorkTime;
                pomodoroIsWork = true;
                pomodoroRunning = true;
                pomodoroPaused = false;
                pomodoroStartTime = now;
                beep(1000, 50, soundLevel);
                break;
              }
              if (btnPressed(BTN_MENU)) {
                pomodoroRunning = false;
                pomodoroInputMode = true;
                playMenuButtonSound();
                break;
              }
              delay(50);
            }
          }
          continue;
        }
        
        pomodoroRemaining = (pomodoroIsWork ? pomodoroWorkTime : pomodoroBreakTime) - elapsedSeconds;
        
        if (now - lastBeepTime >= 1000) {
          lastBeepTime = now;
          if (pomodoroRemaining <= 10) {
            beep(1000, 30, soundLevel);
          }
        }
      }
      
      if (btnPressed(BTN_PAUSE) && pomodoroRunning) {
        pomodoroPaused = !pomodoroPaused;
        if (pomodoroPaused) {
          playPauseSound();
        } else {
          playResumeSound();
          pomodoroStartTime = now - (pomodoroRemaining) * 1000;
        }
        waitRelease();
      }
      
      u8g2.clearBuffer();
      
      u8g2.setFont(u8g2_font_6x10_tr);
      if (pomodoroIsWork) {
        centreStr("WORKING...", 10);
      } else {
        centreStr("BREAK TIME!", 10);
      }
      
      uint32_t minutes = pomodoroRemaining / 60;
      uint32_t seconds = pomodoroRemaining % 60;
      
      char timeStr[20];
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", minutes, seconds);
      u8g2.setFont(u8g2_font_ncenB24_tr);
      centreStr(timeStr, 40);
      
      u8g2.setFont(u8g2_font_5x7_tr);
      char sessStr[20];
      snprintf(sessStr, sizeof(sessStr), "Sessions: %d", pomodoroSessions);
      centreStr(sessStr, 52);
      
      if (pomodoroPaused) {
        centreStr("** PAUSED **", 62);
      }
      
      u8g2.sendBuffer();
    }
    delay(50);
  }
}

// ============================================================
// DEVICE INFO
// ============================================================

void showDeviceInfo() {
  // ── Step 1: "WANNA KNOW" types first ──
  const char* line1 = "WANNA KNOW";
  const char* line2 = "THE MAKER?";
  int len1 = strlen(line1);
  int len2 = strlen(line2);

    for (int i = 0; i <= len1; i++) {
    u8g2.clearBuffer();
    
    // Upper line: "WANNA KNOW" typing
    u8g2.setFont(u8g2_font_ncenB10_tr);
    char display1[20];
    strncpy(display1, line1, i);
    display1[i] = '\0';
    centreStr(display1, 25);
    
    // Lower line: empty for now
    u8g2.setFont(u8g2_font_ncenB10_tr);
    centreStr("", 45);
    
    u8g2.sendBuffer();
    delay(80);
    if (soundEnabled && i % 2 == 0) {
      beep(600 + i * 5, 5, soundLevel);
    }
  }
  
  // Second: Type "THE MAKER" (lower line) while "WANNA KNOW" stays
  for (int i = 0; i <= len2; i++) {
    u8g2.clearBuffer();
    
    // Upper line: "WANNA KNOW" (already complete)
    u8g2.setFont(u8g2_font_ncenB08_tr);
    centreStr(line1, 25);
    
    // Lower line: "THE MAKER" typing
    u8g2.setFont(u8g2_font_ncenB10_tr);
    char display2[20];
    strncpy(display2, line2, i);
    display2[i] = '\0';
    centreStr(display2, 45);
    
    u8g2.sendBuffer();
    delay(80);
    if (soundEnabled && i % 2 == 0) {
      beep(600 + i * 5, 5, soundLevel);
    }
  }
  
  delay(1000);
  
 const char* line3 = "HE IS NONE";
const char* line4 = "BUT....";
int len3 = strlen(line3);
int len4 = strlen(line4);

// First: Type "HE IS NONE" (upper line)
for (int i = 0; i <= len3; i++) {
  u8g2.clearBuffer();
  
  // Upper line: "HE IS NONE" typing
  u8g2.setFont(u8g2_font_ncenB10_tr);
  char display1[20];
  strncpy(display1, line3, i);
  display1[i] = '\0';
  centreStr(display1, 25);
  
  // Lower line: empty for now
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("", 45);
  
  u8g2.sendBuffer();
  delay(80);
  if (soundEnabled && i % 2 == 0) {
    beep(600 + i * 5, 5, soundLevel);
  }
}

delay(300);

// Second: Type "BUT....." (lower line) while "HE IS NONE" stays
for (int i = 0; i <= len4; i++) {
  u8g2.clearBuffer();
  
  // Upper line: "HE IS NONE" (already complete)
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr(line3, 25);
  
  // Lower line: "BUT....." typing
  u8g2.setFont(u8g2_font_ncenB10_tr);
  char display2[20];
  strncpy(display2, line4, i);
  display2[i] = '\0';
  centreStr(display2, 45);
  
  u8g2.sendBuffer();
  delay(80);
  if (soundEnabled && i % 2 == 0) {
    beep(600 + i * 5, 5, soundLevel);
  }
}

delay(1000);





  
  // ── Step 3: Fire effect on RAHUL with THE PHOENIX ──
  for (int fire = 0; fire < 15; fire++) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB24_tr);
    
    // Random vibration for fire effect
    int offsetX = random(-2, 2);
    int offsetY = random(-1, 1);
    
    char text[20];
    snprintf(text, sizeof(text), "RAHUL");
    int w = u8g2.getStrWidth(text);
    u8g2.drawStr((SCREEN_W - w) / 2 + offsetX, 30 + offsetY, text);
    
    u8g2.setFont(u8g2_font_8x13_tr);
    centreStr("THE PHOENIX", 49);
    
    u8g2.sendBuffer();
    delay(60);
    beep(500 + fire * 30, 10, soundLevel);
  }
  
  delay(500);
  
  // ── Step 4: "Wanna see him?" ──
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  centreStr("WANNA SEE HIM?", 30);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Then Press ENTER", 50);
  u8g2.sendBuffer();
  
  // Wait for ENTER button only
  while (true) {
    if (checkMenuAndReturn()) return;
    
    if (btnPressed(BTN_ENTER)) {
      beep(1000, 30, soundLevel);
      waitRelease();
      break;
    }
    if (btnPressed(BTN_MENU)) {
      playMenuButtonSound();
      waitRelease();
      return;
    }
    delay(50);
  }
  
  // ── Step 5: Countdown 3-2-1 with BIGGER numbers (B16 font) ──
  for (int i = 3; i >= 1; i--) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso32_tr );  // B24 is bigger than B16
    char num[3];
    snprintf(num, sizeof(num), "%d", i);
    centreStr(num, 52);
    u8g2.sendBuffer();
    beep(800 + (3 - i) * 100, 60, soundLevel);
    delay(800);
  }
  
  // ── Step 6: "LETS GO!" ──
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB24_tr);
  centreStr("LETS", 25);
  centreStr("GO!", 55);
  u8g2.sendBuffer();
  beep(1200, 120, soundLevel);
  delay(800);
  
  // ── Step 7: Show Rahul's picture ──
  u8g2.clearBuffer();
  drawRahulBitmap();
  u8g2.sendBuffer();
  
  // Wait for user to exit
  while (true) {
    if (checkMenuAndReturn()) return;
    
    if (btnPressed(BTN_MENU) || btnPressed(BTN_ENTER)) {
      playMenuButtonSound();
      waitRelease();
      return;
    }
    delay(50);
  }
}

// ============================================================
// FAVORITES MANAGEMENT
// ============================================================

void loadFavorites() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(400, favoriteCount);
  if (favoriteCount > MAX_FAVORITES) favoriteCount = 0;
  for (int i = 0; i < favoriteCount; i++) {
    EEPROM.get(401 + i, favoriteGames[i]);
    if (favoriteGames[i] >= GAME_COUNT) favoriteCount = 0;
  }
  EEPROM.end();
}

void saveFavorites() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(400, favoriteCount);
  for (int i = 0; i < favoriteCount; i++) {
    EEPROM.put(401 + i, favoriteGames[i]);
  }
  EEPROM.commit();
  EEPROM.end();
}

bool isFavorite(int gameIndex) {
  for (int i = 0; i < favoriteCount; i++) {
    if (favoriteGames[i] == gameIndex) return true;
  }
  return false;
}

void toggleFavorite(int gameIndex) {
  if (isFavorite(gameIndex)) {
    for (int i = 0; i < favoriteCount; i++) {
      if (favoriteGames[i] == gameIndex) {
        for (int j = i; j < favoriteCount - 1; j++) {
          favoriteGames[j] = favoriteGames[j + 1];
        }
        favoriteCount--;
        break;
      }
    }
  } else {
    if (favoriteCount < MAX_FAVORITES) {
      favoriteGames[favoriteCount++] = gameIndex;
    }
  }
  saveFavorites();
}

// ============================================================
// SETTINGS MANAGEMENT
// ============================================================

void loadBrightness() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(450, brightnessLevel);
  if (brightnessLevel < 1 || brightnessLevel > 7) brightnessLevel = 4;
  uint8_t contrast = map(brightnessLevel, 1, 7, 30, 255);
  u8g2.setContrast(contrast);
  EEPROM.end();
}

void saveBrightness() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(450, brightnessLevel);
  EEPROM.commit();
  EEPROM.end();
  uint8_t contrast = map(brightnessLevel, 1, 7, 30, 255);
  u8g2.setContrast(contrast);
}

void loadSoundSetting() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(460, soundLevel);
  if (soundLevel > 3) soundLevel = 3;
  soundEnabled = (soundLevel > 0);
  currentMusicVolume = soundLevel;
  EEPROM.end();
}

void saveSoundSetting() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(460, soundLevel);
  EEPROM.commit();
  EEPROM.end();
  soundEnabled = (soundLevel > 0);
  currentMusicVolume = soundLevel;
}

// ============================================================
// HIGH SCORE MANAGEMENT
// ============================================================

void loadHighScores() {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < GAME_COUNT; i++) {
    uint16_t temp;
    EEPROM.get(i * sizeof(uint16_t), temp);
    if (temp > 9999 || temp == 0xFFFF) temp = 0;
    highScores[i] = temp;
    
    EEPROM.get((i + GAME_COUNT) * sizeof(uint16_t), temp);
    if (temp > 9999 || temp == 0xFFFF) temp = 0;
    totalGamesPlayed[i] = temp;
    
    EEPROM.get((i + GAME_COUNT*2) * sizeof(uint16_t), temp);
    if (temp > 9999 || temp == 0xFFFF) temp = 0;
    rpsWins[i] = temp;
  }
  EEPROM.end();
  loadFavorites();
  loadBrightness();
  loadSoundSetting();
}

void saveTotalGames(int gameIndex) {
  if (gameIndex != 12) return;
  totalGamesPlayed[gameIndex]++;
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put((gameIndex + GAME_COUNT) * sizeof(uint16_t), totalGamesPlayed[gameIndex]);
  EEPROM.commit();
  EEPROM.end();
}

void saveRPSWin(int gameIndex) {
  if (gameIndex != 12) return;
  rpsWins[gameIndex]++;
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put((gameIndex + GAME_COUNT*2) * sizeof(uint16_t), rpsWins[gameIndex]);
  EEPROM.commit();
  EEPROM.end();
}

void saveHighScore(int gameIndex, uint16_t score) {
  if (gameIndex == 12) {
    uint16_t winRate = 0;
    if (totalGamesPlayed[gameIndex] > 0) {
      winRate = (rpsWins[gameIndex] * 100) / totalGamesPlayed[gameIndex];
    }
    if (winRate > highScores[gameIndex]) {
      highScores[gameIndex] = winRate;
      EEPROM.begin(EEPROM_SIZE);
      EEPROM.put(gameIndex * sizeof(uint16_t), highScores[gameIndex]);
      EEPROM.commit();
      EEPROM.end();
      if (soundEnabled) {
        beep(1200, 40, soundLevel);
        delay(60);
        beep(1500, 40, soundLevel);
        delay(60);
        beep(1800, 80, soundLevel);
      }
    }
  } else {
    if (score > highScores[gameIndex]) {
      highScores[gameIndex] = score;
      EEPROM.begin(EEPROM_SIZE);
      EEPROM.put(gameIndex * sizeof(uint16_t), highScores[gameIndex]);
      EEPROM.commit();
      EEPROM.end();
      if (soundEnabled) {
        beep(1200, 40, soundLevel);
        delay(60);
        beep(1500, 40, soundLevel);
        delay(60);
        beep(1800, 80, soundLevel);
      }
    }
  }
}

// ============================================================
// PAUSE & MENU FUNCTIONS
// ============================================================

volatile bool menuPressed = false;
volatile bool gamePaused = false;
String currentGameName = "";
int currentGameIndex = 0;

bool checkPause(const char* gameName) {
  currentGameName = String(gameName);
  
  if (btnPressed(BTN_PAUSE)) {
    gamePaused = !gamePaused;
    if (gamePaused) {
      playPauseSound();
      
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);
      centreStr("** PAUSED **", 20);
      u8g2.setFont(u8g2_font_ncenB08_tr);
      char pauseMsg[30];
      snprintf(pauseMsg, sizeof(pauseMsg), "Game: %s", gameName);
      centreStr(pauseMsg, 38);
      u8g2.setFont(u8g2_font_6x10_tr);
      centreStr("PAUSE to resume", 52);
      centreStr("MENU to quit", 60);
      u8g2.sendBuffer();
      
      while (gamePaused) {
        if (btnPressed(BTN_PAUSE)) {
          gamePaused = false;
          playResumeSound();
          showCountdown();
          return false;
        }
        if (btnPressed(BTN_MENU)) {
          gamePaused = false;
          playMenuButtonSound();
          menuPressed = true;
          return true;
        }
        delay(50);
      }
      return false;
    } else {
      playResumeSound();
      showCountdown();
    }
  }
  return false;
}

bool checkMenuAndReturn() {
  if (btnPressed(BTN_MENU)) {
    playMenuButtonSound();
    menuPressed = true;
    return true;
  }
  return false;
}

// ============================================================
// BUTTON MAPPING - বাটনের অবস্থান দেখানো
// ============================================================

// ============================================================
// BUTTON MAPPING - নির্দিষ্ট পিক্সেল দূরত্বে বাটন
// ============================================================

void showButtonMapping() {
  while (true) {
    if (btnLongPressed(BTN_MENU, 200)) {
      playMenuButtonSound();
      return;
    }
    
    u8g2.clearBuffer();
    
    // ── হেডার ──
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawBox(0, 0, SCREEN_W, 11);
    u8g2.setDrawColor(0);
    centreStr("BUTTON MAP", 9);
    u8g2.setDrawColor(1);
    
    
    // ── বাটনের নাম ──
    u8g2.setFont(u8g2_font_6x10_tr);
    
    // UP (উপরে মাঝখানে)
    u8g2.drawStr(32, 23, "UP");
    
    // DOWN (নিচে মাঝখানে)
    u8g2.drawStr(26, 60, "DOWN");
    
    // LEFT (বামে মাঝখানে)
    u8g2.drawStr(2, 39, "LEFT");
    
    // RIGHT (ডানে মাঝখানে) - 98 এ অবস্থান
    u8g2.drawStr(45, 39, "RIGHT");
    
    // PLAY/PAUSE - RIGHT এর ডানদিকে 20 পিক্সেল দূরে
    // RIGHT এর x = 88, তার ডানদিকে 20 পিক্সেল = 108
    u8g2.drawStr(88, 39, "PAUSE");
    
    // START - UP এর ডানদিকে 40 পিক্সেল দূরে
    // UP এর x = 54, তার ডানদিকে 40 পিক্সেল = 94
    u8g2.drawStr(96, 23, "START");
    
    // BACK - DOWN এর ডানদিকে 40 পিক্সেল দূরে
    // DOWN এর x = 50, তার ডানদিকে 40 পিক্সেল = 90
    u8g2.drawStr(98, 55, "BACK");
    
    // ── এক্সিট ইনফো ──
    
    u8g2.sendBuffer();
    
    if (btnPressed(BTN_MENU)) {
      playMenuButtonSound();
      waitRelease();
      return;
    }
    
    delay(50);
  }
}
// ============================================================
// GAME OVER SCREEN
// ============================================================

void gameOverScreen(uint16_t score, int gameIndex, bool isWin) {
  if (gameIndex == 12 && isWin) {
    saveTotalGames(12);
    saveRPSWin(12);
  }
  
  if (gameIndex == 12) {
    uint16_t winRate = 0;
    if (totalGamesPlayed[12] > 0) {
      winRate = (rpsWins[12] * 100) / totalGamesPlayed[12];
    }
    if (winRate > highScores[12]) {
      highScores[12] = winRate;
      EEPROM.begin(EEPROM_SIZE);
      EEPROM.put(12 * sizeof(uint16_t), highScores[12]);
      EEPROM.commit();
      EEPROM.end();
      if (soundEnabled) {
        beep(1200, 40, soundLevel);
        delay(60);
        beep(1500, 40, soundLevel);
        delay(60);
        beep(1800, 80, soundLevel);
      }
    }
  } else {
    if (score > highScores[gameIndex]) {
      highScores[gameIndex] = score;
      EEPROM.begin(EEPROM_SIZE);
      EEPROM.put(gameIndex * sizeof(uint16_t), highScores[gameIndex]);
      EEPROM.commit();
      EEPROM.end();
      if (soundEnabled) {
        beep(1200, 40, soundLevel);
        delay(60);
        beep(1500, 40, soundLevel);
        delay(60);
        beep(1800, 80, soundLevel);
      }
    }
  }
  
  playGameOverMusic();
  
  delay(1000);
  
  char buf[20];
  snprintf(buf, sizeof(buf), "Score: %u", score);
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("GAME OVER", 20);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr(buf, 41);
  
  char hs[30];
  if (gameIndex == 12) {
    uint16_t winRate = 0;
    if (totalGamesPlayed[12] > 0) {
      winRate = (rpsWins[12] * 100) / totalGamesPlayed[12];
    }
    snprintf(hs, sizeof(hs), "Win Rate: %u%%", winRate);
  } else {
    snprintf(hs, sizeof(hs), "High Score: %u", highScores[gameIndex]);
  }
  centreStr(hs, 53);
  
  u8g2.sendBuffer();
  delay(1000);
  waitRelease();
}

// ============================================================
// GAME SUB MENU
// ============================================================

void showGameSubMenu(const char* gameName, int gameIndex) {
  const char* options[] = {"1. PLAY GAME", "2. HIGH SCORE", "3. ADD FAVORITE"};
  int sel = 0;
  int top = 0;
  const int VISIBLE = 3;
  const int TOTAL_OPTIONS = 3;
  
  GameFunction games[GAME_COUNT] = {
    game_asteroids, game_breakout, game_dino, game_flappy,
    game_snake1, game_snake2, game_pong, game_pacman,
    game_spaceinvaders, game_tetris, game_tank,
    game_maze, game_rps, game_car, game_car_2lane,
    game_trex, game_trex2, game_meteor_defenders,
    game_death_star, game_tictactoe,
    game_memory_match, game_whack_a_mole,
    game_lunar_lander, game_color_matching,
    game_ninja_spike, game_sperm_race, game_frogger,game_frogger2
  };
  
  currentGameIndex = gameIndex;
  
  while (true) {
    if (btnLongPressed(BTN_MENU, 200)) {
      playMenuButtonSound();
      return;
    }
    
    if (sel < top) top = sel;
    if (sel >= top + VISIBLE) top = sel - VISIBLE + 1;
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawRBox(5, 2, SCREEN_W - 10, 12, 2);
    u8g2.setDrawColor(0);
    
    if (isFavorite(gameIndex)) {
      u8g2.drawStr(10, 11, gameName);
      int nameWidth = u8g2.getStrWidth(gameName);
      drawHeart(10 + nameWidth + 7, 4);
    } else {
      centreStr(gameName, 11);
    }
    u8g2.setDrawColor(1);
    
    u8g2.setFont(u8g2_font_6x10_tr);
    for (int i = 0; i < VISIBLE; i++) {
      int idx = top + i;
      if (idx >= TOTAL_OPTIONS) break;
      int y = 25 + i * 13;
      
      if (idx == sel) {
        u8g2.drawRBox(10, y - 7, SCREEN_W - 20, 11, 2);
        u8g2.setDrawColor(0);
        if (idx == 1) {
          char scoreText[30];
          if (gameIndex == 12) {
            uint16_t winRate = 0;
            if (totalGamesPlayed[12] > 0) {
              winRate = (rpsWins[12] * 100) / totalGamesPlayed[12];
            }
            snprintf(scoreText, sizeof(scoreText), "High Score = %u%%", winRate);
          } else {
            snprintf(scoreText, sizeof(scoreText), "High Score = %u", highScores[gameIndex]);
          }
          centreStr(scoreText, y + 3);
        } else {
          centreStr(options[idx], y + 3);
        }
        u8g2.setDrawColor(1);
      } else {
        if (idx == 1) {
          char scoreText[30];
          if (gameIndex == 12) {
            uint16_t winRate = 0;
            if (totalGamesPlayed[12] > 0) {
              winRate = (rpsWins[12] * 100) / totalGamesPlayed[12];
            }
            snprintf(scoreText, sizeof(scoreText), "High Score = %u%%", winRate);
          } else {
            snprintf(scoreText, sizeof(scoreText), "High Score = %u", highScores[gameIndex]);
          }
          centreStr(scoreText, y + 3);
        } else {
          centreStr(options[idx], y + 3);
        }
      }
    }
    
    if (top > 0) u8g2.drawStr(SCREEN_W - 8, 23, "^");
    if (top + VISIBLE < TOTAL_OPTIONS) u8g2.drawStr(SCREEN_W - 8, 62, "v");
    
    u8g2.sendBuffer();
    
    if (btnPressed(BTN_UP)) {
      if (sel > 0) {
        sel--;
        beep(800, 20, soundLevel);
      } else {
        sel = TOTAL_OPTIONS - 1;
        beep(800, 20, soundLevel);
        if (sel >= top + VISIBLE) {
          top = sel - VISIBLE + 1;
        }
      }
      if (sel < top) top = sel;
    }
    else if (btnPressed(BTN_DOWN)) {
      if (sel < TOTAL_OPTIONS - 1) {
        sel++;
        beep(800, 20, soundLevel);
      } else {
        sel = 0;
        beep(800, 20, soundLevel);
        top = 0;
      }
      if (sel >= top + VISIBLE) {
        top = sel - VISIBLE + 1;
      }
    }
    else if (btnPressed(BTN_ENTER)) {
      beep(1000, 30, soundLevel);
      waitRelease();
      
      if (sel == 0) {
        menuPressed = false;
        gamePaused = false;
        uniqueGameStartSound(gameIndex);
        games[gameIndex]();
        
        if (menuPressed) {
          menuPressed = false;
          continue;
        }
        
        bool replay = false;
        bool backToMenu = false;
        
        while (true) {
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_ncenB10_tr);
          centreStr("GAME OVER", 20);
          u8g2.setFont(u8g2_font_6x10_tr);
          centreStr("ENTER to replay", 38);
          centreStr("MENU to game menu", 50);
          u8g2.sendBuffer();
          
          if (btnPressed(BTN_MENU)) {
            playMenuButtonSound();
            waitRelease();
            backToMenu = true;
            break;
          }
          if (btnPressed(BTN_ENTER)) {
            beep(800, 20, soundLevel);
            waitRelease();
            replay = true;
            break;
          }
          delay(50);
        }
        
        if (backToMenu) {
          continue;
        }
        if (replay) {
          uniqueGameStartSound(gameIndex);
          games[gameIndex]();
        }
      }
      else if (sel == 1) {
        beep(800, 20, soundLevel);
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB10_tr);
        centreStr("HIGH SCORE", 20);
        u8g2.setFont(u8g2_font_6x10_tr);
        char scoreText[30];
        if (gameIndex == 12) {
          uint16_t winRate = 0;
          if (totalGamesPlayed[12] > 0) {
            winRate = (rpsWins[12] * 100) / totalGamesPlayed[12];
          }
          snprintf(scoreText, sizeof(scoreText), "%u%%", winRate);
        } else {
          snprintf(scoreText, sizeof(scoreText), "%u", highScores[gameIndex]);
        }
        centreStr(scoreText, 38);
        u8g2.setFont(u8g2_font_5x7_tr);
        centreStr("Press any key", 55);
        u8g2.sendBuffer();
        waitRelease();
        while (!btnHeld(BTN_UP) && !btnHeld(BTN_DOWN) && !btnHeld(BTN_LEFT) &&
               !btnHeld(BTN_RIGHT) && !btnHeld(BTN_MENU) && !btnHeld(BTN_ENTER))
          delay(15);
        waitRelease();
      }
      else if (sel == 2) {
        toggleFavorite(gameIndex);
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB10_tr);
        if (isFavorite(gameIndex)) {
          centreStr("ADDED TO", 28);
          centreStr("FAVORITES!", 42);
          drawHeart(60, 50);
          beep(1200, 40, soundLevel);
          delay(80);
          beep(1500, 40, soundLevel);
        } else {
          centreStr("REMOVED", 28);
          centreStr("FROM FAVORITES", 42);
          beep(600, 40, soundLevel);
          delay(80);
          beep(500, 40, soundLevel);
        }
        u8g2.sendBuffer();
        delay(800);
        waitRelease();
      }
    }
    else if (btnPressed(BTN_MENU)) {
      beep(600, 40, soundLevel);
      waitRelease();
      return;
    }
    delay(100);
  }
}

// ============================================================
// SPLASH SCREEN
// ============================================================

void showSplash() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("GAME CONSOLE", 28);
  u8g2.setFont(u8g2_font_ncenB08_tr);
  centreStr("MADE BY Rahul", 48);
  u8g2.sendBuffer();
  
  if (soundEnabled) {
    beep(523, 100, soundLevel); delay(150);
    beep(659, 100, soundLevel); delay(150);
    beep(784, 100, soundLevel); delay(150);
    beep(523, 100, soundLevel); delay(150);
  }
  
  for (int i = 0; i <= 100; i += 5) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tr);
    centreStr("GAME CONSOLE", 28);
    u8g2.setFont(u8g2_font_ncenB08_tr);
    centreStr("MADE BY Rahul", 48);
    
    u8g2.drawFrame(14, 58, 100, 4);
    u8g2.drawBox(14, 58, i, 4);
    
    u8g2.sendBuffer();
    
    if (i % 10 == 0 && soundEnabled) {
      beep(440 + i, 20, soundLevel);
    }
    delay(50);
  }
  
  if (soundEnabled) {
    delay(200);
    beep(1046, 200, soundLevel); delay(300);
    beep(880, 100, soundLevel); delay(150);
    beep(1046, 300, soundLevel);
  }
  
  delay(300);
  waitRelease();
}

// ============================================================
// MAIN GRID MENU
// ============================================================

void showMainGridMenu() {
  const char* options[] = {"GAME", "MUSIC", "MEDIA", "SETUP"};
  int sel = 0;
  int col = 0;
  int row = 0;

  while (true) {
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawBox(0, 0, SCREEN_W, 11);
    u8g2.setDrawColor(0);
    centreStr("MAIN MENU", 9);
    u8g2.setDrawColor(1);

    int gridX = 4;
    int gridY = 15;
    int boxW = 58;
    int boxH = 22;
    int spacing = 4;

    for (int r = 0; r < 2; r++) {
      for (int c = 0; c < 2; c++) {
        int idx = r * 2 + c;
        int x = gridX + c * (boxW + spacing);
        int y = gridY + r * (boxH + spacing);

        u8g2.setFont(u8g2_font_ncenB08_tr);
        int textY = y + boxH / 2 + 3;

        if (idx == sel) {
          u8g2.drawRBox(x, y, boxW, boxH, 2);
          u8g2.setDrawColor(0);
          centreStrBox(options[idx], x, boxW, textY);
          u8g2.setDrawColor(1);
        } else {
          u8g2.drawFrame(x, y, boxW, boxH);
          centreStrBox(options[idx], x, boxW, textY);
        }
      }
    }

    u8g2.sendBuffer();
    delay(100);

    if (btnPressed(BTN_UP)) {
      if (row > 0) { row--; sel = row * 2 + col; beep(800, 20, soundLevel); }
    }
    else if (btnPressed(BTN_DOWN)) {
      if (row < 1) { row++; sel = row * 2 + col; beep(800, 20, soundLevel); }
    }
    else if (btnPressed(BTN_LEFT)) {
      if (col > 0) { col--; sel = row * 2 + col; beep(800, 20, soundLevel); }
    }
    else if (btnPressed(BTN_RIGHT)) {
      if (col < 1) { col++; sel = row * 2 + col; beep(800, 20, soundLevel); }
    }
    else if (btnPressed(BTN_ENTER)) {
      beep(1200, 40, soundLevel);
      waitRelease();

      if (sel == 0) {
        while (true) {
          int gameSel = menuSelect();
          if (gameSel >= 0 && gameSel < GAME_COUNT) {
            const char* gameNames[GAME_COUNT] = {
              "Asteroids", "Breakout", "Dino Run", "Flappy Bird",
              "Snake 1", "Snake 2", "Pong", "Pacman",
              "Space Invaders", "Tetris", "Tank Battle",
              "Maze Runner", "RPS Game", "Car Racer", 
              "2-Lane Racer", "T-Rex Run", "T-Rex Run 2",
              "Meteor Defenders", "Death Star", "Tic-Tac-Toe",
              "Memory Match", "Whack-A-Mole", "Lunar Lander",
              "Color Match", "Ninja Spike", "Sperm Race", "Frogger","frogger 2"
            };
            showGameSubMenu(gameNames[gameSel], gameSel);
          } else {
            break;
          }
        }
      }
      else if (sel == 1) {
        showMusicMenu();
      }
      else if (sel == 2) {
        showMediaMenu();
      }
      else if (sel == 3) {
        showSetupMenu();
      }
    }
    else if (btnPressed(BTN_MENU)) {
      playMenuButtonSound();
      waitRelease();
      return;
    }
  }
}

// ============================================================
// MEDIA MENU
// ============================================================

void showMediaMenu() {
  const char* options[] = {"1.  IMAGE", "2.  VIDEO"};
  int sel = 0;
  
  while (true) {
    if (btnLongPressed(BTN_MENU, 200)) {
      playMenuButtonSound();
      return;
    }
    
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawBox(0, 0, SCREEN_W, 11);
      u8g2.setDrawColor(0);
      centreStr("MEDIA", 9);
      u8g2.setDrawColor(1);
    
    for (int i = 0; i < 2; i++) {
      int y = 30 + i * 16;
      if (i == sel) {
        u8g2.drawRBox(20, y - 8, SCREEN_W - 40, 14, 2);
        u8g2.setDrawColor(0);
        centreStr(options[i], y + 4);
        u8g2.setDrawColor(1);
      } else {
        centreStr(options[i], y + 4);
      }
    }
    u8g2.sendBuffer();
    
    if (btnPressed(BTN_UP)) { sel = (sel + 1) % 2; beep(800, 20, soundLevel); }
    else if (btnPressed(BTN_DOWN)) { sel = (sel + 1) % 2; beep(800, 20, soundLevel); }
    else if (btnPressed(BTN_ENTER)) { 
      beep(1000, 30, soundLevel);
      waitRelease();
      
      if (sel == 0) {
        showImageMenu();
      } else {
        showVideoMenu();
      }
    }
    else if (btnPressed(BTN_MENU)) { 
      playMenuButtonSound(); 
      waitRelease(); 
      return; 
    }
    delay(100);
  }
}

void showImageMenu() {
  const char* imageOptions[] = {"1. RAHUL"};
  int sel = 0;
  
  while (true) {
    if (btnLongPressed(BTN_MENU, 200)) {
      playMenuButtonSound();
      return;
    }
    
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawBox(0, 0, SCREEN_W, 11);
      u8g2.setDrawColor(0);
      centreStr(" ALL IMAGES", 9);
      u8g2.setDrawColor(1);

    
    u8g2.setFont(u8g2_font_6x10_tr);
    for (int i = 0; i < 1; i++) {
      int y = 30 + i * 16;
      if (i == sel) {
        u8g2.drawRBox(20, y - 8, SCREEN_W - 40, 14, 2);
        u8g2.setDrawColor(0);
        centreStr(imageOptions[i], y + 4);
        u8g2.setDrawColor(1);
      } else {
        centreStr(imageOptions[i], y + 4);
      }
    }
    u8g2.sendBuffer();
    
    if (btnPressed(BTN_UP) || btnPressed(BTN_DOWN)) {
      sel = (sel + 1) % 1;
      beep(800, 20, soundLevel);
    }
    else if (btnPressed(BTN_ENTER)) {
      beep(1000, 30, soundLevel);
      waitRelease();
      
      if (sel == 0) {
        u8g2.clearBuffer();
        drawRahulBitmap();
        u8g2.sendBuffer();
        delay(3000);
        waitRelease();
        
        while (!btnPressed(BTN_MENU) && !btnPressed(BTN_ENTER)) {
          delay(10);
        }
        playMenuButtonSound();
        waitRelease();
      }
    }
    else if (btnPressed(BTN_MENU)) { 
      playMenuButtonSound(); 
      waitRelease(); 
      return; 
    }
    delay(100);
  }
}

void showVideoMenu() {
  while (true) {
    if (btnLongPressed(BTN_MENU, 200)) {
      playMenuButtonSound();
      return;
    }
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tr);
    centreStr("VIDEO", 24);
    u8g2.setFont(u8g2_font_6x10_tr);
    centreStr("Coming Soon!", 40);
    centreStr("Press MENU to go back", 55);
    u8g2.sendBuffer();
    
    if (btnPressed(BTN_MENU)) {
      playMenuButtonSound();
      waitRelease();
      return;
    }
    delay(50);
  }
}

// ============================================================
// SETUP MENU
// ============================================================


// showSetupMenu() ফাংশন আপডেট করুন (লাইন ~1600)

void showSetupMenu() {
  const char* options[] = {"1. SETTINGS", "2. TORCH", "3. FAV GAMES", 
                           "4. TIMER", "5. STOPWATCH", "6. POMODORO", 
                           "7. DEVICE INFO", "8. BUTTON MAP"};
  
  int sel = 0;
  const int TOTAL_OPTIONS = 8;
  const int VISIBLE = 4;
  const int LINE_HEIGHT = 13;
  
  // Hold detection
  static uint32_t lastHoldTime = 0;
  const uint32_t HOLD_DELAY = 120;
  
  while (true) {
    if (btnLongPressed(BTN_MENU, 200)) {
      playMenuButtonSound();
      return;
    }
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    
    // ── হেডার ──
    u8g2.drawBox(0, 0, SCREEN_W, 11);
    u8g2.setDrawColor(0);
    centreStr("SETUP", 9);  // হেডার সেন্টার থাকবে
    u8g2.setDrawColor(1);
    
    u8g2.setFont(u8g2_font_ncenB08_tr);
    
    int startIdx = 0;
    if (sel >= VISIBLE) startIdx = sel - VISIBLE + 1;
    if (startIdx > TOTAL_OPTIONS - VISIBLE) startIdx = TOTAL_OPTIONS - VISIBLE;
    
    // ✅ LEFT ALIGNED - বাম দিক থেকে শুরু
    int leftX = 15;
    
    for (int i = 0; i < VISIBLE && (startIdx + i) < TOTAL_OPTIONS; i++) {
      int idx = startIdx + i;
      int y = 21 + i * LINE_HEIGHT;
      
      if (idx == sel) {
        // সিলেক্টেড আইটেম - বক্স সহ
        int textWidth = u8g2.getStrWidth(options[idx]);
        int boxWidth = textWidth + 24;
        
        u8g2.drawRBox(leftX - 5, y - 7, boxWidth, 11, 2);
        u8g2.setDrawColor(0);
        u8g2.drawStr(leftX, y + 4, options[idx]);
        u8g2.setDrawColor(1);
      } else {
        u8g2.drawStr(leftX, y + 4, options[idx]);
      }
    }
    
    // ── স্ক্রল ইন্ডিকেটর ──
    u8g2.setFont(u8g2_font_5x7_tr);
    if (startIdx > 0) u8g2.drawStr(SCREEN_W - 10, 15, "^");
    if (startIdx + VISIBLE < TOTAL_OPTIONS) u8g2.drawStr(SCREEN_W - 10, 62, "v");
    
    u8g2.sendBuffer();
    
    // ── বাটন হ্যান্ডলিং ──
    uint32_t now = millis();
    
    if (btnPressed(BTN_UP)) { 
      sel = (sel + TOTAL_OPTIONS - 1) % TOTAL_OPTIONS; 
      beep(800, 20, soundLevel);
      lastHoldTime = now;
    }
    else if (btnHeld(BTN_UP) && now - lastHoldTime > HOLD_DELAY) {
      lastHoldTime = now;
      sel = (sel + TOTAL_OPTIONS - 1) % TOTAL_OPTIONS; 
      beep(800, 20, soundLevel);
    }
    
    if (btnPressed(BTN_DOWN)) { 
      sel = (sel + 1) % TOTAL_OPTIONS; 
      beep(800, 20, soundLevel);
      lastHoldTime = now;
    }
    else if (btnHeld(BTN_DOWN) && now - lastHoldTime > HOLD_DELAY) {
      lastHoldTime = now;
      sel = (sel + 1) % TOTAL_OPTIONS; 
      beep(800, 20, soundLevel);
    }
    
    if (btnPressed(BTN_ENTER)) { 
      beep(1200, 40, soundLevel); 
      waitRelease();
      
      if (sel == 0) {
        showSettingsMenu();
      }
      else if (sel == 1) {
        // TORCH
        bool torchOn = true;
        u8g2.setContrast(255);
        u8g2.clearBuffer();
        u8g2.setDrawColor(1);
        u8g2.drawBox(0, 0, SCREEN_W, SCREEN_H);
        u8g2.sendBuffer();
        beep(1200, 50, soundLevel);
        delay(100);
        beep(1500, 50, soundLevel);
        
        while (torchOn) {
          static uint32_t lastRefresh = 0;
          if (millis() - lastRefresh > 100) {
            lastRefresh = millis();
            u8g2.setDrawColor(1);
            u8g2.drawBox(0, 0, SCREEN_W, SCREEN_H);
            u8g2.sendBuffer();
          }
          if (btnPressed(BTN_MENU)) {
            torchOn = false;
            uint8_t contrast = map(brightnessLevel, 1, 7, 30, 255);
            u8g2.setContrast(contrast);
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_ncenB10_tr);
            centreStr("TORCH OFF", 35);
            u8g2.sendBuffer();
            playMenuButtonSound();
            delay(500);
            break;
          }
          delay(20);
        }
        waitRelease();
      }
      else if (sel == 2) {
        showFavoritesMenu();
      }
      else if (sel == 3) {
        showTimerMenu();
      }
      else if (sel == 4) {
        showStopwatchMenu();
      }
      else if (sel == 5) {
        showPomodoroMenu();
      }
      else if (sel == 6) {
        showDeviceInfo();
      }
      else if (sel == 7) {
        showButtonMapping();
      }
    }
    else if (btnPressed(BTN_MENU)) { 
      playMenuButtonSound(); 
      waitRelease();
      return;
    }
    
    delay(80);  // ডিলে কমানো হয়েছে
  }
}


void showMusicMenu() {
  int sel = 0;
  int top = 0;
  const int VISIBLE = 4;
  uint32_t lastUpdate = 0;
  
  while (true) {
    uint32_t now = millis();
    
    if (btnLongPressed(BTN_MENU, 200)) {
      if (musicPlaying) {
        stopMusicPlayer();
      }
      playMenuButtonSound();
      return;
    }
    
    // Volume control during playback
    if (musicPlaying) {
      if (btnPressed(BTN_UP) && currentMusicVolume < 3) {
        currentMusicVolume++;
        beep(800, 30, soundLevel);
        if (musicNotePlaying) {
          noTone(BUZZER_PIN);
          musicNotePlaying = false;
        }
      }
      if (btnPressed(BTN_DOWN) && currentMusicVolume > 0) {
        currentMusicVolume--;
        beep(600, 30, soundLevel);
        if (musicNotePlaying) {
          noTone(BUZZER_PIN);
          musicNotePlaying = false;
        }
      }
    }
    
    if (sel < top) top = sel;
    if (sel >= top + VISIBLE) top = sel - VISIBLE + 1;
    
    u8g2.clearBuffer();
    
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawRBox(0, 0, SCREEN_W, 12, 2);
    u8g2.setDrawColor(0);
    
    char title[30];
    if (musicPlaying && !musicPaused) {
      snprintf(title, sizeof(title), "▶ VOL:%d", currentMusicVolume);
    } else if (musicPaused) {
      snprintf(title, sizeof(title), "⏸ VOL:%d", currentMusicVolume);
    } else {
      snprintf(title, sizeof(title), "MUSIC PLAYER");
    }
    centreStr(title, 10);
    u8g2.setDrawColor(1);
    
    u8g2.setFont(u8g2_font_ncenB08_tr);
    for (int i = 0; i < VISIBLE; i++) {
      int idx = top + i;
      if (idx >= MUSIC_COUNT) break;
      int y = 14 + i * 12;
      
      if (idx == sel) {
        u8g2.drawRBox(2, y-1, SCREEN_W - 4, 11, 2);
        u8g2.setDrawColor(0);
        
        if (musicPlaying && !musicPaused && idx == sel) {
          char displayName[35];
          snprintf(displayName, sizeof(displayName), "▶ %s", MUSIC_NAMES[idx]);
          u8g2.drawStr(6, y + 9, displayName);
        } else if (musicPaused && idx == sel) {
          char displayName[35];
          snprintf(displayName, sizeof(displayName), "⏸ %s", MUSIC_NAMES[idx]);
          u8g2.drawStr(6, y + 9, displayName);
        } else {
          u8g2.drawStr(6, y + 9, MUSIC_NAMES[idx]);
        }
        u8g2.setDrawColor(1);
      } else {
        u8g2.drawStr(6, y + 9, MUSIC_NAMES[idx]);
      }
    }
    
    u8g2.setFont(u8g2_font_6x10_tr);
    if (top > 0) {
      u8g2.drawStr(SCREEN_W - 10, 13, "▲");
    }
    if (top + VISIBLE < MUSIC_COUNT) {
      u8g2.drawStr(SCREEN_W - 10, 62, "▼");
    }
    
    u8g2.sendBuffer();
    
    if (musicPlaying && !musicPaused) {
      playMusicSong(sel);
    }
    
    if (btnPressed(BTN_UP) && !musicPlaying) { 
      sel = (sel + MUSIC_COUNT - 1) % MUSIC_COUNT; 
      beep(800, 20, soundLevel);
    }
    else if (btnPressed(BTN_DOWN) && !musicPlaying) { 
      sel = (sel + 1) % MUSIC_COUNT; 
      beep(800, 20, soundLevel);
    }
    else if (btnPressed(BTN_ENTER) || btnPressed(BTN_PAUSE)) { 
      if (musicPlaying && !musicPaused) {
        musicPaused = true;
        noTone(BUZZER_PIN);
        musicNotePlaying = false;
        beep(500, 40, soundLevel);
      } else if (musicPaused) {
        musicPaused = false;
        musicNotePlaying = false;
        beep(1000, 30, soundLevel);
      } else {
        musicPlaying = true;
        musicPaused = false;
        musicNoteIndex = 0;
        musicNotePlaying = false;
        currentMusicVolume = soundLevel;
        beep(1000, 30, soundLevel);
      }
      waitRelease();
    }
    else if (btnPressed(BTN_MENU)) { 
      if (musicPlaying) {
        stopMusicPlayer();
      }
      playMenuButtonSound(); 
      waitRelease(); 
      return; 
    }
    
    delay(20);
  }
}

// ============================================================
// SETTINGS MENU
// ============================================================

void showSettingsMenu() {
  const char* options[] = {"1. Brightness", "2. Sound", "3. Reset Device"};
  int sel = 0;
  
  while (true) {
    if (btnLongPressed(BTN_MENU, 200)) {
      playMenuButtonSound();
      return;
    }
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    centreStr("SETTINGS", 10);
    
    for (int i = 0; i < 3; i++) {
      int y = 25 + i * 13;
      if (i == sel) {
        u8g2.drawRBox(10, y - 8, SCREEN_W - 20, 12, 2);
        u8g2.setDrawColor(0);
        centreStr(options[i], y + 3);
        u8g2.setDrawColor(1);
      } else {
        centreStr(options[i], y + 3);
      }
    }
    u8g2.sendBuffer();
    
    if (btnPressed(BTN_UP)) { sel = (sel + 2) % 3; beep(800, 20, soundLevel); }
    else if (btnPressed(BTN_DOWN)) { sel = (sel + 1) % 3; beep(800, 20, soundLevel); }
    else if (btnPressed(BTN_ENTER)) { 
      beep(1000, 30, soundLevel); 
      waitRelease();
      
      if (sel == 0) {
        bool adjusting = true;
        while (adjusting) {
          if (btnLongPressed(BTN_MENU, 200)) {
            playMenuButtonSound();
            adjusting = false;
            break;
          }
          
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_ncenB08_tr);
          centreStr("BRIGHTNESS", 10);
          
          char levelText[20];
          if (brightnessLevel == 1) snprintf(levelText, sizeof(levelText), "Very Low");
          else if (brightnessLevel == 2) snprintf(levelText, sizeof(levelText), "Low");
          else if (brightnessLevel == 3) snprintf(levelText, sizeof(levelText), "Med-Low");
          else if (brightnessLevel == 4) snprintf(levelText, sizeof(levelText), "Medium");
          else if (brightnessLevel == 5) snprintf(levelText, sizeof(levelText), "Med-High");
          else if (brightnessLevel == 6) snprintf(levelText, sizeof(levelText), "High");
          else snprintf(levelText, sizeof(levelText), "Very High");
          centreStr(levelText, 28);

          for (int i = 0; i < 7; i++) {
            int x = 12 + i * 15;
            int h = (i < brightnessLevel) ? 14 : 4;
            u8g2.drawBox(x, 42 - h/2, 11, h);
            u8g2.drawFrame(x, 42 - 7, 11, 14);
          }
          
          u8g2.setFont(u8g2_font_5x7_tr);
          centreStr("LEFT=LESS    RIGHT=MORE", 63);
          u8g2.sendBuffer();
          
          if (btnPressed(BTN_LEFT)) { 
            brightnessLevel = max(1, brightnessLevel - 1); 
            uint8_t contrast = map(brightnessLevel, 1, 7, 30, 255);
            u8g2.setContrast(contrast);
            beep(800, 15, soundLevel);
          }
          else if (btnPressed(BTN_RIGHT)) { 
            brightnessLevel = min(7, brightnessLevel + 1); 
            uint8_t contrast = map(brightnessLevel, 1, 7, 30, 255);
            u8g2.setContrast(contrast);
            beep(800, 15, soundLevel);
          }
          else if (btnPressed(BTN_ENTER)) { 
            saveBrightness();
            beep(1000, 30, soundLevel);
            waitRelease();
            adjusting = false;
          }
          else if (btnPressed(BTN_MENU)) { 
            playMenuButtonSound();
            adjusting = false;
          }
          delay(50);
        }
      }
      else if (sel == 1) {
        bool adjusting = true;
        while (adjusting) {
          if (btnLongPressed(BTN_MENU, 200)) {
            playMenuButtonSound();
            adjusting = false;
            break;
          }
          
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_ncenB08_tr);
          centreStr("SOUND", 10);
          
          u8g2.setFont(u8g2_font_ncenB10_tr);
          char soundText[20];
          if (soundLevel == 0) snprintf(soundText, sizeof(soundText), "MUTE");
          else if (soundLevel == 1) snprintf(soundText, sizeof(soundText), "LOW");
          else if (soundLevel == 2) snprintf(soundText, sizeof(soundText), "MEDIUM");
          else if (soundLevel == 3) snprintf(soundText, sizeof(soundText), "HIGH");
          centreStr(soundText, 28);
          
          for (int i = 0; i < 3; i++) {
            int x = 35 + i * 22;
            int h = (i < soundLevel) ? 12 : 3;
            u8g2.drawBox(x, 42 - h/2, 14, h);
            u8g2.drawFrame(x, 42 - 6, 14, 12);
          }
          
          u8g2.setFont(u8g2_font_5x7_tr);
          centreStr("LEFT=LOW     RIGHT=HIGH", 63);
          u8g2.sendBuffer();
          
          if (btnPressed(BTN_LEFT)) { 
            soundLevel = max(0, soundLevel - 1); 
            soundEnabled = (soundLevel > 0);
            currentMusicVolume = soundLevel;
          }
          else if (btnPressed(BTN_RIGHT)) { 
            soundLevel = min(3, soundLevel + 1); 
            soundEnabled = (soundLevel > 0);
            currentMusicVolume = soundLevel;
            if (soundEnabled) {
              beep(1000, 20, soundLevel);
              delay(80);
              beep(1200, 20, soundLevel);
            }
          }
          else if (btnPressed(BTN_ENTER)) { 
            saveSoundSetting();
            beep(1000, 30, soundLevel);
            waitRelease();
            adjusting = false;
          }
          else if (btnPressed(BTN_MENU)) { 
            playMenuButtonSound();
            adjusting = false;
          }
          delay(50);
        }
      }
      else if (sel == 2) {
        bool confirmReset = true;
        int confirmSel = 0;
        const char* confirmOptions[] = {"YES", "NO"};
        
        while (confirmReset) {
          if (btnLongPressed(BTN_MENU, 200)) {
            playMenuButtonSound();
            confirmReset = false;
            break;
          }
          
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_ncenB10_tr);
          centreStr("RESET", 18);
          u8g2.setFont(u8g2_font_6x10_tr);
          centreStr("All data will be", 32);
          centreStr("erased!", 42);
          
          int boxW = 40;
          int boxH = 16;
          int spacing = 10;
          int totalW = boxW * 2 + spacing;
          int startX = (SCREEN_W - totalW) / 2;
          int y = 60;
          
          for (int i = 0; i < 2; i++) {
            int x = startX + i * (boxW + spacing);
            u8g2.setFont(u8g2_font_ncenB08_tr);
            
            if (i == confirmSel) {
              u8g2.drawRBox(x, y - boxH + 2, boxW, boxH, 2);
              u8g2.setDrawColor(0);
              centreStrBox(confirmOptions[i], x, boxW, y -2);
              u8g2.setDrawColor(1);
            } else {
              u8g2.drawFrame(x, y - boxH + 2, boxW, boxH);
              centreStrBox(confirmOptions[i], x, boxW, y -2);
            }
          }
          
          u8g2.sendBuffer();
          delay(100);
          
          if (btnPressed(BTN_LEFT) || btnPressed(BTN_UP)) {
            confirmSel = 0;
            beep(800, 20, soundLevel);
          }
          else if (btnPressed(BTN_RIGHT) || btnPressed(BTN_DOWN)) {
            confirmSel = 1;
            beep(800, 20, soundLevel);
          }
          else if (btnPressed(BTN_ENTER)) {
            beep(1000, 30, soundLevel);
            waitRelease();
            if (confirmSel == 0) {
              resetDevice();
              confirmReset = false;
            } else {
              confirmReset = false;
            }
          }
          else if (btnPressed(BTN_MENU)) {
            playMenuButtonSound();
            confirmReset = false;
          }
        }
      }
    }
    else if (btnPressed(BTN_MENU)) { playMenuButtonSound(); waitRelease(); return; }
    delay(100);
  }
}

// ============================================================
// FAVORITES MENU
// ============================================================

void showFavoritesMenu() {
  if (favoriteCount == 0) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tr);
    centreStr("NO", 24);
    centreStr("FAVORITES", 40);
    u8g2.setFont(u8g2_font_6x10_tr);
    centreStr("Add from game menu", 55);
    u8g2.sendBuffer();
    waitRelease();
    while (!btnHeld(BTN_UP) && !btnHeld(BTN_DOWN) && !btnHeld(BTN_LEFT) &&
           !btnHeld(BTN_RIGHT) && !btnHeld(BTN_MENU) && !btnHeld(BTN_ENTER))
      delay(15);
    waitRelease();
    return;
  }
  
  const char* gameNames[GAME_COUNT] = {
    "Asteroids", "Breakout", "Dino Run", "Flappy Bird",
    "Snake 1", "Snake 2", "Pong", "Pacman",
    "Space Invaders", "Tetris", "Tank Battle",
    "Maze Runner", "RPS Game", "Car Racer",
    "2-Lane Racer", "T-Rex Run", "T-Rex Run 2",
    "Meteor Defenders", "Death Star", "Tic-Tac-Toe",
    "Memory Match", "Whack-A-Mole", "Lunar Lander",
    "Color Match", "Ninja Spike", "Sperm Race", "Frogger","frogger 2"
  };
  
  int sel = 0;
  int top = 0;
  const int VISIBLE = 4;
  
  while (true) {
    if (btnLongPressed(BTN_MENU, 200)) {
      playMenuButtonSound();
      return;
    }
    
    if (favoriteCount == 0) return;
    if (sel >= favoriteCount) sel = favoriteCount - 1;
    if (sel < top) top = sel;
    if (sel >= top + VISIBLE) top = sel - VISIBLE + 1;
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    centreStr("FAVORITES", 10);
    
    u8g2.setFont(u8g2_font_6x10_tr);
    for (int i = 0; i < VISIBLE; i++) {
      int idx = top + i;
      if (idx >= favoriteCount) break;
      int gameIdx = favoriteGames[idx];
      int y = 14 + i * 13;
      
      char displayName[30];
      snprintf(displayName, sizeof(displayName), "%d. %s", idx + 1, gameNames[gameIdx]);
      
      if (idx == sel) {
        u8g2.drawRBox(0, y-1, SCREEN_W, 12, 2);
        u8g2.setDrawColor(0);
        u8g2.drawStr(6, y + 9, displayName);
        int nameWidth = u8g2.getStrWidth(displayName);
        drawHeart(6 + nameWidth + 7, y + 2);
        u8g2.setDrawColor(1);
      } else {
        int nameWidth = u8g2.getStrWidth(displayName);
        u8g2.drawStr(6, y + 9, displayName);
        drawHeart(6 + nameWidth + 7, y + 2);
      }
    }
    
    if (top > 0) u8g2.drawStr(SCREEN_W - 8, 13, "^");
    if (top + VISIBLE < favoriteCount) u8g2.drawStr(SCREEN_W - 8, 62, "v");
    
    u8g2.sendBuffer();
    delay(100);
    
    if (btnPressed(BTN_UP)) { 
      sel = (sel + favoriteCount - 1) % favoriteCount; 
      beep(800, 20, soundLevel); 
    }
    else if (btnPressed(BTN_DOWN)) { 
      sel = (sel + 1) % favoriteCount; 
      beep(800, 20, soundLevel); 
    }
    else if (btnPressed(BTN_ENTER)) { 
      beep(1200, 40, soundLevel); 
      waitRelease();
      int gameIdx = favoriteGames[sel];
      showGameSubMenu(gameNames[gameIdx], gameIdx);
    }
    else if (btnPressed(BTN_MENU)) { 
      playMenuButtonSound(); 
      waitRelease(); 
      return;
    }
  }
}

// ============================================================
// MAIN GAME MENU
// ============================================================

int menuSelect() {
  const char *names[GAME_COUNT] = {
    "1. Asteroids", "2. Breakout", "3. Dino Run", "4. Flappy Bird",
    "5. Snake 1", "6. Snake 2", "7. Pong", "8. Pacman",
    "9. Space Invaders", "10. Tetris", "11. Tank Battle",
    "12. Maze Runner", "13. RPS Game", "14. Car Racer",
    "15. 2-Lane Racer", "16. T-Rex Run", "17. T-Rex Run 2",
    "18. Meteor Defenders", "19. Death Star", "20. Tic-Tac-Toe",
    "21. Memory Match", "22. Whack-A-Mole", "23. Lunar Lander",
    "24. Color Match", "25. Ninja Spike", "26. Sperm Race", "27. Frogger","28. Frogger 2"
  };
  int sel = lastGameIndex;
  if (sel >= GAME_COUNT) sel = 0;
  int top = 0;
  const int VISIBLE = 4;
  
  // 🔥 হোল্ড ট্র্যাকিং - আলাদা টাইমার
  static uint32_t upHoldTime = 0;
  static uint32_t downHoldTime = 0;
  const uint32_t HOLD_DELAY = 150;    // চেপে ধরে রাখলে প্রতি 100ms এ একবার

  while (true) {
    if (btnLongPressed(BTN_MENU, 200)) {
      playMenuButtonSound();
      return -1;
    }
    
    if (sel < top) top = sel;
    if (sel >= top + VISIBLE) top = sel - VISIBLE + 1;

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawBox(0, 0, SCREEN_W, 11);
    u8g2.setDrawColor(0);
    centreStr("SELECT GAME", 9);
    u8g2.setDrawColor(1);

    u8g2.setFont(u8g2_font_ncenB08_tr);
    for (int i = 0; i < VISIBLE; i++) {
      int idx = top + i;
      if (idx >= GAME_COUNT) break;
      int y = 14 + i * 12;
      
      if (idx == sel) {
        u8g2.drawRBox(0, y-1, SCREEN_W, 11, 2);
        u8g2.setDrawColor(0);
        if (isFavorite(idx)) {
          u8g2.drawStr(6, y + 9, names[idx]);
          int nameWidth = u8g2.getStrWidth(names[idx]);
          drawHeart(6 + nameWidth + 7, y + 2);
        } else {
          u8g2.drawStr(6, y + 9, names[idx]);
        }
        u8g2.setDrawColor(1);
      } else {
        if (isFavorite(idx)) {
          int nameWidth = u8g2.getStrWidth(names[idx]);
          u8g2.drawStr(6, y + 9, names[idx]);
          drawHeart(6 + nameWidth + 7, y + 2);
        } else {
          u8g2.drawStr(6, y + 9, names[idx]);
        }
      }
    }

    u8g2.drawFrame(SCREEN_W - 7, 12, 6, 52);
    int thumbH = max(6, 52 / GAME_COUNT * VISIBLE);
    int thumbY = 12 + (sel * (52 - thumbH)) / (GAME_COUNT - 1);
    u8g2.drawBox(SCREEN_W - 6, thumbY, 4, thumbH);

    if (top > 0) u8g2.drawStr(SCREEN_W - 8, 13, "^");
    if (top + VISIBLE < GAME_COUNT) u8g2.drawStr(SCREEN_W - 8, 62, "v");

    u8g2.sendBuffer();
    
    uint32_t now = millis();
    
    // ==========================================
    // 🔥 UP বাটন - চেপে ধরে রাখলে বারবার (100ms)
    // ==========================================
    if (digitalRead(BTN_UP) == LOW) {
      if (now - upHoldTime > HOLD_DELAY) {
        upHoldTime = now;
        sel = (sel + GAME_COUNT - 1) % GAME_COUNT;
        beep(800, 20, soundLevel);
      }
    } else {
      upHoldTime = 0;  // বাটন রিলিজ করলে রিসেট
    }
    
    // ==========================================
    // 🔥 DOWN বাটন - চেপে ধরে রাখলে বারবার (100ms)
    // ==========================================
    if (digitalRead(BTN_DOWN) == LOW) {
      if (now - downHoldTime > HOLD_DELAY) {
        downHoldTime = now;
        sel = (sel + 1) % GAME_COUNT;
        beep(800, 20, soundLevel);
      }
    } else {
      downHoldTime = 0;  // বাটন রিলিজ করলে রিসেট
    }
    
    // ==========================================
    // 🔥 ENTER বাটন - শুধু একবার
    // ==========================================
    if (btnPressed(BTN_ENTER)) { 
      beep(1200, 40, soundLevel); 
      waitRelease(); 
      lastGameIndex = sel;
      return sel; 
    }
    
    // ==========================================
    // 🔥 MENU বাটন - শুধু একবার
    // ==========================================
    if (btnPressed(BTN_MENU)) { 
      playMenuButtonSound(); 
      waitRelease();
      return -1; 
    }
    
    delay(10);  // ডিলে কমানো হয়েছে
  }
}


// int menuSelect() {
//   const char *names[GAME_COUNT] = {
//     "1. Asteroids", "2. Breakout", "3. Dino Run", "4. Flappy Bird",
//     "5. Snake 1", "6. Snake 2", "7. Pong", "8. Pacman",
//     "9. Space Invaders", "10. Tetris", "11. Tank Battle",
//     "12. Maze Runner", "13. RPS Game", "14. Car Racer",
//     "15. 2-Lane Racer", "16. T-Rex Run", "17. T-Rex Run 2",
//     "18. Meteor Defenders", "19. Death Star", "20. Tic-Tac-Toe",
//     "21. Memory Match", "22. Whack-A-Mole", "23. Lunar Lander",
//     "24. Color Match", "25. Ninja Spike", "26. Sperm Race", "27. Frogger"
//   };
//   int sel = lastGameIndex;
//   if (sel >= GAME_COUNT) sel = 0;
//   int top = 0;
//   const int VISIBLE = 4;

//   while (true) {
//     if (btnLongPressed(BTN_MENU, 200)) {
//       playMenuButtonSound();
//       return -1;
//     }
    
//     if (sel < top) top = sel;
//     if (sel >= top + VISIBLE) top = sel - VISIBLE + 1;

//     u8g2.clearBuffer();
//     u8g2.setFont(u8g2_font_ncenB08_tr);
//     u8g2.drawBox(0, 0, SCREEN_W, 11);
//     u8g2.setDrawColor(0);
//     centreStr("SELECT GAME", 9);
//     u8g2.setDrawColor(1);

//     u8g2.setFont(u8g2_font_ncenB08_tr);
//     for (int i = 0; i < VISIBLE; i++) {
//       int idx = top + i;
//       if (idx >= GAME_COUNT) break;
//       int y = 14 + i * 12;
      
//       if (idx == sel) {
//         u8g2.drawRBox(0, y-1, SCREEN_W, 11, 2);
//         u8g2.setDrawColor(0);
//         if (isFavorite(idx)) {
//           u8g2.drawStr(6, y + 9, names[idx]);
//           int nameWidth = u8g2.getStrWidth(names[idx]);
//           drawHeart(6 + nameWidth + 7, y + 2);
//         } else {
//           u8g2.drawStr(6, y + 9, names[idx]);
//         }
//         u8g2.setDrawColor(1);
//       } else {
//         if (isFavorite(idx)) {
//           int nameWidth = u8g2.getStrWidth(names[idx]);
//           u8g2.drawStr(6, y + 9, names[idx]);
//           drawHeart(6 + nameWidth + 7, y + 2);
//         } else {
//           u8g2.drawStr(6, y + 9, names[idx]);
//         }
//       }
//     }

//     u8g2.drawFrame(SCREEN_W - 7, 12, 6, 52);
//     int thumbH = max(6, 52 / GAME_COUNT * VISIBLE);
//     int thumbY = 12 + (sel * (52 - thumbH)) / (GAME_COUNT - 1);
//     u8g2.drawBox(SCREEN_W - 6, thumbY, 4, thumbH);

//     if (top > 0) u8g2.drawStr(SCREEN_W - 8, 13, "^");
//     if (top + VISIBLE < GAME_COUNT) u8g2.drawStr(SCREEN_W - 8, 62, "v");

//     u8g2.sendBuffer();
//     delay(100);
    
//     if (btnPressed(BTN_UP)) { 
//       sel = (sel + GAME_COUNT - 1) % GAME_COUNT; 
//       beep(800, 20, soundLevel); 
//     }
//     else if (btnPressed(BTN_DOWN)) { 
//       sel = (sel + 1) % GAME_COUNT; 
//       beep(800, 20, soundLevel); 
//     }
//     else if (btnPressed(BTN_ENTER)) { 
//       beep(1200, 40, soundLevel); 
//       waitRelease(); 
//       lastGameIndex = sel;
//       return sel; 
//     }
//     else if (btnPressed(BTN_MENU)) { 
//       playMenuButtonSound(); 
//       waitRelease();
//       return -1; 
//     }
//   }
// }

// ============================================================
// NEW GAME: MEMORY MATCH
// ============================================================



void game_asteroids() {
  const int SHIP_W = 9, SHIP_H = 8;
  int shipY = SCREEN_H / 2 - SHIP_H / 2;
  uint8_t lives = 3;
  bool invincible = false;
  uint32_t invincibleStart = 0;
  const uint32_t INVINCIBLE_DURATION = 1500;
  int respawnY = SCREEN_H / 2 - SHIP_H / 2;

  struct Rock { float x, y, spd; uint8_t w, h; bool on; };
  Rock rocks[12];
  for (auto &r : rocks) r.on = false;

  uint32_t score = 0;
  uint32_t lastSpawn = 0;
  uint32_t lastFrame = millis();
  uint32_t spawnGap = 900;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("ASTEROIDS", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("UP/DOWN to dodge", 40);
  centreStr("Survive as long", 52);
  centreStr("as you can!", 62);
  u8g2.sendBuffer();
  delay(1600);
  waitRelease();

  while (true) {
    if (checkPause("ASTEROIDS")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    float dt = (now - lastFrame) / 30.0f;
    lastFrame = now;
    
    if (invincible && (now - invincibleStart) > INVINCIBLE_DURATION) {
      invincible = false;
    }

    if (btnHeld(BTN_UP)) shipY = max(0, shipY - 4);
    if (btnHeld(BTN_DOWN)) shipY = min(SCREEN_H - SHIP_H, shipY + 4);

    // FIXED: Score increases based on time, not per frame
    static uint32_t lastScoreUpdate = 0;
    if (now - lastScoreUpdate > 1000) {
      score++;
      lastScoreUpdate = now;
    }
    spawnGap = max(300UL, 900UL - score / 100);

    if (now - lastSpawn > spawnGap) {
      lastSpawn = now;
      for (auto &r : rocks) {
        if (!r.on) {
          r.x = SCREEN_W + 4;
          r.y = random(2, SCREEN_H - 14);
          r.w = random(5, 13);
          r.h = random(5, 11);
          r.spd = random(18, 40) / 10.0f;
          r.on = true;
          break;
        }
      }
    }

    for (auto &r : rocks) {
      if (!r.on) continue;
      r.x -= r.spd * dt;
      if (r.x + r.w < 0) {
        r.on = false;
        continue;
      }
      
      if (!invincible && r.x < 2 + SHIP_W && r.x + r.w > 2 && 
          r.y < shipY + SHIP_H && r.y + r.h > shipY) {
        lives--;
        if (lives == 0) {
          gameOverScreen(score, 0, false);
          return;
        }
        beep(200, 80, soundLevel);
        invincible = true;
        invincibleStart = now;
        shipY = respawnY;
        for (auto &clearRock : rocks) {
          if (clearRock.on && clearRock.x < SCREEN_W) {
            clearRock.on = false;
          }
        }
        break;
      }
    }

    u8g2.clearBuffer();
    
    for (int i = 0; i < 8; i++) {
      u8g2.drawPixel(random(0, SCREEN_W), random(0, SCREEN_H));
    }
    
    if (!invincible || (now / 100) % 2 == 0) {
      u8g2.drawLine(3, shipY + 6, 15, shipY);
      u8g2.drawLine(3, shipY + 6, 15, shipY + 12);
      u8g2.drawLine(15, shipY, 15, shipY + 12);
      u8g2.drawLine(8, shipY + 3, 12, shipY + 6);
      u8g2.drawLine(8, shipY + 9, 12, shipY + 6);
      
      if ((now / 120) % 2 == 0) {
        u8g2.drawLine(0, shipY + 4, 3, shipY + 5);
        u8g2.drawLine(0, shipY + 5, 3, shipY + 6);
        u8g2.drawLine(0, shipY + 6, 3, shipY + 7);
        u8g2.drawLine(0, shipY + 7, 3, shipY + 8);
      }
    }
    
    for (auto &r : rocks) {
      if (!r.on) continue;
      int rx = (int)r.x, ry = (int)r.y;
      u8g2.drawFrame(rx, ry, r.w, r.h);
    }
    
    u8g2.setFont(u8g2_font_5x7_tr);
    char sc[8];
    ltoa(score, sc, 10);
    u8g2.drawStr(SCREEN_W - u8g2.getStrWidth(sc) - 2, 8, sc);
    
    char livesText[8];
    sprintf(livesText, "L:%d", lives);
    u8g2.drawStr(SCREEN_W - u8g2.getStrWidth(livesText) - 2, SCREEN_H - 5, livesText);
    
    u8g2.sendBuffer();
    int32_t wait = 33 - (int32_t)(millis() - now);
    if (wait > 0) delay(wait);
  }
}

// ============================================================
// GAME: BREAKOUT (Fixed collision)
// ============================================================

void game_breakout() {
  uint8_t bricks[4][10];
  for (int r = 0; r < 4; r++)
    for (int c = 0; c < 10; c++)
      bricks[r][c] = 1;
  int bricksLeft = 40;
  int padX = (SCREEN_W - 20) / 2;
  const int padY = SCREEN_H - 6;
  float bx = 64, by = 48;
  float vx = 1.7f, vy = -0.5f;
  int lives = 3;
  uint16_t score = 0;
  uint32_t lastFrame = millis();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("BREAKOUT", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("LEFT/RIGHT = paddle", 40);
  centreStr("Don't drop the ball!", 54);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    if (checkPause("BREAKOUT")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    float dt = (now - lastFrame) / 30.0f;
    lastFrame = now;
    
    if (btnHeld(BTN_LEFT)) padX = max(0, padX - 7);
    if (btnHeld(BTN_RIGHT)) padX = min(SCREEN_W - 20, padX + 7);
    
    bx += vx * dt;
    by += vy * dt;
    
    if (bx <= 1) { vx = fabsf(vx); bx = 1; beep(600, 10, soundLevel); }
    if (bx >= SCREEN_W - 4) { vx = -fabsf(vx); bx = SCREEN_W - 4; beep(600, 10, soundLevel); }
    if (by <= 1) { vy = fabsf(vy); by = 1; beep(600, 10, soundLevel); }
    
    // FIXED: Better paddle collision detection
    if (by + 3 >= padY && by <= padY + 3 && 
        bx + 2 >= padX && bx <= padX + 20) {
      vy = -fabsf(vy);
      float rel = ((bx + 1) - (padX + 10.0f)) / 10.0f;
      vx = rel * 3.0f;
      if (fabsf(vx) < 0.5f) vx = (vx >= 0) ? 0.5f : -0.5f;
      by = padY - 3;
      beep(900, 15, soundLevel);
    }
    
    if (by > SCREEN_H + 2) {
      lives--;
      beep(200, 200, soundLevel);
      if (lives <= 0) {
        gameOverScreen(score, 1, false);
        return;
      }
      bx = 64; by = 45;
      vx = 1.8f; vy = -2.2f;
      delay(600);
    }
    
    for (int r = 0; r < 4 && bricksLeft > 0; r++) {
      for (int c = 0; c < 10; c++) {
        if (!bricks[r][c]) continue;
        int bkx = c * 12 + 1;
        int bky = r * 6 + 1;
        if (bx + 3 >= bkx && bx <= bkx + 11 && by + 3 >= bky && by <= bky + 5) {
          bricks[r][c] = 0;
          bricksLeft--;
          score += (4 - r) * 10;
          float overlapL = bx + 3 - bkx, overlapR = bkx + 11 - bx;
          float overlapT = by + 3 - bky, overlapB = bky + 5 - by;
          if (min(overlapL, overlapR) < min(overlapT, overlapB))
            vx = -vx;
          else
            vy = -vy;
          beep(1200 + r * 120, 20, soundLevel);
          goto brick_done;
        }
      }
    }
    brick_done:
    
    if (bricksLeft == 0) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);
      centreStr("YOU WIN!", 28);
      u8g2.setFont(u8g2_font_6x10_tr);
      char buf[20];
      snprintf(buf, sizeof(buf), "Score: %u", score);
      centreStr(buf, 44);
      u8g2.sendBuffer();
      beep(1760, 80, soundLevel); delay(90);
      beep(1760, 80, soundLevel); delay(90);
      beep(2093, 200, soundLevel);
      delay(400);
      waitRelease();
      gameOverScreen(score, 1, true);
      return;
    }
    
    u8g2.clearBuffer();
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 10; c++) {
        if (!bricks[r][c]) continue;
        int bkx = c * 12 + 1, bky = r * 6 + 1;
        u8g2.drawBox(bkx, bky, 11, 5);
      }
    }
    u8g2.drawRBox(padX, padY, 30, 3, 1);
    u8g2.drawDisc((int)bx + 1, (int)by + 1, 2);
    u8g2.setFont(u8g2_font_5x7_tr);
    char sc[8];
    itoa(score, sc, 10);
    u8g2.drawStr(1, 63, sc);
    for (int i = 0; i < lives; i++) {
      u8g2.drawDisc(SCREEN_W - 6 - i * 6, 60, 2);
    }
    u8g2.sendBuffer();
    delay(20);
  }
}

// ============================================================
// GAME: DINO RUN (FIXED POSITION - Like Chrome Dino Game)
// ============================================================

void game_dino() {
  const float GRAVITY = 0.25f;
  const float JUMP_SPEED = -5.5f;    // Negative = upward
  const int DINO_W = 10;
  const int DINO_H = 12;
  const int GROUND_Y = 58;
  const float FIXED_DINO_X = 20;     // 🔥 Fixed X position

  float dinoX = FIXED_DINO_X;        // 🔥 Always fixed
  float dinoY = GROUND_Y - DINO_H;
  float velY = 0;
  bool isJumping = false;
  float obsX = 130;
  int obsType = 0;
  float obsSpeed = 2.0f;
  uint16_t score = 0;
  uint32_t lastFrame = millis();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("DINO RUN", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Any button to jump", 40);
  centreStr("Fixed position like", 52);
  centreStr("Chrome Dino game!", 62);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    if (checkPause("DINO RUN")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    float dt = (now - lastFrame) / 20.0f;
    if (dt > 2.0f) dt = 2.0f;
    if (dt < 0.5f) dt = 0.5f;
    lastFrame = now;

    // Jump detection - any button jumps
    bool anyBtn = btnPressed(BTN_UP) || btnPressed(BTN_DOWN) ||
                  btnPressed(BTN_LEFT) || btnPressed(BTN_RIGHT) ||
                  btnPressed(BTN_ENTER);
    
    if (anyBtn && !isJumping) {
      velY = JUMP_SPEED;  // Direct upward velocity
      isJumping = true;
      beep(1000, 15, soundLevel);
    }

    // Physics - apply gravity
    velY += GRAVITY * dt;
    dinoY += velY * dt;

    // 🔥 dinoX is FIXED - never changes!
    dinoX = FIXED_DINO_X;

    // Ground collision
    if (dinoY >= GROUND_Y - DINO_H) {
      dinoY = GROUND_Y - DINO_H;
      velY = 0;
      isJumping = false;
    }
    
    // Ceiling collision
    if (dinoY < 0) {
      dinoY = 0;
      velY = abs(velY) * 0.5f;
    }

    // Obstacles
    obsX -= obsSpeed * dt;
    if (obsX < -30) {
      obsX = 130 + random(0, 60);
      obsType = random(0, 2);
      score++;
      obsSpeed += 0.05f;
      
      // Speed up gradually
      if (score % 10 == 0) {
        beep(1500, 20, soundLevel);
      }
    }

    // Collision detection with obstacles
    int obsW, obsH, obsY;
    if (obsType == 0) {
      obsW = 12; obsH = 16; obsY = GROUND_Y - 16;
    } else {
      obsW = 16; obsH = 20; obsY = GROUND_Y - 20;
    }

    // 🔥 Collision with fixed position dino
    if (dinoX < obsX + obsW && dinoX + DINO_W > obsX && 
        dinoY < obsY + obsH && dinoY + DINO_H > obsY) {
      gameOverScreen(score, 2, false);
      return;
    }

    // ============================================
    // DRAW EVERYTHING
    // ============================================
    u8g2.clearBuffer();
    
    // Ground with grass effect
    u8g2.drawHLine(0, GROUND_Y, SCREEN_W);
    for (int i = 0; i < SCREEN_W; i += 10) {
      u8g2.drawPixel(i, GROUND_Y - 1);
      u8g2.drawPixel(i + 5, GROUND_Y - 2);
    }
    
    // Draw Dino (fixed X position)
    u8g2.drawFrame((int)dinoX, (int)dinoY, DINO_W, DINO_H);
    
    // Dino face/eye
    u8g2.drawBox((int)dinoX + 7, (int)dinoY + 2, 2, 2);
    
    // Dino legs (animation)
    if (!isJumping) {
      if ((millis() / 150) % 2 == 0) {
        // Left leg forward
        u8g2.drawLine((int)dinoX + 2, (int)dinoY + DINO_H, 
                      (int)dinoX + 1, (int)dinoY + DINO_H + 3);
        u8g2.drawLine((int)dinoX + 7, (int)dinoY + DINO_H, 
                      (int)dinoX + 8, (int)dinoY + DINO_H + 3);
      } else {
        // Right leg forward
        u8g2.drawLine((int)dinoX + 2, (int)dinoY + DINO_H, 
                      (int)dinoX + 3, (int)dinoY + DINO_H + 3);
        u8g2.drawLine((int)dinoX + 7, (int)dinoY + DINO_H, 
                      (int)dinoX + 6, (int)dinoY + DINO_H + 3);
      }
    }
    
    // Draw obstacles
    if (obsType == 0) {
      // Cactus type 1
      int x = (int)obsX, y = (int)obsY;
      u8g2.drawBox(x + 3, y, 4, 16);
      u8g2.drawBox(x, y + 4, 3, 6);
      u8g2.drawBox(x + 7, y + 3, 3, 7);
    } else {
      // Cactus type 2
      int x = (int)obsX, y = (int)obsY;
      u8g2.drawBox(x + 4, y, 6, 20);
      u8g2.drawBox(x, y + 6, 4, 8);
      u8g2.drawBox(x + 10, y + 5, 4, 9);
    }

    // Draw score with nice formatting
    u8g2.setFont(u8g2_font_6x10_tr);
    char sc[10];
    snprintf(sc, sizeof(sc), "SCORE: %d", score);
    u8g2.drawStr(SCREEN_W - u8g2.getStrWidth(sc) - 2, 10, sc);
    
    u8g2.sendBuffer();
    delay(20);
  }
}
// ============================================================
// GAME: FLAPPY BIRD (Random Gap Between 36-42)
// ============================================================

void game_flappy() {
  const float GRAVITY = 0.18f;
  const float FLAP_VEL = -2.2f;
  const int PIPE_W = 8;
  const int BIRD_X = 14;
  const int BIRD_W = 7;
  const int BIRD_H = 5;
  const float INITIAL_SPEED = 1.2f;

  float birdY = 30, vel = 0;
  int pX[3] = {128, 0, 0};
  int pGapY[3];
  int pipeGap[3];  // 🔥 NEW: Store individual gap for each pipe
  int score = 0;
  bool scored[3] = {false, false, false};
  uint32_t lastFrame = millis();
  float currentSpeed = INITIAL_SPEED;
  int activePipes = 2;

  // 🔥 Function to get random gap between 36-42
  auto getRandomGap = [&]() {
    // 36, 38, 40, 42 - choose randomly
    const int gaps[] = {36, 38, 40, 42};
    return gaps[random(0, 4)];
  };

  // Initialize pipes with random gaps
  for(int i = 0; i < 3; i++) {
    pipeGap[i] = getRandomGap();  // 🔥 Random gap for each pipe
    pGapY[i] = random(8, SCREEN_H - pipeGap[i] - 8);
    scored[i] = false;
  }
  int initialDistance = random(30, 60);
  pX[1] = pX[0] + initialDistance;
  pX[2] = pX[1] + random(30, 60);

  // Show start screen
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("FLAPPY BIRD", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Any button = Flap", 40);
  centreStr("Gap: 36-42 Random", 54);
  u8g2.sendBuffer();
  waitRelease();
  while (!btnPressed(BTN_UP) && !btnPressed(BTN_DOWN) &&
         !btnPressed(BTN_LEFT) && !btnPressed(BTN_RIGHT))
    delay(10);
  vel = FLAP_VEL;
  beep(900, 25, soundLevel);
  lastFrame = millis();

  while (true) {
    if (checkPause("FLAPPY BIRD")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    float dt = (now - lastFrame) / 20.0f;
    if (dt > 2.0f) dt = 2.0f;
    if (dt < 0.5f) dt = 0.5f;
    lastFrame = now;

    // Difficulty settings
    int minRange, maxRange;
    if(score >= 0 && score <= 20) { minRange = 30; maxRange = 60; }
    else if(score >= 21 && score <= 40) { minRange = 30; maxRange = 50; }
    else if(score >= 41 && score <= 60) { minRange = 25; maxRange = 40; }
    else if(score >= 61 && score <= 80) { minRange = 20; maxRange = 30; }
    else { minRange = 25; maxRange = 25; }

    currentSpeed = INITIAL_SPEED + (score * 0.02f);
    if (currentSpeed > 8.0f) currentSpeed = 8.0f;

    // Flap
    bool anyBtn = btnPressed(BTN_UP) || btnPressed(BTN_DOWN) ||
                  btnPressed(BTN_LEFT) || btnPressed(BTN_RIGHT);
    if (anyBtn) { 
      vel = FLAP_VEL; 
      beep(900, 20, soundLevel); 
    }

    // Physics
    vel += GRAVITY * dt;
    if (vel > 6.0f) vel = 6.0f;
    if (vel < -7.0f) vel = -7.0f;
    birdY += vel * dt;

    // 🔥 Update pipes with random gaps
    for (int i = 0; i < activePipes; i++) {
      pX[i] -= (int)(currentSpeed * dt);
      
      if (pX[i] < -PIPE_W) {
        int horizontalGap = random(minRange, maxRange + 1);
        int farthestX = -999;
        for(int j = 0; j < activePipes; j++) {
          if(pX[j] > farthestX) farthestX = pX[j];
        }
        pX[i] = farthestX + horizontalGap;
        
        // 🔥 NEW: Generate random gap for this pipe
        pipeGap[i] = getRandomGap();  // 36-42 এর মধ্যে র্যান্ডম
        
        pGapY[i] = random(8, SCREEN_H - pipeGap[i] - 8);
        scored[i] = false;
        
        if(score > 15 && activePipes < 3) {
          activePipes = 3;
          int thirdGap = random(minRange, maxRange + 1);
          pX[2] = pX[1] + thirdGap;
          pipeGap[2] = getRandomGap();  // 🔥 Random gap for third pipe
          pGapY[2] = random(8, SCREEN_H - pipeGap[2] - 8);
          scored[2] = false;
        }
      }
      
      // Score point when passing pipe
      if (!scored[i] && pX[i] + PIPE_W < BIRD_X) {
        score++;
        scored[i] = true;
        beep(1300, 20, soundLevel);
      }
    }

    // Collision with top/bottom
    int by = (int)birdY;
    if (by < 0 || by + BIRD_H >= SCREEN_H) {
      gameOverScreen(score, 3, false);
      return;
    }

    // 🔥 Collision with pipes (using individual gaps)
    for (int i = 0; i < activePipes; i++) {
      if (BIRD_X + BIRD_W > pX[i] && BIRD_X < pX[i] + PIPE_W) {
        if (by < pGapY[i] || by + BIRD_H > pGapY[i] + pipeGap[i]) {
          gameOverScreen(score, 3, false);
          return;
        }
      }
    }

    // ============================================
    // DRAW EVERYTHING
    // ============================================
    u8g2.clearBuffer();
    
    // Draw pipes with gap display
    for (int i = 0; i < activePipes; i++) {
      if(pX[i] < SCREEN_W + 20 && pX[i] > -20) {
        // Top pipe
        u8g2.drawBox(pX[i], 0, PIPE_W, pGapY[i]);
        u8g2.drawBox(pX[i] - 1, pGapY[i] - 4, PIPE_W + 2, 5);
        
        // Bottom pipe
        int bottomPipeY = pGapY[i] + pipeGap[i];
        int bottomPipeHeight = SCREEN_H - bottomPipeY;
        u8g2.drawBox(pX[i], bottomPipeY, PIPE_W, bottomPipeHeight);
        u8g2.drawBox(pX[i] - 1, bottomPipeY - 1, PIPE_W + 2, 5);
        
        // 🔥 Show gap size on pipe (optional)
        u8g2.setFont(u8g2_font_5x7_tr);
        char gapStr[4];
        snprintf(gapStr, sizeof(gapStr), "%d", pipeGap[i]);
        u8g2.drawStr(pX[i] + 1, pGapY[i] + 6, gapStr);
      }
    }

    // Draw bird
    u8g2.drawBox(BIRD_X, by, BIRD_W, BIRD_H);
    
    // Draw score
    u8g2.setFont(u8g2_font_6x10_tr);
    char sc[6];
    itoa(score, sc, 10);
    u8g2.drawStr(2, 10, sc);
    
    u8g2.sendBuffer();
    delay(16);
  }
}

// ============================================================
// GAME: SNAKE 1 (Fixed Food Spawning)
// ============================================================

#define SN_COLS 21
#define SN_ROWS 10
#define SN_SZ 6
#define SN_MAXLEN 80

void game_snake1() {
  int sx[SN_MAXLEN], sy[SN_MAXLEN];
  int len = 4;
  int dx = 1, dy = 0;
  int next_dx = 1, next_dy = 0;
  int fx, fy;
  uint32_t lastMove = 0;
  uint16_t spd = 210;
  uint16_t score = 0;
  bool foodEaten = false;  // Track if food was just eaten

  auto placeFood = [&]() {
    bool ok;
    int attempts = 0;
    do {
      ok = true;
      fx = random(0, SN_COLS);
      fy = random(0, SN_ROWS);
      attempts++;
      
      // Check if food is on snake body
      for (int i = 0; i < len; i++) {
        if (sx[i] == fx && sy[i] == fy) { 
          ok = false; 
          break; 
        }
      }
      
      // NEW: Food should not be too close to snake head
      if (ok && len > 0) {
        int dx = abs(fx - sx[0]);
        int dy = abs(fy - sy[0]);
        // Minimum 3 blocks away from head
        if (dx < 3 && dy < 3) {
          ok = false;
        }
      }
      
      // NEW: Food should not be at the edge (optional)
      if (ok) {
        if (fx == 0 || fx == SN_COLS - 1 || fy == 0 || fy == SN_ROWS - 1) {
          // Allow edge spawning, but reduce probability
          if (random(0, 100) < 30) {
            ok = false;
          }
        }
      }
      
      // Safety: if too many attempts, place anywhere valid
      if (attempts > 100) {
        for (int i = 0; i < SN_COLS; i++) {
          for (int j = 0; j < SN_ROWS; j++) {
            bool onSnake = false;
            for (int k = 0; k < len; k++) {
              if (sx[k] == i && sy[k] == j) {
                onSnake = true;
                break;
              }
            }
            if (!onSnake) {
              fx = i;
              fy = j;
              return;
            }
          }
        }
      }
      
    } while (!ok);
  };

  // Initialize snake
  for (int i = 0; i < len; i++) {
    sx[i] = len - 1 - i;
    sy[i] = SN_ROWS / 2;
  }
  placeFood();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("SNAKE 1", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Use buttons to steer", 40);
  centreStr("Eat to grow!", 54);
  u8g2.sendBuffer();
  delay(1400);
  waitRelease();

  while (true) {
    if (checkPause("SNAKE 1")) return;
    if (checkMenuAndReturn()) return;
    
    // Direction control
    if (btnHeld(BTN_UP) && dy != 1) { next_dx = 0; next_dy = -1; }
    if (btnHeld(BTN_DOWN) && dy != -1) { next_dx = 0; next_dy = 1; }
    if (btnHeld(BTN_LEFT) && dx != 1) { next_dx = -1; next_dy = 0; }
    if (btnHeld(BTN_RIGHT) && dx != -1) { next_dx = 1; next_dy = 0; }

    uint32_t now = millis();
    if (now - lastMove < spd) { delay(8); continue; }
    lastMove = now;

    dx = next_dx; dy = next_dy;
    int nx = (sx[0] + dx + SN_COLS) % SN_COLS;
    int ny = (sy[0] + dy + SN_ROWS) % SN_ROWS;

    // Self collision
    for (int i = 1; i < len; i++) {
      if (sx[i] == nx && sy[i] == ny) {
        gameOverScreen(score, 4, false);
        return;
      }
    }

    // Move snake
    for (int i = len - 1; i > 0; i--) {
      sx[i] = sx[i - 1]; 
      sy[i] = sy[i - 1];
    }
    sx[0] = nx; 
    sy[0] = ny;

    // Eat food
    if (nx == fx && ny == fy) {
      score++;
      if (len < SN_MAXLEN) len++;
      spd = max(70, (int)spd - 7);
      beep(1400, 25, soundLevel);
      
      // Place new food away from head
      placeFood();
      
      // Make sure food is not on snake head
      while (fx == sx[0] && fy == sy[0]) {
        placeFood();
      }
    }

    // Draw
    u8g2.clearBuffer();
    for (int i = 0; i < len; i++) {
      int px = 1 + sx[i] * SN_SZ;
      int py = 2 + sy[i] * SN_SZ;
      if (i == 0) {
        u8g2.drawBox(px, py, SN_SZ, SN_SZ);  // Head
      } else {
        u8g2.drawFrame(px + 1, py + 1, SN_SZ - 2, SN_SZ - 2);  // Body
      }
    }
    
    // Draw food
    u8g2.drawDisc(1 + fx * SN_SZ + 3, 2 + fy * SN_SZ + 3, 3);
    
    // Score
    u8g2.setFont(u8g2_font_5x7_tr);
    char sc[6];
    itoa(score, sc, 10);
    u8g2.drawStr(1 + SN_COLS * SN_SZ + 4, 10, "SC");
    u8g2.drawStr(1 + SN_COLS * SN_SZ + 4, 20, sc);
    
    u8g2.sendBuffer();
  }
}


// ============================================================
// GAME: SNAKE 2 (Custom Shrink & Grow - 13 to 7 Cycle)
// ============================================================

#define SN2_COLS 20
#define SN2_ROWS 9
#define SN2_SZ 6
#define SN2_MAXLEN 80
#define SN2_OX 2
#define SN2_OY 2

void game_snake2() {
  int sx[SN2_MAXLEN], sy[SN2_MAXLEN];
  int len = 4;
  int dx = 1, dy = 0;
  int next_dx = 1, next_dy = 0;
  int fx, fy;
  uint32_t lastMove = 0;
  uint16_t spd = 210;
  uint16_t score = 0;
  int foodEaten = 0;
  int level = 1;
  
  bool isShrinking = true;
  const int MAX_LENGTH = 13;
  const int MIN_LENGTH = 5;

  auto placeFood = [&]() {
    bool ok;
    int attempts = 0;
    do {
      ok = true;
      fx = random(0, SN2_COLS);
      fy = random(0, SN2_ROWS);
      attempts++;
      
      for (int i = 0; i < len; i++) {
        if (sx[i] == fx && sy[i] == fy) { 
          ok = false; 
          break; 
        }
      }
      
      if (ok && len > 0) {
        int dx = abs(fx - sx[0]);
        int dy = abs(fy - sy[0]);
        if (dx < 3 && dy < 3) {
          ok = false;
        }
      }
      
      if (attempts > 100) {
        for (int i = 0; i < SN2_COLS; i++) {
          for (int j = 0; j < SN2_ROWS; j++) {
            bool onSnake = false;
            for (int k = 0; k < len; k++) {
              if (sx[k] == i && sy[k] == j) {
                onSnake = true;
                break;
              }
            }
            if (!onSnake) {
              fx = i;
              fy = j;
              return;
            }
          }
        }
      }
      
    } while (!ok);
  };

  // Initialize snake
  for (int i = 0; i < len; i++) {
    sx[i] = len - 1 - i;
    sy[i] = SN2_ROWS / 2;
  }
  placeFood();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("SNAKE 2", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Length cycles: 13→7→13", 40);
  centreStr("Eat 10 to change!", 54);
  u8g2.sendBuffer();
  delay(1400);
  waitRelease();

  while (true) {
    if (checkPause("SNAKE 2")) return;
    if (checkMenuAndReturn()) return;
    
    if (btnHeld(BTN_UP) && dy != 1) { next_dx = 0; next_dy = -1; }
    if (btnHeld(BTN_DOWN) && dy != -1) { next_dx = 0; next_dy = 1; }
    if (btnHeld(BTN_LEFT) && dx != 1) { next_dx = -1; next_dy = 0; }
    if (btnHeld(BTN_RIGHT) && dx != -1) { next_dx = 1; next_dy = 0; }

    uint32_t now = millis();
    if (now - lastMove < spd) { delay(8); continue; }
    lastMove = now;

    dx = next_dx; dy = next_dy;
    int nx = sx[0] + dx;
    int ny = sy[0] + dy;

    if (nx < 0 || nx >= SN2_COLS || ny < 0 || ny >= SN2_ROWS) {
      gameOverScreen(score, 5, false);
      return;
    }

    for (int i = 1; i < len; i++) {
      if (sx[i] == nx && sy[i] == ny) {
        gameOverScreen(score, 5, false);
        return;
      }
    }

    for (int i = len - 1; i > 0; i--) {
      sx[i] = sx[i - 1]; 
      sy[i] = sy[i - 1];
    }
    sx[0] = nx; 
    sy[0] = ny;

    if (nx == fx && ny == fy) {
      score++;
      foodEaten++;
      if (len < SN2_MAXLEN) len++;
      spd = max(70, (int)spd - 7);
      beep(1400, 25, soundLevel);
      
      placeFood();
      
      if (foodEaten >= 10) {
        foodEaten = 0;
        level++;
        
        if (isShrinking) {
          if (len >= MAX_LENGTH) {
            int newLen = len - 1;
            if (newLen >= MIN_LENGTH) {
              for (int i = newLen; i < len; i++) {
                sx[i] = -1;
                sy[i] = -1;
              }
              len = newLen;
              
              if (len <= MIN_LENGTH) {
                isShrinking = false;
                beep(1200, 60, soundLevel);
                delay(80);
                beep(1500, 60, soundLevel);
                delay(80);
                beep(1800, 80, soundLevel);
              } else {
                beep(800, 40, soundLevel);
              }
              
              if (spd < 150) spd += 10;
            }
          }
        }
        else {
          if (len <= MIN_LENGTH) {
            if (len < MAX_LENGTH) {
              int tailX = sx[len - 1];
              int tailY = sy[len - 1];
              sx[len] = tailX;
              sy[len] = tailY;
              len++;
              
              if (len >= MAX_LENGTH) {
                isShrinking = true;
                beep(1800, 60, soundLevel);
                delay(80);
                beep(1500, 60, soundLevel);
                delay(80);
                beep(1200, 80, soundLevel);
              } else {
                beep(1200, 40, soundLevel);
              }
            }
          }
        }
        
        if (len < MIN_LENGTH) len = MIN_LENGTH;
        if (len > MAX_LENGTH) len = MAX_LENGTH;
      }
    }

    // ========== DRAW ONLY THE GRID AND SNAKE ==========
    u8g2.clearBuffer();
    
    // Draw grid border
    int bx = SN2_OX - 1, by = SN2_OY - 1;
    int bw = SN2_COLS * SN2_SZ + 2, bh = SN2_ROWS * SN2_SZ + 2;
    u8g2.drawFrame(bx, by, bw, bh);
    
    // Draw snake
    for (int i = 0; i < len; i++) {
      int px = SN2_OX + sx[i] * SN2_SZ;
      int py = SN2_OY + sy[i] * SN2_SZ;
      if (i == 0) {
        u8g2.drawBox(px, py, SN2_SZ, SN2_SZ);  // Snake head
        u8g2.setDrawColor(0);
        u8g2.drawPixel(px + 2, py + 2);
        u8g2.drawPixel(px + SN2_SZ - 3, py + 2);
        u8g2.setDrawColor(1);
      } else {
        u8g2.drawFrame(px + 1, py + 1, SN2_SZ - 2, SN2_SZ - 2);  // Snake body
      }
    }
    
    // Draw food
    u8g2.drawDisc(SN2_OX + fx * SN2_SZ + 3, SN2_OY + fy * SN2_SZ + 3, 3);
    
    u8g2.sendBuffer();
    delay(20);
  }
}

void game_pong() {
  const int PAD_H = 23;
  const int PAD_W = 3;
  const int WIN = 7;

  float bx = 64, by = 32;
  float vx = 2.5f, vy = 1.8f;
  int pY = 25, cY = 25;
  int pScore = 0, cScore = 0;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("PONG", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("UP/DOWN = move", 40);
  centreStr("First to 7 wins!", 54);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    if (checkPause("PONG")) return;
    if (checkMenuAndReturn()) return;
    
    if (btnHeld(BTN_UP)) pY = max(0, pY - 7);
    if (btnHeld(BTN_DOWN)) pY = min(SCREEN_H - PAD_H, pY + 7);

    // FIXED: Better CPU AI with speed adjustment
    int mid = cY + PAD_H / 2;
    int dist = abs((int)by - mid);
    int cpuSpeed = 2 + (dist / 6);
    if (cpuSpeed > 5) cpuSpeed = 5;
    
    if (mid < (int)by - 1) cY = min(SCREEN_H - PAD_H, cY + cpuSpeed);
    if (mid > (int)by + 1) cY = max(0, cY - cpuSpeed);

    bx += vx; by += vy;

    if (by <= 0) { vy = fabsf(vy); by = 0; beep(500, 10, soundLevel); }
    if (by >= SCREEN_H - 3) { vy = -fabsf(vy); by = SCREEN_H - 3; beep(500, 10, soundLevel); }

    if (vx < 0 && bx <= 4 + PAD_W && bx >= 4 && by + 2 >= cY && by <= cY + PAD_H) {
      vx = fabsf(vx) * 1.05f;
      vy += ((by - (cY + PAD_H / 2.0f)) / (PAD_H / 2.0f)) * 1.2f;
      vy = constrain(vy, -4.0f, 4.0f);
      vx = min(vx, 5.0f);
      bx = 4 + PAD_W;
      beep(900, 15, soundLevel);
    }

    if (bx >= 121 - PAD_W && bx <= 122 && by + 2 >= pY && by <= pY + PAD_H) {
      vx = -fabsf(vx) * 1.05f;
      vy += ((by - (pY + PAD_H / 2.0f)) / (PAD_H / 2.0f)) * 1.5f;
      vy = constrain(vy, -4.5f, 4.5f);
      vx = max(vx, -5.0f);
      bx = 121 - PAD_W - 1;
      beep(1000, 15, soundLevel);
    }

    if (bx < 0) { pScore++; beep(1400, 80, soundLevel); bx = 64; by = 32; vx = 2.5f; vy = 1.8f; pY = 25; cY = 25; delay(600); }
    if (bx > SCREEN_W) { cScore++; beep(200, 200, soundLevel); bx = 64; by = 32; vx = -2.5f; vy = 1.8f; pY = 25; cY = 25; delay(600); }

    if (pScore >= WIN || cScore >= WIN) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);
      centreStr(pScore >= WIN ? "YOU WIN!" : "CPU WINS", 28);
      u8g2.setFont(u8g2_font_6x10_tr);
      char buf[20];
      snprintf(buf, sizeof(buf), "%d  :  %d", pScore, cScore);
      centreStr(buf, 44);
      u8g2.sendBuffer();
      delay(400);
      waitRelease();
      bool isWin = (pScore >= WIN);
      gameOverScreen(isWin ? pScore : cScore, 6, isWin);
      return;
    }

    u8g2.clearBuffer();
    for (int y = 0; y < SCREEN_H; y += 6) u8g2.drawPixel(63, y);
    u8g2.drawBox(4, cY, PAD_W, PAD_H);
    u8g2.drawBox(121, pY, PAD_W, PAD_H);
    u8g2.drawBox((int)bx, (int)by, 3, 3);

    u8g2.setFont(u8g2_font_6x10_tr);
    char ps[4], cs[4];
    itoa(pScore, ps, 10);
    itoa(cScore, cs, 10);
    u8g2.drawStr(42, 10, cs);
    u8g2.drawStr(78, 10, ps);
    u8g2.sendBuffer();
    delay(16);
  }
}

// ============================================================
// GAME: PACMAN (Fixed grid calculation)
// ============================================================

void game_pacman() {
  int px = 64, py = 32;
  int pdx = 0, pdy = 0;
  int score = 0;

  bool maze[5][10];
  for (int y = 0; y < 5; y++)
    for (int x = 0; x < 10; x++)
      maze[y][x] = true;

  int gx = 10, gy = 10;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("PAC-MAN", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Use buttons to move", 40);
  centreStr("Eat all dots!", 54);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    if (checkPause("PACMAN")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t start = millis();

    if (btnHeld(BTN_UP)) { pdx = 0; pdy = -2; }
    if (btnHeld(BTN_DOWN)) { pdx = 0; pdy = 2; }
    if (btnHeld(BTN_LEFT)) { pdx = -2; pdy = 0; }
    if (btnHeld(BTN_RIGHT)) { pdx = 2; pdy = 0; }

    px += pdx; py += pdy;
    if (px < 4) px = 4;
    if (px > SCREEN_W - 4) px = SCREEN_W - 4;
    if (py < 4) py = 4;
    if (py > SCREEN_H - 4) py = SCREEN_H - 4;

    // FIXED: Better grid position calculation
    int mx = (px * 10) / SCREEN_W;
    int my = (py * 5) / SCREEN_H;
    if (mx >= 0 && mx < 10 && my >= 0 && my < 5) {
      if (maze[my][mx]) {
        maze[my][mx] = false;
        score += 10;
        beep(1000, 8, soundLevel);
      }
    }

    static uint8_t gSpeedCount = 0;
    if (++gSpeedCount % 2 == 0) {
      if (gx < px) gx++;
      else if (gx > px) gx--;
      if (gy < py) gy++;
      else if (gy > py) gy--;
    }

    if (abs(px - gx) < 6 && abs(py - gy) < 6) {
      gameOverScreen(score, 7, false);
      return;
    }

    bool win = true;
    for (int y = 0; y < 5; y++)
      for (int x = 0; x < 10; x++)
        if (maze[y][x]) win = false;

    if (win) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);
      centreStr("YOU WIN!", 32);
      u8g2.sendBuffer();
      delay(2000);
      gameOverScreen(score, 7, true);
      return;
    }

    u8g2.clearBuffer();
    for (int y = 0; y < 5; y++) {
      for (int x = 0; x < 10; x++) {
        if (maze[y][x]) {
          int dx = (x * SCREEN_W / 10) + (SCREEN_W / 20);
          int dy = (y * SCREEN_H / 5) + (SCREEN_H / 10);
          u8g2.drawPixel(dx, dy);
        }
      }
    }
    u8g2.drawDisc(px, py, 4);
    u8g2.drawFrame(gx - 3, gy - 3, 7, 7);
    u8g2.setFont(u8g2_font_5x7_tr);
    char sc[10];
    itoa(score, sc, 10);
    u8g2.drawStr(1, 63, sc);
    u8g2.sendBuffer();
    delay(30);
  }
}

// ============================================================
// GAME: SPACE INVADERS (Fixed bullet detection)
// ============================================================

void game_spaceinvaders() {
  bool inv[3][8];
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 8; c++)
      inv[r][c] = true;
  int invLeft = 24;
  int gridX = 4, gridY = 4;
  int8_t gridDX = 1;
  uint32_t lastInvMove = 0;
  uint16_t invSpeed = 500;
  int shipX = SCREEN_W / 2 - 4;
  const int shipY = SCREEN_H - 10;
  
  struct Bullet { float x, y; bool on; };
  Bullet pb[3], eb[3];
  for (auto &b : pb) b.on = false;
  for (auto &b : eb) b.on = false;
  
  uint32_t lastShoot = 0, lastEnemyShoot = 0;
  uint16_t score = 0;
  uint8_t wave = 1;
  uint32_t lastFrame = millis();
  bool animFrame = false;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("INVADERS", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("L/R = Move", 40);
  centreStr("ENTER = Fire", 52);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    if (checkPause("SPACE INVADERS")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    float dt = (now - lastFrame) / 20.0f;
    lastFrame = now;
    animFrame = (now / 300) % 2;
    
    if (btnHeld(BTN_LEFT)) shipX = max(0, shipX - 3);
    if (btnHeld(BTN_RIGHT)) shipX = min(SCREEN_W - 10, shipX + 3);
    
    if (btnPressed(BTN_ENTER) && now - lastShoot > 450) {
      for (auto &b : pb)
        if (!b.on) {
          b.x = shipX + 4; b.y = shipY - 1; b.on = true;
          beep(1500, 10, soundLevel);
          break;
        }
      lastShoot = now;
    }
    
    for (auto &b : pb) {
      if (!b.on) continue;
      b.y -= 4.0f * dt;
      if (b.y < 0) { b.on = false; continue; }
      int bc = (int(b.x) - gridX) / 9, br = (int(b.y) - gridY) / 8;
      if (bc >= 0 && bc < 8 && br >= 0 && br < 3 && inv[br][bc]) {
        int ix = gridX + bc * 9, iy = gridY + br * 8;
        if (b.x >= ix && b.x <= ix + 7 && b.y >= iy && b.y <= iy + 5) {
          inv[br][bc] = false;
          invLeft--;
          score += (3 - br) * 10;
          b.on = false;
          beep(800 - br * 100, 30, soundLevel);
        }
      }
    }
    
    if (invLeft > 0 && now - lastEnemyShoot > max(400U, 1200U - score * 2)) {
      lastEnemyShoot = now;
      int tries = 20;
      while (tries--) {
        int c = random(0, 8);
        for (int r = 2; r >= 0; r--) {
          if (inv[r][c]) {
            for (auto &b : eb)
              if (!b.on) {
                b.x = gridX + c * 9 + 3.5f;
                b.y = gridY + r * 8 + 5;
                b.on = true;
                break;
              }
            goto shot_done;
          }
        }
      }
      shot_done:;
    }
    
    // FIXED: Better bullet collision detection
    for (auto &b : eb) {
      if (!b.on) continue;
      b.y += 3.0f * dt;
      if (b.y > SCREEN_H) { b.on = false; continue; }
      if (b.x >= shipX && b.x <= shipX + 10 && b.y >= shipY && b.y <= shipY + 7) {
        gameOverScreen(score, 8, false);
        return;
      }
    }
    
    if (now - lastInvMove > invSpeed) {
      lastInvMove = now;
      gridX += gridDX;
      int leftC = 8, rightC = -1;
      for (int r = 0; r < 3; r++)
        for (int c = 0; c < 8; c++)
          if (inv[r][c]) {
            leftC = min(leftC, c);
            rightC = max(rightC, c);
          }
      if (gridX + rightC * 9 + 7 >= SCREEN_W - 2 || gridX + leftC * 9 <= 2) {
        gridDX = -gridDX;
        gridY += 3;
      }
      if (gridY + 3 * 8 >= shipY - 2) {
        gameOverScreen(score, 8, false);
        return;
      }
    }
    
    if (invLeft == 0) {
      wave++;
      invSpeed = (uint16_t)max(80, (int)invSpeed - 60);
      for (int r = 0; r < 3; r++)
        for (int c = 0; c < 8; c++)
          inv[r][c] = true;
      invLeft = 24;
      gridX = 4; gridY = 4; gridDX = 1;
      beep(1760, 80, soundLevel); delay(90);
      beep(2093, 160, soundLevel); delay(300);
    }
    
    u8g2.clearBuffer();
    for (int r = 0; r < 3; r++) {
      for (int c = 0; c < 8; c++) {
        if (!inv[r][c]) continue;
        int ix = gridX + c * 9, iy = gridY + r * 8;
        u8g2.drawBox(ix + 1, iy, 5, 2);
        u8g2.drawBox(ix, iy + 2, 7, 2);
      }
    }
    
    u8g2.drawBox(shipX + 3, shipY, 4, 2);
    u8g2.drawBox(shipX + 1, shipY + 2, 8, 3);
    u8g2.drawBox(shipX, shipY + 4, 10, 3);
    
    for (auto &b : pb) if (b.on) u8g2.drawBox((int)b.x, (int)b.y, 1, 4);
    for (auto &b : eb) if (b.on) u8g2.drawPixel((int)b.x, (int)b.y);
    
    u8g2.setFont(u8g2_font_5x7_tr);
    char sc[8];
    itoa(score, sc, 10);
    u8g2.drawStr(1, 8, sc);
    char wv[8];
    snprintf(wv, sizeof(wv), "W%u", wave);
    u8g2.drawStr(SCREEN_W - 18, 8, wv);
    u8g2.sendBuffer();
    delay(20);
  }
}

// ============================================================
// GAME: TETRIS (Fixed scoring)
// ============================================================

#define TT_COLS 8
#define TT_ROWS 10
#define TT_SZ 6
#define TT_OX 40
#define TT_OY 2

struct TetPiece {
  int8_t x, y;
  uint8_t type;
  int8_t cells[4][2];
};

const int8_t PIECES[7][4][2] PROGMEM = {
  {{0,0},{1,0},{2,0},{3,0}},
  {{0,0},{0,1},{1,1},{2,1}},
  {{2,0},{0,1},{1,1},{2,1}},
  {{0,0},{1,0},{1,1},{2,1}},
  {{1,0},{2,0},{0,1},{1,1}},
  {{1,0},{0,1},{1,1},{2,1}},
  {{0,0},{1,0},{0,1},{1,1}},
};

uint8_t board[TT_ROWS][TT_COLS];

void loadPiece(TetPiece &p, uint8_t t) {
  p.type = t;
  for (int i = 0; i < 4; i++) {
    p.cells[i][0] = pgm_read_byte(&PIECES[t][i][0]);
    p.cells[i][1] = pgm_read_byte(&PIECES[t][i][1]);
  }
}

bool ttFits(TetPiece &p, int dx, int dy) {
  for (int i = 0; i < 4; i++) {
    int nx = p.x + p.cells[i][0] + dx;
    int ny = p.y + p.cells[i][1] + dy;
    if (nx < 0 || nx >= TT_COLS || ny >= TT_ROWS) return false;
    if (ny >= 0 && board[ny][nx]) return false;
  }
  return true;
}

void ttRotate(TetPiece &p) {
  int8_t tmp[4][2];
  for (int i = 0; i < 4; i++) {
    int8_t rx = p.cells[i][0] - p.cells[0][0];
    int8_t ry = p.cells[i][1] - p.cells[0][1];
    tmp[i][0] = p.cells[0][0] - ry;
    tmp[i][1] = p.cells[0][1] + rx;
  }
  int8_t saved[4][2];
  memcpy(saved, p.cells, sizeof(saved));
  memcpy(p.cells, tmp, sizeof(tmp));
  if (!ttFits(p, 0, 0)) memcpy(p.cells, saved, sizeof(saved));
}

void game_tetris() {
  memset(board, 0, sizeof(board));
  uint16_t score = 0;
  uint8_t level = 1;
  uint32_t dropInterval = 600, lastDrop = millis(), lastMove = 0;
  TetPiece cur, next;
  loadPiece(cur, random(0, 7));
  cur.x = TT_COLS / 2 - 1; cur.y = 0;
  loadPiece(next, random(0, 7));

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("TETRIS", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("L/R=move  UP=rot", 40);
  centreStr("DN=fast drop", 54);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    if (checkPause("TETRIS")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    if (now - lastMove > 130) {
      if (btnHeld(BTN_LEFT) && ttFits(cur, -1, 0)) { cur.x--; lastMove = now; beep(700, 10, soundLevel); }
      if (btnHeld(BTN_RIGHT) && ttFits(cur, 1, 0)) { cur.x++; lastMove = now; beep(700, 10, soundLevel); }
    }
    if (btnPressed(BTN_UP)) { ttRotate(cur); beep(900, 10, soundLevel); }
    
    uint32_t interval = btnHeld(BTN_DOWN) ? 60 : dropInterval;
    if (now - lastDrop > interval) {
      lastDrop = now;
      if (ttFits(cur, 0, 1)) {
        cur.y++;
      } else {
        int8_t abs[4][2];
        for (int i = 0; i < 4; i++) {
          abs[i][0] = cur.x + cur.cells[i][0];
          abs[i][1] = cur.y + cur.cells[i][1];
          if (abs[i][1] >= 0) board[abs[i][1]][abs[i][0]] = 1;
        }
        beep(500, 15, soundLevel);
        int cleared = 0;
        for (int r = TT_ROWS - 1; r >= 0; r--) {
          bool full = true;
          for (int c = 0; c < TT_COLS; c++) if (!board[r][c]) { full = false; break; }
          if (full) {
            cleared++;
            for (int rr = r; rr > 0; rr--) memcpy(board[rr], board[rr-1], TT_COLS);
            memset(board[0], 0, TT_COLS);
            r++;
          }
        }
        if (cleared) {
          // FIXED: Standard Tetris scoring without level multiplier
          static const uint16_t pts[5] = {0, 40, 100, 300, 1200};
          score += pts[min(cleared, 4)];
          beep(1200, 30, soundLevel); delay(35); beep(1400, 30, soundLevel);
          level = 1 + score / 200;
          dropInterval = max(80U, 600U - (level - 1) * 60);
        }
        cur = next;
        cur.x = TT_COLS / 2 - 1; cur.y = 0;
        loadPiece(next, random(0, 7));
        if (!ttFits(cur, 0, 0)) {
          gameOverScreen(score, 9, false);
          return;
        }
      }
    }
    
    u8g2.clearBuffer();
    u8g2.drawFrame(TT_OX - 1, TT_OY - 1, TT_COLS * TT_SZ + 2, TT_ROWS * TT_SZ + 2);
    for (int r = 0; r < TT_ROWS; r++)
      for (int c = 0; c < TT_COLS; c++)
        if (board[r][c]) u8g2.drawBox(TT_OX + c * TT_SZ, TT_OY + r * TT_SZ, TT_SZ - 1, TT_SZ - 1);
    
    int8_t abs[4][2];
    for (int i = 0; i < 4; i++) {
      abs[i][0] = cur.x + cur.cells[i][0];
      abs[i][1] = cur.y + cur.cells[i][1];
      if (abs[i][1] >= 0) u8g2.drawBox(TT_OX + abs[i][0] * TT_SZ, TT_OY + abs[i][1] * TT_SZ, TT_SZ - 1, TT_SZ - 1);
    }
    
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(1, 10, "SCR");
    char sc[8]; itoa(score, sc, 10);
    u8g2.drawStr(1, 18, sc);
    u8g2.drawStr(1, 30, "LVL");
    char lv[4]; itoa(level, lv, 10);
    u8g2.drawStr(1, 38, lv);
    u8g2.sendBuffer();
    delay(16);
  }
}

// ============================================================
// GAME: TANK BATTLE (UPGRADED - আরও মজাদার!)
// ============================================================

void game_tank() {
  // ==========================================
  // GAME SETTINGS
  // ==========================================
  const int TANK_SIZE = 10;
  const int PLAYER_SPEED = 2;
  const int BULLET_SPEED = 4;
  const int MAX_ENEMIES = 5;
  const int MAX_PLAYER_BULLETS = 5;
  const int MAX_ENEMY_BULLETS = 10;
  const int SPAWN_DELAY = 3000;
  const int POWERUP_DURATION = 5000;  // 5 seconds
  // ==========================================

  struct Tank {
    float x, y;
    int8_t dx, dy;
    int8_t facingDx, facingDy;
    bool active;
    uint32_t lastMove;
    uint32_t lastShot;
    int health;
    int speed;
    uint32_t spawnTime;
  };
  
  struct Bullet {
    float x, y;
    int8_t dx, dy;
    bool active;
    bool isPlayer;
    int damage;
  };
  
  struct PowerUp {
    float x, y;
    int type;  // 0 = Health, 1 = Speed, 2 = Triple Shot, 3 = Shield
    bool active;
    uint32_t spawnTime;
  };

  // ==========================================
  // PLAYER TANK
  // ==========================================
  Tank player;
  player.x = SCREEN_W / 2 - TANK_SIZE / 2;
  player.y = SCREEN_H - TANK_SIZE - 10;
  player.dx = 0;
  player.dy = 0;
  player.facingDx = 0;
  player.facingDy = -1;
  player.active = true;
  player.lastMove = 0;
  player.lastShot = 0;
  player.health = 3;
  player.speed = PLAYER_SPEED;
  player.spawnTime = 0;

  // ==========================================
  // ENEMY TANKS
  // ==========================================
  Tank enemies[MAX_ENEMIES];
  for (int i = 0; i < MAX_ENEMIES; i++) {
    enemies[i].active = false;
    enemies[i].health = 2;
    enemies[i].speed = 1;
  }

  // ==========================================
  // BULLETS
  // ==========================================
  Bullet pBullets[MAX_PLAYER_BULLETS];
  for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
    pBullets[i].active = false;
  }
  
  Bullet eBullets[MAX_ENEMY_BULLETS];
  for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
    eBullets[i].active = false;
  }

  // ==========================================
  // POWER-UPS
  // ==========================================
  PowerUp powerups[3];
  for (int i = 0; i < 3; i++) {
    powerups[i].active = false;
  }

  // ==========================================
  // GAME VARIABLES
  // ==========================================
  uint16_t score = 0;
  uint32_t lastSpawn = 0;
  uint32_t lastFrame = millis();
  uint32_t lastFire = 0;
  uint32_t lastPowerupSpawn = 0;
  uint32_t gameTime = 0;
  int level = 1;
  int enemiesDestroyed = 0;
  bool gameRunning = true;
  
  // Player power-up states
  bool tripleShot = false;
  bool shieldActive = false;
  uint32_t powerupTimer = 0;
  uint32_t shieldTimer = 0;
  uint32_t tripleTimer = 0;

  // ==========================================
  // HELPER FUNCTIONS
  // ==========================================
  auto spawnEnemy = [&]() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
      if (!enemies[i].active) {
        // Spawn from top with random position
        enemies[i].x = random(10, SCREEN_W - TANK_SIZE - 10);
        enemies[i].y = 5;
        enemies[i].dx = 0;
        enemies[i].dy = 1;
        enemies[i].facingDx = 0;
        enemies[i].facingDy = 1;
        enemies[i].active = true;
        enemies[i].lastMove = millis();
        enemies[i].lastShot = millis();
        enemies[i].health = 1 + (level / 3);  // Health increases with level
        enemies[i].speed = 1 + (level / 4);
        enemies[i].spawnTime = millis();
        beep(600, 50, soundLevel);
        return;
      }
    }
  };

  auto spawnPowerup = [&]() {
    for (int i = 0; i < 3; i++) {
      if (!powerups[i].active) {
        powerups[i].x = random(20, SCREEN_W - 20);
        powerups[i].y = random(20, SCREEN_H - 20);
        powerups[i].type = random(0, 4);  // 0=Health, 1=Speed, 2=Triple, 3=Shield
        powerups[i].active = true;
        powerups[i].spawnTime = millis();
        return;
      }
    }
  };

  auto firePlayerBullet = [&](float x, float y, int dx, int dy) {
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
      if (!pBullets[i].active) {
        pBullets[i].x = x;
        pBullets[i].y = y;
        pBullets[i].dx = dx;
        pBullets[i].dy = dy;
        pBullets[i].active = true;
        pBullets[i].isPlayer = true;
        pBullets[i].damage = 1;
        return true;
      }
    }
    return false;
  };

  auto fireEnemyBullet = [&](float x, float y, int dx, int dy) {
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
      if (!eBullets[i].active) {
        eBullets[i].x = x;
        eBullets[i].y = y;
        eBullets[i].dx = dx;
        eBullets[i].dy = dy;
        eBullets[i].active = true;
        eBullets[i].isPlayer = false;
        eBullets[i].damage = 1;
        return true;
      }
    }
    return false;
  };

  // ==========================================
  // SHOW START SCREEN
  // ==========================================
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("TANK BATTLE", 20);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("D-pad = Move", 35);
  centreStr("ENTER = Fire", 45);
  centreStr("Destroy all enemies!", 55);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  // ==========================================
  // GAME LOOP
  // ==========================================
  while (true) {
    if (checkPause("TANK BATTLE")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    float dt = (now - lastFrame) / 16.0f;
    if (dt > 3.0f) dt = 3.0f;
    lastFrame = now;
    gameTime += dt;

    // ==========================================
    // SPAWN ENEMIES
    // ==========================================
    int spawnDelay = max(1000, SPAWN_DELAY - level * 100);
    if (now - lastSpawn > spawnDelay) {
      int activeEnemies = 0;
      for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) activeEnemies++;
      }
      if (activeEnemies < 3 + level / 2) {
        spawnEnemy();
        lastSpawn = now;
      }
    }

    // ==========================================
    // SPAWN POWER-UPS
    // ==========================================
    if (random(0, 100) < 2 && now - lastPowerupSpawn > 5000) {
      spawnPowerup();
      lastPowerupSpawn = now;
    }

    // ==========================================
    // PLAYER MOVEMENT
    // ==========================================
    int moveSpeed = player.speed;
    
    if (btnHeld(BTN_UP)) { 
      player.y -= moveSpeed * dt; 
      player.facingDx = 0; 
      player.facingDy = -1; 
    }
    else if (btnHeld(BTN_DOWN)) { 
      player.y += moveSpeed * dt; 
      player.facingDx = 0; 
      player.facingDy = 1; 
    }
    else if (btnHeld(BTN_LEFT)) { 
      player.x -= moveSpeed * dt; 
      player.facingDx = -1; 
      player.facingDy = 0; 
    }
    else if (btnHeld(BTN_RIGHT)) { 
      player.x += moveSpeed * dt; 
      player.facingDx = 1; 
      player.facingDy = 0; 
    }

    player.x = constrain(player.x, 0, SCREEN_W - TANK_SIZE);
    player.y = constrain(player.y, 0, SCREEN_H - TANK_SIZE);

    // ==========================================
    // PLAYER SHOOTING
    // ==========================================
    if (btnPressed(BTN_ENTER) && now - lastFire > 250) {
      float bulletX = player.x + TANK_SIZE/2 - 2;
      float bulletY = player.y + TANK_SIZE/2 - 2;
      
      if (tripleShot) {
        // Triple shot - 3 bullets in different directions
        firePlayerBullet(bulletX, bulletY, player.facingDx, player.facingDy);
        firePlayerBullet(bulletX, bulletY, player.facingDx + 1, player.facingDy);
        firePlayerBullet(bulletX, bulletY, player.facingDx - 1, player.facingDy);
        beep(1200, 20, soundLevel);
      } else {
        firePlayerBullet(bulletX, bulletY, player.facingDx, player.facingDy);
        beep(1000, 15, soundLevel);
      }
      lastFire = now;
    }

    // ==========================================
    // ENEMY AI
    // ==========================================
    for (int i = 0; i < MAX_ENEMIES; i++) {
      if (!enemies[i].active) continue;
      
      Tank &e = enemies[i];
      
      // Enemy movement
      if (now - e.lastMove > 1000) {
        // Random direction change
        int dir = random(0, 4);
        if (dir == 0) { e.dx = 0; e.dy = -1; }
        else if (dir == 1) { e.dx = 1; e.dy = 0; }
        else if (dir == 2) { e.dx = 0; e.dy = 1; }
        else { e.dx = -1; e.dy = 0; }
        e.facingDx = e.dx;
        e.facingDy = e.dy;
        e.lastMove = now;
      }
      
      e.x += e.dx * e.speed * dt;
      e.y += e.dy * e.speed * dt;
      e.x = constrain(e.x, 0, SCREEN_W - TANK_SIZE);
      e.y = constrain(e.y, 0, SCREEN_H - TANK_SIZE);
      
      // Enemy shooting
      if (now - e.lastShot > 1500 - level * 50) {
        float bulletX = e.x + TANK_SIZE/2 - 2;
        float bulletY = e.y + TANK_SIZE/2 - 2;
        fireEnemyBullet(bulletX, bulletY, e.facingDx, e.facingDy);
        e.lastShot = now;
        beep(400, 10, soundLevel);
      }
    }

    // ==========================================
    // PLAYER BULLETS
    // ==========================================
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
      if (!pBullets[i].active) continue;
      
      pBullets[i].x += pBullets[i].dx * BULLET_SPEED * dt;
      pBullets[i].y += pBullets[i].dy * BULLET_SPEED * dt;
      
      // Remove if off screen
      if (pBullets[i].x < 0 || pBullets[i].x > SCREEN_W || 
          pBullets[i].y < 0 || pBullets[i].y > SCREEN_H) {
        pBullets[i].active = false;
        continue;
      }
      
      // Check collision with enemies
      for (int j = 0; j < MAX_ENEMIES; j++) {
        if (!enemies[j].active) continue;
        
        if (pBullets[i].x > enemies[j].x && pBullets[i].x < enemies[j].x + TANK_SIZE &&
            pBullets[i].y > enemies[j].y && pBullets[i].y < enemies[j].y + TANK_SIZE) {
          
          enemies[j].health -= pBullets[i].damage;
          pBullets[i].active = false;
          
          if (enemies[j].health <= 0) {
            // Enemy destroyed!
            enemies[j].active = false;
            score += 10 + level * 5;
            enemiesDestroyed++;
            
            // Chance to drop power-up
            if (random(0, 100) < 20) {
              spawnPowerup();
            }
            
            beep(800, 30, soundLevel);
            delay(30);
            beep(1000, 30, soundLevel);
            
            // Level up
            if (enemiesDestroyed % 5 == 0) {
              level++;
              u8g2.clearBuffer();
              u8g2.setFont(u8g2_font_ncenB10_tr);
              char msg[30];
              snprintf(msg, sizeof(msg), "LEVEL %d!", level);
              centreStr(msg, 28);
              u8g2.setFont(u8g2_font_6x10_tr);
              centreStr("Enemies get tougher!", 44);
              u8g2.sendBuffer();
              delay(1000);
              beep(1760, 80, soundLevel);
              delay(100);
              beep(2093, 100, soundLevel);
            }
          } else {
            beep(600, 20, soundLevel);
          }
          break;
        }
      }
    }

    // ==========================================
    // ENEMY BULLETS
    // ==========================================
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
      if (!eBullets[i].active) continue;
      
      eBullets[i].x += eBullets[i].dx * BULLET_SPEED * 0.7f * dt;
      eBullets[i].y += eBullets[i].dy * BULLET_SPEED * 0.7f * dt;
      
      if (eBullets[i].x < 0 || eBullets[i].x > SCREEN_W || 
          eBullets[i].y < 0 || eBullets[i].y > SCREEN_H) {
        eBullets[i].active = false;
        continue;
      }
      
      // Check collision with player
      if (!shieldActive &&
          eBullets[i].x > player.x && eBullets[i].x < player.x + TANK_SIZE &&
          eBullets[i].y > player.y && eBullets[i].y < player.y + TANK_SIZE) {
        
        player.health--;
        eBullets[i].active = false;
        beep(200, 200, soundLevel);
        
        if (player.health <= 0) {
          gameOverScreen(score, 10, false);
          return;
        }
        
        // Brief invincibility
        shieldActive = true;
        shieldTimer = now + 2000;
      }
    }

    // ==========================================
    // POWER-UPS
    // ==========================================
    for (int i = 0; i < 3; i++) {
      if (!powerups[i].active) continue;
      
      // Power-up expires after 10 seconds
      if (now - powerups[i].spawnTime > 10000) {
        powerups[i].active = false;
        continue;
      }
      
      // Check if player collects power-up
      if (powerups[i].x > player.x && powerups[i].x < player.x + TANK_SIZE &&
          powerups[i].y > player.y && powerups[i].y < player.y + TANK_SIZE) {
        
        switch(powerups[i].type) {
          case 0: // Health
            player.health = min(5, player.health + 1);
            beep(1600, 50, soundLevel);
            break;
          case 1: // Speed
            player.speed = 4;
            powerupTimer = now + POWERUP_DURATION;
            beep(1400, 50, soundLevel);
            break;
          case 2: // Triple Shot
            tripleShot = true;
            tripleTimer = now + POWERUP_DURATION;
            beep(1800, 50, soundLevel);
            break;
          case 3: // Shield
            shieldActive = true;
            shieldTimer = now + POWERUP_DURATION;
            beep(2000, 50, soundLevel);
            break;
        }
        powerups[i].active = false;
      }
    }

    // ==========================================
    // POWER-UP TIMERS
    // ==========================================
    if (now > powerupTimer && player.speed > PLAYER_SPEED) {
      player.speed = PLAYER_SPEED;
    }
    if (now > tripleTimer && tripleShot) {
      tripleShot = false;
    }
    if (now > shieldTimer && shieldActive) {
      shieldActive = false;
    }

    // ==========================================
    // DRAW EVERYTHING
    // ==========================================
    u8g2.clearBuffer();
    
    // Draw grid background (optional)
    for (int x = 0; x < SCREEN_W; x += 20) {
      for (int y = 0; y < SCREEN_H; y += 20) {
        if ((x/20 + y/20) % 2 == 0) {
          u8g2.setDrawColor(0);
          u8g2.drawBox(x, y, 20, 20);
          u8g2.setDrawColor(1);
        }
      }
    }
    
    // Draw power-ups
    for (int i = 0; i < 3; i++) {
      if (!powerups[i].active) continue;
      
      // Animate power-ups
      if ((now / 200) % 2 == 0) {
        u8g2.drawCircle((int)powerups[i].x + 5, (int)powerups[i].y + 5, 6);
        u8g2.drawCircle((int)powerups[i].x + 5, (int)powerups[i].y + 5, 4);
      }
      
      // Draw power-up symbol
      u8g2.setFont(u8g2_font_5x7_tr);
      char symbol;
      switch(powerups[i].type) {
        case 0: symbol = 'H'; break;
        case 1: symbol = 'S'; break;
        case 2: symbol = 'T'; break;
        case 3: symbol = 'B'; break;
      }
      u8g2.drawStr((int)powerups[i].x + 3, (int)powerups[i].y + 8, &symbol);
    }
    
    // Draw enemy tanks
    for (int i = 0; i < MAX_ENEMIES; i++) {
      if (!enemies[i].active) continue;
      
      int ex = (int)enemies[i].x;
      int ey = (int)enemies[i].y;
      
      // Enemy tank body
      u8g2.drawBox(ex, ey, TANK_SIZE, TANK_SIZE);
      
      // Enemy tank turret
      u8g2.setDrawColor(0);
      u8g2.drawBox(ex + 3, ey + 3, 4, 4);
      u8g2.setDrawColor(1);
      
      // Health bar
      u8g2.drawFrame(ex, ey - 4, TANK_SIZE, 3);
      int healthWidth = (enemies[i].health * TANK_SIZE) / 3;
      u8g2.drawBox(ex, ey - 4, healthWidth, 3);
    }
    
    // Draw player tank
    int px = (int)player.x;
    int py = (int)player.y;
    
    // Shield effect
    if (shieldActive) {
      u8g2.drawCircle(px + TANK_SIZE/2, py + TANK_SIZE/2, TANK_SIZE + 4);
    }
    
    // Player tank body
    u8g2.drawRBox(px, py, TANK_SIZE, TANK_SIZE, 2);
    
    // Player tank turret
    u8g2.setDrawColor(0);
    u8g2.drawBox(px + 3, py + 3, 4, 4);
    u8g2.setDrawColor(1);
    
    // Player health
    for (int i = 0; i < player.health; i++) {
      u8g2.drawBox(5 + i * 8, SCREEN_H - 8, 6, 6);
    }
    
    // Draw bullets
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
      if (pBullets[i].active) {
        u8g2.drawDisc((int)pBullets[i].x, (int)pBullets[i].y, 2);
      }
    }
    
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
      if (eBullets[i].active) {
        u8g2.drawPixel((int)eBullets[i].x, (int)eBullets[i].y);
      }
    }
    
    // Draw UI
    u8g2.setFont(u8g2_font_5x7_tr);
    
    // Score
    char sc[10];
    itoa(score, sc, 10);
    u8g2.drawStr(SCREEN_W - 40, 8, "SC:");
    u8g2.drawStr(SCREEN_W - 20, 8, sc);
    
    // Level
    char lv[5];
    itoa(level, lv, 10);
    u8g2.drawStr(SCREEN_W - 40, 16, "LV:");
    u8g2.drawStr(SCREEN_W - 20, 16, lv);
    
    // Power-up status
    if (tripleShot) {
      u8g2.drawStr(5, 16, "3X");
    }
    if (player.speed > PLAYER_SPEED) {
      u8g2.drawStr(5, 24, "SPD");
    }
    
    u8g2.sendBuffer();
    delay(20);
  }
}

// ============================================================
// GAME: MAZE RUNNER (Fixed maze data reading)
// ============================================================

const uint8_t MAZE_L1[8][16] PROGMEM = {
  {11,9,3,13,5,3,9,1,3,9,3,13,5,1,3,11},
  {12,6,12,5,3,12,6,10,12,6,12,3,9,6,10,10},
  {9,5,3,11,12,5,5,6,9,5,3,12,6,11,10,10},
  {12,3,12,6,9,5,5,3,8,3,12,3,9,4,6,10},
  {9,2,9,3,10,9,5,2,10,12,3,14,12,3,9,2},
  {10,14,10,10,10,10,13,2,10,11,12,5,3,10,10,10},
  {10,9,6,12,6,12,3,14,10,8,3,9,6,12,6,10},
  {12,4,5,5,5,7,12,5,4,6,14,12,5,5,5,6}
};

void game_maze() {
  int px = 0, py = 0;
  int ex = 15, ey = 7;
  uint32_t startTime = millis(), lastMove = 0;
  const uint16_t moveDelay = 160;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("MAZE RUNNER", 22);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Reach the Exit!", 38);
  centreStr("Use buttons to move", 52);
  u8g2.sendBuffer();
  delay(1400);
  waitRelease();

  while (true) {
    if (checkPause("MAZE RUNNER")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    if (now - lastMove > moveDelay) {
      uint8_t walls = pgm_read_byte(&MAZE_L1[py][px]);
      bool moved = false;
      // FIXED: Correct wall bit checking
      if (btnHeld(BTN_UP) && !(walls & 1) && py > 0) { py--; moved = true; }
      if (btnHeld(BTN_RIGHT) && !(walls & 2) && px < 15) { px++; moved = true; }
      if (btnHeld(BTN_DOWN) && !(walls & 4) && py < 7) { py++; moved = true; }
      if (btnHeld(BTN_LEFT) && !(walls & 8) && px > 0) { px--; moved = true; }
      if (moved) { beep(900, 10, soundLevel); lastMove = now; }
    }

    if (px == ex && py == ey) {
      uint16_t secs = (millis() - startTime) / 1000;
      beep(1047, 60, soundLevel); delay(70);
      beep(1319, 60, soundLevel); delay(70);
      beep(1568, 160, soundLevel); delay(200);
      
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);
      centreStr("YOU WIN!", 28);
      u8g2.setFont(u8g2_font_6x10_tr);
      char timeBuf[24];
      snprintf(timeBuf, sizeof(timeBuf), "Time: %us", secs);
      centreStr(timeBuf, 44);
      centreStr("Press any key", 58);
      u8g2.sendBuffer();
      waitRelease();
      while (!btnHeld(BTN_UP) && !btnHeld(BTN_DOWN) &&
             !btnHeld(BTN_LEFT) && !btnHeld(BTN_RIGHT))
        delay(15);
      waitRelease();
      gameOverScreen(secs, 11, true);
      return;
    }

    const int OX = 8, OY = 4;
    u8g2.clearBuffer();
    for (int r = 0; r < 8; r++) {
      for (int c = 0; c < 16; c++) {
        uint8_t w = pgm_read_byte(&MAZE_L1[r][c]);
        int cx = OX + c * 7, cy = OY + r * 7;
        if (w & 1) u8g2.drawHLine(cx, cy, 8);
        if (w & 2) u8g2.drawVLine(cx + 7, cy, 8);
        if (w & 4) u8g2.drawHLine(cx, cy + 7, 8);
        if (w & 8) u8g2.drawVLine(cx, cy, 8);
      }
    }
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(OX + ex * 7 + 1, OY + ey * 7 + 6, "E");
    u8g2.drawDisc(OX + px * 7 + 3, OY + py * 7 + 3, 2);
    
    char tbuf[8];
    itoa((millis() - startTime) / 1000, tbuf, 10);
    u8g2.drawStr(SCREEN_W - u8g2.getStrWidth(tbuf) - 1, 8, tbuf);
    u8g2.sendBuffer();
    delay(20);
  }
}

// ============================================================
// GAME: ROCK PAPER SCISSORS
// ============================================================

void game_rps() {
  const char* moves[] = {"ROCK", "PAPER", "SCISSORS"};
  int playerScore = 0, cpuScore = 0;
  int playerChoice = 0, cpuChoice = 0;
  int state = 0;
  uint32_t resultTime = 0;
  int totalRounds = 0;
  int playerWins = 0;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("RPS GAME", 20);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("UP/DOWN to choose", 40);
  centreStr("ENTER to play!", 54);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    if (checkPause("RPS")) return;
    if (checkMenuAndReturn()) return;
    
    if (state == 0) {
      if (btnPressed(BTN_UP)) { playerChoice = (playerChoice + 2) % 3; beep(600, 10, soundLevel); }
      if (btnPressed(BTN_DOWN)) { playerChoice = (playerChoice + 1) % 3; beep(600, 10, soundLevel); }
      
      if (btnPressed(BTN_ENTER)) {
        cpuChoice = random(0, 3);
        state = 1;
        resultTime = millis();
        beep(900, 20, soundLevel);
        totalRounds++;
        saveTotalGames(12);
      }
      
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      centreStr("CHOOSE YOUR MOVE", 10);
      
      u8g2.setFont(u8g2_font_6x10_tr);
      for (int i = 0; i < 3; i++) {
        int y = 25 + i * 12;
        if (i == playerChoice) {
          u8g2.drawRBox(20, y-8, SCREEN_W-40, 12, 2);
          u8g2.setDrawColor(0);
          centreStr(moves[i], y+3);
          u8g2.setDrawColor(1);
        } else {
          centreStr(moves[i], y+3);
        }
      }
      
      char scoreStr[20];
      snprintf(scoreStr, sizeof(scoreStr), "You:%d CPU:%d", playerScore, cpuScore);
      u8g2.setFont(u8g2_font_5x7_tr);
      centreStr(scoreStr, 60);
      u8g2.sendBuffer();
    }
    
    else if (state == 1) {
      if (millis() - resultTime > 2000) {
        state = 0;
        continue;
      }
      
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      centreStr("RESULT!", 10);
      
      u8g2.setFont(u8g2_font_6x10_tr);
      char youStr[20];
      snprintf(youStr, sizeof(youStr), "You: %s", moves[playerChoice]);
      centreStr(youStr, 28);
      
      char cpuStr[20];
      snprintf(cpuStr, sizeof(cpuStr), "CPU: %s", moves[cpuChoice]);
      centreStr(cpuStr, 40);
      
      int result = (playerChoice - cpuChoice + 3) % 3;
      const char* resultText;
      if (result == 0) { 
        resultText = "DRAW!"; 
        beep(500, 40, soundLevel);
      }
      else if (result == 1) { 
        resultText = "YOU WIN!"; 
        playerScore++;
        playerWins++;
        saveRPSWin(12);
        uint16_t winRate = 0;
        if (totalGamesPlayed[12] > 0) {
          winRate = (rpsWins[12] * 100) / totalGamesPlayed[12];
        }
        if (winRate > highScores[12]) {
          highScores[12] = winRate;
          EEPROM.begin(EEPROM_SIZE);
          EEPROM.put(12 * sizeof(uint16_t), highScores[12]);
          EEPROM.commit();
          EEPROM.end();
        }
        beep(1200, 30, soundLevel); 
        delay(80); 
        beep(1500, 30, soundLevel); 
      }
      else { 
        resultText = "CPU WINS!"; 
        cpuScore++; 
        beep(300, 80, soundLevel); 
      }
      
      u8g2.setFont(u8g2_font_ncenB08_tr);
      centreStr(resultText, 54);
      
      char scoreStr[30];
      snprintf(scoreStr, sizeof(scoreStr), "You:%d CPU:%d", playerScore, cpuScore);
      u8g2.setFont(u8g2_font_5x7_tr);
      centreStr(scoreStr, 62);
      
      u8g2.sendBuffer();
    }
    delay(50);
  }
}

// ============================================================
// GAME: CAR RACER (Fixed movement)
// ============================================================

void game_car() {
  const int CAR_W = 10, CAR_H = 6;
  int playerX = 64 - CAR_W/2;
  int targetLane = 1;
  const int LANE_WIDTH = 42;
  const int LANE_START = 1;
  
  struct Obstacle { float x, y; int lane; bool active; float speed; };
  Obstacle obs[8];
  for (int i = 0; i < 8; i++) obs[i].active = false;
  
  uint32_t lastSpawn = 0;
  uint32_t lastFrame = millis();
  uint16_t score = 0;
  float baseSpeed = 1.5f;
  int lives = 3;
  
  const float SPEED_TYPES[4] = {0.6f, 0.9f, 1.35f, 1.7f};
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("CAR RACER", 20);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Avoid obstacles!", 38);
  centreStr("Use LEFT/RIGHT to move", 52);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    if (checkPause("CAR RACER")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    float dt = (now - lastFrame) / 20.0f;
    if (dt > 3.0f) dt = 3.0f; // Limit dt to prevent overshoot
    lastFrame = now;
    
    if (btnPressed(BTN_LEFT)) targetLane = max(0, targetLane - 1);
    if (btnPressed(BTN_RIGHT)) targetLane = min(2, targetLane + 1);
    
    int targetX = LANE_START + targetLane * LANE_WIDTH + (LANE_WIDTH - CAR_W) / 2;
    // FIXED: Smooth movement with clamp
    playerX += (targetX - playerX) * 0.12f * dt;
    playerX = constrain(playerX, LANE_START, LANE_START + 2 * LANE_WIDTH + (LANE_WIDTH - CAR_W));
    
    if (now - lastSpawn > (uint32_t)max(500, 1500 - score * 2)) {
      lastSpawn = now;
      for (int i = 0; i < 8; i++) {
        if (!obs[i].active) {
          obs[i].lane = random(0, 3);
          obs[i].x = LANE_START + obs[i].lane * LANE_WIDTH + (LANE_WIDTH - 12) / 2;
          obs[i].y = -10;
          obs[i].active = true;
          obs[i].speed = SPEED_TYPES[random(0, 4)] * (1.0f + score / 250.0f);
          break;
        }
      }
      baseSpeed = 1.5f + score / 200.0f;
    }
    
    for (int i = 0; i < 8; i++) {
      if (!obs[i].active) continue;
      obs[i].y += obs[i].speed * dt;
      if (obs[i].y > SCREEN_H) {
        obs[i].active = false;
        score++;
        if (score % 5 == 0) beep(1500, 15, soundLevel);
        continue;
      }
      
      if (obs[i].y + 10 > SCREEN_H - 12 && obs[i].y < SCREEN_H - 6 + 6 &&
          obs[i].x < playerX + CAR_W && obs[i].x + 12 > playerX) {
        lives--;
        beep(200, 160, soundLevel);
        if (lives <= 0) {
          gameOverScreen(score, 13, false);
          return;
        }
        obs[i].active = false;
      }
    }
    
    u8g2.clearBuffer();
    
    for (int i = 0; i < 3; i++) {
      int x = LANE_START + i * LANE_WIDTH;
      u8g2.drawFrame(x, 0, LANE_WIDTH, SCREEN_H);
      for (int y = 8; y < SCREEN_H; y += 16) {
        u8g2.drawHLine(x + LANE_WIDTH/2 - 1, y, 2);
      }
    }
    
    u8g2.drawBox((int)playerX, SCREEN_H - 12, CAR_W, CAR_H);
    u8g2.drawBox((int)playerX + 2, SCREEN_H - 14, 6, 2);
    u8g2.setDrawColor(0);
    u8g2.drawBox((int)playerX + 3, SCREEN_H - 11, 4, 2);
    u8g2.setDrawColor(1);
    
    for (int i = 0; i < 8; i++) {
      if (!obs[i].active) continue;
      u8g2.drawBox((int)obs[i].x, (int)obs[i].y, 12, 10);
      u8g2.setDrawColor(0);
      u8g2.drawBox((int)obs[i].x + 2, (int)obs[i].y + 2, 8, 6);
      u8g2.setDrawColor(1);
    }
    
    u8g2.setFont(u8g2_font_5x7_tr);
    char sc[8];
    itoa(score, sc, 10);
    u8g2.drawStr(1, 8, sc);
    
    for (int i = 0; i < lives; i++) {
      u8g2.drawBox(SCREEN_W - 6 - i * 7, 4, 5, 4);
    }
    
    u8g2.sendBuffer();
    delay(20);
  }
}

// ============================================================
// GAME: CAR RACER 2 LANE (FIXED - সঠিকভাবে লেন পরিবর্তন হয়)
// ============================================================

void game_car_2lane() {
  const int CAR_W = 10, CAR_H = 6;
  int playerX = 64 - CAR_W/2;
  int targetLane = 0;  // 0 = Left lane, 1 = Right lane
  const int LANE_WIDTH = 63;  // SCREEN_W / 2 - 1
  const int LANE_START = 1;
  
  struct Obstacle { 
    float x, y; 
    int lane; 
    bool active; 
    float speed; 
    int type;  // 0 = car, 1 = truck, 2 = bike
  };
  
  Obstacle obs[10];  // বেশি obstacle রাখলাম
  for (int i = 0; i < 10; i++) obs[i].active = false;
  
  uint32_t lastSpawn = 0;
  uint32_t lastFrame = millis();
  uint16_t score = 0;
  float baseSpeed = 1.5f;
  int lives = 3;
  int laneHistory[2] = {0, 0};  // Track which lanes have obstacles
  
  const float SPEED_TYPES[4] = {0.6f, 0.9f, 1.35f, 1.7f};
  
  // Show start screen
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("2-LANE CAR RACER", 18);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Avoid obstacles!", 36);
  centreStr("LEFT/RIGHT = switch lanes", 50);
  u8g2.setFont(u8g2_font_5x7_tr);
  centreStr("2 lanes - double the challenge!", 62);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    if (checkPause("2-LANE RACER")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    float dt = (now - lastFrame) / 20.0f;
    if (dt > 3.0f) dt = 3.0f;
    lastFrame = now;
    
    // Lane switching with button presses
    if (btnPressed(BTN_LEFT)) {
      targetLane = max(0, targetLane - 1);
      beep(800, 15, soundLevel);
    }
    if (btnPressed(BTN_RIGHT)) {
      targetLane = min(1, targetLane + 1);
      beep(800, 15, soundLevel);
    }
    
    // Calculate target X position for each lane
    int targetX = LANE_START + targetLane * LANE_WIDTH + (LANE_WIDTH - CAR_W) / 2  ;
    // Smooth movement
    playerX += (targetX - playerX) * 0.15f * dt;
    playerX = constrain(playerX, LANE_START, LANE_START + LANE_WIDTH - CAR_W);
    
    // 🔥 FIXED: Spawn obstacles with random lanes
    int spawnInterval = max(300, 1200 - score * 2);
    if (now - lastSpawn > (uint32_t)spawnInterval) {
      lastSpawn = now;
      
      // Check if we need to spawn
      int activeCount = 0;
      for (int i = 0; i < 10; i++) {
        if (obs[i].active) activeCount++;
      }
      
      // Max 4 obstacles at a time
      if (activeCount < 4) {
        for (int i = 0; i < 10; i++) {
          if (!obs[i].active) {
            // 🔥 RANDOM LANE - 0 or 1
            obs[i].lane = random(0, 2);
            
            // Different obstacle types
            obs[i].type = random(0, 3);  // 0=car, 1=truck, 2=bike
            
            // Calculate X position based on lane
            if (obs[i].type == 0) {  // Car
              obs[i].x = LANE_START + obs[i].lane * LANE_WIDTH + (LANE_WIDTH - 12) / 2;
            } else if (obs[i].type == 1) {  // Truck (wider)
              obs[i].x = LANE_START + obs[i].lane * LANE_WIDTH + (LANE_WIDTH - 16) / 2;
            } else {  // Bike (narrower)
              obs[i].x = LANE_START + obs[i].lane * LANE_WIDTH + (LANE_WIDTH - 8) / 2;
            }
            
            obs[i].y = -20 - random(0, 30);  // Random start position
            obs[i].active = true;
            obs[i].speed = SPEED_TYPES[random(0, 4)] * (1.0f + score / 200.0f);
            
            // Debug: beep to confirm spawn
            // beep(600, 5, soundLevel);
            break;
          }
        }
      }
      baseSpeed = 1.5f + score / 200.0f;
    }
    
    // Update obstacles and check collisions
    for (int i = 0; i < 10; i++) {
      if (!obs[i].active) continue;
      
      // Move obstacle down
      obs[i].y += obs[i].speed * dt;
      
      // Remove if off screen
      if (obs[i].y > SCREEN_H + 20) {
        obs[i].active = false;
        score++;
        if (score % 5 == 0) {
          beep(1500, 15, soundLevel);
        }
        continue;
      }
      
      // Get obstacle dimensions based on type
      int obsW, obsH;
      if (obs[i].type == 0) { obsW = 12; obsH = 10; }
      else if (obs[i].type == 1) { obsW = 16; obsH = 12; }
      else { obsW = 8; obsH = 8; }
      
      // Collision detection with player
      if (obs[i].y + obsH > SCREEN_H - 12 && obs[i].y < SCREEN_H - 6 + 6 &&
          obs[i].x < playerX + CAR_W && obs[i].x + obsW > playerX) {
        lives--;
        beep(200, 160, soundLevel);
        if (lives <= 0) {
          gameOverScreen(score, 13, false);
          return;
        }
        obs[i].active = false;
        
        // Flash effect for invincibility
        for (int f = 0; f < 4; f++) {
          u8g2.clearBuffer();
          if (f % 2 == 0) {
            u8g2.drawRBox((int)playerX, SCREEN_H - 12, CAR_W, CAR_H, 2);
          }
          u8g2.sendBuffer();
          delay(80);
        }
      }
    }
    
    // ============================================
    // DRAW EVERYTHING
    // ============================================
    u8g2.clearBuffer();
    
    // 🔥 Draw 2 lanes with proper markings
    for (int i = 0; i < 2; i++) {
      int x = LANE_START + i * LANE_WIDTH;
      
      // Lane border
      u8g2.drawFrame(x, 0, LANE_WIDTH, SCREEN_H);
      
      // Road edge markings (left and right edges)
      if (i == 0) {
        for (int y = 4; y < SCREEN_H; y += 12) {
          u8g2.drawVLine(x, y, 4);
        }
      }
      if (i == 1) {
        for (int y = 4; y < SCREEN_H; y += 12) {
          u8g2.drawVLine(x + LANE_WIDTH - 1, y, 4);
        }
      }
      
      // Lane markings (dashed lines inside lane)
      if (i == 0) {
        for (int y = 8; y < SCREEN_H; y += 20) {
          u8g2.drawHLine(x + 15, y, 8);
          u8g2.drawHLine(x + 35, y, 8);
        }
      }
      if (i == 1) {
        for (int y = 8; y < SCREEN_H; y += 20) {
          u8g2.drawHLine(x + 15, y, 8);
          u8g2.drawHLine(x + 35, y, 8);
        }
      }
    }
    
    // Draw center divider with animation
    int dividerOffset = (int)(now / 100) % 20;
    for (int y = dividerOffset; y < SCREEN_H; y += 40) {
      u8g2.drawHLine(SCREEN_W / 2 - 2, y, 4);
      u8g2.drawHLine(SCREEN_W / 2 - 2, y + 10, 4);
      u8g2.drawHLine(SCREEN_W / 2 - 2, y + 20, 4);
    }
    
    // 🔥 Draw obstacles with different types
    for (int i = 0; i < 10; i++) {
      if (!obs[i].active) continue;
      
      int ox = (int)obs[i].x;
      int oy = (int)obs[i].y;
      
      if (obs[i].type == 0) {
        // Car - Red
        u8g2.drawBox(ox, oy, 12, 10);
        u8g2.setDrawColor(0);
        u8g2.drawBox(ox + 2, oy + 2, 8, 3);
        u8g2.drawBox(ox + 2, oy + 6, 8, 2);
        u8g2.setDrawColor(1);
        // Headlights
        u8g2.drawBox(ox + 1, oy + 1, 3, 1);
        u8g2.drawBox(ox + 8, oy + 1, 3, 1);
      } 
      else if (obs[i].type == 1) {
        // Truck - Blue (wider)
        u8g2.drawBox(ox, oy, 16, 12);
        u8g2.setDrawColor(0);
        u8g2.drawBox(ox + 3, oy + 2, 10, 4);
        u8g2.drawBox(ox + 2, oy + 7, 12, 3);
        u8g2.setDrawColor(1);
      } 
      else {
        // Bike - Green (narrower)
        u8g2.drawBox(ox + 2, oy, 4, 8);
        u8g2.drawBox(ox, oy + 2, 8, 4);
        u8g2.drawBox(ox + 2, oy + 8, 4, 2);
        // Wheels
        u8g2.drawCircle(ox + 1, oy + 8, 2);
        u8g2.drawCircle(ox + 7, oy + 8, 2);
      }
    }
    
    // 🔥 Draw player car with glow effect
    // Glow
    u8g2.setDrawColor(0);
    u8g2.drawRBox((int)playerX - 2, SCREEN_H - 14, CAR_W + 4, CAR_H + 4, 2);
    u8g2.setDrawColor(1);
    
    // Car body
    u8g2.drawRBox((int)playerX, SCREEN_H - 12, CAR_W, CAR_H, 2);
    u8g2.setDrawColor(0);
    // Headlights
    u8g2.drawBox((int)playerX + 1, SCREEN_H - 10, 3, 2);
    u8g2.drawBox((int)playerX + CAR_W - 4, SCREEN_H - 10, 3, 2);
    u8g2.setDrawColor(1);
    // Windshield
    u8g2.drawBox((int)playerX + 2, SCREEN_H - 14, 6, 2);
    
    // 🔥 Draw UI
    u8g2.setFont(u8g2_font_5x7_tr);
    char sc[8];
    itoa(score, sc, 10);
    u8g2.drawStr(1, 8, "SC:");
    u8g2.drawStr(16, 8, sc);
    
    // Draw lives as hearts
    for (int i = 0; i < lives; i++) {
      drawHeart(SCREEN_W - 10 - i * 10, 1);
    }
    
    // Lane indicator with arrows
    u8g2.setFont(u8g2_font_6x10_tr);
    if (targetLane == 0) {
      u8g2.drawStr(1, 20, "<-");
    } else {
      u8g2.drawStr(1, 20, "->");
    }
    
    // Show current speed
    u8g2.setFont(u8g2_font_5x7_tr);
    char speedStr[10];
    snprintf(speedStr, sizeof(speedStr), "SPD:%d", (int)(baseSpeed * 10));
    u8g2.drawStr(SCREEN_W - 30, 20, speedStr);
    
    u8g2.sendBuffer();
    delay(20);
  }
}


void game_trex() {
  // ---------------- Sprites ----------------
  static const uint8_t SPRITE_TREX[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x3f, 0x00, 0x00, 0xf0, 0x7f, 0x00, 0x00, 0x30, 0x7f, 0x00,
    0x00, 0xf0, 0x7f, 0x00, 0x00, 0xf0, 0x7f, 0x00, 0x00, 0xf0, 0x7f, 0x00, 0x00, 0xf0, 0x03, 0x00,
    0x00, 0xf0, 0x1f, 0x00, 0x00, 0xf0, 0x01, 0x00, 0x02, 0xf8, 0x00, 0x00, 0x02, 0xfe, 0x01, 0x00,
    0x06, 0xff, 0x07, 0x00, 0x8e, 0xff, 0x04, 0x00, 0xfe, 0xff, 0x00, 0x00, 0xfe, 0xff, 0x00, 0x00,
    0xfc, 0xff, 0x00, 0x00, 0xf8, 0x7f, 0x00, 0x00, 0xe0, 0x3f, 0x00, 0x00, 0xc0, 0x1f, 0x00, 0x00,
    0x80, 0x1b, 0x00, 0x00, 0x80, 0x11, 0x00, 0x00, 0x80, 0x11, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00,
    0x80, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

  static const uint8_t SPRITE_CACTUS1[] PROGMEM = {
    0x00, 0x00, 0x70, 0x00, 0x70, 0x00, 0x70, 0x00, 0x70, 0x00, 0x70, 0x00, 0x72, 0x02, 0x76, 0x02,
    0x76, 0x02, 0x76, 0x02, 0x76, 0x02, 0x76, 0x02, 0x76, 0x02, 0xfe, 0x03, 0x7c, 0x00, 0x70, 0x00,
    0x70, 0x00, 0x70, 0x00, 0x70, 0x00, 0x70, 0x00, 0x70, 0x00, 0x70, 0x00, 0x00, 0x00,
  };

  static const uint8_t SPRITE_CACTUS2[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x18, 0xc0, 0x01, 0x18, 0xc0, 0x01, 0x18, 0xd8, 0x01, 0x58,
    0xd8, 0x01, 0x58, 0xd8, 0x09, 0x58, 0xd8, 0x09, 0x7a, 0xd8, 0x09, 0x1a, 0xd8, 0x09, 0x1a, 0xf1,
    0x09, 0x1a, 0xc1, 0x09, 0x5a, 0xc5, 0x0f, 0x5e, 0xc5, 0x01, 0x5c, 0xc5, 0x01, 0x98, 0xc7, 0x01,
    0x18, 0xc1, 0x01, 0x18, 0xc1, 0x01, 0x18, 0xc1, 0x01, 0x18, 0xc1, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
  };

  static const uint8_t SPRITE_CACTUS3[] PROGMEM = {
    0x00, 0x00, 0x00, 0x70, 0xc0, 0x01, 0x70, 0xc0, 0x01, 0x70, 0xd0, 0x09, 0x70, 0xdb, 0x09, 0x70,
    0xdb, 0x09, 0x72, 0xdb, 0x09, 0x72, 0xdb, 0x09, 0x72, 0xfb, 0x09, 0x72, 0xf3, 0x09, 0xf2, 0xc1,
    0x09, 0xf2, 0xc0, 0x0f, 0x7e, 0xc0, 0x07, 0x7c, 0xc0, 0x01, 0x70, 0xc0, 0x01, 0x70, 0xc0, 0x01,
    0x70, 0xc0, 0x01, 0x70, 0xc0, 0x01, 0x70, 0xc0, 0x01, 0x70, 0xc0, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
  };

  const int TREX_WIDTH = 25, TREX_HEIGHT = 26;
  const int CACTUS1_WIDTH = 11, CACTUS1_HEIGHT = 23;
  const int CACTUS2_WIDTH = 21, CACTUS2_HEIGHT = 23;
  const int CACTUS3_WIDTH = 21, CACTUS3_HEIGHT = 23;

  // ---------------- Game constants ----------------
  const float OBSTACLE_VELOCITY = -2.5f;

  // ---------------- Local types ----------------
  enum TrexState { RUNNING, JUMPING };

  // 🔥 FIXED: সব কনস্ট্যান্ট স্ট্রাকটের ভিতরে ডিফাইন করা হয়েছে
  struct Trex {
    float x, y;
    float velocityY;
    TrexState state;
    
    // সব কনস্ট্যান্ট এখানে ডিফাইন করুন
    float trexX;
    float trexGroundY;
    float trexVelocity;
    float gravity;
    int trexWidth;
    int trexHeight;

    Trex() { 
      // কনস্ট্যান্ট ভ্যালু সেট করুন
      trexX = 10.0f;
      trexGroundY = (float)(SCREEN_H - 10) - 26.0f + 2.0f;  // SCREEN_H - 10 - 26 + 2
      trexVelocity = -7.0f;
      gravity = 5.0f;
      trexWidth = 25;
      trexHeight = 26;
      
      x = trexX; 
      y = trexGroundY; 
      velocityY = 0.0f; 
      state = RUNNING; 
    }

    void jump() {
      if (state == RUNNING) {
        velocityY = trexVelocity;
        state = JUMPING;
        beep(700, 15, soundLevel);
      }
    }

    void updateState(float dt) {
      if (state == JUMPING) {
        y += velocityY * dt;
        velocityY += (gravity / 10.0f) * dt;
        if (y >= trexGroundY) {
          y = trexGroundY;
          velocityY = 0.0f;
          state = RUNNING;
        }
      }
    }

    void draw() { 
      u8g2.drawXBMP((int)x, (int)y, trexWidth, trexHeight, SPRITE_TREX); 
    }

    float getX() { return x; }
    float getY() { return y; }
    float getWidth() { return (float)(trexWidth - 8); }
    float getHeight() { return (float)(trexHeight - 4); }
  };

  // 🔥 FIXED: Obstacle struct - সব কনস্ট্যান্ট ভিতরে
  struct Obstacle {
    float x;
    const uint8_t* sprite;
    float w, h;
    float obstacleGap;
    float cactusY;
    int cactusWidth;
    int cactusHeight;

    Obstacle() { 
      obstacleGap = 120.0f;
      cactusY = (float)(SCREEN_H - 10) - 23.0f + 2.0f;  // SCREEN_H - 10 - 23 + 2
      cactusWidth = 11;
      cactusHeight = 23;
      x = (float)SCREEN_W + obstacleGap; 
      randomize(); 
    }

    void randomize() {
      x = (float)SCREEN_W + random((int)(obstacleGap + 50.0f), (int)(obstacleGap + 150.0f));
      int type = random(0, 3);
      switch (type) {
        case 0: 
          sprite = SPRITE_CACTUS1; 
          w = 11.0f; 
          h = 23.0f; 
          break;
        case 1: 
          sprite = SPRITE_CACTUS2; 
          w = 21.0f; 
          h = 23.0f; 
          break;
        default: 
          sprite = SPRITE_CACTUS3; 
          w = 21.0f; 
          h = 23.0f; 
          break;
      }
    }

    void draw() { 
      u8g2.drawXBMP((int)x, (int)cactusY, (int)w, (int)h, sprite); 
    }

    float getX() { return x; }
    float getY() { return cactusY; }
    float getWidth() { return w; }
    float getHeight() { return h; }
  };

  // ---------------- Game state ----------------
  Trex trex;
  Obstacle obs[2];
  unsigned long score = 0;
  uint32_t lastFrame = millis();

  obs[0] = Obstacle();
  obs[1] = Obstacle();
  obs[1].x = obs[0].x + (float)SCREEN_W / 2.0f + 120.0f;  // OBSTACLE_GAP = 120

  // Show start screen
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("T-REX RUNNER", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Any direction to jump!", 40);
  u8g2.sendBuffer();
  delay(1200);
  waitRelease();

  while (true) {
    if (checkPause("T-REX RUNNER")) return;
    if (checkMenuAndReturn()) return;

    uint32_t now = millis();
    float dt = (now - lastFrame) / 20.0f;
    if (dt > 3.0f) dt = 3.0f;
    if (dt < 0.5f) dt = 0.5f;
    lastFrame = now;

    // Jump input - all 4 direction buttons
    if (btnPressed(BTN_UP) || btnPressed(BTN_DOWN) || 
        btnPressed(BTN_LEFT) || btnPressed(BTN_RIGHT)) {
      trex.jump();
    }

    score++;

    // Speed calculation
    float levelFloat = (float)score / 200.0f;
    float dynamicVelocity = OBSTACLE_VELOCITY - floorf(levelFloat) * 0.1f;
    if (dynamicVelocity < -6.5f) dynamicVelocity = -6.5f;

    // Update obstacles
    for (int i = 0; i < 2; i++) {
      obs[i].x += dynamicVelocity * dt;
      if (obs[i].x < -obs[i].w) {
        obs[i].randomize();
      }
    }

    trex.updateState(dt);

    // Collision check
    bool hit = false;
    for (int i = 0; i < 2; i++) {
      if (trex.getX() < obs[i].getX() + obs[i].getWidth() &&
          trex.getX() + trex.getWidth() > obs[i].getX() &&
          trex.getY() < obs[i].getY() + obs[i].getHeight() &&
          trex.getY() + trex.getHeight() > obs[i].getY()) {
        hit = true;
        break;
      }
    }

    if (hit) {
      beep(200, 160, soundLevel);
      gameOverScreen(score, 14, false);
      return;
    }

    // ---------------- Draw ----------------
    u8g2.clearBuffer();

    u8g2.drawHLine(0, (int)(SCREEN_H - 10) + 1, SCREEN_W);
    trex.draw();
    for (int i = 0; i < 2; i++) obs[i].draw();

    // Draw score and speed
    u8g2.setFont(u8g2_font_5x7_tr);
    char sc[16];
    snprintf(sc, sizeof(sc), "Score:%lu", score);
    u8g2.drawStr(SCREEN_W - 60, 8, sc);
    
    char spd[16];
    snprintf(spd, sizeof(spd), "SPD:%.1f", abs(dynamicVelocity));
    u8g2.drawStr(2, 8, spd);

    u8g2.sendBuffer();
    delay(20);
  }
}

// ============================================================
// T-REX RUNNER - Fixed Memory Leak Version
// ============================================================

// ============================================================
// T-REX RUNNER 2 (trex2) - Complete Working Function
// ============================================================

void game_trex2() {
  const int GAME_INDEX = 16;

  // ── SPRITE DATA ──
  static const uint8_t trex_up_1s_bitmap[] PROGMEM = {
    0x16, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xfe, 0xfa,
    0xfe, 0xfe, 0xbe, 0xbe, 0xbe, 0x3e, 0x3c, 0x00, 0x00, 0x3f, 0x7c, 0xf8, 0xf0, 0xf0, 0xf8, 0xfc,
    0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x04, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x03, 0x3f, 0x2f, 0x07, 0x03, 0x07, 0x3f, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };

  static const uint8_t trex_up_2s_bitmap[] PROGMEM = {
    0x16, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xfe, 0xfa,
    0xfe, 0xfe, 0xbe, 0xbe, 0xbe, 0x3e, 0x3c, 0x00, 0x00, 0x3f, 0x7c, 0xf8, 0xf0, 0xf0, 0xf8, 0xfc,
    0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x04, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x03, 0x3f, 0x2f, 0x07, 0x03, 0x03, 0x07, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };

  static const uint8_t trex_up_3s_bitmap[] PROGMEM = {
    0x16, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xfe, 0xfa,
    0xfe, 0xfe, 0xbe, 0xbe, 0xbe, 0x3e, 0x3c, 0x00, 0x00, 0x3f, 0x7c, 0xf8, 0xf0, 0xf0, 0xf8, 0xfc,
    0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x04, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x03, 0x07, 0x0f, 0x0b, 0x03, 0x07, 0x3f, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };

  static const uint8_t trex_duck_1s_bitmap[] PROGMEM = {
    0x1d, 0x0f, 0x00, 0x0e, 0x1c, 0x3c, 0x78, 0xf8, 0xf8, 0xf8, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xf8, 0xf8, 0xfc, 0xfe, 0xf6, 0xfe, 0xfe, 0x7e, 0x7e, 0x7e, 0x7e, 0x7c, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x0b, 0x03, 0x3f, 0x2f, 0x07, 0x03, 0x01, 0x01, 0x07, 0x05,
    0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00
  };

  static const uint8_t trex_duck_2s_bitmap[] PROGMEM = {
    0x1d, 0x0f, 0x00, 0x0e, 0x1c, 0x3c, 0x78, 0xf8, 0xf8, 0xf8, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xf8, 0xf8, 0xfc, 0xfe, 0xf6, 0xfe, 0xfe, 0x7e, 0x7e, 0x7e, 0x7e, 0x7c, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3d, 0x2f, 0x07, 0x03, 0x03, 0x07, 0x07, 0x05, 0x01, 0x07, 0x05,
    0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00
  };

  static const uint8_t trex_dead_1_bitmap[] PROGMEM = {
    0x16, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xfe, 0xe2,
    0xea, 0xe2, 0xfe, 0xfe, 0xfe, 0x7e, 0x7c, 0x00, 0x00, 0x3f, 0x7c, 0xf8, 0xf0, 0xf0, 0xf8, 0xfc,
    0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x04, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x03, 0x3f, 0x2f, 0x07, 0x03, 0x07, 0x3f, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };

  static const uint8_t cacti_big_big_bitmap[] PROGMEM = {
    0x1b, 0x1a, 0x00, 0x00, 0x80, 0x00, 0x00, 0xfc, 0xfe, 0xfe, 0xfc, 0x00, 0x80, 0xc0, 0x80, 0x00,
    0xe0, 0xf0, 0xe0, 0x00, 0xfc, 0xfe, 0xfe, 0xfc, 0x00, 0x80, 0xc0, 0x80, 0x00, 0x00, 0x7f, 0xff,
    0xff, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xc0, 0xff, 0x7f, 0x3f, 0x00, 0x0f, 0x1f, 0x3f, 0x38, 0xff,
    0xff, 0xff, 0xff, 0xe0, 0xff, 0x7f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x81, 0x81, 0xff, 0xff, 0xff,
    0xff, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
  };

  static const uint8_t cacti_big_small_bitmap[] PROGMEM = {
    0x17, 0x1a, 0x00, 0x00, 0x80, 0x00, 0x00, 0xfc, 0xfe, 0xfe, 0xfc, 0x00, 0x80, 0xc0, 0x80, 0x00,
    0x00, 0x00, 0x00, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xc0, 0xff, 0xff,
    0xff, 0xff, 0xc0, 0xff, 0x7f, 0x3f, 0x00, 0x7e, 0xff, 0x80, 0xff, 0xff, 0x00, 0xfc, 0xfc, 0x00,
    0x00, 0x00, 0x00, 0x81, 0x81, 0xff, 0xff, 0xff, 0xff, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00,
    0x00, 0xff, 0xff, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00
  };

  static const uint8_t cacti_small_big_bitmap[] PROGMEM = {
    0x17, 0x1a, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
    0xfc, 0xfe, 0xfe, 0xfc, 0x00, 0x80, 0xc0, 0x80, 0x00, 0x00, 0x7e, 0xff, 0x80, 0xff, 0xff, 0x00,
    0xfc, 0xfc, 0x00, 0x7f, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xc0, 0xff, 0x7f, 0x3f, 0x00,
    0x00, 0x80, 0x00, 0x00, 0xff, 0xff, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x81, 0x81, 0xff, 0xff,
    0xff, 0xff, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  static const uint8_t cacti_small_small_small_bitmap[] PROGMEM = {
    0x1b, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xf8,
    0x00, 0xff, 0xff, 0x80, 0xfe, 0x7c, 0x00, 0xfe, 0x00, 0xff, 0xff, 0xff, 0x00, 0xf8, 0x00, 0x7f,
    0xfe, 0x80, 0xff, 0xff, 0x00, 0xfe, 0xfc, 0x00, 0x00, 0x01, 0x03, 0x02, 0xff, 0xff, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x06, 0xff, 0xff, 0xff, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00
  };

  static const uint8_t pterodactyl_1_bitmap[] PROGMEM = {
    0x17, 0x14, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x03, 0x07, 0xff, 0xff, 0xff, 0x7f, 0x3f, 0x3f, 0x3e, 0x3c, 0x3c, 0x1c, 0x14, 0x14, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  static const uint8_t pterodactyl_2_bitmap[] PROGMEM = {
    0x17, 0x14, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf0, 0xc0, 0x0e, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3e, 0x3c, 0x3c, 0x1c, 0x14, 0x14, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  static const uint8_t ground_1_bitmap[] PROGMEM = {
    0x40, 0x0c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x90, 0x90, 0x90, 0x90, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08,
    0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
  };

  static const uint8_t ground_2_bitmap[] PROGMEM = {
    0x40, 0x0c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,
    0x00, 0x00
  };

  static const uint8_t ground_3_bitmap[] PROGMEM = {
    0x40, 0x0c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x90, 0x90,
    0x90, 0x90, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
  };

  static const uint8_t ground_4_bitmap[] PROGMEM = {
    0x40, 0x0c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x18, 0x0c, 0x04, 0x06, 0x03, 0x01, 0x01, 0x01, 0x01, 0x03, 0x04, 0x04, 0x08, 0x18, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x18, 0x08, 0x04, 0x04, 0x06, 0x03, 0x01, 0x01, 0x01,
    0x01, 0x03, 0x04, 0x04, 0x08, 0x18, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x01, 0x00
  };

  static const uint8_t ground_5_bitmap[] PROGMEM = {
    0x40, 0x0c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x18, 0x0c, 0x04, 0x06, 0x03,
    0x01, 0x01, 0x01, 0x01, 0x03, 0x06, 0x04, 0x08, 0x18, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x90, 0x90, 0x90, 0x90, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0x60, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x60, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01,
    0x01, 0x00
  };

  // ── BitmapMasked Struct ──
  struct BitmapMasked {
    uint8_t width;
    uint8_t height;
    const uint8_t* data;
    
    BitmapMasked(const uint8_t* bitmapData) {
      width = pgm_read_byte(bitmapData);
      height = pgm_read_byte(bitmapData + 1);
      data = bitmapData + 2;
    }
  };

  // ── Static Bitmap Objects ──
  static BitmapMasked BM_TREX_UP1(trex_up_1s_bitmap);
  static BitmapMasked BM_TREX_UP2(trex_up_2s_bitmap);
  static BitmapMasked BM_TREX_UP3(trex_up_3s_bitmap);
  static BitmapMasked BM_TREX_DUCK1(trex_duck_1s_bitmap);
  static BitmapMasked BM_TREX_DUCK2(trex_duck_2s_bitmap);
  static BitmapMasked BM_TREX_DEAD(trex_dead_1_bitmap);
  
  static BitmapMasked BM_CACTUS_BB(cacti_big_big_bitmap);
  static BitmapMasked BM_CACTUS_BS(cacti_big_small_bitmap);
  static BitmapMasked BM_CACTUS_SB(cacti_small_big_bitmap);
  static BitmapMasked BM_CACTUS_SSS(cacti_small_small_small_bitmap);
  
  static BitmapMasked BM_PTERO1(pterodactyl_1_bitmap);
  static BitmapMasked BM_PTERO2(pterodactyl_2_bitmap);
  
  static BitmapMasked BM_GROUND1(ground_1_bitmap);
  static BitmapMasked BM_GROUND2(ground_2_bitmap);
  static BitmapMasked BM_GROUND3(ground_3_bitmap);
  static BitmapMasked BM_GROUND4(ground_4_bitmap);
  static BitmapMasked BM_GROUND5(ground_5_bitmap);

  // ── Sprite Struct ──
  struct Sprite {
    const BitmapMasked* bitmap;
    int16_t x, y;
    bool active;
    
    Sprite() : bitmap(nullptr), x(0), y(0), active(false) {}
    Sprite(const BitmapMasked* bm, int16_t xPos, int16_t yPos) 
      : bitmap(bm), x(xPos), y(yPos), active(true) {}
    
    void draw() {
      if (!bitmap || !active) return;
      u8g2.drawXBMP(x, y, bitmap->width, bitmap->height, bitmap->data);
    }
  };

  // ── TrexPlayer ──
  struct TrexPlayer {
    enum State { UP, DUCK, DEAD };
    
    State state;
    Sprite sprite;
    int8_t vy, dy;
    bool skipStep;
    uint8_t bitmapId;
    uint8_t blinkCnt;
    const BitmapMasked* sprites[6];
    
    TrexPlayer() {
      sprites[0] = &BM_TREX_UP1;
      sprites[1] = &BM_TREX_UP2;
      sprites[2] = &BM_TREX_UP3;
      sprites[3] = &BM_TREX_DUCK1;
      sprites[4] = &BM_TREX_DUCK2;
      sprites[5] = &BM_TREX_DEAD;
      
      state = UP;
      sprite = Sprite(sprites[0], 10, 36);
      vy = 0; dy = 0;
      skipStep = false;
      bitmapId = 0;
      blinkCnt = 0;
    }
    
    void step() { animationStep(); motionStep(); }
    
    void jump() {
      if (isJumping() || state == DEAD) return;
      vy = (state == UP) ? 7 : 5;
      beep(700, 15, soundLevel);
    }
    
    void duck(bool toDuck) {
      if (toDuck && state == UP && !isJumping()) state = DUCK;
      else if (!toDuck && state == DUCK) state = UP;
    }
    
    void die() { state = DEAD; vy = 0; sprite.bitmap = sprites[5]; }
    void blink() { blinkCnt = 32; }
    bool isBlinking() { return blinkCnt > 0; }
    bool isJumping() const { return dy != 0 || vy != 0; }
    
  private:
    void motionStep() {
      if (abs(vy) <= 1 && !skipStep) { skipStep = true; return; }
      skipStep = false;
      
      dy += vy;
      sprite.y -= vy;
      if (dy) --vy; else vy = 0;
      
      if (sprite.y + sprite.bitmap->height >= 60) {
        sprite.y = 60 - sprite.bitmap->height;
        vy = 0; dy = 0;
      }
    }
    
    void animationStep() {
      if (blinkCnt) { blinkCnt--; return; }
      
      uint8_t start = 0, end = 0;
      if (state == UP) { start = 0; end = isJumping() ? 1 : 3; }
      else if (state == DUCK) { start = 3; end = isJumping() ? 3 : 5; }
      else { start = 5; end = 6; }
      
      if (!(bitmapId >= start && bitmapId < end)) bitmapId = start;
      if (bitmapId + 1 < end) bitmapId++; else bitmapId = start;
      
      sprite.bitmap = sprites[bitmapId];
    }
  };

  // ── Ground ──
  struct Ground {
    Sprite sprite;
    int8_t speed;
    
    Ground(int16_t startX) : speed(3) {
      sprite = Sprite(&BM_GROUND1, startX, 60);
    }
    
    void step() {
      for (int i = 0; i < speed; i++) {
        sprite.x--;
        if (sprite.x + sprite.bitmap->width < 0) {
          const BitmapMasked* pick;
          switch (random(0, 5)) {
            case 0: pick = &BM_GROUND1; break;
            case 1: pick = &BM_GROUND2; break;
            case 2: pick = &BM_GROUND3; break;
            case 3: pick = &BM_GROUND4; break;
            default: pick = &BM_GROUND5; break;
          }
          sprite.bitmap = pick;
          sprite.x = 128;
        }
      }
    }
    
    void draw() { sprite.draw(); }
  };

  // ── Cactus ──
  struct Cactus {
    Sprite sprite;
    int8_t speed;
    uint8_t respawnWait;
    
    static const BitmapMasked* pickType(int type) {
      switch (type) {
        case 0: return &BM_CACTUS_BB;
        case 1: return &BM_CACTUS_BS;
        case 2: return &BM_CACTUS_SB;
        default: return &BM_CACTUS_SSS;
      }
    }
    
    Cactus() : speed(3), respawnWait(0) {
      sprite = Sprite(pickType(random(0, 4)), 200 + random(0, 100), 60);
    }
    
    void step() {
      if (!sprite.active) {
        if (respawnWait) { respawnWait--; return; }
        sprite.bitmap = pickType(random(0, 4));
        sprite.x = 128 + random(0, 60);
        sprite.y = 60 - sprite.bitmap->height;
        sprite.active = true;
        respawnWait = random(10, 50);
        return;
      }
      
      sprite.x -= speed;
      if (sprite.x + sprite.bitmap->width < 0) {
        sprite.active = false;
        respawnWait = random(10, 30);
      }
    }
    
    void draw() { sprite.draw(); }
    
    bool checkCollision(const TrexPlayer& player) {
      if (!sprite.active) return false;
      int margin = 4;
      return (player.sprite.x + margin < sprite.x + sprite.bitmap->width - margin &&
              player.sprite.x + player.sprite.bitmap->width - margin > sprite.x + margin &&
              player.sprite.y + margin < sprite.y + sprite.bitmap->height - margin &&
              player.sprite.y + player.sprite.bitmap->height - margin > sprite.y + margin);
    }
  };

  // ── Pterodactyl ──
  struct Pterodactyl {
    Sprite sprite;
    int8_t speed;
    uint8_t respawnWait, animSkip;
    bool frame;
    
    Pterodactyl() : speed(5), respawnWait(0), animSkip(0), frame(false) {
      sprite = Sprite(&BM_PTERO1, -50, 25);
      sprite.active = false;
    }
    
    void step() {
      if (animSkip) {
        animSkip--;
      } else {
        animSkip = 6;
        frame = !frame;
        if (sprite.active) sprite.bitmap = frame ? &BM_PTERO1 : &BM_PTERO2;
      }
      
      if (!sprite.active) {
        if (respawnWait) { respawnWait--; return; }
        if (random(0, 100) < 15) {
          sprite.bitmap = &BM_PTERO1;
          sprite.x = 128 + random(0, 40);
          sprite.y = random(15, 35);
          sprite.active = true;
          respawnWait = random(50, 150);
        }
        return;
      }
      
      sprite.x -= speed;
      if (sprite.x + sprite.bitmap->width < 0) {
        sprite.active = false;
        respawnWait = random(30, 80);
      }
    }
    
    void draw() { sprite.draw(); }
    
    bool checkCollision(const TrexPlayer& player) {
      if (!sprite.active) return false;
      int margin = 4;
      return (player.sprite.x + margin < sprite.x + sprite.bitmap->width - margin &&
              player.sprite.x + player.sprite.bitmap->width - margin > sprite.x + margin &&
              player.sprite.y + margin < sprite.y + sprite.bitmap->height - margin &&
              player.sprite.y + player.sprite.bitmap->height - margin > sprite.y + margin);
    }
  };

  // ── Game State ──
  uint16_t hiScore = 0;
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(GAME_INDEX * sizeof(uint16_t), hiScore);
  if (hiScore == 0xFFFF || hiScore > 9999) hiScore = 0;
  EEPROM.end();
  
  TrexPlayer trex;
  Ground ground1(-1), ground2(63), ground3(127);
  Cactus cactus1, cactus2;
  Pterodactyl pterodactyl;
  
  uint16_t score = 0;
  uint8_t lives = 3;
  bool gameOver = false;
  uint32_t lastFrame = millis();
  uint8_t targetFPS = 23;
  
  // ── Start Screen ──
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("T-REX RUNNER 2", 20);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Any direction to jump!", 36);
  centreStr("Duck with DOWN", 48);
  u8g2.setFont(u8g2_font_5x7_tr);
  centreStr("Press any key to start", 60);
  u8g2.sendBuffer();
  
  waitRelease();
  while (!btnPressed(BTN_UP) && !btnPressed(BTN_DOWN) && 
         !btnPressed(BTN_LEFT) && !btnPressed(BTN_RIGHT) &&
         !btnPressed(BTN_ENTER)) {
    delay(10);
  }
  waitRelease();
  
  // ── Main Game Loop ──
  while (true) {
    if (checkPause("T-REX RUNNER 2")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    float dt = (now - lastFrame) / 16.0f;
    if (dt > 3.0f) dt = 3.0f;
    if (dt < 0.5f) dt = 0.5f;
    lastFrame = now;
    
    // Input
    if (btnPressed(BTN_UP) || btnPressed(BTN_LEFT) || 
        btnPressed(BTN_RIGHT) || btnPressed(BTN_ENTER)) {
      trex.jump();
    }
    trex.duck(btnHeld(BTN_DOWN));
    
    if (!gameOver) {
      score++;
      
      ground1.step(); ground2.step(); ground3.step();
      cactus1.step(); cactus2.step(); pterodactyl.step();
      trex.step();
      
      // Collision detection
      bool hit = false;
      if (cactus1.checkCollision(trex)) hit = true;
      if (!hit && cactus2.checkCollision(trex)) hit = true;
      if (!hit && pterodactyl.checkCollision(trex)) hit = true;
      
      if (hit && !trex.isBlinking()) {
        lives--;
        trex.blink();
        if (lives <= 0) {
          trex.die();
          gameOver = true;
          beep(200, 300, soundLevel);
        } else {
          beep(200, 150, soundLevel);
        }
      }
      
      // Speed up
      if (score % 256 == 0 && targetFPS < 48) {
        targetFPS++;
      }
      
      // High score
      if (score > hiScore) {
        hiScore = score;
        EEPROM.begin(EEPROM_SIZE);
        EEPROM.put(GAME_INDEX * sizeof(uint16_t), hiScore);
        EEPROM.commit();
        EEPROM.end();
      }
    }
    
    // ── Draw ──
    u8g2.clearBuffer();
    
    ground1.draw(); ground2.draw(); ground3.draw();
    cactus1.draw(); cactus2.draw(); pterodactyl.draw();
    trex.sprite.draw();
    
    u8g2.setFont(u8g2_font_5x7_tr);
    
    char scoreText[16];
    snprintf(scoreText, sizeof(scoreText), "SCORE:%d", score);
    u8g2.drawStr(SCREEN_W - u8g2.getStrWidth(scoreText) - 2, 8, scoreText);
    
    if (hiScore > 0) {
      char hiText[16];
      snprintf(hiText, sizeof(hiText), "HI:%d", hiScore);
      u8g2.drawStr(SCREEN_W - u8g2.getStrWidth(hiText) - 2, 16, hiText);
    }
    
    for (int i = 0; i < lives; i++) {
      drawHeart(2 + i * 8, 1);
    }
    
    char speedText[16];
    snprintf(speedText, sizeof(speedText), "SPD:%d", targetFPS - 23 + 1);
    u8g2.drawStr(2, 55, speedText);
    
    if (gameOver) {
      u8g2.setDrawColor(0);
      u8g2.drawBox(0, 18, SCREEN_W, 30);
      u8g2.setDrawColor(1);
      
      u8g2.setFont(u8g2_font_ncenB10_tr);
      centreStr("GAME OVER", 32);
      
      u8g2.setFont(u8g2_font_5x7_tr);
      centreStr("Press ENTER to restart", 46);
      centreStr("MENU to quit", 54);
      
      u8g2.sendBuffer();
      
      while (gameOver) {
        if (btnPressed(BTN_ENTER) || btnPressed(BTN_UP)) {
          gameOver = false;
          score = 0;
          lives = 3;
          targetFPS = 23;
          trex = TrexPlayer();
          cactus1 = Cactus();
          cactus2 = Cactus();
          pterodactyl = Pterodactyl();
          beep(800, 30, soundLevel);
          delay(100);
          beep(1000, 30, soundLevel);
          break;
        }
        if (btnPressed(BTN_MENU)) {
          playMenuButtonSound();
          gameOverScreen(score, GAME_INDEX, false);
          return;
        }
        delay(50);
      }
    }
    
    u8g2.sendBuffer();
    
    int frameTime = 1000 / targetFPS;
    int elapsed = millis() - lastFrame;
    if (elapsed < frameTime) {
      delay(frameTime - elapsed);
    }
  }
}



// ============================================================
// METEOR DEFENDERS - Fixed Version (All bugs fixed)
// আগে: pixel-by-pixel check (ভুল)
// ============================================================
// METEOR DEFENDERS - Fixed Collision Detection
// ============================================================

void game_meteor_defenders() {
  const int GAME_INDEX = 17;

  // ── Game Variables ──
  bool quit = false;
  float stepUD = 0;

  // Ship coordinates (triangle)
  float x1 = 8, y1 = 6;
  float x2 = 8, y2 = 20;
  float x3 = 23, y3 = 13;

  // Bullet
  float bulx = 0, buly = 0;
  bool bullet = false;

  // Meteors
  struct Meteor {
    float x, y;
    bool active;
    int size;
    float speed;
  };
  
  Meteor meteors[5];
  for (int i = 0; i < 5; i++) {
    meteors[i].active = false;
    meteors[i].size = 4;
    meteors[i].speed = 1.0f;
  }

  float bulletspeed = 6.0f;
  int luck = 0;
  float trispeed = 2.5f;
  unsigned int score = 0;
  unsigned int highscore = 0;
  int meteorCount = 1;
  uint32_t lastMeteorSpawn = 0;
  uint32_t lastFrame = millis();

  // ── Load High Score ──
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(GAME_INDEX * sizeof(uint16_t), highscore);
  if (highscore == 0xFFFF || highscore > 9999) highscore = 0;
  EEPROM.end();

  // ── Show Start Screen ──
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("METEOR", 20);
  centreStr("DEFENDERS", 36);
  u8g2.setFont(u8g2_font_5x7_tr);
  char hs[20];
  snprintf(hs, sizeof(hs), "HI: %u", highscore);
  centreStr(hs, 48);
  centreStr("UP/DOWN=Move  LEFT/RIGHT/ENTER=Shoot", 60);
  u8g2.sendBuffer();

  waitRelease();
  while (!btnPressed(BTN_UP) && !btnPressed(BTN_DOWN) &&
         !btnPressed(BTN_LEFT) && !btnPressed(BTN_RIGHT) &&
         !btnPressed(BTN_ENTER)) {
    delay(10);
  }
  waitRelease();

  // ── Game Loop ──
  while (true) {
    if (checkPause("METEOR DEFENDERS")) return;
    if (checkMenuAndReturn()) return;

    // ── Reset Game State ──
    quit = false;
    stepUD = 0;
    x1 = 8; y1 = 6;
    x2 = 8; y2 = 20;
    x3 = 23; y3 = 13;
    bulx = 0; buly = 0;
    bullet = false;
    for (int i = 0; i < 5; i++) {
      meteors[i].active = false;
      meteors[i].size = 5 + random(0, 2);
      meteors[i].speed = 0.6f + (random(0, 20) / 40.0f);
    }
    bulletspeed = 6.0f;
    trispeed = 2.5f;
    score = 0;
    meteorCount = 1;
    luck = random(30, 80);
    lastMeteorSpawn = millis();
    lastFrame = millis();

    // ── Main Game ──
    while (!quit) {
      if (checkPause("METEOR DEFENDERS")) return;
      if (checkMenuAndReturn()) return;

      uint32_t now = millis();
      float dt = (now - lastFrame) / 16.0f;
      if (dt > 2.0f) dt = 2.0f;
      lastFrame = now;

      // ── Boundary Check ──
      float minY = min(y1, min(y2, y3));
      float maxY = max(y1, max(y2, y3));
      
      if (minY < 0) {
        float diff = -minY;
        y1 += diff;
        y2 += diff;
        y3 += diff;
      }
      if (maxY > 63) {
        float diff = 63 - maxY;
        y1 += diff;
        y2 += diff;
        y3 += diff;
      }

      // ── Input Handling ──
      if (btnHeld(BTN_DOWN)) {
        stepUD = trispeed;
      }
      if (btnHeld(BTN_UP)) {
        stepUD = -trispeed;
      }
      if (btnPressed(BTN_LEFT) || btnPressed(BTN_RIGHT) || btnPressed(BTN_ENTER)) {
        if (!bullet) {
          bullet = true;
          bulx = x3;
          buly = y3;
          beep(900, 10, soundLevel);
        }
      }

      // ── Bullet Update ──
      if (bullet) {
        bulx += bulletspeed * dt;
        if (bulx > SCREEN_W + 10) {
          bullet = false;
        }
      }

      // ── Meteor Spawning ──
      if (score < 250 ) meteorCount = 1;
      if(score > 250 && score < 600) meteorCount = random(1,2);
      if (score > 600 && score < 1000 ) meteorCount = 2;
      if (score > 1000 && score < 1300) meteorCount = random(2,3);
      if (score > 1300 && score < 1500) meteorCount = random(3,4);
      if(score > 1600) meteorCount = 4 ;
  

      int spawnInterval = 80 - (int)(score / 50);
      if (spawnInterval < 20) spawnInterval = 20;
      if (spawnInterval > 80) spawnInterval = 80;

      if (now - lastMeteorSpawn > (uint32_t)spawnInterval) {
        int activeCount = 0;
        for (int i = 0; i < 5; i++) {
          if (meteors[i].active) activeCount++;
        }
        
        if (activeCount < meteorCount) {
          for (int i = 0; i < 5; i++) {
            if (!meteors[i].active) {
              meteors[i].active = true;
              meteors[i].x = SCREEN_W + random(25, 45);
              meteors[i].y = random(5, 58);
              meteors[i].size = 5 + random(0, 2);
              meteors[i].speed = 0.7f + (random(0, 20) / 40.0f);
              break;
            }
          }
        }
        lastMeteorSpawn = now;
      }

      // ── Update Meteors & Collision ──
      for (int i = 0; i < 5; i++) {
        if (!meteors[i].active) continue;
        
        meteors[i].x -= meteors[i].speed * dt;
        
        // 🔥 FIXED: Game Over if meteor reaches left side
        if (meteors[i].x < -10) {
          meteors[i].active = false;
          quit = true;
          beep(200, 200, soundLevel);
          break;
        }

        // 🔥 FIXED: Collision with Bullet (more accurate)
        if (bullet) {
          float bulletRadius = 2.0f;
          float meteorRadius = meteors[i].size;
          float dx = bulx - meteors[i].x;
          float dy = buly - meteors[i].y;
          float distance = sqrt(dx*dx + dy*dy);
          
          if (distance < meteorRadius + bulletRadius) {
            meteors[i].active = false;
            bullet = false;
            score += 10;
            beep(1200, 20, soundLevel);
            
            // Explosion effect
            u8g2.drawLine(bulx - 5, buly - 5, bulx + 5, buly + 5);
            u8g2.drawLine(bulx - 5, buly + 5, bulx + 5, buly - 5);
            u8g2.drawCircle((int)bulx, (int)buly, 3);
            u8g2.sendBuffer();
            delay(50);
            continue;
          }
        }

        // 🔥 FIXED: Collision with Ship (more accurate)
        float shipCenterX = x3;
        float shipCenterY = y3;
        float dx = meteors[i].x - shipCenterX;
        float dy = meteors[i].y - shipCenterY;
        float dist = sqrt(dx*dx + dy*dy);
        
        if (dist < meteors[i].size + 5) {
          meteors[i].active = false;
          quit = true;
          beep(200, 300, soundLevel);
          break;
        }
      }

      // ── Update Ship Position ──
      y1 += stepUD * dt;
      y3 += stepUD * dt;
      y2 += stepUD * dt;
      stepUD = 0;

      // ── Luck Counter ──
      if (luck > 0) {
        luck--;
      } else {
        luck = -1;
      }
      score++;

      // ── Draw Everything ──
      u8g2.clearBuffer();

      // Draw planet
      for (int r = 58; r <= 64; r++) {
        u8g2.drawCircle(-58, 32, r);
      }
      
      // Draw ship
      u8g2.drawTriangle((int)x1, (int)y1, (int)x2, (int)y2, (int)x3, (int)y3);
      u8g2.drawTriangle((int)x1+1, (int)y1, (int)x2-1, (int)y2, (int)x3, (int)y3);

      // Draw bullet (bigger for visibility)
      if (bullet) {
        u8g2.drawBox((int)bulx-1, (int)buly-1, 3, 3);
      }

      // Draw meteors (filled)
      for (int i = 0; i < 5; i++) {
        if (!meteors[i].active) continue;
        int size = meteors[i].size;
        for (int r = size; r > 0; r--) {
          u8g2.drawCircle((int)meteors[i].x, (int)meteors[i].y, r);
        }
      }

      // ── UI ──
      u8g2.setFont(u8g2_font_5x7_tr);
      
      char sc[10];
      snprintf(sc, sizeof(sc), "%u", score);
      u8g2.drawStr(2, 8, sc);

      if (highscore > 0) {
        char hs2[16];
        snprintf(hs2, sizeof(hs2), "HI:%u", highscore);
        u8g2.drawStr(SCREEN_W - u8g2.getStrWidth(hs2) - 2, 8, hs2);
      }

      char mc[10];
      snprintf(mc, sizeof(mc), "M:%d", meteorCount);
      u8g2.drawStr(2, 16, mc);

      u8g2.sendBuffer();
      delay(16);
    }

    // ── Game Over ──
    if (score > highscore) {
      highscore = score;
      EEPROM.begin(EEPROM_SIZE);
      EEPROM.put(GAME_INDEX * sizeof(uint16_t), highscore);
      EEPROM.commit();
      EEPROM.end();
      beep(1200, 40, soundLevel);
      delay(60);
      beep(1500, 40, soundLevel);
      delay(60);
      beep(1800, 80, soundLevel);
    }

    // ── Game Over Screen ──
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tr);
    centreStr("GAME OVER", 20);
    u8g2.setFont(u8g2_font_6x10_tr);
    char buf[20];
    snprintf(buf, sizeof(buf), "Score: %u", score);
    centreStr(buf, 38);
    char hs3[20];
    snprintf(hs3, sizeof(hs3), "Best: %u", highscore);
    centreStr(hs3, 52);
    u8g2.setFont(u8g2_font_5x7_tr);
    centreStr("ENTER=Replay  MENU=Quit", 62);
    u8g2.sendBuffer();

    bool waiting = true;
    while (waiting) {
      if (btnPressed(BTN_ENTER) || btnPressed(BTN_UP)) {
        waiting = false;
        beep(800, 30, soundLevel);
        break;
      }
      if (btnPressed(BTN_MENU)) {
        playMenuButtonSound();
        gameOverScreen(score, GAME_INDEX, false);
        return;
      }
      delay(50);
    }
  }
}



// ============================================================
// DEATH STAR - Space Shooter Game
// ============================================================

// ============================================================
// DEATH STAR - Space Shooter Game (HOLD TO SHOOT MULTIPLE BULLETS)
// ============================================================

void game_death_star() {
  const int GAME_INDEX = 18;

  // ── Game Variables ──
  int metx = 0, mety = 0;
  int postoji = 0;
  int nep = 8;
  int smjer = 0;
  int go = 0;
  int rx = 95, ry = 0;
  int rx2 = 95, ry2 = 0;
  int rx3 = 95, ry3 = 0;
  int rx4 = 95, ry4 = 0;
  int bodovi = 0;
  int brzina = 3;
  int bkugle = 1;
  int najmanja = 600;
  int najveca = 1200;
  int promjer = 10;
  int zivoti = 5;
  int poc = 0;
  int ispaljeno = 0;
  int nivo = 1;
  int centar = 95;
  unsigned long pocetno = 0;
  unsigned long odabrano = 0;
  unsigned long trenutno = 0;
  unsigned long nivovrije = 0;
  int poz = 30;
  unsigned int highscore = 0;

  // ── 🔥 NEW: Multiple bullets system ──
  #define MAX_BULLETS 10
  struct Bullet {
    float x, y;
    bool active;
  };
  Bullet bullets[MAX_BULLETS];
  for (int i = 0; i < MAX_BULLETS; i++) {
    bullets[i].active = false;
  }
  unsigned long lastShotTime = 0;
  int shootDelay = 350;  // milliseconds between shots when holding

  // ── Death Star Sprite (48x48) ──
  static const unsigned char storm[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7F, 0xFE, 0x00, 0x00, 0x00, 0x07, 0x80, 0x01, 0xE0, 0x00, 0x00, 0x0C,
    0x00, 0x00, 0x20, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0x30, 0x00, 0x00, 0x04, 0x00,
    0x00, 0x20, 0x00, 0x00, 0x04, 0x00, 0x00, 0x20, 0x00, 0x00, 0x04, 0x00, 0x00, 0x60, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x40, 0x00, 0x00, 0x02, 0x00, 0x00, 0x40, 0x00, 0x00, 0x01, 0x00, 0x00, 0x40,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x40, 0x00, 0x00, 0x01, 0x00, 0x00, 0x7F, 0xE0, 0x00, 0x01, 0x00,
    0x00, 0x7F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xD7, 0xFF, 0xFF,
    0xE1, 0x00, 0x01, 0xBF, 0xFC, 0x1F, 0xFA, 0x80, 0x01, 0xBF, 0xF1, 0xCF, 0xFA, 0x80, 0x01, 0x3F,
    0xC2, 0x37, 0xF7, 0x80, 0x01, 0xEF, 0x9C, 0x01, 0xE7, 0xC0, 0x01, 0xE0, 0x70, 0x06, 0x06, 0x80,
    0x01, 0xE0, 0xC0, 0x03, 0x06, 0x80, 0x01, 0xFF, 0x80, 0x01, 0xFF, 0x80, 0x01, 0xF8, 0x00, 0x00,
    0x1D, 0xC0, 0x03, 0x70, 0x00, 0x80, 0x0C, 0x60, 0x05, 0xB0, 0x07, 0xF0, 0x08, 0x90, 0x09, 0x10,
    0x1F, 0xF8, 0x09, 0xD0, 0x0B, 0x90, 0x1F, 0x7C, 0x03, 0xF0, 0x0F, 0xC0, 0xFC, 0x0F, 0x07, 0x90,
    0x0D, 0x43, 0xC0, 0x03, 0x07, 0x90, 0x05, 0x64, 0x00, 0x00, 0xCF, 0x10, 0x07, 0xFC, 0x00, 0x00,
    0x26, 0x10, 0x01, 0x80, 0x00, 0x00, 0x10, 0x20, 0x01, 0x00, 0x00, 0x00, 0x0E, 0x40, 0x01, 0x80,
    0x07, 0xF0, 0x01, 0x80, 0x00, 0x80, 0x07, 0xC8, 0x00, 0x80, 0x00, 0x80, 0x0B, 0xE8, 0x00, 0x80,
    0x00, 0x87, 0x97, 0xE9, 0xE0, 0x80, 0x00, 0x87, 0xDF, 0xEF, 0xA0, 0x80, 0x00, 0x4B, 0xFF, 0xFF,
    0xA0, 0x80, 0x00, 0x6B, 0xDF, 0xFB, 0xA3, 0x00, 0x00, 0x24, 0x97, 0xE8, 0x24, 0x00, 0x00, 0x1E,
    0x1F, 0xC0, 0x2C, 0x00, 0x00, 0x07, 0xF8, 0x1F, 0xF0, 0x00, 0x00, 0x00, 0x0F, 0xF8, 0x00, 0x00
  };

  // ── X-Wing Sprite (16x16) ──
  static const unsigned char dioda16[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x3F, 0xF0, 0x3C, 0x00, 0x3C, 0x00, 0xFF, 0x00, 0x7F, 0xFF,
    0x7F, 0xFF, 0xFF, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x1F, 0xF0, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  // ── Load High Score ──
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(GAME_INDEX * sizeof(uint16_t), highscore);
  if (highscore == 0xFFFF || highscore > 9999) highscore = 0;
  EEPROM.end();

  // ── Show Start Screen ──
  u8g2.clearBuffer();
  u8g2.drawXBMP(6, 8, 48, 48, storm);
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(65, 20, "xWing");
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(65, 30, "vs");
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(65, 48, "Death");
  u8g2.drawStr(65, 60, "Star");
  u8g2.setFont(u8g2_font_5x7_tr);
  char hs[20];
  snprintf(hs, sizeof(hs), "HI: %u", highscore);
  u8g2.drawStr(65, 70, hs);
  u8g2.sendBuffer();

  waitRelease();
  while (!btnPressed(BTN_UP) && !btnPressed(BTN_DOWN) &&
         !btnPressed(BTN_LEFT) && !btnPressed(BTN_RIGHT) &&
         !btnPressed(BTN_ENTER)) {
    delay(10);
  }
  waitRelease();

  // ── Reset Function ──
  auto ponovo = [&]() {
    metx = 0; mety = 0;
    postoji = 0;
    nep = 8;
    smjer = 0;
    go = 0;
    rx = 95; ry = 0;
    rx2 = 95; ry2 = 0;
    rx3 = 95; ry3 = 0;
    rx4 = 95; ry4 = 0;
    bodovi = 0;
    brzina = 3;
    bkugle = 1;
    najmanja = 600;
    najveca = 1200;
    promjer = 10;
    zivoti = 5;
    poc = 0;
    ispaljeno = 0;
    nivo = 1;
    centar = 95;
    pocetno = 0;
    odabrano = 0;
    trenutno = 0;
    nivovrije = 0;
    poz = 30;
    for (int i = 0; i < MAX_BULLETS; i++) {
      bullets[i].active = false;
    }
    lastShotTime = 0;
  };

  // ── Game Loop ──
  while (true) {
    if (checkPause("DEATH STAR")) return;
    if (checkMenuAndReturn()) return;

    ponovo();

    while (go == 0) {
      if (checkPause("DEATH STAR")) return;
      if (checkMenuAndReturn()) return;

      u8g2.clearBuffer();

      // ── Draw stars (background) ──
      static int stars[20][2];
      static bool starsInit = false;
      if (!starsInit) {
        for (int i = 0; i < 20; i++) {
          stars[i][0] = random(0, 128);
          stars[i][1] = random(0, 64);
        }
        starsInit = true;
      }
      for (int i = 0; i < 20; i++) {
        u8g2.drawPixel(stars[i][0], stars[i][1]);
        stars[i][0]--;
        if (stars[i][0] < 0) {
          stars[i][0] = 128;
          stars[i][1] = random(0, 64);
        }
      }

      // ── Enemy shooting timer ──
      if (poc == 0) {
        pocetno = millis();
        odabrano = random(najmanja, najveca);
        poc = 1;
      }
      trenutno = millis();

      // ── Level up ──
      if ((trenutno - nivovrije) > 50000) {
        nivovrije = trenutno;
        nivo = nivo + 1;
        brzina = brzina + 1;
        if (nivo % 2 == 0) {
          bkugle = bkugle + 1;
          promjer = promjer - 1;
          if (promjer < 4) promjer = 4;
        }
        najmanja = najmanja - 50;
        najveca = najveca - 50;
        if (najmanja < 200) najmanja = 200;
        if (najveca < 400) najveca = 400;
      }

      // ── Enemy bullets ──
      if ((odabrano + pocetno) < trenutno) {
        poc = 0;
        ispaljeno = ispaljeno + 1;
        if (ispaljeno == 1) { rx = 95; ry = nep; }
        if (ispaljeno == 2) { rx2 = 95; ry2 = nep; }
        if (ispaljeno == 3) { rx3 = 95; ry3 = nep; }
        if (ispaljeno == 4) { rx4 = 95; ry4 = nep; }
      }

      if (ispaljeno > 0) {
        u8g2.drawCircle(rx, ry, 2);
        rx = rx - brzina;
      }
      if (ispaljeno > 1) {
        u8g2.drawCircle(rx2, ry2, 1);
        rx2 = rx2 - brzina;
      }
      if (ispaljeno > 2) {
        u8g2.drawCircle(rx3, ry3, 4);
        rx3 = rx3 - brzina;
      }
      if (ispaljeno > 3) {
        u8g2.drawCircle(rx4, ry4, 2);
        rx4 = rx4 - brzina;
      }

      // ── Player movement ──
      if (btnHeld(BTN_UP) && poz >= 2) {
        poz = poz - 2;
      }
      if (btnHeld(BTN_DOWN) && poz <= 46) {
        poz = poz + 2;
      }

      // ── 🔥 FIXED: Hold to shoot multiple bullets ──
      if (btnHeld(BTN_LEFT) || btnHeld(BTN_RIGHT) || btnHeld(BTN_ENTER)) {
        unsigned long currentTime = millis();
        if (currentTime - lastShotTime > shootDelay) {
          // Find inactive bullet
          bool shotFired = false;
          for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active) {
              bullets[i].active = true;
              bullets[i].x = 6;
              bullets[i].y = poz + 8;
              lastShotTime = currentTime;
              beep(1200, 10, soundLevel);
              shotFired = true;
              break;
            }
          }
        }
      }

      // ── Update bullets ──
      for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
          bullets[i].x += 8;
          u8g2.drawLine(bullets[i].x, bullets[i].y, bullets[i].x + 4, bullets[i].y);
          
          // Remove if off screen
          if (bullets[i].x > 128) {
            bullets[i].active = false;
          }
        }
      }

      // ── Draw X-Wing ──
      u8g2.drawXBMP(4, poz, 16, 16, dioda16);

      // ── Draw Death Star ──
      for (int r = promjer; r > 0; r--) {
        u8g2.drawCircle(centar, nep, r);
      }
      u8g2.setDrawColor(0);
      int innerRadius = promjer / 3;
      for (int r = innerRadius; r > 0; r--) {
        u8g2.drawCircle(centar + 2, nep + 3, r);
      }
      u8g2.setDrawColor(1);

      // ── UI ──
      u8g2.setFont(u8g2_font_5x7_tr);
      char sc[10];
      snprintf(sc, sizeof(sc), "%d", bodovi);
      u8g2.drawStr(33, 57, "score:");
      u8g2.drawStr(68, 57, sc);

      char lv[10];
      snprintf(lv, sizeof(lv), "%d", zivoti);
      u8g2.drawStr(33, 8, "lives:");
      u8g2.drawStr(68, 8, lv);

      char nv[10];
      snprintf(nv, sizeof(nv), "%d", nivo);
      u8g2.drawStr(110, 8, "L:");
      u8g2.drawStr(122, 8, nv);

      char tm[10];
      snprintf(tm, sizeof(tm), "%lu", trenutno / 1000);
      u8g2.drawStr(108, 57, tm);

      // ── Enemy movement ──
      if (smjer == 0) {
        nep = nep + bkugle;
      } else {
        nep = nep - bkugle;
      }
      if (nep >= (64 - promjer)) smjer = 1;
      if (nep <= promjer) smjer = 0;

      // ── 🔥 Collision: All bullets vs Death Star ──
      for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        
        if (bullets[i].y >= nep - promjer && bullets[i].y <= nep + promjer) {
          if (bullets[i].x > (centar - promjer) && bullets[i].x < (centar + promjer)) {
            bullets[i].active = false;
            beep(500, 20, soundLevel);
            bodovi = bodovi + 1;
          }
        }
      }

      // ── Collision: Enemy bullets vs Player ──
      int pozicija = poz + 8;

      auto checkHit = [&](int rx_, int ry_) {
        if (ry_ >= pozicija - 8 && ry_ <= pozicija + 8) {
          if (rx_ < 12 && rx_ > 4) {
            beep(100, 100, soundLevel);
            zivoti = zivoti - 1;
            return true;
          }
        }
        return false;
      };

      if (checkHit(rx, ry)) { rx = 95; ry = -50; }
      if (checkHit(rx2, ry2)) { rx2 = -50; ry2 = -50; }
      if (checkHit(rx3, ry3)) { rx3 = -50; ry3 = -50; }
      if (checkHit(rx4, ry4)) { rx4 = 200; ry4 = -50; ispaljeno = 0; }

      if (rx4 < 1) {
        ispaljeno = 0;
        rx4 = 200;
      }

      // ── Game Over ──
      if (zivoti == 0) {
        go = 1;
        break;
      }

      u8g2.sendBuffer();
      delay(16);
    }

    // ── Game Over Screen ──
    if (zivoti == 0) {
      beep(200, 300, soundLevel);
      delay(300);
      beep(250, 200, soundLevel);
      delay(200);
      beep(300, 300, soundLevel);
      delay(300);
    }

    // ── Update High Score ──
    if (bodovi > highscore) {
      highscore = bodovi;
      EEPROM.begin(EEPROM_SIZE);
      EEPROM.put(GAME_INDEX * sizeof(uint16_t), highscore);
      EEPROM.commit();
      EEPROM.end();
      beep(1200, 40, soundLevel);
      delay(60);
      beep(1500, 40, soundLevel);
      delay(60);
      beep(1800, 80, soundLevel);
    }

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tr);
    centreStr("GAME OVER!", 20);
    u8g2.setFont(u8g2_font_6x10_tr);
    char buf[20];
    snprintf(buf, sizeof(buf), "Score: %d", bodovi);
    centreStr(buf, 38);
    char hs2[20];
    snprintf(hs2, sizeof(hs2), "Best: %u", highscore);
    centreStr(hs2, 50);
    u8g2.setFont(u8g2_font_5x7_tr);
    centreStr("ENTER=Replay  MENU=Quit", 62);
    u8g2.sendBuffer();

    bool waiting = true;
    while (waiting) {
      if (btnPressed(BTN_ENTER) || btnPressed(BTN_UP)) {
        waiting = false;
        beep(800, 30, soundLevel);
        break;
      }
      if (btnPressed(BTN_MENU)) {
        playMenuButtonSound();
        gameOverScreen(bodovi, GAME_INDEX, false);
        return;
      }
      delay(50);
    }
  }
}



void game_memory_match() {
  const int GRID_SIZE = 4;
  const int TOTAL_CARDS = GRID_SIZE * GRID_SIZE;
  const int PAIRS = TOTAL_CARDS / 2;
  
  int cards[GRID_SIZE][GRID_SIZE];
  bool revealed[GRID_SIZE][GRID_SIZE];
  int selectedX = -1, selectedY = -1;
  int firstX = -1, firstY = -1;
  int firstValue = -1;
  int pairsFound = 0;
  int attempts = 0;
  bool waiting = false;
  uint32_t waitStart = 0;
  bool gameOver = false;
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("MEMORY MATCH", 20);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Find all pairs!", 38);
  centreStr("Use D-pad to move", 50);
  centreStr("ENTER to flip", 60);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();
  
  int values[PAIRS];
  for (int i = 0; i < PAIRS; i++) {
    values[i] = i + 1;
  }
  
  int shuffled[TOTAL_CARDS];
  for (int i = 0; i < TOTAL_CARDS; i++) {
    shuffled[i] = values[i % PAIRS];
  }
  
  for (int i = TOTAL_CARDS - 1; i > 0; i--) {
    int j = random(0, i + 1);
    int temp = shuffled[i];
    shuffled[i] = shuffled[j];
    shuffled[j] = temp;
  }
  
  int idx = 0;
  for (int r = 0; r < GRID_SIZE; r++) {
    for (int c = 0; c < GRID_SIZE; c++) {
      cards[r][c] = shuffled[idx++];
      revealed[r][c] = false;
    }
  }
  
  int cursorX = 0, cursorY = 0;
  
  while (!gameOver) {
    if (checkPause("MEMORY MATCH")) return;
    if (checkMenuAndReturn()) return;
    
    if (waiting) {
      if (millis() - waitStart > 1000) {
        revealed[firstX][firstY] = false;
        revealed[selectedX][selectedY] = false;
        firstX = -1; firstY = -1; firstValue = -1;
        selectedX = -1; selectedY = -1;
        waiting = false;
      }
    } else {
      if (btnPressed(BTN_UP) && cursorY > 0) { cursorY--; beep(600, 10, soundLevel); }
      if (btnPressed(BTN_DOWN) && cursorY < GRID_SIZE - 1) { cursorY++; beep(600, 10, soundLevel); }
      if (btnPressed(BTN_LEFT) && cursorX > 0) { cursorX--; beep(600, 10, soundLevel); }
      if (btnPressed(BTN_RIGHT) && cursorX < GRID_SIZE - 1) { cursorX++; beep(600, 10, soundLevel); }
      
      if (btnPressed(BTN_ENTER) && !revealed[cursorY][cursorX]) {
        revealed[cursorY][cursorX] = true;
        
        if (firstX == -1) {
          firstX = cursorX;
          firstY = cursorY;
          firstValue = cards[cursorY][cursorX];
        } else {
          selectedX = cursorX;
          selectedY = cursorY;
          int secondValue = cards[cursorY][cursorX];
          attempts++;
          
          if (firstValue == secondValue) {
            pairsFound++;
            beep(1200, 30, soundLevel);
            firstX = -1; firstY = -1; firstValue = -1;
            selectedX = -1; selectedY = -1;
            
            if (pairsFound == PAIRS) {
              gameOver = true;
              uint16_t score = max(0, 100 - attempts * 2 + pairsFound * 5);
              gameOverScreen(score, 20, true);
              return;
            }
          } else {
            beep(300, 80, soundLevel);
            waiting = true;
            waitStart = millis();
          }
        }
      }
    }
    
    u8g2.clearBuffer();
    
    int cellSize = 18;
    int startX = (SCREEN_W - GRID_SIZE * cellSize) / 2;
    int startY = (SCREEN_H - GRID_SIZE * cellSize) / 2 + 4;
    
    for (int r = 0; r < GRID_SIZE; r++) {
      for (int c = 0; c < GRID_SIZE; c++) {
        int x = startX + c * cellSize;
        int y = startY + r * cellSize;
        
        if (revealed[r][c]) {
          u8g2.drawBox(x, y, cellSize - 1, cellSize - 1);
          u8g2.setDrawColor(0);
          char val[3];
          snprintf(val, sizeof(val), "%d", cards[r][c]);
          u8g2.setFont(u8g2_font_ncenB10_tr);
          centreStr(val, y + cellSize/2 + 3);
          u8g2.setDrawColor(1);
        } else {
          u8g2.drawFrame(x, y, cellSize - 1, cellSize - 1);
          u8g2.setFont(u8g2_font_5x7_tr);
          centreStr("?", y + cellSize/2 + 3);
        }
        
        if (r == cursorY && c == cursorX && !waiting) {
          u8g2.drawFrame(x - 1, y - 1, cellSize + 1, cellSize + 1);
        }
      }
    }
    
    u8g2.setFont(u8g2_font_5x7_tr);
    char info[20];
    snprintf(info, sizeof(info), "Pairs: %d/%d", pairsFound, PAIRS);
    centreStr(info, SCREEN_H - 2);
    
    u8g2.sendBuffer();
    delay(20);
  }
}

// ============================================================
// NEW GAME: WHACK-A-MOLE
// ============================================================

void game_whack_a_mole() {
  const int HOLES = 9;
  const int COLS = 3;
  const int ROWS = 3;
  
  struct Hole {
    int x, y;
    bool hasMole;
    uint32_t moleAppearTime;
  };
  
  Hole holes[HOLES];
  int score = 0;
  int lives = 5;
  uint32_t lastMoleSpawn = 0;
  uint32_t gameStart = millis();
  int cursorX = 0, cursorY = 0;
  bool gameOver = false;
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("WHACK-A-MOLE", 20);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Hit moles with ENTER!", 38);
  centreStr("Don't let them escape!", 50);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();
  
  int startX = 15, startY = 15;
  int spacing = 30;
  for (int i = 0; i < HOLES; i++) {
    int row = i / COLS;
    int col = i % COLS;
    holes[i].x = startX + col * spacing;
    holes[i].y = startY + row * spacing;
    holes[i].hasMole = false;
    holes[i].moleAppearTime = 0;
  }
  
  while (!gameOver) {
    if (checkPause("WHACK-A-MOLE")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    
    if (now - lastMoleSpawn > (uint32_t)random(600, 1500)) {
      lastMoleSpawn = now;
      int emptyHoles = 0;
      for (int i = 0; i < HOLES; i++) {
        if (!holes[i].hasMole) emptyHoles++;
      }
      
      if (emptyHoles > 0 && random(0, 10) < 7) {
        int idx = random(0, HOLES);
        int attempts = 0;
        while (holes[idx].hasMole && attempts < 20) {
          idx = random(0, HOLES);
          attempts++;
        }
        if (!holes[idx].hasMole) {
          holes[idx].hasMole = true;
          holes[idx].moleAppearTime = now;
        }
      }
    }
    
    for (int i = 0; i < HOLES; i++) {
      if (holes[i].hasMole && now - holes[i].moleAppearTime > 2000) {
        holes[i].hasMole = false;
        lives--;
        beep(300, 100, soundLevel);
        if (lives <= 0) {
          gameOver = true;
          gameOverScreen(score, 21, false);
          return;
        }
      }
    }
    
    if (btnPressed(BTN_UP) && cursorY > 0) { cursorY--; beep(600, 10, soundLevel); }
    if (btnPressed(BTN_DOWN) && cursorY < ROWS - 1) { cursorY++; beep(600, 10, soundLevel); }
    if (btnPressed(BTN_LEFT) && cursorX > 0) { cursorX--; beep(600, 10, soundLevel); }
    if (btnPressed(BTN_RIGHT) && cursorX < COLS - 1) { cursorX++; beep(600, 10, soundLevel); }
    
    if (btnPressed(BTN_ENTER)) {
      int idx = cursorY * COLS + cursorX;
      if (holes[idx].hasMole) {
        holes[idx].hasMole = false;
        score += 10;
        beep(1000, 30, soundLevel);
        beep(1200, 20, soundLevel);
      } else {
        beep(200, 50, soundLevel);
      }
    }
    
    u8g2.clearBuffer();
    
    for (int i = 0; i < HOLES; i++) {
      int row = i / COLS;
      int col = i % COLS;
      int x = holes[i].x;
      int y = holes[i].y;
      
      u8g2.drawCircle(x + 10, y + 10, 10);
      u8g2.setDrawColor(0);
      u8g2.drawCircle(x + 10, y + 10, 8);
      u8g2.setDrawColor(1);
      
      if (holes[i].hasMole) {
        u8g2.drawCircle(x + 10, y + 8, 6);
        u8g2.setDrawColor(0);
        u8g2.drawPixel(x + 8, y + 7);
        u8g2.drawPixel(x + 12, y + 7);
        u8g2.drawPixel(x + 10, y + 9);
        u8g2.setDrawColor(1);
      }
      
      if (row == cursorY && col == cursorX) {
        u8g2.drawFrame(x - 2, y - 2, 24, 24);
      }
    }
    
    u8g2.setFont(u8g2_font_5x7_tr);
    char info[20];
    snprintf(info, sizeof(info), "Score: %d", score);
    u8g2.drawStr(2, 8, info);
    snprintf(info, sizeof(info), "Lives: %d", lives);
    u8g2.drawStr(SCREEN_W - 40, 8, info);
    
    u8g2.sendBuffer();
    delay(20);
  }
}

// ============================================================
// NEW GAME: LUNAR LANDER
// ============================================================

void game_lunar_lander() {
  float x = 64, y = 10;
  float vx = 0, vy = 0;
  float fuel = 100;
  bool landed = false;
  bool crashed = false;
  uint32_t startTime = millis();
  float thrust = 0;
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("LUNAR LANDER", 20);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Land softly on the moon!", 38);
  centreStr("UP=Thrust  L/R=Move", 52);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();
  
  while (!landed && !crashed) {
    if (checkPause("LUNAR LANDER")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    float dt = 0.05;
    
    thrust = 0;
    if (btnHeld(BTN_UP) && fuel > 0) {
      thrust = 0.3;
      fuel -= 0.2;
      if (fuel < 0) fuel = 0;
    }
    if (btnHeld(BTN_LEFT)) vx -= 0.02;
    if (btnHeld(BTN_RIGHT)) vx += 0.02;
    
    vy += 0.01;
    vy -= thrust;
    x += vx;
    y += vy;
    
    if (x < 2) { x = 2; vx = 0; }
    if (x > SCREEN_W - 8) { x = SCREEN_W - 8; vx = 0; }
    
    if (y >= SCREEN_H - 10) {
      y = SCREEN_H - 10;
      if (abs(vy) < 0.3 && abs(vx) < 0.1) {
        landed = true;
        beep(1200, 100, soundLevel);
        delay(100);
        beep(1500, 100, soundLevel);
      } else {
        crashed = true;
        beep(200, 300, soundLevel);
      }
    }
    
    u8g2.clearBuffer();
    
    u8g2.drawHLine(0, SCREEN_H - 8, SCREEN_W);
    for (int i = 0; i < SCREEN_W; i += 5) {
      int h = random(0, 4);
      u8g2.drawVLine(i, SCREEN_H - 8 - h, h);
    }
    
    int lx = (int)x, ly = (int)y;
    u8g2.drawTriangle(lx + 4, ly, lx, ly + 8, lx + 8, ly + 8);
    u8g2.drawLine(lx + 2, ly + 4, lx + 6, ly + 4);
    
    if (thrust > 0) {
      u8g2.drawLine(lx + 2, ly + 8, lx + 1, ly + 13);
      u8g2.drawLine(lx + 4, ly + 8, lx + 4, ly + 15);
      u8g2.drawLine(lx + 6, ly + 8, lx + 7, ly + 13);
    }
    
    u8g2.setFont(u8g2_font_5x7_tr);
    char info[20];
    snprintf(info, sizeof(info), "Fuel: %.0f", fuel);
    u8g2.drawStr(2, 8, info);
    snprintf(info, sizeof(info), "V: %.1f", vy);
    u8g2.drawStr(2, 16, info);
    
    u8g2.sendBuffer();
    delay(30);
  }
  
  if (landed) {
    uint32_t time = (millis() - startTime) / 1000;
    uint16_t score = 1000 - time * 2 + (int)(fuel * 5);
    if (score < 0) score = 0;
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB18_tr);
    centreStr("LANDED!", 28);
    u8g2.setFont(u8g2_font_6x10_tr);
    char buf[20];
    snprintf(buf, sizeof(buf), "Score: %d", score);
    centreStr(buf, 44);
    u8g2.sendBuffer();
    delay(2000);
    gameOverScreen(score, 22, true);
  } else {
    gameOverScreen(0, 22, false);
  }
}

// ============================================================
// NEW GAME: COLOR MATCHING
// ============================================================

void game_color_matching() {
  const char* colors[] = {"RED", "GREEN", "BLUE", "YELLOW", "WHITE"};
  int targetColor = random(0, 5);
  int selectedColor = 0;
  int score = 0;
  int lives = 5;
  bool gameOver = false;
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("COLOR MATCH", 20);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Match the color!", 38);
  centreStr("UP/DOWN to select", 50);
  centreStr("ENTER to confirm", 60);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();
  
  while (!gameOver) {
    if (checkPause("COLOR MATCH")) return;
    if (checkMenuAndReturn()) return;
    
    if (btnPressed(BTN_UP)) {
      selectedColor = (selectedColor + 4) % 5;
      beep(600, 10, soundLevel);
    }
    if (btnPressed(BTN_DOWN)) {
      selectedColor = (selectedColor + 1) % 5;
      beep(600, 10, soundLevel);
    }
    
    if (btnPressed(BTN_ENTER)) {
      if (selectedColor == targetColor) {
        score += 10;
        beep(1200, 30, soundLevel);
        beep(1500, 20, soundLevel);
        targetColor = random(0, 5);
      } else {
        lives--;
        beep(300, 100, soundLevel);
        if (lives <= 0) {
          gameOver = true;
          gameOverScreen(score, 23, false);
          return;
        }
      }
    }
    
    u8g2.clearBuffer();
    
    u8g2.setFont(u8g2_font_ncenB10_tr);
    centreStr("MATCH THIS COLOR", 10);
    
    u8g2.drawBox(44, 18, 40, 16);
    u8g2.setDrawColor(0);
    u8g2.drawFrame(44, 18, 40, 16);
    u8g2.setDrawColor(1);
    
    u8g2.setFont(u8g2_font_6x10_tr);
    char selStr[20];
    snprintf(selStr, sizeof(selStr), "Selected: %s", colors[selectedColor]);
    centreStr(selStr, 42);
    
    u8g2.setFont(u8g2_font_5x7_tr);
    for (int i = 0; i < 5; i++) {
      int y = 48 + i * 3;
      if (i == selectedColor) {
        u8g2.drawStr(2, y + 5, ">");
      }
      u8g2.drawStr(8, y + 5, colors[i]);
    }
    
    char info[20];
    snprintf(info, sizeof(info), "Score: %d", score);
    u8g2.drawStr(SCREEN_W - 40, 10, info);
    snprintf(info, sizeof(info), "Lives: %d", lives);
    u8g2.drawStr(SCREEN_W - 40, 18, info);
    
    u8g2.sendBuffer();
    delay(50);
  }
}

// ============================================================
// NEW GAME: NINJA SPIKE
// ============================================================

void game_ninja_spike() {
  const int SPIKE_W = 4;
  const int SPIKE_H = 8;
  const int PLAYER_W = 6;
  const int PLAYER_H = 8;
  
  float playerX = 64 - PLAYER_W/2;
  float playerY = 56;
  float spikeX = 110;
  float spikeY = 30;
  float spikeSpeed = 2.5;
  int score = 0;
  bool gameOver = false;
  uint32_t lastFrame = millis();
  bool playerJumping = false;
  float jumpVel = 0;
  float gravity = 0.2;
  bool spikeDirection = true;
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("NINJA SPIKE", 20);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Dodge the spike!", 38);
  centreStr("UP to jump", 50);
  centreStr("Avoid at all costs!", 60);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();
  
  while (!gameOver) {
    if (checkPause("NINJA SPIKE")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    float dt = (now - lastFrame) / 16.0f;
    if (dt > 2.0f) dt = 2.0f;
    lastFrame = now;
    
    if (btnPressed(BTN_UP) && !playerJumping) {
      jumpVel = -4.5;
      playerJumping = true;
      beep(800, 15, soundLevel);
    }
    
    if (btnHeld(BTN_LEFT)) playerX -= 2.5 * dt;
    if (btnHeld(BTN_RIGHT)) playerX += 2.5 * dt;
    
    if (playerJumping) {
      playerY += jumpVel * dt;
      jumpVel += gravity * dt;
      if (playerY >= 56) {
        playerY = 56;
        playerJumping = false;
        jumpVel = 0;
      }
    }
    
    if (playerX < 0) playerX = 0;
    if (playerX > SCREEN_W - PLAYER_W) playerX = SCREEN_W - PLAYER_W;
    
    if (spikeDirection) {
      spikeY += 1.5 * dt;
      if (spikeY > 56 - SPIKE_H) spikeDirection = false;
    } else {
      spikeY -= 1.5 * dt;
      if (spikeY < 10) spikeDirection = true;
    }
    
    spikeX -= spikeSpeed * dt;
    if (spikeX < -10) {
      spikeX = SCREEN_W + 10;
      spikeY = random(10, 50);
      spikeSpeed = 2.5 + score * 0.01;
      score++;
    }
    
    if (playerX < spikeX + SPIKE_W && playerX + PLAYER_W > spikeX &&
        playerY < spikeY + SPIKE_H && playerY + PLAYER_H > spikeY) {
      gameOver = true;
      beep(200, 200, soundLevel);
      gameOverScreen(score, 24, false);
      return;
    }
    
    u8g2.clearBuffer();
    
    u8g2.drawBox((int)playerX, (int)playerY, PLAYER_W, PLAYER_H);
    u8g2.drawBox((int)playerX + 1, (int)playerY - 2, 4, 2);
    
    u8g2.drawTriangle((int)spikeX + SPIKE_W/2, (int)spikeY,
                      (int)spikeX, (int)spikeY + SPIKE_H,
                      (int)spikeX + SPIKE_W, (int)spikeY + SPIKE_H);
    
    u8g2.setFont(u8g2_font_5x7_tr);
    char info[20];
    snprintf(info, sizeof(info), "Score: %d", score);
    u8g2.drawStr(2, 8, info);
    snprintf(info, sizeof(info), "SPD: %.1f", spikeSpeed);
    u8g2.drawStr(SCREEN_W - 40, 8, info);
    
    u8g2.sendBuffer();
    delay(16);
  }
}

// ============================================================
// NEW GAME: SPERM RACE
// ============================================================
void game_sperm_race() {
  const int ENEMY_COUNT = 4;
  const int WASTE_COUNT = 8;
  const int PLAYER_R = 3;
  const int HEART_W = 8;
  const int HEART_H = 8;
  const int ENEMY_RADIUS = 3;
  const int TOP_BAR_H = 11;

  float moveSpeed = 2.2;
  float gameSpeed = 1.3;

  float playerX = 12, playerY = (TOP_BAR_H + SCREEN_H) / 2.0;
  unsigned long score = 0;
  static unsigned long highScore = 0;

  int lives = 3;
  bool dead = false;

  bool isBlinking = false;
  bool isHealing = false;
  int blinkCount = 0;
  unsigned long blinkTimer = 0;

  unsigned long lastScoreUpdate = 0;
  uint32_t lastFrame = millis();
  float tailPhase = 0;

  struct Enemy { float x, y; };
  Enemy enemies[ENEMY_COUNT];

  struct Particle { float x, y; float speed; };
  Particle waste[WASTE_COUNT];

  bool heartActive = false;
  float heartX = 0, heartY = 0;
  unsigned long nextHeartSpawn = millis() + 7000;

  auto getSafeY = [&](int index) {
    float ny;
    bool safe;
    int attempts = 0;
    do {
      safe = true;
      ny = random(TOP_BAR_H + 2, SCREEN_H - 6);
      attempts++;
      for (int i = 0; i < ENEMY_COUNT; i++) {
        if (i != index && fabs(ny - enemies[i].y) < 18) { safe = false; break; }
      }
    } while (!safe && attempts < 15);
    return ny;
  };

  auto resetGame = [&]() {
    score = 0;
    lives = 3;
    gameSpeed = 1.3;
    playerX = 12;
    playerY = (TOP_BAR_H + SCREEN_H) / 2.0;
    dead = false;
    isBlinking = false;
    heartActive = false;
    nextHeartSpawn = millis() + 7000;

    for (int i = 0; i < ENEMY_COUNT; i++) {
      enemies[i].x = SCREEN_W + i * 25 + 10;
      enemies[i].y = getSafeY(i);
    }
    for (int i = 0; i < WASTE_COUNT; i++) {
      waste[i].x = random(0, SCREEN_W);
      waste[i].y = random(TOP_BAR_H + 2, SCREEN_H - 2);
      waste[i].speed = 0.4 + random(0, 8) * 0.1;
    }
  };

  resetGame();

  // ---- Title Screen ----
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("SPERM RACE", 18);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Dodge the red eggs", 32);
  centreStr("Grab hearts for +life", 44);
  u8g2.sendBuffer();
  delay(1800);
  waitRelease();

  while (!dead) {
    if (checkPause("SPERM RACE")) return;
    if (checkMenuAndReturn()) return;

    uint32_t now = millis();
    float dt = (now - lastFrame) / 16.0f;
    if (dt > 2.0f) dt = 2.0f;
    lastFrame = now;

    // Score increase + gradual speed up
    if (now - lastScoreUpdate > 250) {
      score++;
      lastScoreUpdate = now;
      if (gameSpeed < 2.5) gameSpeed += 0.003;
    }

    // ---- Movement ----
    if (btnHeld(BTN_UP))    playerY -= moveSpeed * dt;
    if (btnHeld(BTN_DOWN))  playerY += moveSpeed * dt;
    if (btnHeld(BTN_LEFT))  playerX -= moveSpeed * dt;
    if (btnHeld(BTN_RIGHT)) playerX += moveSpeed * dt;

    if (playerX < PLAYER_R) playerX = PLAYER_R;
    if (playerX > SCREEN_W - PLAYER_R) playerX = SCREEN_W - PLAYER_R;
    if (playerY < TOP_BAR_H + 2) playerY = TOP_BAR_H + 2;
    if (playerY > SCREEN_H - 2) playerY = SCREEN_H - 2;

    // ---- Background particles (NO BLINKING, always visible) ----
    for (int i = 0; i < WASTE_COUNT; i++) {
      waste[i].x -= waste[i].speed * dt;
      if (waste[i].x < 0) {
        waste[i].x = SCREEN_W;
        waste[i].y = random(TOP_BAR_H + 2, SCREEN_H - 2);
        waste[i].speed = 0.4 + random(0, 8) * 0.1;
      }
    }

    // ---- Enemies ----
    bool hit = false;
    for (int i = 0; i < ENEMY_COUNT; i++) {
      enemies[i].x -= gameSpeed * dt;
      if (enemies[i].x < -6) {
        enemies[i].x = SCREEN_W + 6 + random(0, 30);
        enemies[i].y = getSafeY(i);
      }

      float dx = playerX - enemies[i].x;
      float dy = playerY - enemies[i].y;
      if (!isBlinking && dx * dx + dy * dy < 36) {
        hit = true;
        enemies[i].x = SCREEN_W + 6 + random(0, 30);
        enemies[i].y = getSafeY(i);
      }
    }

    if (hit) {
      lives--;
      beep(200, 150, soundLevel);
      if (lives <= 0) {
        dead = true;
        if (score > highScore) highScore = score;
        gameOverScreen(score, 25, false);
        return;
      }
      isBlinking = true;
      isHealing = false;
      blinkCount = 0;
      blinkTimer = now;
    }

    // ---- Heart spawn every 7 seconds ----
    if (!heartActive && now >= nextHeartSpawn) {
      heartActive = true;
      heartX = SCREEN_W + 6;
      heartY = random(TOP_BAR_H + 2, SCREEN_H - HEART_H - 2);
    }

    if (heartActive) {
      heartX -= (gameSpeed + 0.3) * dt;

      if (heartX < -HEART_W) {
        heartActive = false;
        nextHeartSpawn = now + 7000;
      } else {
        float hdx = playerX - heartX;
        float hdy = playerY - heartY;
        if (!isBlinking && hdx * hdx + hdy * hdy < 49) {
          if (lives < 5) lives++;
          heartActive = false;
          nextHeartSpawn = now + 7000;
          beep(1200, 40, soundLevel);
          delay(60);
          beep(1600, 60, soundLevel);
          isBlinking = true;
          isHealing = true;
          blinkCount = 0;
          blinkTimer = now;
        }
      }
    }

    // ---- Blink logic ----
    if (isBlinking) {
      if (now - blinkTimer > 80) {
        blinkTimer = now;
        blinkCount++;
      }
      if (blinkCount >= 6) isBlinking = false;
    }

    // ==========================================
    // TAIL ANIMATION - FASTER & BIGGER CURVE
    // ==========================================
    tailPhase += 1.4;

    // ==========================================
    // DRAWING
    // ==========================================
    u8g2.clearBuffer();

    // ---- Background particles (NO BLINKING, always drawn) ----
    for (int i = 0; i < WASTE_COUNT; i++) {
      u8g2.drawPixel((int)waste[i].x, (int)waste[i].y);
    }

    // ---- Enemies - fully coloured (filled discs) ----
    for (int i = 0; i < ENEMY_COUNT; i++) {
      u8g2.drawDisc((int)enemies[i].x, (int)enemies[i].y, ENEMY_RADIUS);
    }

    // Heart pickup
    if (heartActive) {
      u8g2.drawXBMP((int)heartX, (int)heartY, HEART_W, HEART_H, heart_bmp);
    }

    // ---- Player (sperm) with tail ----
    bool drawPlayer = true;
    if (isBlinking && !isHealing && (blinkCount % 2 == 0)) drawPlayer = false;

    if (drawPlayer) {
      u8g2.drawDisc((int)playerX, (int)playerY, PLAYER_R);

      // Tail: length = 7, faster & bigger sine curve
      for (int i = 0; i < 7; i++) {
        int tx = (int)playerX - PLAYER_R - i * 2;
        int ty = (int)playerY + (int)(sin(tailPhase + i * 0.5) * (i * 0.6));
        u8g2.drawPixel(tx, ty);
      }
    }

    u8g2.setDrawColor(1);  // White fill
    u8g2.drawBox(0, 0, SCREEN_W, TOP_BAR_H);
    u8g2.setDrawColor(0);  // Black text and hearts
    u8g2.setFont(u8g2_font_5x7_tr);

    char info[24];
    snprintf(info, sizeof(info), "HI:%lu", highScore);
    u8g2.drawStr(2, 8, info);
    snprintf(info, sizeof(info), "S:%lu", score);
    u8g2.drawStr(SCREEN_W - 30, 8, info);

    // Lives (hearts) inside the box
    for (int i = 0; i < lives; i++) {
      u8g2.drawXBMP(32 + i * (HEART_W + 2), 2, HEART_W, HEART_H, heart_bmp);
    }

    u8g2.setDrawColor(1);  // Reset to white for drawing
    u8g2.sendBuffer();
    delay(16);
  }
}

// ============================================================
// NEW GAME: FROGGER
// ============================================================

void game_frogger() {
  const int FROG_W = 6;
  const int FROG_H = 6;
  const int LANE_COUNT = 4;
  const int CAR_COUNT_PER_LANE = 3;

  // ðŸ”¥ à¦²à§‡à¦¨à¦—à§à¦²à§‹à¦° Y à¦ªà¦œà¦¿à¦¶à¦¨ (à¦—à¦¾à¦¡à¦¼à¦¿à¦° à¦²à§‡à¦¨à§‡à¦° à¦¸à¦¾à¦¥à§‡ à¦à¦•à§à¦¸à§à¦¯à¦¾à¦•à§à¦Ÿ à¦®à¦¿à¦² à¦¥à¦¾à¦•à¦¤à§‡ à¦¹à¦¬à§‡)
  const int laneY[LANE_COUNT] = {48, 36, 24, 12}; // à¦¨à¦¿à¦š à¦¥à§‡à¦•à§‡ à¦‰à¦ªà¦°à§‡à¦° à¦¦à¦¿à¦•à§‡ à¦•à§à¦°à¦®
  const int SAFE_BOTTOM_Y = 56; // à¦¶à§à¦°à§à¦° à¦¨à¦¿à¦°à¦¾à¦ªà¦¦ à¦œà¦¾à¦¯à¦¼à¦—à¦¾
  const int SAFE_TOP_Y = 2;     // à¦«à¦¿à¦¨à¦¿à¦¶ à¦²à¦¾à¦‡à¦¨à§‡à¦° à¦¨à¦¿à¦°à¦¾à¦ªà¦¦ à¦œà¦¾à¦¯à¦¼à¦—à¦¾

  // frogLane: 0 = à¦¨à¦¿à¦šà§‡à¦° à¦¨à¦¿à¦°à¦¾à¦ªà¦¦ à¦œà¦¾à¦¯à¦¼à¦—à¦¾, 1..LANE_COUNT = à¦°à¦¾à¦¸à§à¦¤à¦¾à¦° à¦²à§‡à¦¨, LANE_COUNT+1 = à¦‰à¦ªà¦°à§‡ à¦ªà§Œà¦à¦›à§‡ à¦—à§‡à¦›à§‡
  int frogLane = 0;

  auto laneToY = [&](int lane) -> int {
    if (lane <= 0) return SAFE_BOTTOM_Y;
    if (lane > LANE_COUNT) return SAFE_TOP_Y;
    return laneY[lane - 1];
  };

  float frogX = 64 - FROG_W / 2;
  int frogY = laneToY(frogLane);
  bool gameOver = false;
  int score = 0;
  int lives = 3;
  uint32_t lastFrame = millis();

  struct Car {
    float x, y;
    float speed;
    int width, height;
    bool active;
  };

  Car cars[LANE_COUNT][CAR_COUNT_PER_LANE];

  for (int lane = 0; lane < LANE_COUNT; lane++) {
    for (int c = 0; c < CAR_COUNT_PER_LANE; c++) {
      cars[lane][c].y = 12 + lane * 12;
      cars[lane][c].x = random(0, SCREEN_W);
      cars[lane][c].speed = (lane % 2 == 0) ? (0.45 + random(0, 5) * 0.1) : -(0.45 + random(0, 5) * 0.1);
      cars[lane][c].width = random(6, 12);
      cars[lane][c].height = 6;
      cars[lane][c].active = true;
    }
  }

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("FROGGER", 20);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Cross the road!", 38);
  centreStr("D-pad to move", 50);
  centreStr("Reach the top!", 60);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (!gameOver) {
    if (checkPause("FROGGER")) return;
    if (checkMenuAndReturn()) return;

    uint32_t now = millis();
    float dt = (now - lastFrame) / 16.0f;
    if (dt > 2.0f) dt = 2.0f;
    lastFrame = now;

    // ==========================================
    // ðŸ”¥ à¦®à§à¦­à¦®à§‡à¦¨à§à¦Ÿ - à¦à¦–à¦¨ lane-index à¦­à¦¿à¦¤à§à¦¤à¦¿à¦•, à¦¤à¦¾à¦‡ à¦¸à¦¬à¦¸à¦®à¦¯à¦¼ à¦²à§‡à¦¨à§‡à¦° à¦¸à¦¾à¦¥à§‡ exact align à¦¹à¦¬à§‡
    // ==========================================
    if (btnPressed(BTN_UP) && frogLane <= LANE_COUNT) {
      frogLane++;
      frogY = laneToY(frogLane);
      beep(600, 10, soundLevel);
    }
    if (btnPressed(BTN_DOWN) && frogLane > 0) {
      frogLane--;
      frogY = laneToY(frogLane);
      beep(600, 10, soundLevel);
    }
    if (btnPressed(BTN_LEFT)) {
      frogX -= 10;
      if (frogX < 0) frogX = 0;
      beep(600, 10, soundLevel);
    }
    if (btnPressed(BTN_RIGHT)) {
      frogX += 10;
      if (frogX > SCREEN_W - FROG_W) frogX = SCREEN_W - FROG_W;
      beep(600, 10, soundLevel);
    }

    // ==========================================
    // ðŸ”¥ à¦«à¦¿à¦¨à¦¿à¦¶ à¦²à¦¾à¦‡à¦¨à§‡ à¦ªà§Œà¦à¦›à¦¾à¦²à§‡
    // ==========================================
    if (frogLane > LANE_COUNT) {
      score += 50;
      frogLane = 0;
      frogY = laneToY(frogLane);
      frogX = 64 - FROG_W / 2;
      beep(1200, 50, soundLevel);
      delay(200);
      beep(1500, 50, soundLevel);
    }

    // ==========================================
    // ðŸ”¥ à¦—à¦¾à¦¡à¦¼à¦¿ à¦†à¦ªà¦¡à§‡à¦Ÿ + à¦•à¦²à¦¿à¦¶à¦¨ à¦šà§‡à¦• (à¦à¦•à¦‡ à¦«à§à¦°à§‡à¦®à§‡ à¦à¦•à¦¾à¦§à¦¿à¦•à¦¬à¦¾à¦° à¦²à¦¾à¦‡à¦« à¦•à¦¾à¦Ÿà¦¾ à¦¬à¦¨à§à¦§ à¦•à¦°à¦¤à§‡ break à¦¬à§à¦¯à¦¬à¦¹à¦¾à¦° à¦•à¦°à¦¾ à¦¹à¦¯à¦¼à§‡à¦›à§‡)
    // ==========================================
    bool hit = false;
    for (int lane = 0; lane < LANE_COUNT && !hit; lane++) {
      for (int c = 0; c < CAR_COUNT_PER_LANE; c++) {
        cars[lane][c].x += cars[lane][c].speed * dt;

        if (cars[lane][c].x > SCREEN_W + 10) {
          cars[lane][c].x = -20;
          cars[lane][c].speed = (lane % 2 == 0) ? (1.0 + random(0, 10) * 0.1) : -(1.0 + random(0, 10) * 0.1);
        }
        if (cars[lane][c].x < -20) {
          cars[lane][c].x = SCREEN_W + 10;
          cars[lane][c].speed = (lane % 2 == 0) ? (1.0 + random(0, 10) * 0.1) : -(1.0 + random(0, 10) * 0.1);
        }

        if (frogY >= cars[lane][c].y - 3 && frogY <= cars[lane][c].y + 3) {
          if (frogX < cars[lane][c].x + cars[lane][c].width &&
              frogX + FROG_W > cars[lane][c].x) {
            hit = true;
            lives--;
            beep(200, 150, soundLevel);
            frogLane = 0;
            frogY = laneToY(frogLane);
            frogX = 64 - FROG_W / 2;

            if (lives <= 0) {
              gameOver = true;
              gameOverScreen(score, 26, false);
              return;
            }
            delay(500);
            break; // à¦à¦‡ à¦²à§‡à¦¨à§‡à¦° à¦¬à¦¾à¦•à¦¿ à¦—à¦¾à¦¡à¦¼à¦¿ à¦šà§‡à¦• à¦•à¦°à¦¾à¦° à¦¦à¦°à¦•à¦¾à¦° à¦¨à§‡à¦‡
          }
        }
      }
    }

    u8g2.clearBuffer();

    for (int i = 0; i < LANE_COUNT; i++) {
      int y = 12 + i * 12;
      u8g2.drawHLine(0, y, SCREEN_W);
      u8g2.drawHLine(0, y + 6, SCREEN_W);
    }

    for (int lane = 0; lane < LANE_COUNT; lane++) {
      for (int c = 0; c < CAR_COUNT_PER_LANE; c++) {
        u8g2.drawBox((int)cars[lane][c].x, (int)cars[lane][c].y,
                     cars[lane][c].width, cars[lane][c].height);
      }
    }

    u8g2.drawBox((int)frogX, (int)frogY, FROG_W, FROG_H);
    u8g2.drawCircle((int)frogX + FROG_W / 2, (int)frogY - 1, 3);

    u8g2.setFont(u8g2_font_5x7_tr);
    char info[20];
    snprintf(info, sizeof(info), "Score: %d", score);
    u8g2.drawStr(2, 8, info);
    snprintf(info, sizeof(info), "Lives: %d", lives);
    u8g2.drawStr(SCREEN_W - 30, 8, info);

    u8g2.sendBuffer();
    delay(16);
  }
}


void game_tictactoe() {
  const int GAME_INDEX = 19;

  // ── Game Board ──
  char board[3][3] = {
    {' ', ' ', ' '},
    {' ', ' ', ' '},
    {' ', ' ', ' '}
  };

  int cursorX = 0, cursorY = 0;
  bool playerTurn = true;
  bool gameOver = false;
  char winner = ' ';
  int moveCount = 0;
  int playerScore = 0;
  int cpuScore = 0;
  int drawScore = 0;
  unsigned long cpuMoveTime = 0;
  bool cpuThinking = false;
  
  // Win line positions
  int winLineStartX = 0, winLineStartY = 0;
  int winLineEndX = 0, winLineEndY = 0;
  bool showWinLine = false;
  unsigned long winLineTime = 0;
  bool showResultScreen = false;
  bool showDrawScreen = false;
  unsigned long drawScreenTime = 0;

  // ── Grid Settings ──
  const int startX = 25, startY = 2;
  const int cellSize = 20;
  const int endX = startX + cellSize * 3;  // 85
  const int endY = startY + cellSize * 3;  // 62

  // ── Helper Functions ──
  auto checkWinner = [&]() -> char {
    // Check rows
    for (int row = 0; row < 3; row++) {
      if (board[row][0] == board[row][1] && board[row][1] == board[row][2] && board[row][0] != ' ') {
        winLineStartX = startX;
        winLineStartY = startY + row * cellSize + cellSize / 2;
        winLineEndX = endX;
        winLineEndY = startY + row * cellSize + cellSize / 2;
        return board[row][0];
      }
    }
    // Check columns
    for (int col = 0; col < 3; col++) {
      if (board[0][col] == board[1][col] && board[1][col] == board[2][col] && board[0][col] != ' ') {
        winLineStartX = startX + col * cellSize + cellSize / 2;
        winLineStartY = startY;
        winLineEndX = startX + col * cellSize + cellSize / 2;
        winLineEndY = endY;
        return board[0][col];
      }
    }
    // Check diagonals
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != ' ') {
      winLineStartX = startX;
      winLineStartY = startY;
      winLineEndX = endX;
      winLineEndY = endY;
      return board[0][0];
    }
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != ' ') {
      winLineStartX = endX;
      winLineStartY = startY;
      winLineEndX = startX;
      winLineEndY = endY;
      return board[0][2];
    }
    return ' ';
  };

  auto drawThickLine = [&](int x1, int y1, int x2, int y2, int thickness) {
    for (int i = -thickness/2; i <= thickness/2; i++) {
      u8g2.drawLine(x1 + i, y1, x2 + i, y2);
    }
  };

  auto isBoardFull = [&]() -> bool {
    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        if (board[row][col] == ' ') return false;
      }
    }
    return true;
  };

  auto resetBoard = [&]() {
    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        board[row][col] = ' ';
      }
    }
    cursorX = 0;
    cursorY = 0;
    playerTurn = true;
    gameOver = false;
    winner = ' ';
    moveCount = 0;
    cpuThinking = false;
    showWinLine = false;
    showResultScreen = false;
    showDrawScreen = false;
  };

  // ── CPU Move (Smart AI) ──
  auto cpuMove = [&]() {
    // 1. Check if CPU can win
    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        if (board[row][col] == ' ') {
          board[row][col] = 'O';
          if (checkWinner() == 'O') {
            return;
          }
          board[row][col] = ' ';
        }
      }
    }

    // 2. Block player from winning
    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        if (board[row][col] == ' ') {
          board[row][col] = 'X';
          if (checkWinner() == 'X') {
            board[row][col] = 'O';
            return;
          }
          board[row][col] = ' ';
        }
      }
    }

    // 3. Take center if available
    if (board[1][1] == ' ') {
      board[1][1] = 'O';
      return;
    }

    // 4. Take corners
    int corners[4][2] = {{0,0}, {0,2}, {2,0}, {2,2}};
    for (int i = 0; i < 4; i++) {
      if (board[corners[i][0]][corners[i][1]] == ' ') {
        board[corners[i][0]][corners[i][1]] = 'O';
        return;
      }
    }

    // 5. Take any empty space
    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        if (board[row][col] == ' ') {
          board[row][col] = 'O';
          return;
        }
      }
    }
  };

  // ── Show Start Screen ──
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("TIC-TAC-TOE", 20);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("You = X  CPU = O", 36);
  centreStr("D-pad to move", 48);
  centreStr("ENTER to place", 60);
  u8g2.sendBuffer();

  waitRelease();
  while (!btnPressed(BTN_UP) && !btnPressed(BTN_DOWN) &&
         !btnPressed(BTN_LEFT) && !btnPressed(BTN_RIGHT) &&
         !btnPressed(BTN_ENTER)) {
    delay(10);
  }
  waitRelease();

  // ── Game Loop ──
  while (true) {
    if (checkPause("TIC-TAC-TOE")) return;
    if (checkMenuAndReturn()) return;

    resetBoard();

    while (!gameOver) {
      if (checkPause("TIC-TAC-TOE")) return;
      if (checkMenuAndReturn()) return;

      // ── Player Move ──
      if (playerTurn && !cpuThinking) {
        if (btnPressed(BTN_UP) && cursorY > 0) {
          cursorY--;
          beep(600, 10, soundLevel);
        }
        if (btnPressed(BTN_DOWN) && cursorY < 2) {
          cursorY++;
          beep(600, 10, soundLevel);
        }
        if (btnPressed(BTN_LEFT) && cursorX > 0) {
          cursorX--;
          beep(600, 10, soundLevel);
        }
        if (btnPressed(BTN_RIGHT) && cursorX < 2) {
          cursorX++;
          beep(600, 10, soundLevel);
        }

        if (btnPressed(BTN_ENTER)) {
          if (board[cursorY][cursorX] == ' ') {
            board[cursorY][cursorX] = 'X';
            moveCount++;
            beep(800, 15, soundLevel);
            playerTurn = false;
            cpuThinking = true;
            cpuMoveTime = millis();

            winner = checkWinner();
            if (winner != ' ') {
              gameOver = true;
              showWinLine = true;
              winLineTime = millis();
              if (winner == 'X') {
                playerScore++;
                beep(1200, 40, soundLevel);
                delay(60);
                beep(1500, 40, soundLevel);
                delay(60);
                beep(1800, 80, soundLevel);
              }
            } else if (isBoardFull()) {
              gameOver = true;
              showDrawScreen = true;
              drawScreenTime = millis();
              drawScore++;
              beep(500, 100, soundLevel);
            }
          }
        }
      }

      // ── CPU Move with Delay ──
      if (!playerTurn && !gameOver && cpuThinking) {
        if (millis() - cpuMoveTime > 600) {
          cpuMove();
          moveCount++;
          beep(500, 10, soundLevel);
          playerTurn = true;
          cpuThinking = false;

          winner = checkWinner();
          if (winner != ' ') {
            gameOver = true;
            showWinLine = true;
            winLineTime = millis();
            if (winner == 'O') {
              cpuScore++;
              beep(200, 300, soundLevel);
            }
          } else if (isBoardFull()) {
            gameOver = true;
            showDrawScreen = true;
            drawScreenTime = millis();
            drawScore++;
            beep(500, 100, soundLevel);
          }
        }
      }

      // ── Draw ──
      u8g2.clearBuffer();

      // Draw board
      int startX_draw = startX, startY_draw = startY;
      int cellSize_draw = cellSize;

      // Draw grid lines
      u8g2.drawLine(startX_draw + cellSize_draw, startY_draw, startX_draw + cellSize_draw, startY_draw + cellSize_draw * 3);
      u8g2.drawLine(startX_draw + cellSize_draw * 2, startY_draw, startX_draw + cellSize_draw * 2, startY_draw + cellSize_draw * 3);
      u8g2.drawLine(startX_draw, startY_draw + cellSize_draw, startX_draw + cellSize_draw * 3, startY_draw + cellSize_draw);
      u8g2.drawLine(startX_draw, startY_draw + cellSize_draw * 2, startX_draw + cellSize_draw * 3, startY_draw + cellSize_draw * 2);

      // Draw board border
      u8g2.drawFrame(startX_draw - 1, startY_draw - 1, cellSize_draw * 3 + 2, cellSize_draw * 3 + 2);

      // Draw X and O
      u8g2.setFont(u8g2_font_ncenB10_tr);
      for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
          int x = startX_draw + col * cellSize_draw + cellSize_draw / 2 - 5;
          int y = startY_draw + row * cellSize_draw + cellSize_draw / 2 + 4;

          if (board[row][col] == 'X') {
            u8g2.drawStr(x, y, "X");
          } else if (board[row][col] == 'O') {
            u8g2.drawStr(x, y, "O");
          }
        }
      }

      // ── Draw Win Line ──
      if (showWinLine && !showResultScreen) {
        drawThickLine(winLineStartX, winLineStartY, winLineEndX, winLineEndY, 3);
        u8g2.sendBuffer();
        delay(1000);
        showResultScreen = true;
      }

      // ── Draw "DRAW" on blank screen ──
      if (showDrawScreen && !showResultScreen) {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB24_tr);
        centreStr("DRAW!", 35);
        u8g2.sendBuffer();
        delay(1000);
        showResultScreen = true;
      }

      // ── Draw Cursor ──
      if (playerTurn && !gameOver && !cpuThinking && !showResultScreen) {
        if ((millis() / 300) % 2 == 0) {
          int cx = startX_draw + cursorX * cellSize_draw;
          int cy = startY_draw + cursorY * cellSize_draw;
          u8g2.drawFrame(cx, cy, cellSize_draw, cellSize_draw);
        }
      }

      // ── Result Screen ──
      if (showResultScreen) {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB18_tr);
        
        if (winner == 'X') {
          centreStr("YOU WIN!", 30);
          u8g2.setFont(u8g2_font_6x10_tr);
          centreStr("Congratulations!", 50);
        } else if (winner == 'O') {
          centreStr("CPU WINS!", 30);
          u8g2.setFont(u8g2_font_6x10_tr);
          centreStr("Better luck next time!", 50);
        } else {
          centreStr("DRAW!", 30);
          u8g2.setFont(u8g2_font_6x10_tr);
          centreStr("Good game!", 50);
        }
        
        u8g2.setFont(u8g2_font_5x7_tr);
        centreStr("PRESS ENTER TO CONTINUE", 62);
        u8g2.sendBuffer();

        while (showResultScreen) {
          if (btnPressed(BTN_ENTER) || btnPressed(BTN_UP)) {
            showResultScreen = false;
            gameOver = false;
            resetBoard();
            beep(800, 30, soundLevel);
            break;
          }
          if (btnPressed(BTN_MENU)) {
            playMenuButtonSound();
            gameOverScreen(playerScore, GAME_INDEX, false);
            return;
          }
          delay(50);
        }
        continue;
      }

      u8g2.sendBuffer();
      delay(16);
    }
  }
}



void game_frogger2() {
  const int FROG_W = 6;
  const int FROG_H = 6;
  const int ROAD_LANES = 2;
  const int RIVER_LANES = 2;
  const int CARS_PER_LANE = 2;
  const int LOGS_PER_LANE = 2;
  const int PAD_COUNT = 3;
 
  // 🔥 সবগুলো "রো" - নিচ থেকে উপরে - lane-index snapping (কোনো misalignment bug নেই)
  // 0=স্টার্ট, 1-2=রোড, 3=মিডিয়ান, 4-5=নদী, 6=গোল (lily pad সারি)
  const int TOTAL_ROWS = 7;
  const int laneY[TOTAL_ROWS] = {58, 49, 40, 31, 22, 13, 4};
  int frogRow = 0;
 
  const int padCenterX[PAD_COUNT] = {14, 60, 106};
  const int padHalfW = 12;
  bool padFilled[PAD_COUNT] = {false, false, false};
 
  float frogX = 64 - FROG_W / 2;
  int frogY = laneY[frogRow];
  int lives = 3;
  int score = 0;
  int level = 1;
  bool gameOver = false;
  uint32_t lastFrame = millis();
 
  struct Mover {
    float x, y;
    float speed;
    int width, height;
  };
 
  Mover cars[ROAD_LANES][CARS_PER_LANE];
  Mover logs[RIVER_LANES][LOGS_PER_LANE];
 
  auto spawnCars = [&]() {
    for (int lane = 0; lane < ROAD_LANES; lane++) {
      float baseSpeed = 0.9 + level * 0.15;
      for (int c = 0; c < CARS_PER_LANE; c++) {
        cars[lane][c].y = laneY[1 + lane];
        cars[lane][c].x = random(0, SCREEN_W) + c * (SCREEN_W / CARS_PER_LANE);
        cars[lane][c].speed = (lane % 2 == 0) ? (baseSpeed + random(0, 5) * 0.1)
                                                : -(baseSpeed + random(0, 5) * 0.1);
        cars[lane][c].width = random(6, 12);
        cars[lane][c].height = 6;
      }
    }
  };
 
  auto spawnLogs = [&]() {
    for (int lane = 0; lane < RIVER_LANES; lane++) {
      float baseSpeed = 0.6 + level * 0.1;
      for (int c = 0; c < LOGS_PER_LANE; c++) {
        logs[lane][c].y = laneY[4 + lane];
        logs[lane][c].x = random(0, SCREEN_W) + c * (SCREEN_W / LOGS_PER_LANE);
        logs[lane][c].speed = (lane % 2 == 0) ? (baseSpeed + random(0, 4) * 0.1)
                                                : -(baseSpeed + random(0, 4) * 0.1);
        logs[lane][c].width = random(20, 34);
        logs[lane][c].height = 6;
      }
    }
  };
 
  spawnCars();
  spawnLogs();
 
  auto resetFrog = [&]() {
    frogRow = 0;
    frogY = laneY[frogRow];
    frogX = 64 - FROG_W / 2;
  };
 
  // ---------------- টাইটেল স্ক্রিন ----------------
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("FROGGER 2", 18);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Cross road & river", 32);
  centreStr("Ride logs, avoid cars", 44);
  centreStr("Fill 3 lily pads!", 56);
  u8g2.sendBuffer();
  delay(1800);
  waitRelease();
 
  while (!gameOver) {
    if (checkPause("FROGGER 2")) return;
    if (checkMenuAndReturn()) return;
 
    uint32_t now = millis();
    float dt = (now - lastFrame) / 16.0f;
    if (dt > 2.0f) dt = 2.0f;
    lastFrame = now;
 
    // ==========================================
    // 🔥 মুভমেন্ট - lane-index ভিত্তিক, তাই সবসময় সঠিক রো-তে ল্যান্ড করবে
    // ==========================================
    if (btnPressed(BTN_UP) && frogRow < TOTAL_ROWS - 1) {
      frogRow++;
      frogY = laneY[frogRow];
      beep(700, 15, soundLevel);
    }
    if (btnPressed(BTN_DOWN) && frogRow > 0) {
      frogRow--;
      frogY = laneY[frogRow];
      beep(500, 15, soundLevel);
    }
    if (btnPressed(BTN_LEFT)) {
      frogX -= 10;
      if (frogX < 0) frogX = 0;
      beep(600, 10, soundLevel);
    }
    if (btnPressed(BTN_RIGHT)) {
      frogX += 10;
      if (frogX > SCREEN_W - FROG_W) frogX = SCREEN_W - FROG_W;
      beep(600, 10, soundLevel);
    }
 
    // ==========================================
    // 🔥 রোড লেন গাড়ি আপডেট + কলিশন
    // ==========================================
    bool died = false;
    for (int lane = 0; lane < ROAD_LANES && !died; lane++) {
      for (int c = 0; c < CARS_PER_LANE; c++) {
        cars[lane][c].x += cars[lane][c].speed * dt;
        if (cars[lane][c].x > SCREEN_W + 12) cars[lane][c].x = -14;
        if (cars[lane][c].x < -14) cars[lane][c].x = SCREEN_W + 12;
 
        if (frogRow == 1 + lane) {
          if (frogX < cars[lane][c].x + cars[lane][c].width &&
              frogX + FROG_W > cars[lane][c].x) {
            died = true;
            break;
          }
        }
      }
    }
 
    // ==========================================
    // 🔥 নদী লেন লগ আপডেট + রাইড / ডুবে যাওয়া চেক
    // ==========================================
    bool onLog = false;
    float rideSpeed = 0;
    for (int lane = 0; lane < RIVER_LANES; lane++) {
      for (int c = 0; c < LOGS_PER_LANE; c++) {
        logs[lane][c].x += logs[lane][c].speed * dt;
        if (logs[lane][c].x > SCREEN_W + 20) logs[lane][c].x = -logs[lane][c].width - 4;
        if (logs[lane][c].x < -logs[lane][c].width - 4) logs[lane][c].x = SCREEN_W + 20;
 
        if (frogRow == 4 + lane) {
          if (frogX + FROG_W > logs[lane][c].x &&
              frogX < logs[lane][c].x + logs[lane][c].width) {
            onLog = true;
            rideSpeed = logs[lane][c].speed;
          }
        }
      }
    }
 
    // নদীতে থেকে কোনো লগে না থাকলে ডুবে যাবে
    if ((frogRow == 4 || frogRow == 5) && !onLog) {
      died = true;
    }
 
    // লগে চড়ে থাকলে ফ্রগ লগের সাথে ভাসবে
    if (onLog) {
      frogX += rideSpeed * dt;
      if (frogX < 0 || frogX > SCREEN_W - FROG_W) {
        died = true; // পানিতে পড়ে গেল কিনারার বাইরে গিয়ে
      }
    }
 
    if (died) {
      lives--;
      beep(200, 150, soundLevel);
      resetFrog();
      if (lives <= 0) {
        gameOver = true;
        // ⚠️ gameNames[] এ "Frogger 2" যোগ করে সঠিক ইনডেক্স এখানে বসান
        gameOverScreen(score, 27, false);
        return;
      }
      delay(400);
    }
 
    // ==========================================
    // 🔥 গোল সারিতে পৌঁছালে - lily pad চেক
    // ==========================================
    if (frogRow == TOTAL_ROWS - 1) {
      int padHit = -1;
      for (int p = 0; p < PAD_COUNT; p++) {
        if (frogX + FROG_W / 2 > padCenterX[p] - padHalfW &&
            frogX + FROG_W / 2 < padCenterX[p] + padHalfW) {
          padHit = p;
          break;
        }
      }
 
      if (padHit == -1 || padFilled[padHit]) {
        // ফাঁকা জায়গায় বা আগে থেকে ভরা প্যাডে পড়লে
        lives--;
        beep(200, 150, soundLevel);
        resetFrog();
        if (lives <= 0) {
          gameOver = true;
          gameOverScreen(score, 27, false);
          return;
        }
      } else {
        padFilled[padHit] = true;
        score += 100;
        beep(1200, 40, soundLevel);
        delay(80);
        beep(1600, 60, soundLevel);
        resetFrog();
 
        bool allFilled = true;
        for (int p = 0; p < PAD_COUNT; p++) if (!padFilled[p]) allFilled = false;
 
        if (allFilled) {
          score += 300;
          level++;
          for (int p = 0; p < PAD_COUNT; p++) padFilled[p] = false;
          spawnCars();
          spawnLogs();
          beep(1500, 80, soundLevel);
          delay(100);
          beep(1900, 100, soundLevel);
          delay(100);
          beep(2200, 120, soundLevel);
        }
      }
    }
 
    // ==========================================
    // 🔥 ড্রয়িং
    // ==========================================
    u8g2.clearBuffer();
 
    // নদী (dashed lines দিয়ে পানি বোঝানো)
    for (int i = 0; i < RIVER_LANES; i++) {
      int y = laneY[4 + i] - 4;
      for (int x = 0; x < SCREEN_W; x += 6) {
        u8g2.drawPixel(x, y);
        u8g2.drawPixel(x + 2, y + 8);
      }
    }
 
    // রাস্তার লেন বর্ডার
    for (int i = 0; i < ROAD_LANES; i++) {
      int y = laneY[1 + i] - 4;
      u8g2.drawHLine(0, y, SCREEN_W);
      u8g2.drawHLine(0, y + 9, SCREEN_W);
    }
 
    // মিডিয়ান স্ট্রিপ
    u8g2.drawHLine(0, laneY[3] - 4, SCREEN_W);
 
    // লিলি প্যাড

 
    // গাড়ি
    for (int lane = 0; lane < ROAD_LANES; lane++) {
      for (int c = 0; c < CARS_PER_LANE; c++) {
        u8g2.drawBox((int)cars[lane][c].x, (int)cars[lane][c].y,
                     cars[lane][c].width, cars[lane][c].height);
      }
    }
 
    // লগ
    for (int lane = 0; lane < RIVER_LANES; lane++) {
      for (int c = 0; c < LOGS_PER_LANE; c++) {
        u8g2.drawRFrame((int)logs[lane][c].x, (int)logs[lane][c].y,
                        logs[lane][c].width, logs[lane][c].height, 2);
      }
    }
 
    // ব্যাঙ
    u8g2.drawBox((int)frogX, frogY, FROG_W, FROG_H);
    u8g2.drawCircle((int)frogX + FROG_W / 2, frogY - 1, 3);
 
    // HUD
    u8g2.setFont(u8g2_font_5x7_tr);
    char info[24];
    snprintf(info, sizeof(info), "S:%d", score);
    u8g2.drawStr(1, 64, info);
    snprintf(info, sizeof(info), "L:%d", lives);
    u8g2.drawStr(SCREEN_W - 22, 64, info);
 
    u8g2.sendBuffer();
    delay(16);
  }
}


// ============================================================
// SETUP & LOOP
// ============================================================

void setup() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_MENU, INPUT_PULLUP);
  pinMode(BTN_PAUSE, INPUT_PULLUP);
  pinMode(BTN_ENTER, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setClock(400000);

  randomSeed(esp_random());
  loadHighScores();

  u8g2.begin();
  uint8_t contrast = map(brightnessLevel, 1, 7, 30, 255);
  u8g2.setContrast(contrast);
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.setDrawColor(1);
  u8g2.setBitmapMode(0);
  showSplash();
}

void loop() {
  showMainGridMenu();
} 
