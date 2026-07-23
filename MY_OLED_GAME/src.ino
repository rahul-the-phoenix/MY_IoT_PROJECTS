// ============================================================
// COMPLETE GAME CONSOLE FOR ESP32-C3
// 14 Games Total with Favorites, Settings, and Button Config
// ============================================================

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <EEPROM.h>

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
#define GAME_COUNT 14
#define MAX_FAVORITES 10

uint16_t highScores[GAME_COUNT];
uint16_t totalGamesPlayed[GAME_COUNT];
uint16_t rpsWins[GAME_COUNT];
uint8_t favoriteGames[MAX_FAVORITES];
uint8_t favoriteCount = 0;
uint8_t brightnessLevel = 200;
bool soundEnabled = true;

// ============================================================
// FORWARD DECLARATIONS
// ============================================================

typedef void (*GameFunction)();

bool btnPressed(uint8_t pin);
bool btnHeld(uint8_t pin);
void beep(uint16_t freq, uint16_t ms);
void playMenuButtonSound();
void waitRelease();
void centreStr(const char *s, uint8_t y);
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
int showGameMenu(const char* gameName, int gameIndex);
void runGameWithMenu(GameFunction game, const char* gameName, int gameIndex);
int menuSelect();
void showSplash();
void showMainMenu();
void showFavoritesMenu();
void showSettingsMenu();
void showButtonConfig();
void toggleFavorite(int gameIndex);
bool isFavorite(int gameIndex);
void saveFavorites();
void loadFavorites();
void saveBrightness();
void loadBrightness();
void saveSoundSetting();
void loadSoundSetting();

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

// Tetris helper functions
void loadPiece(struct TetPiece &p, uint8_t t);
bool ttFits(struct TetPiece &p, int dx, int dy);
void ttRotate(struct TetPiece &p);

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
  bool edge = cur && !lastSt[idx] && (millis() - lastTime[idx] > 40);
  if (cur != lastSt[idx]) lastTime[idx] = millis();
  lastSt[idx] = cur;
  return edge;
}

bool btnHeld(uint8_t pin) { 
  return digitalRead(pin) == LOW; 
}

void beep(uint16_t freq, uint16_t ms) {
  if (!soundEnabled || freq == 0 || ms == 0) return;
  tone(BUZZER_PIN, freq, ms);
}

void playMenuButtonSound() {
  if (!soundEnabled) return;
  tone(BUZZER_PIN, 600, 150);
  delay(150);
  tone(BUZZER_PIN, 800, 150);
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

void playStartMusic() {
  if (!soundEnabled) return;
  beep(523, 150); delay(200);
  beep(659, 150); delay(200);
  beep(784, 150); delay(200);
  beep(523, 150); delay(200);
  beep(784, 150); delay(200);
  beep(1046, 300); delay(400);
  beep(880, 150); delay(200);
  beep(1046, 500);
}

void playGameOverMusic() {
  if (!soundEnabled) return;
  beep(392, 250); delay(300);
  beep(349, 250); delay(300);
  beep(329, 250); delay(300);
  beep(261, 500); delay(600);
  beep(196, 700);
}

void playPauseSound() {
  if (!soundEnabled) return;
  beep(500, 100);
  delay(120);
  beep(400, 100);
}

void playResumeSound() {
  if (!soundEnabled) return;
  beep(400, 100);
  delay(120);
  beep(500, 100);
}

void levelCompleteMusic() {
  if (!soundEnabled) return;
  beep(523, 150); delay(150);
  beep(659, 150); delay(150);
  beep(784, 150); delay(150);
  beep(1047, 300);
}

void uniqueGameStartSound(int gameIndex) {
  if (!soundEnabled) return;
  switch(gameIndex) {
    case 0: // Asteroids
      beep(200, 100); delay(100); beep(300, 100); delay(100); beep(400, 150);
      break;
    case 1: // Breakout
      beep(500, 80); delay(80); beep(600, 80); delay(80); beep(700, 100);
      break;
    case 2: // Dino
      beep(400, 60); delay(60); beep(600, 60); delay(60); beep(800, 80);
      break;
    case 3: // Flappy
      beep(700, 50); delay(50); beep(900, 50); delay(50); beep(1100, 80);
      break;
    case 4: // Snake1
      beep(300, 80); delay(80); beep(500, 80); delay(80); beep(700, 100);
      break;
    case 5: // Snake2
      beep(350, 80); delay(80); beep(550, 80); delay(80); beep(750, 100);
      break;
    case 6: // Pong
      beep(600, 60); delay(60); beep(800, 60); delay(60); beep(1000, 80);
      break;
    case 7: // Pacman
      beep(400, 100); delay(80); beep(600, 100); delay(80); beep(800, 120);
      break;
    case 8: // Space Invaders
      beep(200, 80); delay(80); beep(400, 80); delay(80); beep(600, 100);
      break;
    case 9: // Tetris
      beep(500, 100); delay(80); beep(700, 100); delay(80); beep(900, 120);
      break;
    case 10: // Tank
      beep(250, 80); delay(80); beep(450, 80); delay(80); beep(650, 100);
      break;
    case 11: // Maze
      beep(550, 60); delay(60); beep(750, 60); delay(60); beep(950, 80);
      break;
    case 12: // RPS
      beep(300, 100); delay(80); beep(500, 100); delay(80); beep(700, 120);
      break;
    case 13: // Car
      beep(450, 60); delay(60); beep(650, 60); delay(60); beep(850, 80);
      break;
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
    // Remove from favorites
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
    // Add to favorites
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
  if (brightnessLevel < 0 || brightnessLevel > 255) brightnessLevel = 200;
  u8g2.setContrast(brightnessLevel);
  EEPROM.end();
}

void saveBrightness() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(450, brightnessLevel);
  EEPROM.commit();
  EEPROM.end();
  u8g2.setContrast(brightnessLevel);
}

void loadSoundSetting() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(460, soundEnabled);
  if (soundEnabled != 0 && soundEnabled != 1) soundEnabled = true;
  EEPROM.end();
}

void saveSoundSetting() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(460, soundEnabled);
  EEPROM.commit();
  EEPROM.end();
}

// ============================================================
// HIGH SCORE MANAGEMENT
// ============================================================

void loadHighScores() {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < GAME_COUNT; i++) {
    EEPROM.get(i * sizeof(uint16_t), highScores[i]);
    if (highScores[i] > 10000) highScores[i] = 0;
    EEPROM.get((i + GAME_COUNT) * sizeof(uint16_t), totalGamesPlayed[i]);
    if (totalGamesPlayed[i] > 10000) totalGamesPlayed[i] = 0;
    EEPROM.get((i + GAME_COUNT*2) * sizeof(uint16_t), rpsWins[i]);
    if (rpsWins[i] > 10000) rpsWins[i] = 0;
  }
  EEPROM.end();
  loadFavorites();
  loadBrightness();
  loadSoundSetting();
}

