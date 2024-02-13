/*
  Fireworks app header
  Written by Tyler Gordon Hill
*/
#include "firework.h"

class Fireworks {
public:
  uint64_t betweenTimer;
  uint8_t betweenDur;
  Firework fireworks[10];
  Fireworks();
  void display(Adafruit_SSD1306 & display);
};
