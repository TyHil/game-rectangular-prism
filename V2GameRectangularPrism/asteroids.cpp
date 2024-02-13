/*
  Asteroids app implementation
  Written by Tyler Gordon Hill
*/
#include "asteroids.h"
#include "helper.h"

void displayAsteroidsHighScores(Adafruit_SSD1306& display) { //displays list of high scores for Asteroids
  display.setTextSize(1); //top row text
  display.setCursor(30, 0);
  display.print(" High Scores");
  display.setCursor(0, 9);
  display.print("Place");
  display.setCursor(35, 9);
  display.print("Name");
  display.setCursor(62, 9);
  display.print("Level");
  display.setCursor(95, 9);
  display.print("Score");
  const uint8_t xVals[5] = {35, 41, 47, 62, 95};
  for (uint8_t i = 0; i < 5; i++) { //top 5 high scores
    display.setCursor(0, 10 + 9 * (i + 1));
    display.print(i + 1); //place
    for (uint8_t j = 0; j < 5; j++) {
      display.setCursor(xVals[j], 10 + 9 * (i + 1));
      if (j < 3) display.print(char(readEEPROM(5 * i + j))); //name
      else if (j == 3) {
        display.print(readEEPROM(5 * i + 3)); //level
        if (readEEPROM(5 * i + 4) >= 7 * readEEPROM(5 * i + 3) + 6) display.print("W"); //win/loss
        else display.print("L");
      } else display.print(readEEPROM(5 * i + j)); //score
    }
  }
}

void displayAsteroidsLevel(Adafruit_SSD1306& display, uint8_t _level) {
  display.setTextSize(2);
  display.setCursor(40, 0);
  display.print("Level");
  display.setTextSize(4);
  display.setCursor(40, 20);
  display.print(_level);
  display.setTextSize(1);
  display.setCursor(20, 55);
  display.print("Max Score: ");
  display.print(12 * _level + 15);
}

void newHighScore(Adafruit_SSD1306& display, int8_t level, uint8_t score) { //sets a new high score for Asteroids
  int8_t i; //move lesser high scores down
  for (i = 3; i >= 0; i--) if (score > readEEPROM(5 * i + 4) or (score == readEEPROM(5 * i + 4) and level > readEEPROM(5 * i + 3))) for (uint8_t j = 0; j < 5; j++) updateEEPROM(5 * (i + 1) + j, readEEPROM(5 * i + j));
    else break;
  i++;
  uint8_t name[3] = {65, 65, 65}; //name storage
  uint64_t generalTimer = 0;
  for (uint8_t k = 0; k < 3; k++) { //name choice
    delay(200);
    while (digitalRead(2) == 0 and digitalRead(3) == 0) {
      display.clearDisplay(); //print directions
      display.setTextSize(2);
      display.setCursor(45, 0);
      display.print("New");
      display.setCursor(15, 21);
      display.print("High Score");
      display.setCursor(45, 42);
      for (uint8_t l = 0; l < 3; l++) { //pring current name
        display.print(char(name[l]));
      }
      display.fillRect(12 * k + 44, 41, 12, 16, WHITE); //fill current selection
      display.setCursor(12 * k + 45, 42);
      display.setTextColor(BLACK);
      display.print(char(name[k]));
      display.setTextColor(WHITE);
      if (millis() - generalTimer >= 100) { //move current selection down
        if (digitalRead(5)) {
          name[k] = max(name[k] - 1, 65);
          generalTimer = millis();
        } else if (digitalRead(4)) { //up
          name[k] = min(name[k] + 1, 90);
          generalTimer = millis();
        }
      }
      display.display();
    }
  }
  for (uint8_t j = 0; j < 3; j++) updateEEPROM(5 * i + j, name[j]); //set new high score in EEPROM
  updateEEPROM(5 * i + 3, level);
  updateEEPROM(5 * i + 4, score);
  display.clearDisplay();
  displayAsteroidsHighScores(display); //display high scores
  display.display();
  delay(100);
  waitAnyClick();
  delay(100);
}



/* Asteroids */

Asteroids::Asteroids(bool setTiltToTurn, uint8_t setLevel) {
  tiltToTurn = setTiltToTurn;
  level = setLevel;
  score = 0;
  ship = Ship(64, 32, M_PI, 0);
  for (uint8_t i = 0; i < 2; i++) asteroids[i] = Asteroid(8, 0, 0);
  for (uint8_t i = 2; i < level + 2; i++) asteroids[i] = Asteroid(16, 0, 0);
  for (uint8_t i = level + 2; i < 2 * (level + 1); i++) asteroids[i] = Asteroid(0, 0, 0);
  laserButtonTiming = millis();
  shipTurnTiming = millis();
  scoreTime = millis();
  frameTimer = millis();
  Level tilt;
  if (tiltToTurn) {
    tilt = Level();
    tilt.correctPitch();
  }
}