void saveTotalGames(int gameIndex) {
  totalGamesPlayed[gameIndex]++;
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put((gameIndex + GAME_COUNT) * sizeof(uint16_t), totalGamesPlayed[gameIndex]);
  EEPROM.commit();
  EEPROM.end();
}

void saveRPSWin(int gameIndex) {
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
        beep(1200, 50);
        delay(80);
        beep(1500, 50);
        delay(80);
        beep(1800, 100);
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
        beep(1200, 50);
        delay(80);
        beep(1500, 50);
        delay(80);
        beep(1800, 100);
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
int lastGameIndex = 0;

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
        }
        if (btnPressed(BTN_MENU)) {
          gamePaused = false;
          playMenuButtonSound();
          return true;
        }
        delay(50);
      }
      return false;
    } else {
      playResumeSound();
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
// GAME OVER AND MENU SYSTEM
// ============================================================

void gameOverScreen(uint16_t score, int gameIndex, bool isWin) {
  char buf[20];
  snprintf(buf, sizeof(buf), "Score: %u", score);
  
  if (isWin) saveTotalGames(gameIndex);
  
  if (gameIndex == 12 && isWin) {
    saveRPSWin(gameIndex);
  }
  
  saveHighScore(gameIndex, score);
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("GAME OVER", 20);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr(buf, 35);
  
  char hs[30];
  if (gameIndex == 12) {
    uint16_t winRate = 0;
    if (totalGamesPlayed[gameIndex] > 0) {
      winRate = (rpsWins[gameIndex] * 100) / totalGamesPlayed[gameIndex];
    }
    snprintf(hs, sizeof(hs), "Win Rate: %u%%", winRate);
  } else {
    snprintf(hs, sizeof(hs), "High Score: %u", highScores[gameIndex]);
  }
  centreStr(hs, 47);
  
  char total[30];
  snprintf(total, sizeof(total), "Total: %u", totalGamesPlayed[gameIndex]);
  centreStr(total, 58);
  u8g2.sendBuffer();
  playGameOverMusic();
  delay(400);
  waitRelease();
}

int showGameMenu(const char* gameName, int gameIndex) {
  const char* options[] = {"1. PLAY GAME", "2. HIGH SCORE", "3. GAME RULES", "4. FAVORITE"};
  int sel = 0;
  
  while (true) {
    u8g2.clearBuffer();
    
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawRBox(5, 2, SCREEN_W - 10, 12, 2);
    u8g2.setDrawColor(0);
    centreStr(gameName, 11);
    u8g2.setDrawColor(1);
    
    u8g2.setFont(u8g2_font_ncenB08_tr);
    for (int i = 0; i < 4; i++) {
      int y = 33 + i * 13;
      if (i == sel) {
        u8g2.drawRBox(10, y - 8, SCREEN_W - 20, 12, 2);
        u8g2.setDrawColor(0);
        centreStr(options[i], y + 3);
        u8g2.setDrawColor(1);
      } else {
        centreStr(options[i], y + 3);
      }
    }
    
    // Show favorite status
    u8g2.setFont(u8g2_font_5x7_tr);
    if (isFavorite(gameIndex)) {
      centreStr("⭐ FAVORITE", 62);
    }
    
    u8g2.sendBuffer();
    
    if (btnPressed(BTN_UP)) { sel = (sel + 3) % 4; beep(800, 25); }
    else if (btnPressed(BTN_DOWN)) { sel = (sel + 1) % 4; beep(800, 25); }
    else if (btnPressed(BTN_ENTER)) { beep(1000, 40); waitRelease(); return sel; }
    else if (btnPressed(BTN_MENU)) { beep(600, 50); waitRelease(); return -1; }
    delay(100);
  }
}

void runGameWithMenu(GameFunction game, const char* gameName, int gameIndex) {
  lastGameIndex = gameIndex;
  while (true) {
    int choice = showGameMenu(gameName, gameIndex);
    if (choice == -1) return;
    
    else if (choice == 0) {
      menuPressed = false;
      gamePaused = false;
      uniqueGameStartSound(gameIndex);
      game();
      
      if (menuPressed) {
        menuPressed = false;
        continue;
      }
      
      delay(500);
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);
      centreStr("GAME OVER", 20);
      u8g2.setFont(u8g2_font_6x10_tr);
      centreStr("ENTER to replay", 38);
      centreStr("MENU to game menu", 50);
      u8g2.sendBuffer();
      
      bool waitForInput = true;
      while (waitForInput) {
        if (btnPressed(BTN_MENU)) {
          playMenuButtonSound();
          waitRelease();
          waitForInput = false;
          break;
        }
        if (btnPressed(BTN_ENTER)) {
          beep(800, 25);
          waitRelease();
          waitForInput = false;
          break;
        }
        delay(50);
      }
    }
    
    else if (choice == 1) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);
      centreStr("HIGH SCORE", 20);
      u8g2.setFont(u8g2_font_ncenB08_tr);
      
      if (gameIndex == 12) {
        centreStr("🌟 You are a", 38);
        centreStr("Rock Paper", 48);
        centreStr("Scissors Champion! 🏆", 58);
      } else {
        char hs[30];
        snprintf(hs, sizeof(hs), "%u", highScores[gameIndex]);
        centreStr(hs, 38);
        char total[30];
        snprintf(total, sizeof(total), "Total Games: %u", totalGamesPlayed[gameIndex]);
        centreStr(total, 50);
      }
      u8g2.sendBuffer();
      waitRelease();
      while (!btnHeld(BTN_UP) && !btnHeld(BTN_DOWN) && !btnHeld(BTN_LEFT) &&
             !btnHeld(BTN_RIGHT) && !btnHeld(BTN_MENU) && !btnHeld(BTN_ENTER))
        delay(15);
      waitRelease();
    }
    
    else if (choice == 2) {
      const char* rules[14] = {
        "Dodge asteroids and survive as long as possible!",
        "Break all bricks using the bouncing ball!",
        "Jump over cacti and run as far as you can!",
        "Flap through pipes and avoid hitting them!",
        "Eat food to grow and avoid self-collision!",
        "Eat food within boundary and shrink after 10!",
        "First to score 7 wins! Move your paddle!",
        "Eat all dots while avoiding the ghost!",
        "Shoot all aliens while dodging their fire!",
        "Complete lines to score and prevent stacking!",
        "Destroy enemy tanks with your cannon!",
        "Find the exit in each maze as fast as possible!",
        "Beat the computer in unlimited rounds!",
        "Avoid obstacles and stay alive on 3 lanes!"
      };
      
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x10_tr);
      centreStr("RULES", 10);
      
      const char* ruleText = rules[gameIndex];
      int len = strlen(ruleText);
      int y = 22;
      int lineStart = 0;
      
      while (lineStart < len && y < 60) {
        int lineEnd = lineStart;
        int lastSpace = lineStart;
        
        while (lineEnd < len && (lineEnd - lineStart) < 16) {
          if (ruleText[lineEnd] == ' ') lastSpace = lineEnd;
          lineEnd++;
        }
        
        if (lineEnd < len && lastSpace > lineStart) {
          lineEnd = lastSpace;
        }
        
        char line[25];
        int idx = 0;
        for (int i = lineStart; i < lineEnd && idx < 24; i++) {
          if (ruleText[i] != ' ') {
            line[idx++] = ruleText[i];
          } else if (i > lineStart && ruleText[i-1] != ' ') {
            line[idx++] = ' ';
          }
        }
        line[idx] = '\0';
        
        if (strlen(line) > 0) {
          centreStr(line, y);
          y += 11;
        }
        
        lineStart = lineEnd + 1;
        while (lineStart < len && ruleText[lineStart] == ' ') lineStart++;
      }
      
      u8g2.sendBuffer();
      waitRelease();
      while (!btnHeld(BTN_UP) && !btnHeld(BTN_DOWN) && !btnHeld(BTN_LEFT) &&
             !btnHeld(BTN_RIGHT) && !btnHeld(BTN_MENU) && !btnHeld(BTN_ENTER))
        delay(15);
      waitRelease();
    }
    
    else if (choice == 3) {
      toggleFavorite(gameIndex);
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);
      if (isFavorite(gameIndex)) {
        centreStr("⭐ ADDED TO", 24);
        centreStr("FAVORITES!", 40);
        beep(1200, 50);
        delay(100);
        beep(1500, 50);
      } else {
        centreStr("❌ REMOVED", 24);
        centreStr("FROM FAVORITES", 40);
        beep(600, 50);
        delay(100);
        beep(500, 50);
      }
      u8g2.sendBuffer();
      delay(800);
      waitRelease();
    }
  }
}

