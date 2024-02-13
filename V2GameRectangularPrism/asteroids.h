/*
  Asteroids app header
  Written by Tyler Gordon Hill
*/
#include "level.h"
#include "shipAsteroidLaser.h"
#define MAX_LEVEL 20

void displayAsteroidsHighScores(Adafruit_SSD1306 & display);
void displayAsteroidsLevel(Adafruit_SSD1306 & display, uint8_t _level);

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
  Asteroids(bool setTiltToTurn, uint8_t setLevel);
  void turning();
  void move(Adafruit_SSD1306 & display);
  void asteroidLaserCollision();
  void winCheck(Adafruit_SSD1306 & display);
  void loseCheck(Adafruit_SSD1306 & display);
  void run(Adafruit_SSD1306 & display);
};
