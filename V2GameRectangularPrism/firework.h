/*
  Firework library header for Fireworks
  Written by Tyler Gordon Hill
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class spark {
   public:
    bool offScreen = 0;
    float X, Y, XVelocity, YVelocity; //position, velocity
    spark();
    void reset();
    bool moveAndDisplay(uint8_t fireworkX, float fireworkY, Adafruit_SSD1306& display);
};

class firework {
  public:
    bool offScreen = 1;
    uint8_t X; //X position
    float Y, YVelocity;
    spark sparks[30];
    firework();
    void reset();
    void moveAndDisplay(Adafruit_SSD1306& display);
};