// ============================================================
// SPLASH SCREEN
// ============================================================

void showSplash() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("🎮 GAME CONSOLE", 20);
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("MADE BY Rahul", 40);
  u8g2.sendBuffer();
  playStartMusic();
  while (!btnHeld(BTN_UP) && !btnHeld(BTN_DOWN) && !btnHeld(BTN_LEFT) &&
         !btnHeld(BTN_RIGHT) && !btnHeld(BTN_ENTER))
    delay(15);
  waitRelease();
}

// ============================================================
// MAIN MENU FUNCTIONS
// ============================================================

void showButtonConfig() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  centreStr("BUTTON CONFIG", 10);
  
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(10, 25, "UP    : Move Up");
  u8g2.drawStr(10, 35, "DOWN  : Move Down");
  u8g2.drawStr(10, 45, "LEFT  : Move Left");
  u8g2.drawStr(10, 55, "RIGHT : Move Right");
  
  u8g2.sendBuffer();
  waitRelease();
  
  // Second page for additional buttons
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  centreStr("BUTTON CONFIG (2)", 10);
  
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(10, 25, "ENTER : Select/Play");
  u8g2.drawStr(10, 35, "MENU  : Back");
  u8g2.drawStr(10, 45, "PAUSE : Pause/Resume");
  
  u8g2.sendBuffer();
  waitRelease();
  
  while (!btnHeld(BTN_UP) && !btnHeld(BTN_DOWN) && !btnHeld(BTN_LEFT) &&
         !btnHeld(BTN_RIGHT) && !btnHeld(BTN_MENU) && !btnHeld(BTN_ENTER))
    delay(15);
  waitRelease();
}

void showSettingsMenu() {
  const char* options[] = {"1. Brightness", "2. Sound", "3. Back"};
  int sel = 0;
  
  while (true) {
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
    
    if (btnPressed(BTN_UP)) { sel = (sel + 2) % 3; beep(800, 25); }
    else if (btnPressed(BTN_DOWN)) { sel = (sel + 1) % 3; beep(800, 25); }
    else if (btnPressed(BTN_ENTER)) { 
      beep(1000, 40); 
      waitRelease();
      
      if (sel == 0) {
        // Brightness control
        bool adjusting = true;
        while (adjusting) {
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_ncenB08_tr);
          centreStr("BRIGHTNESS", 10);
          
          char buf[20];
          snprintf(buf, sizeof(buf), "Level: %d", brightnessLevel);
          centreStr(buf, 30);
          
          // Draw visual bar
          int barWidth = map(brightnessLevel, 0, 255, 0, 100);
          u8g2.drawBox(14, 40, barWidth, 10);
          u8g2.drawFrame(14, 40, 100, 10);
          
          u8g2.setFont(u8g2_font_5x7_tr);
          centreStr("UP/DOWN adjust", 56);
          centreStr("ENTER save  MENU back", 63);
          u8g2.sendBuffer();
          
          if (btnPressed(BTN_UP)) { 
            brightnessLevel = min(255, brightnessLevel + 10); 
            u8g2.setContrast(brightnessLevel);
            beep(800, 20);
          }
          else if (btnPressed(BTN_DOWN)) { 
            brightnessLevel = max(0, brightnessLevel - 10); 
            u8g2.setContrast(brightnessLevel);
            beep(800, 20);
          }
          else if (btnPressed(BTN_ENTER)) { 
            saveBrightness();
            beep(1000, 40);
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
        // Sound control
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        centreStr("SOUND", 20);
        u8g2.setFont(u8g2_font_ncenB10_tr);
        centreStr(soundEnabled ? "🔊 ON" : "🔇 OFF", 40);
        u8g2.setFont(u8g2_font_6x10_tr);
        centreStr("ENTER to toggle", 55);
        u8g2.sendBuffer();
        
        waitRelease();
        while (true) {
          if (btnPressed(BTN_ENTER)) {
            soundEnabled = !soundEnabled;
            saveSoundSetting();
            if (soundEnabled) {
              beep(1000, 50);
              delay(100);
              beep(1200, 50);
            }
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_ncenB08_tr);
            centreStr("SOUND", 20);
            u8g2.setFont(u8g2_font_ncenB10_tr);
            centreStr(soundEnabled ? "🔊 ON" : "🔇 OFF", 40);
            u8g2.setFont(u8g2_font_6x10_tr);
            centreStr("ENTER to toggle", 55);
            u8g2.sendBuffer();
            waitRelease();
          }
          else if (btnPressed(BTN_MENU)) {
            playMenuButtonSound();
            waitRelease();
            break;
          }
          delay(50);
        }
      }
      else if (sel == 2) {
        return;
      }
    }
    else if (btnPressed(BTN_MENU)) { playMenuButtonSound(); waitRelease(); return; }
    delay(100);
  }
}

