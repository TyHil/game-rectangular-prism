/*
  Helper functions header
  Written by Tyler Gordon Hill
*/
#pragma once
#include <Arduino.h>



/* Wait Functions */

void waitAnyClick();
void waitAllUnclick();



/* EEPROM Functions */

uint8_t readEEPROM(unsigned int location);
void updateEEPROM(unsigned int location, uint8_t data);
