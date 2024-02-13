/*
  Helper functions header
  Written by Tyler Gordon Hill
*/
#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>



/* Reset Function */

void resetFunc();



/* Wait Functions */

void waitAnyClick();
void waitAllUnclick();



/* EEPROM Functions */

uint8_t readEEPROM(unsigned int location);
void updateEEPROM(unsigned int location, uint8_t data);



/* Board */

void grid(uint8_t XDim, uint8_t YDim, Adafruit_SSD1306& display);