void showFavoritesMenu() {
  if (favoriteCount == 0) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tr);
    centreStr("⭐ NO", 24);
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
    "Maze Runner", "RPS Game", "Car Racer"
  };
  
  GameFunction games[GAME_COUNT] = {
    game_asteroids, game_breakout, game_dino, game_flappy,
    game_snake1, game_snake2, game_pong, game_pacman,
    game_spaceinvaders, game_tetris, game_tank,
    game_maze, game_rps, game_car
  };
  
  int sel = 0;
  int top = 0;
  const int VISIBLE = 4;
  
  while (true) {
    if (sel < top) top = sel;
    if (sel >= top + VISIBLE) top = sel - VISIBLE + 1;
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    centreStr("⭐ FAVORITES", 10);
    
    for (int i = 0; i < VISIBLE; i++) {
      int idx = top + i;
      if (idx >= favoriteCount) break;
      int gameIdx = favoriteGames[idx];
      int y = 14 + i * 13;
      if (idx == sel) {
        u8g2.drawRBox(0, y-1, SCREEN_W, 12, 2);
        u8g2.setDrawColor(0);
        u8g2.drawStr(6, y + 9, gameNames[gameIdx]);
        u8g2.setDrawColor(1);
      } else {
        u8g2.drawStr(6, y + 9, gameNames[gameIdx]);
      }
    }
    
    if (top > 0) u8g2.drawStr(SCREEN_W - 8, 13, "^");
    if (top + VISIBLE < favoriteCount) u8g2.drawStr(SCREEN_W - 8, 62, "v");
    
    u8g2.sendBuffer();
    delay(100);
    
    if (btnPressed(BTN_UP)) { 
      sel = (sel + favoriteCount - 1) % favoriteCount; 
      beep(800, 25); 
    }
    else if (btnPressed(BTN_DOWN)) { 
      sel = (sel + 1) % favoriteCount; 
      beep(800, 25); 
    }
    else if (btnPressed(BTN_ENTER)) { 
      beep(1200, 60); 
      waitRelease();
      int gameIdx = favoriteGames[sel];
      runGameWithMenu(games[gameIdx], gameNames[gameIdx], gameIdx);
      return;
    }
    else if (btnPressed(BTN_MENU)) { 
      playMenuButtonSound(); 
      waitRelease(); 
      return; 
    }
  }
}

void showMainMenu() {
  const char* options[] = {"1. GAME MENU", "2. ⭐ FAVORITES", "3. SETTINGS", "4. BUTTON CONFIG"};
  int sel = 0;
  
  const char* gameNames[GAME_COUNT] = {
    "Asteroids", "Breakout", "Dino Run", "Flappy Bird",
    "Snake 1", "Snake 2", "Pong", "Pacman",
    "Space Invaders", "Tetris", "Tank Battle",
    "Maze Runner", "RPS Game", "Car Racer"
  };
  
  GameFunction games[GAME_COUNT] = {
    game_asteroids, game_breakout, game_dino, game_flappy,
    game_snake1, game_snake2, game_pong, game_pacman,
    game_spaceinvaders, game_tetris, game_tank,
    game_maze, game_rps, game_car
  };
  
  while (true) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawBox(0, 0, SCREEN_W, 11);
    u8g2.setDrawColor(0);
    centreStr("🎮 MAIN MENU 🎮", 9);
    u8g2.setDrawColor(1);
    
    for (int i = 0; i < 4; i++) {
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
    
    // Show favorite count
    u8g2.setFont(u8g2_font_5x7_tr);
    char favCount[20];
    snprintf(favCount, sizeof(favCount), "Favorites: %d", favoriteCount);
    centreStr(favCount, 63);
    
    u8g2.sendBuffer();
    delay(100);
    
    if (btnPressed(BTN_UP)) { sel = (sel + 3) % 4; beep(800, 25); }
    else if (btnPressed(BTN_DOWN)) { sel = (sel + 1) % 4; beep(800, 25); }
    else if (btnPressed(BTN_ENTER)) { 
      beep(1200, 60); 
      waitRelease();
      
      if (sel == 0) {
        int gameSel = menuSelect();
        if (gameSel >= 0 && gameSel < GAME_COUNT) {
          runGameWithMenu(games[gameSel], gameNames[gameSel], gameSel);
        }
      }
      else if (sel == 1) {
        showFavoritesMenu();
      }
      else if (sel == 2) {
        showSettingsMenu();
      }
      else if (sel == 3) {
        showButtonConfig();
      }
    }
    else if (btnPressed(BTN_MENU)) { 
      playMenuButtonSound(); 
      waitRelease(); 
    }
  }
}

