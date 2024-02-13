/*
  Screen header
  Written by Tyler Gordon Hill
*/
#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

class Screen {
public:
  int16_t screen, minScreen, maxScreen;
  uint64_t generalTimer;
  Screen(int16_t _maxScreen, int16_t start);
  Screen(int16_t _minScreen, int16_t _maxScreen, int16_t start);
  bool buttons();
};
