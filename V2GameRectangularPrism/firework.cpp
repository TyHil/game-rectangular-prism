/*
  Firework library implementation for Fireworks
  Written by Tyler Gordon Hill
*/
#include "firework.h"



/* Spark */

Spark::Spark() {
  offScreen = 1;
}

void Spark::reset() {
  X = 0;
  Y = 0;
  float dir = random(0, 2 * M_PI * 100) / 100.0;
  float mag = random(100, 400) / 100.0;
  XVelocity = mag * cos(dir);
  YVelocity = mag * sin(dir);
  offScreen = 0;
}

bool Spark::moveAndDisplay(uint8_t fireworkX, float fireworkY, Adafruit_SSD1306& display) {
  if (offScreen) {
    return 1;
  }
  display.drawPixel(fireworkX + X, fireworkY + Y, WHITE);
  if (fireworkX + X < 0 or fireworkX + X > 127 or fireworkY + Y > 63) {
    offScreen = 1;
  }
  X += XVelocity; //change position
  Y += YVelocity;
  XVelocity *= .9;
  YVelocity *= .9;
  return 0;
}



/* Firework */

Firework::Firework() {
  offScreen = 1;
  for (uint8_t i = 0; i < 30; i++) {
    sparks[i] = Spark();
  }
}

void Firework::reset() {
  X = random(0, 128);
  Y = random(0, 64);
  YVelocity = 0;
  offScreen = 0;
  for (uint8_t i = 0; i < 30; i++) {
    sparks[i].reset();
  }
}

void Firework::moveAndDisplay(Adafruit_SSD1306& display) {
  if (!offScreen) {
    uint8_t sparksOffScreen = 0;
    for (uint8_t i = 0; i < 30; i++) {
      sparksOffScreen += sparks[i].moveAndDisplay(X, Y, display);
    }
    if (sparksOffScreen >= 30) {
      offScreen = 1;
    }
    YVelocity += 0.1;
    Y += YVelocity;
  }
}