// ============================================================
// GAME: ASTEROIDS
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

    score++;
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
          gameOverScreen(score / 10, 0, false);
          return;
        }
        beep(200, 100);
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
    ltoa(score / 10, sc, 10);
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
// GAME: BREAKOUT
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
    
    if (btnHeld(BTN_LEFT)) padX = max(0, padX - 4);
    if (btnHeld(BTN_RIGHT)) padX = min(SCREEN_W - 20, padX + 4);
    
    bx += vx * dt;
    by += vy * dt;
    
    if (bx <= 1) { vx = fabsf(vx); bx = 1; beep(600, 15); }
    if (bx >= SCREEN_W - 4) { vx = -fabsf(vx); bx = SCREEN_W - 4; beep(600, 15); }
    if (by <= 1) { vy = fabsf(vy); by = 1; beep(600, 15); }
    
    if (vy > 0 && by + 3 >= padY && by + 3 <= padY + 3 + 2 &&
        bx + 2 >= padX && bx <= padX + 20) {
      vy = -fabsf(vy);
      float rel = ((bx + 1) - (padX + 10.0f)) / 10.0f;
      vx = rel * 3.0f;
      if (fabsf(vx) < 0.5f) vx = (vx >= 0) ? 0.5f : -0.5f;
      by = padY - 3;
      beep(900, 20);
    }
    
    if (by > SCREEN_H + 2) {
      lives--;
      beep(200, 300);
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
          beep(1200 + r * 120, 25);
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
      beep(1760, 100); delay(110);
      beep(1760, 100); delay(110);
      beep(2093, 300);
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
    u8g2.drawRBox(padX, padY, 20, 3, 1);
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
// GAME: DINO RUN
// ============================================================

void game_dino() {
  const float GRAVITY = 0.45f;
  const float JUMP_FORCE = -7.0f;
  const int DINO_X = 15;
  const int GROUND_Y = 58;

  float dinoY = GROUND_Y - 12;
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
  centreStr("Avoid obstacles!", 54);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    if (checkPause("DINO RUN")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    float dt = (now - lastFrame) / 20.0f;
    if (dt > 2.0f) dt = 2.0f;
    lastFrame = now;

    bool anyBtn = btnPressed(BTN_UP) || btnPressed(BTN_DOWN) ||
                  btnPressed(BTN_LEFT) || btnPressed(BTN_RIGHT);
    if (anyBtn && !isJumping) {
      velY = JUMP_FORCE;
      isJumping = true;
      beep(1000, 20);
    }

    velY += GRAVITY * dt;
    dinoY += velY * dt;

    if (dinoY >= GROUND_Y - 12) {
      dinoY = GROUND_Y - 12;
      velY = 0;
      isJumping = false;
    }

    obsX -= obsSpeed * dt;
    if (obsX < -30) {
      obsX = 130 + random(0, 50);
      obsType = random(0, 2);
      score++;
      obsSpeed += 0.05f;
    }

    int dinoW = 10, dinoH = 12;
    int obsW, obsH, obsY;
    if (obsType == 0) {
      obsW = 12; obsH = 16; obsY = GROUND_Y - 16;
    } else {
      obsW = 16; obsH = 20; obsY = GROUND_Y - 20;
    }

    if (DINO_X < obsX + obsW && DINO_X + dinoW > obsX && 
        dinoY < obsY + obsH && dinoY + dinoH > obsY) {
      gameOverScreen(score, 2, false);
      return;
    }

    u8g2.clearBuffer();
    u8g2.drawHLine(0, GROUND_Y, SCREEN_W);
    u8g2.drawFrame(DINO_X, (int)dinoY, 10, 12);
    u8g2.drawBox(DINO_X + 7, (int)dinoY + 2, 2, 2);
    
    if (obsType == 0) {
      int x = (int)obsX, y = (int)obsY;
      u8g2.drawBox(x + 3, y, 4, 16);
      u8g2.drawBox(x, y + 4, 3, 6);
      u8g2.drawBox(x + 7, y + 3, 3, 7);
    } else {
      int x = (int)obsX, y = (int)obsY;
      u8g2.drawBox(x + 4, y, 6, 20);
      u8g2.drawBox(x, y + 6, 4, 8);
      u8g2.drawBox(x + 10, y + 5, 4, 9);
    }

    u8g2.setFont(u8g2_font_6x10_tr);
    char sc[6];
    itoa(score, sc, 10);
    u8g2.drawStr(100, 10, sc);
    u8g2.sendBuffer();
    delay(20);
  }
}

// ============================================================
// GAME: FLAPPY BIRD
// ============================================================

void game_flappy() {
  const float GRAVITY = 0.30f;
  const float FLAP_VEL = -2.20f;
  const int PIPE_W = 8;
  const int BIRD_X = 14;
  const int BIRD_W = 7;
  const int BIRD_H = 5;
  const int VERTICAL_GAP = 36;
  const float INITIAL_SPEED = 1.2f;

  float birdY = 30, vel = 0;
  int pX[3] = {128, 0, 0};
  int pGapY[3];
  int score = 0;
  bool scored[3] = {false, false, false};
  uint32_t lastFrame = 0;
  float currentSpeed = INITIAL_SPEED;
  int activePipes = 2;

  for(int i = 0; i < 2; i++) {
    pGapY[i] = random(8, SCREEN_H - VERTICAL_GAP - 8);
    scored[i] = false;
  }
  int initialDistance = random(30, 60);
  pX[1] = pX[0] + initialDistance;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("FLAPPY BIRD", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Any button = Flap", 40);
  centreStr("Press to start", 54);
  u8g2.sendBuffer();
  waitRelease();
  while (!btnPressed(BTN_UP) && !btnPressed(BTN_DOWN) &&
         !btnPressed(BTN_LEFT) && !btnPressed(BTN_RIGHT))
    delay(10);
  vel = FLAP_VEL;
  beep(900, 35);
  lastFrame = millis();

  while (true) {
    if (checkPause("FLAPPY BIRD")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    float dt = (now - lastFrame) / 30.0f;
    if (dt > 3.0f) dt = 3.0f;
    lastFrame = now;

    int minRange, maxRange;
    if(score >= 0 && score <= 20) { minRange = 30; maxRange = 60; }
    else if(score >= 21 && score <= 40) { minRange = 30; maxRange = 50; }
    else if(score >= 41 && score <= 60) { minRange = 25; maxRange = 40; }
    else if(score >= 61 && score <= 80) { minRange = 20; maxRange = 30; }
    else { minRange = 25; maxRange = 25; }

    currentSpeed = INITIAL_SPEED + (score * 0.02f);
    if (currentSpeed > 8.0f) currentSpeed = 8.0f;

    bool anyBtn = btnPressed(BTN_UP) || btnPressed(BTN_DOWN) ||
                  btnPressed(BTN_LEFT) || btnPressed(BTN_RIGHT);
    if (anyBtn) { vel = FLAP_VEL; beep(900, 30); }

    vel += GRAVITY * dt;
    birdY += vel * dt;

    for (int i = 0; i < activePipes; i++) {
      pX[i] -= (int)(currentSpeed * dt);
      
      if (pX[i] < -PIPE_W) {
        int horizontalGap = random(minRange, maxRange + 1);
        int farthestX = -999;
        for(int j = 0; j < activePipes; j++) {
          if(pX[j] > farthestX) farthestX = pX[j];
        }
        pX[i] = farthestX + horizontalGap;
        pGapY[i] = random(8, SCREEN_H - VERTICAL_GAP - 8);
        scored[i] = false;
        
        if(score > 15 && activePipes < 3) {
          activePipes = 3;
          int thirdGap = random(minRange, maxRange + 1);
          pX[2] = pX[1] + thirdGap;
          pGapY[2] = random(8, SCREEN_H - VERTICAL_GAP - 8);
          scored[2] = false;
        }
      }
      
      if (!scored[i] && pX[i] + PIPE_W < BIRD_X) {
        score++;
        scored[i] = true;
        beep(1300, 25);
      }
    }

    int by = (int)birdY;
    if (by < 0 || by + BIRD_H >= SCREEN_H) {
      gameOverScreen(score, 3, false);
      return;
    }

    for (int i = 0; i < activePipes; i++) {
      if (BIRD_X + BIRD_W > pX[i] && BIRD_X < pX[i] + PIPE_W) {
        if (by < pGapY[i] || by + BIRD_H > pGapY[i] + VERTICAL_GAP) {
          gameOverScreen(score, 3, false);
          return;
        }
      }
    }

    u8g2.clearBuffer();
    for (int i = 0; i < activePipes; i++) {
      if(pX[i] < SCREEN_W + 20 && pX[i] > -20) {
        u8g2.drawBox(pX[i], 0, PIPE_W, pGapY[i]);
        u8g2.drawBox(pX[i] - 1, pGapY[i] - 4, PIPE_W + 2, 5);
        int bottomPipeY = pGapY[i] + VERTICAL_GAP;
        int bottomPipeHeight = SCREEN_H - bottomPipeY;
        u8g2.drawBox(pX[i], bottomPipeY, PIPE_W, bottomPipeHeight);
        u8g2.drawBox(pX[i] - 1, bottomPipeY - 1, PIPE_W + 2, 5);
      }
    }

    u8g2.drawBox(BIRD_X, by, BIRD_W, BIRD_H);
    u8g2.setFont(u8g2_font_6x10_tr);
    char sc[6];
    itoa(score, sc, 10);
    u8g2.drawStr(2, 10, sc);
    u8g2.sendBuffer();
    delay(33);
  }
}

// ============================================================
// GAME: SNAKE 1 (Original - No Boundary)
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

  auto placeFood = [&]() {
    bool ok;
    do {
      ok = true;
      fx = random(0, SN_COLS);
      fy = random(0, SN_ROWS);
      for (int i = 0; i < len; i++)
        if (sx[i] == fx && sy[i] == fy) { ok = false; break; }
    } while (!ok);
  };

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
    
    if (btnHeld(BTN_UP) && dy == 0) { next_dx = 0; next_dy = -1; }
    if (btnHeld(BTN_DOWN) && dy == 0) { next_dx = 0; next_dy = 1; }
    if (btnHeld(BTN_LEFT) && dx == 0) { next_dx = -1; next_dy = 0; }
    if (btnHeld(BTN_RIGHT) && dx == 0) { next_dx = 1; next_dy = 0; }

    uint32_t now = millis();
    if (now - lastMove < spd) { delay(8); continue; }
    lastMove = now;

    dx = next_dx; dy = next_dy;
    int nx = (sx[0] + dx + SN_COLS) % SN_COLS;
    int ny = (sy[0] + dy + SN_ROWS) % SN_ROWS;

    for (int i = 1; i < len; i++) {
      if (sx[i] == nx && sy[i] == ny) {
        gameOverScreen(score, 4, false);
        return;
      }
    }

    for (int i = len - 1; i > 0; i--) {
      sx[i] = sx[i - 1]; sy[i] = sy[i - 1];
    }
    sx[0] = nx; sy[0] = ny;

    if (nx == fx && ny == fy) {
      score++;
      if (len < SN_MAXLEN) len++;
      spd = max(70, (int)spd - 7);
      beep(1400, 35);
      placeFood();
    }

    u8g2.clearBuffer();
    for (int i = 0; i < len; i++) {
      int px = 1 + sx[i] * SN_SZ;
      int py = 2 + sy[i] * SN_SZ;
      if (i == 0) u8g2.drawBox(px, py, SN_SZ, SN_SZ);
      else u8g2.drawFrame(px + 1, py + 1, SN_SZ - 2, SN_SZ - 2);
    }
    u8g2.drawDisc(1 + fx * SN_SZ + 3, 2 + fy * SN_SZ + 3, 3);
    
    u8g2.setFont(u8g2_font_5x7_tr);
    char sc[6];
    itoa(score, sc, 10);
    u8g2.drawStr(1 + SN_COLS * SN_SZ + 4, 10, "SC");
    u8g2.drawStr(1 + SN_COLS * SN_SZ + 4, 20, sc);
    u8g2.sendBuffer();
  }
}

// ============================================================
// GAME: SNAKE 2 (With Boundary - Shrink at 10 Food)
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

  auto placeFood = [&]() {
    bool ok;
    do {
      ok = true;
      fx = random(0, SN2_COLS);
      fy = random(0, SN2_ROWS);
      for (int i = 0; i < len; i++)
        if (sx[i] == fx && sy[i] == fy) { ok = false; break; }
    } while (!ok);
  };

  for (int i = 0; i < len; i++) {
    sx[i] = len - 1 - i;
    sy[i] = SN2_ROWS / 2;
  }
  placeFood();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("SNAKE 2", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Boundary version!", 40);
  centreStr("Eat 10 to shrink!", 54);
  u8g2.sendBuffer();
  delay(1400);
  waitRelease();

  while (true) {
    if (checkPause("SNAKE 2")) return;
    if (checkMenuAndReturn()) return;
    
    if (btnHeld(BTN_UP) && dy == 0) { next_dx = 0; next_dy = -1; }
    if (btnHeld(BTN_DOWN) && dy == 0) { next_dx = 0; next_dy = 1; }
    if (btnHeld(BTN_LEFT) && dx == 0) { next_dx = -1; next_dy = 0; }
    if (btnHeld(BTN_RIGHT) && dx == 0) { next_dx = 1; next_dy = 0; }

    uint32_t now = millis();
    if (now - lastMove < spd) { delay(8); continue; }
    lastMove = now;

    dx = next_dx; dy = next_dy;
    int nx = sx[0] + dx;
    int ny = sy[0] + dy;

    // Check boundary collision - full screen boundary
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
      sx[i] = sx[i - 1]; sy[i] = sy[i - 1];
    }
    sx[0] = nx; sy[0] = ny;

    if (nx == fx && ny == fy) {
      score++;
      foodEaten++;
      if (len < SN2_MAXLEN) len++;
      spd = max(70, (int)spd - 7);
      beep(1400, 35);
      placeFood();
      
      if (foodEaten >= 10) {
        foodEaten = 0;
        if (len > 4) {
          len = 4;
          levelCompleteMusic();
        }
      }
    }

    u8g2.clearBuffer();
    
    // Draw boundary - full screen
    int bx = SN2_OX - 1, by = SN2_OY - 1;
    int bw = SN2_COLS * SN2_SZ + 2, bh = SN2_ROWS * SN2_SZ + 2;
    u8g2.drawFrame(bx, by, bw, bh);
    
    for (int i = 0; i < len; i++) {
      int px = SN2_OX + sx[i] * SN2_SZ;
      int py = SN2_OY + sy[i] * SN2_SZ;
      if (i == 0) u8g2.drawBox(px, py, SN2_SZ, SN2_SZ);
      else u8g2.drawFrame(px + 1, py + 1, SN2_SZ - 2, SN2_SZ - 2);
    }
    u8g2.drawDisc(SN2_OX + fx * SN2_SZ + 3, SN2_OY + fy * SN2_SZ + 3, 3);
    
    u8g2.setFont(u8g2_font_5x7_tr);
    char sc[6];
    itoa(score, sc, 10);
    u8g2.drawStr(SCREEN_W - 20, 8, sc);
    char foodStr[6];
    itoa(foodEaten, foodStr, 10);
    u8g2.drawStr(SCREEN_W - 20, 16, foodStr);
    u8g2.drawStr(SCREEN_W - 20, 24, "/10");
    u8g2.sendBuffer();
  }
}

