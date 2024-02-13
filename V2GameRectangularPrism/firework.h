/*
  Firework library header for Fireworks
  Written by Tyler Gordon Hill
*/
#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

class Spark {
public:
  bool offScreen = 0;
  float X, Y, XVelocity, YVelocity; //position, velocity
  Spark();
  void reset();
  bool moveAndDisplay(uint8_t fireworkX, float fireworkY, Adafruit_SSD1306 & display);
};

class Firework {
public:
  bool offScreen = 1;
  uint8_t X; //X position
  float Y, YVelocity;
  Spark sparks[30];
  Firework();
  void reset();
  void moveAndDisplay(Adafruit_SSD1306 & display);
};
