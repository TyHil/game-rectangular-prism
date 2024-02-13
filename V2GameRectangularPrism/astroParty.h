/*
  AstroParty app header
  Written by Tyler Gordon Hill
*/
#include "shipAsteroidLaser.h"

class AstroParty {
  public:
    Ship ships[2];
    Asteroid asteroids[12];
    Laser lasers[2][2];
    uint64_t laserButtonTiming[2], shipTurnTiming[2], lastNoTurn[2], lastTurn[2], secondLastNoTurn[2], asteroidSpawn; //timers for button presses
    int64_t textDisplay;
    bool turnDir, win, winner;
    uint8_t textDisplayNum;
    uint64_t frameTimer;
    AstroParty();
    void setup();
    void displayStart(Adafruit_SSD1306& display);
    void turning();
    void move(Adafruit_SSD1306& display);
    void asteroidLaserCollision();
    void winCheck(Adafruit_SSD1306& display);
    void powerUpDisplay(Adafruit_SSD1306& display);
    void run(Adafruit_SSD1306& display);
};
