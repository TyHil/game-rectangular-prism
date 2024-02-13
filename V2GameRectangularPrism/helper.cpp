/*
  Helper functions implementation
  Written by Tyler Gordon Hill
*/
#include "helper.h"
#include <Wire.h>



/* Wait Functions */

void waitAnyClick() { //waits until any button is pressed
  while (digitalRead(2) == 0 and digitalRead(3) == 0 and digitalRead(4) == 0 and digitalRead(5) == 0) {}
}

void waitAllUnclick() { //waits until none of the buttons are clicked
  while (digitalRead(2) or digitalRead(3) or digitalRead(4) or digitalRead(5)) {}
}



/* EEPROM Functions */

uint8_t readEEPROM(unsigned int location) { //read the value of a spot in EEPROM
  Wire.beginTransmission(0x50);
  Wire.write((int)(location >> 8)); //MSB
  Wire.write((int)(location & 0xFF)); //LSB
  Wire.endTransmission();
  Wire.requestFrom(0x50, 1);
  if (Wire.available()) return Wire.read();
  else return 0;
}

void updateEEPROM(unsigned int location, uint8_t data) { //update the value of a spot in EEPROM
  if (readEEPROM(location) != data) { //avoid writing if data is the same
    Wire.beginTransmission(0x50);
    Wire.write((int)(location >> 8)); //MSB
    Wire.write((int)(location & 0xFF)); //LSB
    Wire.write(data);
    Wire.endTransmission();
    delay(5);
  }
}



/* Board */

void grid(uint8_t XDim, uint8_t YDim, Adafruit_SSD1306& display) { //draw grid for Clonium or Minesweeper
  const boolean area = XDim > 8 or YDim > 4;
  for (uint8_t i = 0; i <= XDim * (-8 * area + 16); i += 16 / (area + 1)) display.drawLine(i, 0, i, YDim * (-8 * area + 16), WHITE);
  for (uint8_t i = 0; i <= YDim * (-8 * area + 16); i += 16 / (area + 1)) display.drawLine(0, i, XDim * (-8 * area + 16), i, WHITE);
  if (XDim % 8 == 0) display.drawLine(127, 0, 127, YDim * (-8 * area + 16), WHITE); //rightmost line
  if (YDim % 4 == 0) display.drawLine(0, 63, XDim * (-8 * area + 16), 63, WHITE); //bottom line
}
