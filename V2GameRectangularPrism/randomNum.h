/*
  Random Number Generator app header
  Written by Tyler Gordon Hill
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class RandomNum {
  public:
    int16_t nums[3]; //min, max, and number of decimal places
    float result;
    RandomNum();
    void incNums(int16_t screen);
    void decNums(int16_t screen);
    void generate();
    void display(Adafruit_SSD1306& display, int16_t screen);
};
