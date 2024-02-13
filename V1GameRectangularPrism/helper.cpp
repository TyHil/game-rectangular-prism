/*
  Helper functions implementation
  Written by Tyler Gordon Hill
*/
#include "helper.h"
#include <Arduino.h>



/* Reset Function */

void (*resetFunc) (void) = nullptr;



/* Wait Functions */

void waitAnyClick() { //waits until any button is pressed
  while (digitalRead(2) == 1 and digitalRead(3) == 1 and digitalRead(4) == 1 and digitalRead(5) == 1) {}
}

void waitAllUnclick() { //waits until none of the buttons are clicked
  while (digitalRead(2) == 0 or digitalRead(3) == 0 or digitalRead(4) == 0 or digitalRead(5) == 0) {}
}
