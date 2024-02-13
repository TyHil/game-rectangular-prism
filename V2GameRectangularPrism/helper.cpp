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
