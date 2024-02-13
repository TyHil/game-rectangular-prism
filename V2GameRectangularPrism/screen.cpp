/*
  Screen implementation
  Written by Tyler Gordon Hill
*/
#include "screen.h"



/* Screen */

Screen::Screen(int16_t _maxScreen, int16_t start) {
  minScreen = 0;
  maxScreen = _maxScreen;
  screen = start;
  generalTimer = millis();
}

Screen::Screen(int16_t _minScreen, int16_t _maxScreen, int16_t start) {
  minScreen = _minScreen;
  maxScreen = _maxScreen;
  screen = start;
  generalTimer = millis();
}

bool Screen::buttons() {
  if (millis() - generalTimer >= 100) {
    if (digitalRead(4)) {
      screen = min(screen + 1, maxScreen);
      generalTimer = millis();
    } else if (digitalRead(5)) {
      screen = max(screen - 1, minScreen);
      generalTimer = millis();
    }
    return true;
  }
  return false;
}
