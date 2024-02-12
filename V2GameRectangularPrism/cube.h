/*
  Cube app header
  Written by Tyler Gordon Hill
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class Cube {
  public:
    float rx, ry, rz;
    Cube();
    float mxx(float a, float x, float y, float z);
    float mxy(float a, float x, float y, float z);
    float mxz(float a, float x, float y, float z);
    float myx(float a, float x, float y, float z);
    float myy(float a, float x, float y, float z);
    float myz(float a, float x, float y, float z);
    float mzx(float a, float x, float y, float z);
    float mzy(float a, float x, float y, float z);
    float mzz(float a, float x, float y, float z);
    void display(Adafruit_SSD1306& display);
    void increment();
    void run(Adafruit_SSD1306& display);
};
