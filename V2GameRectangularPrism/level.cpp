/*
  Level app implementation
  Written by Tyler Gordon Hill
*/
#include <Arduino_LSM6DS3.h>

#include "level.h"



/* Level */

Level::Level() {
  pitch = 0;
  pitchCorrection = 0;
  IMU.begin();
}

float Level::getPitchWOCorrection() {
  if (IMU.accelerationAvailable()) {
    float accelX, accelY, accelZ;
    IMU.readAcceleration(accelX, accelY, accelZ);
    return accelX / sqrt(accelY * accelY + accelZ * accelZ); //slope of level line
  } else {
    return 0;
  }
}

float Level::getPitch() {
  return getPitchWOCorrection() - pitchCorrection;
}

void Level::correctPitch() {
  pitchCorrection = getPitchWOCorrection();
}

void Level::display(Adafruit_SSD1306 & display) {
  if (IMU.accelerationAvailable()) {
    display.clearDisplay();
    pitch = getPitch();
    int8_t angle = atan(pitch) * 180 / M_PI; //degrees
    display.setTextSize(4);
    display.setCursor(64 - 10 * (String(angle).length() + 1) - 2 * String(angle).length(), 16);
    display.print(angle);
    display.write(0xF8);
    for (uint8_t x = 0; x < 128; x++)
      for (int16_t y = min(max((pitch) * (x - 64) + 32, 0), 66) - 2; y < 64; y++) //for pixels past level line
        display.drawPixel(x, y, !display.getPixel(x, y)); //invert color
    display.display();
  }
}
