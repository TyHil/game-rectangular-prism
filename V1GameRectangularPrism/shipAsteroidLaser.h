/*
  Ship, asteroid, and laser, library header for Asteroids and Astro Party
  Written by Tyler Gordon Hill
*/
#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

class Laser {
public:
  float dir, XVelocity, YVelocity; //direction, velocity
  uint8_t X, Y; //position
  int8_t XDist, YDist; //distance away detection
  uint64_t shotTime; //for timing between shots
  bool hit = true; //has the laser hit something
  Laser();
  void setUp(float setDir, uint8_t setX, uint8_t setY, float setXVelocity, float setYVelocity);
  bool readyToShoot();
  bool readyToMove();
  void moveAndDisplay(Adafruit_SSD1306 & display);
};

class Ship {
public:
  float X, Y, dir, dirV, XVelocity = 0, YVelocity = 0; //position, direction, rotaion velocity, movement velocities
  int16_t XPoints[3][3], YPoints[3][3]; //Points of 3 ship triangles
  uint8_t power = 0;
  uint64_t flash;
  bool color;
  uint64_t shotTime; //for timing between shots
  Laser lasers[2];
  Ship();
  Ship(float setX, float setY, float setDir, bool setColor);
  void turn(float amount);
  void boost(bool CW);
  void moveAndDisplay(bool changePos, bool shoot, Adafruit_SSD1306 & display);
  bool pointInShip(uint8_t XPoint, uint8_t YPoint);
};

class Asteroid {
public:
  float X, Y, dir; //position, direction
  uint8_t Size; //size
  bool power; //powerup
  Asteroid();
  Asteroid(uint8_t setSize, bool setPower, bool fullscreen);
  uint8_t hit(float hitDir);
  void moveAndDisplay(Adafruit_SSD1306 & display);
  bool pointInAsteroid(uint8_t XPoint, uint8_t YPoint);
};
