/*
  AstroParty app header
  Written by Tyler Gordon Hill
*/
#include "shipAsteroidLaser.h"

void displayAstroPartyStart(Adafruit_SSD1306& display);

class AstroParty {
  public:
    Ship ships[2];
    Asteroid asteroids[12];
    uint64_t shipTurnTiming[2], lastNoTurn[2], lastTurn[2], secondLastNoTurn[2], asteroidSpawn; //timers for button presses
    int64_t textDisplay;
    bool turnDir, win, winner;
    uint8_t textDisplayNum;
    uint64_t frameTimer;
    AstroParty();
    void turning();
    void move(Adafruit_SSD1306& display);
    void asteroidLaserCollision();
    void winCheck(Adafruit_SSD1306& display);
    void powerUpDisplay(Adafruit_SSD1306& display);
    void run(Adafruit_SSD1306& display);
};
