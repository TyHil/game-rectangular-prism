/*
  Cube app header
  Written by Tyler Gordon Hill
*/
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

enum Dimension { xx, xy, xz, yx, yy, yz, zx, zy, zz };

class Cube {
public:
  float rx, ry, rz;
  Cube();
  float matrix(Dimension dimension, float a, float x, float y, float z);
  void display(Adafruit_SSD1306 & display);
  void increment();
  void run(Adafruit_SSD1306 & display);
};
