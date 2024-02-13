/*
  Fireworks app implementation
  Written by Tyler Gordon Hill
*/
#include "fireworks.h"



/* Fireworks */

Fireworks::Fireworks() {
  betweenTimer = millis();
  betweenDur = 0;
  Firework fireworks[10]; // = Firework(random(0, 128), random(0, 64));
  for (uint8_t i = 0; i < 10; i++) fireworks[i] = Firework();
}

void Fireworks::display(Adafruit_SSD1306 & display) {
  if (millis() - betweenTimer >= betweenDur * 10) {
    uint8_t i;
    for (i = 0; i < 10 and !fireworks[i].offScreen; i++) {}
    if (i < 10) fireworks[i].reset();
    betweenDur = random(0, 50);
    betweenTimer = millis();
  }
  for (uint8_t i = 0; i < 10; i++) fireworks[i].moveAndDisplay(display);
}
