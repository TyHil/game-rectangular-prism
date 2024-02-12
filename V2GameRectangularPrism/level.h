/*
  Level app header
  Written by Tyler Gordon Hill
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class Level {
  public:
    float pitch, pitchCorrection; //calibration
    Level();
    float getPitchWOCorrection();
    float getPitch();
    void correctPitch();
    void display(Adafruit_SSD1306& display);
};
