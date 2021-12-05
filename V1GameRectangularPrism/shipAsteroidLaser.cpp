/*
  Ship, asteroid, and laser, library implementation for Asteroids and Astro Party
  Written by Tyler Gordon Hill
*/
#include "shipAsteroidLaser.h"

/*Functions*/

float floatMod(float val, int mod) { //modulus for floating point numbers
  return (val > 0) * (val - mod * (int) (val / mod)) + (val < 0) * (val + mod * (int) (-1 * val / mod + 1));
}
float area(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3) { //Calculates the area of a triangle for laser ship collision detection
  return abs((x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2.0);
}

/*Ship Methods*/

ship::ship() {}
ship::ship(float setX, float setY, float setDir, bool setColor) {
  X = setX;
  Y = setY;
  dir = setDir;
  color = setColor;
}
void ship::CWTurn() {
  dir += M_PI / 8;
}
void ship::CCWTurn() {
  dir -= M_PI / 8;
}
void ship::moveAndDisplay(bool changePos, bool lasersReadyToShoot[2], Adafruit_SSD1306& display) {
  if (changePos) {
    XVelocity += sin(dir) / 2;
    YVelocity += cos(dir) / 2;
  }
  XVelocity *= .8; //limits speed like constant friction
  YVelocity *= .8;
  X = floatMod(X + XVelocity, 128); //move by speed and loop
  Y = floatMod(Y + YVelocity, 64);
  const float changesX[3] = {sin(dir) * 9, sin(dir + 2.09) * 6, sin(dir + 4.18) * 6,}, changesY[3] = {cos(dir) * 9, cos(dir + 2.09) * 6, cos(dir + 4.18) * 6}; //numbers used a lot
  for (uint8_t i = 0; i < 3; i++) { //same 2nd and 3rd dimension calulations
    XPoints[i][i] = floatMod(X + changesX[i], 128);
    YPoints[i][i] = floatMod(Y + changesY[i], 64);
  }
  for (uint8_t i = 0; i < 3; i++) { //different 2nd and 3rd dimension calulated based off of same 2nd and 3rd dimensions
    for (uint8_t j = 0; j < 3; j++) {
      if (i != j) {
        XPoints[i][j] = XPoints[j][j] - changesX[j] + changesX[i];
        YPoints[i][j] = YPoints[j][j] - changesY[j] + changesY[i];
      }
    }
  }
  for (uint8_t i = 0; i < 3; i++) { //display ship triangles
    if (color) display.fillTriangle(XPoints[0][i], YPoints[0][i], XPoints[1][i], YPoints[1][i], XPoints[2][i], YPoints[2][i], WHITE);
    else display.drawTriangle(XPoints[0][i], YPoints[0][i], XPoints[1][i], YPoints[1][i], XPoints[2][i], YPoints[2][i], WHITE);
    if (changePos) {
      display.drawLine(.33 * XPoints[2][i] + .66 * XPoints[1][i], .33 * YPoints[2][i] + .66 * YPoints[1][i], (XPoints[1][i] + XPoints[2][i]) / 2 - sin(dir) * 5, (YPoints[1][i] + YPoints[2][i]) / 2 - cos(dir) * 5, WHITE);
      display.drawLine(.66 * XPoints[2][i] + .33 * XPoints[1][i], .66 * YPoints[2][i] + .33 * YPoints[1][i], (XPoints[1][i] + XPoints[2][i]) / 2 - sin(dir) * 5, (YPoints[1][i] + YPoints[2][i]) / 2 - cos(dir) * 5, WHITE);
    }
  }
  for (uint8_t i = 0; i < 2; i++) { //display lasers in ship
    if (lasersReadyToShoot[i]) display.drawPixel(i * floatMod(X + changesX[1] / 3, 128) + (-i + 1) * floatMod(X + changesX[2] / 3, 128), i * floatMod(Y + changesY[1] / 3, 64) + (-i + 1) * floatMod(Y + changesY[2] / 3, 64), -color + 1);
  }
  delete lasersReadyToShoot;
}
bool ship::pointInShip(uint8_t XGiven, uint8_t YGiven) {
  bool hit = 0;
  for (uint8_t i = 0; i < 3; i++) {
    float sum = 0;
    for (uint8_t j = 0; j < 3; j++) sum += area(XGiven, YGiven, XPoints[j][i], YPoints[j][i], XPoints[(j + 1) % 3][i], YPoints[(j + 1) % 3][i]);
    if (sum <= area(XPoints[0][i], YPoints[0][i], XPoints[1][i], YPoints[1][i], XPoints[2][i], YPoints[2][i])) hit = 1; //calculated by adding the areas of three triangles (from the laser and each set of 2 ship points) and seeing if that is less than or equal to the area of the ship
  }
  return hit;
}

/*Asteroid Methods*/

asteroid::asteroid() {}
asteroid::asteroid(uint8_t setSize, bool fullscreen) {
  Size = setSize;
  if (setSize != 0) {
    dir = random(0, 629) / 100.0; //random dir
    if (fullscreen) X = random(0, 128); //random anywhere
    else X = random(96, 160 - setSize) % 128; //random avoiding center
    Y = random(0, 64);
  }
}
bool asteroid::hit(float hitDir) {
  if (Size == 16) { //split the asteroid if it's big
    Size = 8;
    dir = (hitDir + dir) / 2;
    return 1;
  } else { //remove asteroid
    Size = 0;
    return 0;
  }
}
void asteroid::moveAndDisplay(Adafruit_SSD1306& display) {
  if (Size) {
    X = floatMod(X + sin(dir) / 2, 128); //change position
    Y = floatMod(Y + cos(dir) / 2, 64);
    display.drawRect(X, Y, Size, Size, WHITE);
    if (X > 128 - Size)display.drawRect(-1, Y, Size - (128 - X) + 1, Size, WHITE); //draw 2 squares if its over the edge one for each side
    if (Y > 64 - Size)display.drawRect(X, -1, Size, Size - (64 - Y) + 1, WHITE);
    if (X > 128 - Size and Y > 64 - Size) display.drawRect(-1, -1, Size - (128 - X) + 1, Size - (64 - Y) + 1, WHITE); //draw 4 squares if its over both edges one for each side
  }
}
bool asteroid::pointInAsteroid(uint8_t XGiven, uint8_t YGiven) { //point in square
  return ((X + Size > 128 and (XGiven >= 0 and XGiven < (uint8_t) (X + Size) % 128 or XGiven >= X and XGiven < 128) or X + Size <= 128 and (XGiven >= X and XGiven < X + Size)) and/*big and between X and Y*/ (Y + Size > 64 and (YGiven >= 0 and YGiven < (uint8_t) (Y + Size) % 64 or YGiven >= Y and YGiven < 64) or Y + Size <= 64 and (YGiven >= Y and YGiven < Y + Size)));
}

/*Laser Methods*/

void laser::setUp(float setDir, uint8_t setX, uint8_t setY, float setXVelocity, float setYVelocity) {
  dir = setDir;
  Time = millis();
  hit = false;
  X = setX;
  Y = setY;
  XDist = 0;
  YDist = 0;
  XVelocity = setXVelocity;
  YVelocity = setYVelocity;

}
bool laser::readyToShoot() { //ready to shoot every second
  return millis() - Time > 1000;
}
bool laser::readyToMove() { //ready to move when unhit and travelled less than 64 pixels
  return !hit and sqrt(pow(XDist, 2) + pow(YDist, 2)) <= 64;
}
void laser::moveAndDisplay(Adafruit_SSD1306& display) {
  X = (uint8_t) (X + sin(dir) * 5 + XVelocity) % 128; //move laser
  Y = (uint8_t) (Y + cos(dir) * 5 + YVelocity) % 64;
  XDist = (int8_t) (XDist + sin(dir) * 5); //move limit for how far it can go
  YDist = (int8_t) (YDist + cos(dir) * 5);
  display.fillRect(X, Y, 2, 2, WHITE);
}
