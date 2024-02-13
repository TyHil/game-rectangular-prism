/*
  Asteroids app header
  Written by Tyler Gordon Hill
*/
#include "shipAsteroidLaser.h"
#include "level.h"
#define MAX_LEVEL 20

class Asteroids {
  public:
    bool tiltToTurn;
    uint8_t level;
    uint8_t score;
    Ship ship;
    Asteroid asteroids[2 * MAX_LEVEL + 1];
    Laser lasers[2];
    uint64_t laserButtonTiming, shipTurnTiming, scoreTime; //timer for button presses and score
    uint64_t frameTimer;
    Level tilt;
    Asteroids();
    void setup(uint8_t setLevel);
    void displayHighScores(Adafruit_SSD1306& display);
    void displaySettings(Adafruit_SSD1306& display);
    void displayLevel(Adafruit_SSD1306& display, uint8_t _level);
    void newHighScore(Adafruit_SSD1306& display, int8_t level, uint8_t score);
    void turning();
    void move(Adafruit_SSD1306& display);
    void asteroidLaserCollision();
    void winCheck(Adafruit_SSD1306& display);
    void loseCheck(Adafruit_SSD1306& display);
    void run(Adafruit_SSD1306& display);
};