// ============================================================
// GAME: PONG
// ============================================================

void game_pong() {
  const int PAD_H = 18;
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
    
    if (btnHeld(BTN_UP)) pY = max(0, pY - 3);
    if (btnHeld(BTN_DOWN)) pY = min(SCREEN_H - PAD_H, pY + 3);

    int mid = cY + PAD_H / 2;
    if (mid < (int)by - 1) cY = min(SCREEN_H - PAD_H, cY + 2);
    if (mid > (int)by + 1) cY = max(0, cY - 2);

    bx += vx; by += vy;

    if (by <= 0) { vy = fabsf(vy); by = 0; beep(500, 15); }
    if (by >= SCREEN_H - 3) { vy = -fabsf(vy); by = SCREEN_H - 3; beep(500, 15); }

    if (vx < 0 && bx <= 4 + PAD_W && bx >= 4 && by + 2 >= cY && by <= cY + PAD_H) {
      vx = fabsf(vx) * 1.05f;
      vy += ((by - (cY + PAD_H / 2.0f)) / (PAD_H / 2.0f)) * 1.2f;
      vy = constrain(vy, -4.0f, 4.0f);
      vx = min(vx, 5.0f);
      bx = 4 + PAD_W;
      beep(900, 20);
    }

    if (bx >= 121 - PAD_W && bx <= 122 && by + 2 >= pY && by <= pY + PAD_H) {
      vx = -fabsf(vx) * 1.05f;
      vy += ((by - (pY + PAD_H / 2.0f)) / (PAD_H / 2.0f)) * 1.5f;
      vy = constrain(vy, -4.5f, 4.5f);
      vx = max(vx, -5.0f);
      bx = 121 - PAD_W - 1;
      beep(1000, 20);
    }

    if (bx < 0) { pScore++; beep(1400, 100); bx = 64; by = 32; vx = 2.5f; vy = 1.8f; pY = 25; cY = 25; delay(600); }
    if (bx > SCREEN_W) { cScore++; beep(200, 250); bx = 64; by = 32; vx = -2.5f; vy = 1.8f; pY = 25; cY = 25; delay(600); }

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
// GAME: PACMAN
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

    int mx = (px * 10) / SCREEN_W;
    int my = (py * 5) / SCREEN_H;
    if (mx >= 0 && mx < 10 && my >= 0 && my < 5) {
      if (maze[my][mx]) {
        maze[my][mx] = false;
        score += 10;
        beep(1000, 10);
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
// GAME: SPACE INVADERS
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
          beep(1500, 15);
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
          beep(800 - br * 100, 40);
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
      beep(1760, 100); delay(110);
      beep(2093, 200); delay(400);
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
// GAME: TETRIS
// ============================================================

#define TT_COLS 8
#define TT_ROWS 13
#define TT_SZ 5
#define TT_OX 44
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
      if (btnHeld(BTN_LEFT) && ttFits(cur, -1, 0)) { cur.x--; lastMove = now; beep(700, 15); }
      if (btnHeld(BTN_RIGHT) && ttFits(cur, 1, 0)) { cur.x++; lastMove = now; beep(700, 15); }
    }
    if (btnPressed(BTN_UP)) { ttRotate(cur); beep(900, 15); }
    
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
        beep(500, 20);
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
          static const uint16_t pts[5] = {0, 40, 100, 300, 1200};
          score += pts[min(cleared, 4)] * level;
          beep(1200, 40); delay(45); beep(1400, 40);
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
// GAME: TANK BATTLE
// ============================================================

void game_tank() {
  struct Tank { float x, y; int8_t dx, dy; bool active; uint32_t lastMove; };
  struct TankBullet { float x, y; int8_t dx, dy; bool active; };
  
  Tank player = {64, 50, 0, 0, true, 0};
  Tank enemies[3];
  for (int i = 0; i < 3; i++) enemies[i].active = false;
  
  TankBullet pBullet[3];
  for (int i = 0; i < 3; i++) pBullet[i].active = false;
  TankBullet eBullets[5];
  for (int i = 0; i < 5; i++) eBullets[i].active = false;
  
  uint16_t score = 0;
  uint32_t lastSpawn = 0;
  uint32_t lastFrame = millis();
  uint32_t lastFire = 0;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("TANK BATTLE", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("D-pad = Move", 40);
  centreStr("ENTER = Fire", 52);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    if (checkPause("TANK BATTLE")) return;
    if (checkMenuAndReturn()) return;
    
    uint32_t now = millis();
    float dt = (now - lastFrame) / 20.0f;
    lastFrame = now;

    if (btnHeld(BTN_UP)) { player.y -= 1.5f * dt; player.dx = 0; player.dy = -1; }
    else if (btnHeld(BTN_DOWN)) { player.y += 1.5f * dt; player.dx = 0; player.dy = 1; }
    else if (btnHeld(BTN_LEFT)) { player.x -= 1.5f * dt; player.dx = -1; player.dy = 0; }
    else if (btnHeld(BTN_RIGHT)) { player.x += 1.5f * dt; player.dx = 1; player.dy = 0; }

    player.x = constrain(player.x, 0, SCREEN_W - 8);
    player.y = constrain(player.y, 0, SCREEN_H - 8);

    if (btnPressed(BTN_ENTER) && now - lastFire > 300) {
      for (int i = 0; i < 3; i++) {
        if (!pBullet[i].active) {
          pBullet[i].x = player.x + 3; pBullet[i].y = player.y + 3;
          pBullet[i].dx = player.dx; pBullet[i].dy = player.dy;
          pBullet[i].active = true;
          beep(1200, 20);
          lastFire = now;
          break;
        }
      }
    }

    if (now - lastSpawn > 2000) {
      for (int i = 0; i < 3; i++) {
        if (!enemies[i].active) {
          enemies[i].x = random(0, SCREEN_W - 8);
          enemies[i].y = 0;
          enemies[i].dx = 0; enemies[i].dy = 1;
          enemies[i].active = true;
          enemies[i].lastMove = now;
          lastSpawn = now;
          break;
        }
      }
    }

    for (int i = 0; i < 3; i++) {
      if (!enemies[i].active) continue;
      if (now - enemies[i].lastMove > 800) {
        int dir = random(0, 4);
        if (dir == 0) { enemies[i].dx = 0; enemies[i].dy = -1; }
        else if (dir == 1) { enemies[i].dx = 1; enemies[i].dy = 0; }
        else if (dir == 2) { enemies[i].dx = 0; enemies[i].dy = 1; }
        else { enemies[i].dx = -1; enemies[i].dy = 0; }
        enemies[i].lastMove = now;
        if (random(0, 100) < 40) {
          for (int j = 0; j < 5; j++) {
            if (!eBullets[j].active) {
              eBullets[j].x = enemies[i].x + 3; eBullets[j].y = enemies[i].y + 3;
              eBullets[j].dx = enemies[i].dx; eBullets[j].dy = enemies[i].dy;
              eBullets[j].active = true;
              break;
            }
          }
        }
      }
      enemies[i].x += enemies[i].dx * 0.8f * dt;
      enemies[i].y += enemies[i].dy * 0.8f * dt;
      enemies[i].x = constrain(enemies[i].x, 0, SCREEN_W - 8);
      enemies[i].y = constrain(enemies[i].y, 0, SCREEN_H - 8);
    }

    for (int i = 0; i < 3; i++) {
      if (pBullet[i].active) {
        pBullet[i].x += pBullet[i].dx * 3.0f * dt;
        pBullet[i].y += pBullet[i].dy * 3.0f * dt;
        if (pBullet[i].x < 0 || pBullet[i].x > SCREEN_W || pBullet[i].y < 0 || pBullet[i].y > SCREEN_H)
          pBullet[i].active = false;
      }
    }
    
    for (int i = 0; i < 5; i++) {
      if (eBullets[i].active) {
        eBullets[i].x += eBullets[i].dx * 2.0f * dt;
        eBullets[i].y += eBullets[i].dy * 2.0f * dt;
        if (eBullets[i].x < 0 || eBullets[i].x > SCREEN_W || eBullets[i].y < 0 || eBullets[i].y > SCREEN_H)
          eBullets[i].active = false;
      }
    }

    for (int i = 0; i < 3; i++) {
      if (!pBullet[i].active) continue;
      for (int j = 0; j < 3; j++) {
        if (enemies[j].active && pBullet[i].x > enemies[j].x && pBullet[i].x < enemies[j].x + 8 &&
            pBullet[i].y > enemies[j].y && pBullet[i].y < enemies[j].y + 8) {
          enemies[j].active = false;
          pBullet[i].active = false;
          score += 50;
          beep(800, 40);
          break;
        }
      }
    }
    
    for (int i = 0; i < 5; i++) {
      if (eBullets[i].active && eBullets[i].x > player.x && eBullets[i].x < player.x + 8 &&
          eBullets[i].y > player.y && eBullets[i].y < player.y + 8) {
        gameOverScreen(score, 10, false);
        return;
      }
    }

    u8g2.clearBuffer();
    u8g2.drawFrame((int)player.x, (int)player.y, 8, 8);
    for (int i = 0; i < 3; i++) {
      if (enemies[i].active) {
        u8g2.drawBox((int)enemies[i].x, (int)enemies[i].y, 8, 8);
      }
    }
    for (int i = 0; i < 3; i++) {
      if (pBullet[i].active) u8g2.drawDisc((int)pBullet[i].x, (int)pBullet[i].y, 1);
    }
    for (int i = 0; i < 5; i++) {
      if (eBullets[i].active) u8g2.drawPixel((int)eBullets[i].x, (int)eBullets[i].y);
    }
    
    u8g2.setFont(u8g2_font_6x10_tr);
    char sc[6]; itoa(score, sc, 10);
    u8g2.drawStr(2, 10, sc);
    u8g2.sendBuffer();
    delay(25);
  }
}

// ============================================================
// GAME: MAZE RUNNER
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
      if (btnHeld(BTN_UP) && !(walls & 1) && py > 0) { py--; moved = true; }
      if (btnHeld(BTN_RIGHT) && !(walls & 2) && px < 15) { px++; moved = true; }
      if (btnHeld(BTN_DOWN) && !(walls & 4) && py < 7) { py++; moved = true; }
      if (btnHeld(BTN_LEFT) && !(walls & 8) && px > 0) { px--; moved = true; }
      if (moved) { beep(900, 15); lastMove = now; }
    }

    if (px == ex && py == ey) {
      uint16_t secs = (millis() - startTime) / 1000;
      beep(1047, 80); delay(90);
      beep(1319, 80); delay(90);
      beep(1568, 200); delay(300);
      
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
// GAME: ROCK PAPER SCISSORS (RPS Game - Unlimited)
// ============================================================

void game_rps() {
  const char* moves[] = {"✊ ROCK", "✋ PAPER", "✌ SCISSORS"};
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
      if (btnPressed(BTN_UP)) { playerChoice = (playerChoice + 2) % 3; beep(600, 15); }
      if (btnPressed(BTN_DOWN)) { playerChoice = (playerChoice + 1) % 3; beep(600, 15); }
      
      if (btnPressed(BTN_ENTER)) {
        cpuChoice = random(0, 3);
        state = 1;
        resultTime = millis();
        beep(900, 30);
        totalRounds++;
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
      if (result == 0) { resultText = "DRAW!"; beep(500, 50); }
      else if (result == 1) { 
        resultText = "YOU WIN! 🎉"; 
        playerScore++;
        playerWins++;
        saveTotalGames(12);
        saveRPSWin(12);
        saveHighScore(12, playerScore);
        beep(1200, 40); delay(100); beep(1500, 40); 
      }
      else { 
        resultText = "CPU WINS! 😢"; 
        cpuScore++; 
        saveTotalGames(12);
        saveHighScore(12, playerScore);
        beep(300, 100); 
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
// GAME: CAR RACER (Full Width, Reduced Speed)
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
    lastFrame = now;
    
    if (btnPressed(BTN_LEFT)) targetLane = max(0, targetLane - 1);
    if (btnPressed(BTN_RIGHT)) targetLane = min(2, targetLane + 1);
    
    int targetX = LANE_START + targetLane * LANE_WIDTH + (LANE_WIDTH - CAR_W) / 2;
    playerX += (targetX - playerX) * 0.15f * dt;
    
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
        if (score % 5 == 0) beep(1500, 20);
        continue;
      }
      
      if (obs[i].y + 10 > SCREEN_H - 12 && obs[i].y < SCREEN_H - 6 + 6 &&
          obs[i].x < playerX + CAR_W && obs[i].x + 12 > playerX) {
        lives--;
        beep(200, 200);
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
// MAIN MENU - GAME SELECTION
// ============================================================

#define TOTAL_GAMES 14

int menuSelect() {
  const char *names[TOTAL_GAMES] = {
      "1. Asteroids", "2. Breakout", "3. Dino Run", "4. Flappy Bird",
      "5. Snake 1", "6. Snake 2", "7. Pong", "8. Pacman",
      "9. Space Invaders", "10. Tetris", "11. Tank Battle",
      "12. Maze Runner", "13. RPS Game", "14. Car Racer"
  };
  int sel = lastGameIndex;
  if (sel >= TOTAL_GAMES) sel = 0;
  int top = 0;
  const int VISIBLE = 4;

  while (true) {
    if (sel < top) top = sel;
    if (sel >= top + VISIBLE) top = sel - VISIBLE + 1;

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawBox(0, 0, SCREEN_W, 11);
    u8g2.setDrawColor(0);
    centreStr("🎮 GAME MENU 🎮", 9);
    u8g2.setDrawColor(1);

    for (int i = 0; i < VISIBLE; i++) {
      int idx = top + i;
      if (idx >= TOTAL_GAMES) break;
      int y = 14 + i * 13;
      if (idx == sel) {
        u8g2.drawRBox(0, y-1, SCREEN_W, 12, 2);
        u8g2.setDrawColor(0);
        u8g2.drawStr(6, y + 9, names[idx]);
        u8g2.setDrawColor(1);
      } else {
        u8g2.drawStr(6, y + 9, names[idx]);
      }
    }

    u8g2.drawFrame(SCREEN_W - 7, 12, 6, 52);
    int thumbH = max(6, 52 / TOTAL_GAMES * VISIBLE);
    int thumbY = 12 + (sel * (52 - thumbH)) / (TOTAL_GAMES - 1);
    u8g2.drawBox(SCREEN_W - 6, thumbY, 4, thumbH);

    if (top > 0) u8g2.drawStr(SCREEN_W - 8, 13, "^");
    if (top + VISIBLE < TOTAL_GAMES) u8g2.drawStr(SCREEN_W - 8, 62, "v");

    u8g2.sendBuffer();
    delay(100);
    
    if (btnPressed(BTN_UP)) { sel = (sel + TOTAL_GAMES - 1) % TOTAL_GAMES; beep(800, 25); }
    else if (btnPressed(BTN_DOWN)) { sel = (sel + 1) % TOTAL_GAMES; beep(800, 25); }
    else if (btnPressed(BTN_ENTER)) { beep(1200, 60); waitRelease(); return sel; }
    else if (btnPressed(BTN_MENU)) { playMenuButtonSound(); waitRelease(); return -1; }
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
  u8g2.setContrast(brightnessLevel);
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.setDrawColor(1);
  u8g2.setBitmapMode(0);
  showSplash();
}

void loop() {
  showMainMenu();
}