void Asteroids::turning() {
  if (millis() - shipTurnTiming >= 50) {
    if (tiltToTurn) { //turning
      int8_t angle = atan(tilt.getPitch()) * 180 / M_PI; //degrees
      if (angle > 2 or angle < -2) {
        ship.turn((M_PI * min(angle, 20)) / (8 * 20));
      }
      shipTurnTiming = millis();
    } else {
      if (digitalRead(5)) {
        ship.turn(M_PI / 8);
        shipTurnTiming = millis();
      } else if (digitalRead(4)) {
        ship.turn(M_PI / -8);
        shipTurnTiming = millis();
      }
    }
  }
}

void Asteroids::move(Adafruit_SSD1306& display) {
  ship.moveAndDisplay(digitalRead(2), digitalRead(tiltToTurn ? 5 : 3), display); //only move if button pressed
  for (uint8_t i = 0; i < 2 * (level + 1); i++) {
    asteroids[i].moveAndDisplay(display);
  }
}

void Asteroids::asteroidLaserCollision() {
  for (uint8_t i = 0; i < 2 * (level + 1); i++) {
    if (asteroids[i].Size != 0) {
      for (uint8_t m = 0; m < 2; m++) { //every laser
        if (ship.lasers[m].readyToMove()) {
          for (uint8_t n = 0; n < 2; n++) { //every point on every laser
            for (uint8_t o = 0; o < 2; o++) {
              if (asteroids[i].pointInAsteroid(ship.lasers[m].X + n, ship.lasers[m].Y + o)) {
                ship.lasers[m].hit = true;
                asteroids[i].hit(ship.lasers[m].dir);
                if (asteroids[i].Size == 8) { //spilts and a new asteroid needs to be created
                  score += 1;
                  uint8_t l;
                  for (l = 0; asteroids[l].Size != 0; l++) {}
                  asteroids[l].Size = 8;
                  asteroids[l].X = asteroids[i].X + 8;
                  asteroids[l].Y = asteroids[i].Y + 8;
                  asteroids[l].dir = asteroids[i].dir - M_PI;
                } else score += 3;
              }
            }
          }
        }
      }
    }
  }
}

void Asteroids::winCheck(Adafruit_SSD1306& display) { //win check: all asteroids have size 0
  uint8_t i;
  for (i = 0; i < 2 * (level + 1) and asteroids[i].Size == 0; i++) {}
  if (i >= 2 * (level + 1)) { //uesd to be > 21
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(45, 0);
    display.print("You");
    display.setCursor(45, 21);
    display.print("Win");
    display.setCursor(45, 42);
    score += max(map(millis() - scoreTime, 0, 60000, level * 5 + 9, 0), 0);
    display.print(score);
    display.display();
    delay(500);
    waitAnyClick();
    delay(500);
    if (score > readEEPROM(24) or (score == readEEPROM(24) and level > readEEPROM(23))) newHighScore(display, level, score);
    resetFunc();
  }
}

void Asteroids::loseCheck(Adafruit_SSD1306& display) {
  for (uint8_t i = 0; i < 2 * (level + 1); i++) {
    bool over = false;
    for (uint8_t m = 0; m < 3; m++) if (asteroids[i].pointInAsteroid(ship.XPoints[m][m], ship.YPoints[m][m])) over = true; //if any ship corners
    if (over or asteroids[i].pointInAsteroid(ship.X, ship.Y)) { //or the ship center are in an asteroid
      display.display();
      display.setTextSize(2);
      display.setCursor(40, 0);
      display.print("Game");
      display.setCursor(40, 21);
      display.print("Over");
      display.setCursor(45, 42);
      display.print(score);
      display.display();
      delay(500);
      waitAnyClick();
      delay(500);
      if (score > readEEPROM(24)) newHighScore(display, level, score);
      resetFunc();
    }
  }
}

void Asteroids::run(Adafruit_SSD1306& display) {
  while (true) {
    display.clearDisplay();
    turning();
    move(display);
    asteroidLaserCollision();
    winCheck(display);
    loseCheck(display);
    display.display();
    while (millis() - frameTimer < 40) {} //regulate to 25 fps on different levels
    frameTimer = millis();
  }
}
