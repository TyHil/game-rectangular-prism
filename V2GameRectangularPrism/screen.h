/*
  Screen header
  Written by Tyler Gordon Hill
*/
#pragma once
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class Screen {
  public:
    int16_t screen, maxScreen;
    uint64_t generalTimer;
    Screen(int16_t _maxScreen, int16_t start);
    bool buttons();
};
