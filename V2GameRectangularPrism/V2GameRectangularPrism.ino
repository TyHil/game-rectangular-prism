/*
  Game Rectangular Prism Version 2 Code
  Asteroids, Astro Party, Clonium, and Minesweeper games and Random Number Generator
  and Level utilities on an Arduino Nano 33 IoT with a 64x128 OLED screen, 4 capacitive
  touch buttons, an EEPROM module, a 9v battery, and a power switch all in a black 3D
  printed case.
  Written by Tyler Gordon Hill
  Version 6.2
*/
#include "shipAsteroidLaser.h"
#include "boards.h"
#include "firework.h"
#include <Arduino_LSM6DS3.h>
Adafruit_SSD1306 display(128, 64, &Wire, -1);
uint8_t game; //which game is being played
boolean disp = true; //limits display refreshes when nothing has changed



/* Wait Functions */

void waitAnyClick() { //waits until any button is pressed
  while (digitalRead(2) == 0 and digitalRead(3) == 0 and digitalRead(4) == 0 and digitalRead(5) == 0) {}
}
void waitAllUnclick() { //waits until none of the buttons are clicked
  while (digitalRead(2) or digitalRead(3) or digitalRead(4) or digitalRead(5)) {}
}



/* EEPROM Functions */

byte readEEPROM(unsigned int location) { //read the value of a spot in EEPROM for highscores and last game played
  Wire.beginTransmission(0x50);
  Wire.write((int)(location >> 8)); //MSB
  Wire.write((int)(location & 0xFF)); //LSB
  Wire.endTransmission();
  Wire.requestFrom(0x50, 1);
  if (Wire.available()) return Wire.read();
  else return 0;
}
void updateEEPROM(unsigned int location, byte data) { //update the value of a spot in EEPROM for hghscores and last game played
  if (readEEPROM(location) != data) { //avoid writing if data is the same
    Wire.beginTransmission(0x50);
    Wire.write((int)(location >> 8)); //MSB
    Wire.write((int)(location & 0xFF)); //LSB
    Wire.write(data);
    Wire.endTransmission();
    delay(5);
  }
}



/* Asteroids Functions */

void high() { //displays list of high scores for Asteroids
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
void newhigh(int8_t level, uint8_t score) { //sets a new high score for Asteroids
  int8_t i; //move lesser high scores down
  for (i = 3; i >= 0; i--) if (score > readEEPROM(5 * i + 4) or (score == readEEPROM(5 * i + 4) and level > readEEPROM(5 * i + 3))) for (uint8_t j = 0; j < 5; j++) updateEEPROM(5 * (i + 1) + j, readEEPROM(5 * i + j));
    else break;
  i++;
  uint8_t name[3] = {65, 65, 65}; //name storage
  unsigned long generalTimer = 0;
  for (uint8_t k = 0; k < 3; k++) { //name choice
    delay(200);
    while (digitalRead(2) == 0 and digitalRead(3) == 0) {
      display.clearDisplay(); //print directions
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
  high(); //display high scores
  display.display();
  delay(100);
  waitAnyClick();
  delay(100);
}



/* Game Selection Functions */

void gameChangerDisplay() { //displays menu to change between different games
  const String names[8] = {"Switch Game", "Asteroids", "Astro Party", "Clonium", "Minesweeper", "Random Num", "Level", "Fireworks"}; //game names
  display.setTextSize(1);
  for (uint8_t i = 0; i < 8; i++) { //game list
    display.setCursor((i == 0) ? 30 : ((i < 7) ? 1 : 68), (i == 0) ? 0 : (9 * ((i - 1) % 6) + 9));
    display.print(names[i]);
  }
  display.fillRect((game < 6) ? 0 : 67, 9 * (game % 6 + 1) - 1, 67, 9, WHITE); //highlight current selection
  display.setCursor((game < 6) ? 1 : 68, 9 * (game % 6 + 1));
  display.setTextColor(BLACK);
  display.print(names[game + 1]);
  display.setTextColor(WHITE);
}
void gameChanger() { //update game selection and restart arduino on choice
  if (digitalRead(5)) {
    game = (game + 1) % 7;
    disp = true;
  } else if (digitalRead(2) or digitalRead(3)) {
    updateEEPROM(25, game);
    digitalWrite(6, LOW);
  }
}



/* IMU Functions */

float getPitch() {
  if (IMU.accelerationAvailable()) {
    float accelX, accelY, accelZ;
    IMU.readAcceleration(accelX, accelY, accelZ);
    return accelX / sqrt(accelY * accelY + accelZ * accelZ); //slope of level line
  } else {
    return 0;
  }
}



/* Device Start */

void setup() {
  digitalWrite(6, HIGH);
  pinMode(6, OUTPUT);
  //Serial.begin(9600); //Serial.println("");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Wire.begin();
  randomSeed(analogRead(1)); //better random
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  display.setTextColor(WHITE);
  game = readEEPROM(25); //read game last played
  delay(100);



  /*Asteroids*/

  if (game == 0) {
    int8_t level = 1;
    unsigned long generalTimer;
    bool tiltToTurn = 0;

    /*Setup*/
    while ((digitalRead(2) == 0 and digitalRead(3) == 0) or level == 0 or level == -1 or level == -2) { //menu and level choice
      if (disp) { //only display if something changes
        disp = false;
        display.clearDisplay();
        if (level == -1) high(); //highscores
        else if (level == -2) gameChangerDisplay(); //game selection
        else if (level == 0) { //game selection
          display.setTextSize(1); //top row text
          display.setCursor(40, 0);
          display.print("Settings");
          if (tiltToTurn) {
            display.fillRect(0, 8, 9, 9, WHITE);
          } else {
            display.drawRect(0, 8, 9, 9, WHITE);
          }
          display.setCursor(12, 9);
          display.print("Tilt to turn");
        }
        else { //level choice
          display.setTextSize(2);
          display.setCursor(40, 0);
          display.print("Level");
          display.setTextSize(4);
          display.setCursor(40, 20);
          display.print(level);
          display.setTextSize(1);
          display.setCursor(20, 55);
          display.print("Max Score: ");
          display.print(12 * level + 15);
        }
        display.display();
      }
      if (millis() - generalTimer >= 100) {
        if (level != -2 and digitalRead(5)) {
          level = max(level - 1, -2);
          generalTimer = millis();
          disp = true;
        } else if (digitalRead(4)) {
          level = min(level + 1, 20);
          generalTimer = millis();
          disp = true;
        } else if (level == 0 and (digitalRead(2) or digitalRead(3))) {
          tiltToTurn = !tiltToTurn;
          generalTimer = millis();
          disp = true;
        } else if (level == -2) gameChanger();
      }
      delay(50);
    }
    waitAllUnclick();
    display.setTextSize(2);
    uint8_t score = 0;
    Ship ship = Ship(64, 32, M_PI, 0);
    Asteroid* asteroids = new Asteroid[2 * (level + 1)];
    Laser* lasers = new Laser[2];
    for (uint8_t i = 0; i < 2; i++) asteroids[i] = Asteroid(8, 0, 0);
    for (uint8_t i = 2; i < level + 2; i++) asteroids[i] = Asteroid(16, 0, 0);
    for (uint8_t i = level + 2; i < 2 * (level + 1); i++) asteroids[i] = Asteroid(0, 0, 0);
    unsigned long laserButtonTiming, shipTurnTiming, scoreTime = millis(); //timer for button presses and score
    float pitchCorrection;
    if (tiltToTurn) {
      IMU.begin();
      pitchCorrection = getPitch();
    }

    /*Game*/
    while (true) {
      display.clearDisplay();
      if (millis() - shipTurnTiming >= 50) {
        if (tiltToTurn) { //turning
          int8_t angle = atan(getPitch() - pitchCorrection) * 180 / M_PI; //degrees
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
      ship.moveAndDisplay(digitalRead(2), new bool[2] {lasers[0].readyToShoot(), lasers[1].readyToShoot()}, display); //only move if button pressed
      for (uint8_t i = 0; i < 2 * (level + 1); i++) {
        asteroids[i].moveAndDisplay(display);
      }
      for (uint8_t i = 0; i < 2; i++) { //shoot laser on button press
        if (digitalRead(tiltToTurn ? 5 : 3) and lasers[i].readyToShoot() and millis() - laserButtonTiming > 100) {
          lasers[i].setUp(ship.dir, ship.X + sin(ship.dir) * 3, ship.Y + cos(ship.dir) * 3, ship.XVelocity, ship.YVelocity); //from tip of ship with additionall ship velocity added
          laserButtonTiming = millis();
        }
        if (lasers[i].readyToMove()) {
          lasers[i].moveAndDisplay(display);
        }
      }
      for (uint8_t i = 0; i < 2 * (level + 1); i++) { //asteroid laser collision
        if (asteroids[i].Size != 0) {
          for (uint8_t m = 0; m < 2; m++) { //every laser
            if (lasers[m].readyToMove()) {
              for (uint8_t n = 0; n < 2; n++) { //every point on every laser
                for (uint8_t o = 0; o < 2; o++) {
                  if (asteroids[i].pointInAsteroid(lasers[m].X + n, lasers[m].Y + o)) {
                    lasers[m].hit = true;
                    asteroids[i].hit(lasers[m].dir);
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
      uint8_t i; //win check: all asteroids have size 0
      for (i = 0; i < 2 * (level + 1) and asteroids[i].Size == 0; i++) {}
      if (i >= 2 * (level + 1)) { //uesd to be > 21
        display.clearDisplay();
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
        if (score > readEEPROM(24) or (score == readEEPROM(24) and level > readEEPROM(23))) newhigh(level, score);
        digitalWrite(6, LOW);
      }
      for (uint8_t i = 0; i < 2 * (level + 1); i++) { //game over check
        bool over = false;
        for (uint8_t m = 0; m < 3; m++) if (asteroids[i].pointInAsteroid(ship.XPoints[m][m], ship.YPoints[m][m])) over = true; //if any ship corners
        if (over or asteroids[i].pointInAsteroid(ship.X, ship.Y)) { //or the ship center are in an asteroid
          display.display();
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
          if (score > readEEPROM(24)) newhigh(level, score);
          digitalWrite(6, LOW);
        }
      }
      display.display();
      while (millis() - generalTimer < 40) {} //regulate to 25 fps on different levels
      generalTimer = millis();
    }
  }



  /*Astro Party*/

  else if (game == 1) { //comments are more sparse as the code is largely similar to Asteroids
    int8_t level = 1;
    unsigned long generalTimer;

    /*Setup*/
    while ((digitalRead(2) == 0 and digitalRead(3) == 0) or level == 0) {
      if (disp) { //only display if something changes
        disp = false;
        display.clearDisplay();
        if (level == 1) { //start display
          display.setTextSize(1);
          display.setCursor(30, 0);
          display.print("Astro Party");
          display.setTextSize(4);
          display.setCursor(5, 20);
          display.print("Start");
        } else gameChangerDisplay(); //game selection
        display.display();
      }
      if (millis() - generalTimer >= 100) {
        if (level == 1 and digitalRead(5)) {
          level = max(level - 1, 0);
          generalTimer = millis();
          disp = true;
        } else if (digitalRead(4)) {
          level = min(level + 1, 1);
          generalTimer = millis();
          disp = true;
        } else if (level == 0) gameChanger();
      }
      delay(50);
    }
    waitAllUnclick();
    Ship* ships = new Ship[2];
    Asteroid* asteroids = new Asteroid[12];
    Laser** lasers = new Laser*[2];
    unsigned long laserButtonTiming[2], shipTurnTiming[2], lastNoTurn[2], lastTurn[2], secondLastNoTurn[2], asteroidSpawn = millis(), textDisplay; //timers for button presses
    bool turnDir = 1, win = 0, winner;
    String powers[] = {"Reverse", "Laser"};
    uint8_t textDisplayNum;
    for (uint8_t i = 0; i < 2; i++) lasers[i] = new Laser[2];
    ships[0] = Ship(32, 32, 0, 0);
    ships[1] = Ship(96, 32, (3 / 2) * M_PI, 1);
    for (uint8_t i = 0; i < 4; i++) asteroids[i] = Asteroid(8, 0, 1);
    for (uint8_t i = 4; i < 8; i++) asteroids[i] = Asteroid(16, i == 4 or i == 5, 1);
    for (uint8_t i = 8; i < 12; i++) asteroids[i] = Asteroid(0, 0, 1);

    /*Game*/
    while (true) {
      display.clearDisplay();
      for (uint8_t i = 0; i < 12; i++) { //asteroid movement and display
        asteroids[i].moveAndDisplay(display);
      }
      if (millis() - asteroidSpawn >= 15000) { //spawn asteroids
        for (uint8_t j = 0; j < 2; j++) {
          uint8_t i, k = 0, count16 = 0, count0 = 0;
          for (i = 0; i < 12; i++) {
            if (asteroids[k].Size != 0) k++;
            if (asteroids[i].Size == 16) count16++;
            else if (asteroids[i].Size == 0) count0++;
          }
          if (asteroids[k].Size == 0 and count0 - 1 > count16) {
            asteroids[k].Size = 16;
            asteroids[k].dir = random(0, 629) / 100.0; //random dir
            asteroids[k].X = random(0, 128); //random anywhere
            asteroids[k].Y = random(0, 64);
            asteroids[k].power = random(0, 2);
          }
        }
        asteroidSpawn = millis();
      }
      for (uint8_t z = 0; z < 2; z++) { //z is used to diferentiate between ships
        if (millis() - shipTurnTiming[z] >= 50) { //turning
          if (digitalRead(-3 * z + 5)) {
            if (millis() - lastNoTurn[z] <= 80 and millis() - lastTurn[z] <= 280 and millis() - secondLastNoTurn[z] <= 280) {
              ships[z].boost(turnDir);
            } else {
              ships[z].turn(turnDir ? M_PI / 8 : M_PI / -8);
              lastTurn[z] = millis();
              secondLastNoTurn[z] = lastNoTurn[z];
            }
            shipTurnTiming[z] = millis();
          } else {
            lastNoTurn[z] = millis();
          }
        }
        ships[z].moveAndDisplay(1, new bool[2] {lasers[z][0].readyToShoot(), lasers[z][1].readyToShoot()}, display); //always moving
        for (uint8_t i = 0; i < 2; i++) { //shoot laser on button press
          if (digitalRead(-z + 4) and lasers[z][i].readyToShoot() and millis() - laserButtonTiming[z] > 100) {
            if (ships[z].power) {
              if (ships[z].power == 2) {
                for (int8_t x = -1; x < 2; x++) {
                  for (int8_t y = -1; y < 2; y++) {
                    display.drawLine(ships[z].XPoints[0][0] + x, ships[z].YPoints[0][0] + y, (int16_t) (ships[z].XPoints[0][0] + sin(ships[z].dir) * 142) + x, (uint16_t) (ships[z].YPoints[0][0] + cos(ships[z].dir) * 142) + y, WHITE);
                  }
                }
                display.display();
                for (uint8_t i = 0; i <= 142; i++) {
                  if (ships[-z + 1].pointInShip(ships[z].XPoints[0][0] + sin(ships[z].dir) * i, ships[z].YPoints[0][0] + cos(ships[z].dir) * i)) { //game won and over
                    win = 1;
                    winner = !z;
                  }
                }
              }
              ships[z].power = 0;
            } else {
              lasers[z][i].setUp(ships[z].dir, ships[z].X + sin(ships[z].dir) * 3, ships[z].Y + cos(ships[z].dir) * 3, ships[z].XVelocity, ships[z].YVelocity);
              laserButtonTiming[z] = millis();
            }
          }
          if (lasers[z][i].readyToMove()) {
            lasers[z][i].moveAndDisplay(display);
          }
        }
        for (uint8_t i = 0; i < 12; i++) { //asteroid laser collision
          if (asteroids[i].Size != 0) {
            for (uint8_t m = 0; m < 2; m++) { //every laser
              if (lasers[z][m].readyToMove()) {
                for (uint8_t n = 0; n < 2; n++) { //every point on every laser
                  for (uint8_t o = 0; o < 2; o++) {
                    if (asteroids[i].pointInAsteroid(lasers[z][m].X + n, lasers[z][m].Y + o)) {
                      lasers[z][m].hit = true;
                      if (asteroids[i].hit(lasers[z][m].dir)) {
                        uint8_t power = random(1, 3);
                        textDisplayNum = power - 1;
                        textDisplay = millis();
                        if (power == 1) {
                          turnDir = !turnDir;
                        } else if (power > 1) {
                          ships[z].power = power;
                        }
                      }
                      if (asteroids[i].Size == 8) {
                        uint8_t l;
                        for (l = 0; asteroids[l].Size != 0; l++) {}
                        asteroids[l].Size = 8;
                        asteroids[l].X = asteroids[i].X + 8;
                        asteroids[l].Y = asteroids[i].Y + 8;
                        asteroids[l].dir = asteroids[i].dir - M_PI;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      for (uint8_t z = 0; z < 2; z++) { //each ship    laser ship collision (point in traingle)
        for (uint8_t i = 0; i < 2; i++) { //each laser
          if (lasers[-z + 1][i].readyToMove()) {
            for (uint8_t j = 0; j < 2; j++) { //every point...
              for (uint8_t k = 0; k < 2; k++) { //...on each laser
                if (ships[z].pointInShip(lasers[-z + 1][i].X, lasers[-z + 1][i].Y)) { //game won and over
                  win = 1;
                  winner = z;
                }
              }
            }
          }
        }
      }
      if (millis() - textDisplay < 500) {
        display.setCursor((128 - powers[textDisplayNum].length() * 12) / 2, 24);
        display.setTextSize(2);
        display.print(powers[textDisplayNum]);
      }
      display.display();
      if (win) {
        display.display();
        delay(1000);
        display.fillRect(26, 21, 18, 19, BLACK);
        if (winner) display.drawTriangle(35, 24, 30, 36, 40, 36, WHITE);
        else display.fillTriangle(35, 24, 30, 36, 40, 36, WHITE);
        display.setCursor(44, 24);
        display.setTextSize(2);
        display.print(" Wins");
        display.display();
        delay(500);
        waitAnyClick();
        delay(500);
        digitalWrite(6, LOW);
      }
      while (millis() - generalTimer < 40) {} //regulate to 25 fps
      generalTimer = millis();
    }
  }



  /*Clonium*/

  else if (game == 2) {
    const String names[7] = {"X", "Y", "Small", "Large", "2P", "1P", "0P"}; //gamemodes
    uint8_t nums[2] = {8, 4};
    int8_t level = 4;
    unsigned long generalTimer;

    /*Setup*/
    while ((digitalRead(2) == 0 and digitalRead(3) == 0) or (level != 4 and level != 5 and level != 6)) { //choose gamemode
      if (disp) { //only display if something changes
        disp = false;
        display.clearDisplay();
        if (level >= 0) { //start screen
          display.setTextSize(1);
          display.setCursor(40, 0);
          display.print("Clonium");
          for (uint8_t i = 0; i < 4; i++) {
            uint8_t y = 16;
            for (uint8_t j = 0; j < min(i, 3); j++) y += (names[j].length() + 1) * 6 + (j == 0) * 18 + (j == 1) * 6;
            display.setCursor(y, 15);
            display.setTextSize(1);
            display.print(names[i + (i == 3)]);
            display.setCursor(y, 30);
            if (i < 2) {
              display.setTextSize(2);
              display.print(nums[i]);
            } else {
              display.print(names[i + 1 + (i == 3)]);
            } if (i == 3) {
              display.setCursor(y, 45);
              display.print(names[6]);
            }
          }
          display.setTextColor(BLACK);
          uint8_t y = 16;
          for (uint8_t j = 0; j < min(level - (level == 3), 3); j++) y += (names[j].length() + 1) * 6 + (j == 0) * 18 + (j == 1) * 6;
          display.fillRect(y - 1, 15 * max(level + 2 * (level == 3), 4) - 46, names[level].length() * 6 + 1, 9, WHITE);
          display.setCursor(y, 15 * max(level + 2 * (level == 3), 4) - 45);
          display.setTextSize(1);
          display.print(names[level]);
          display.setTextColor(WHITE);
        } else gameChangerDisplay(); //game selection
        display.display();
      }
      if (millis() - generalTimer >= 100) {
        if (digitalRead(4)) {
          level = min(level + 1, 6);
          generalTimer = millis();
          disp = true;
        }
        if (level >= 0) {
          if (digitalRead(5)) {
            level = max(level - 1, -1);
            generalTimer = millis();
            disp = true;
          } else if (digitalRead(3) and level < 2) {
            nums[level] = max(nums[level] - 1, 1 + 1 * (nums[0] * nums[1] == 2));
            generalTimer = millis();
            disp = true;
          } else if (digitalRead(2) and level < 2) {
            nums[level] = min(nums[level] + 1, !level * 16 + level * 8);
            generalTimer = millis();
            disp = true;
          } else if (level == 2 and (digitalRead(3) or digitalRead(2))) { //small board
            nums[0] = 8;
            nums[1] = 4;
            generalTimer = millis();
            disp = true;
          } else if (level == 3 and (digitalRead(3) or digitalRead(2))) { //large board
            nums[0] = 16;
            nums[1] = 8;
            generalTimer = millis();
            disp = true;
          }
        } else if (level == -1) gameChanger();
        delay(50);
      }
      delay(50);
    }
    boolean spaceFromEdges = nums[0] > 3 and nums[1] > 3 and (nums[0] != 4 or nums[1] != 4); //is the board big enough to accomidate space away from the edges for starting pieces
    uint8_t mx = min(spaceFromEdges, nums[0] - 1), my = min(spaceFromEdges, nums[1] - 1), mxLast[2] = {mx, max(nums[0] - 1 - spaceFromEdges, 0)}, myLast[2] = {my, max(nums[1] - 1  - spaceFromEdges, 0)}; //play choice, last choice
    boolean turn = 0, flash; //turn, flashing selection
    unsigned long flashTime = 0;
    CloniumBoard board = CloniumBoard(nums[0], nums[1]);
    board.draw(display);
    display.display();
    waitAllUnclick();

    /*Game*/
    while (true) {
      if (level >= 5 and turn or level == 6) { //CPU
        board.CPUMove(turn, display);
      } else { //player
        disp = true;
        if (level == 4) { //move selection back to old choice in 2 player for ease of use
          mx = mxLast[turn];
          my = myLast[turn];
        }
        while (digitalRead(2) == 0 or board.getTeam(mx, my) - 1 != turn) {
          if (disp) { //only display if something changes
            board.draw(display);
            flash = 0;
          }
          if (disp or millis() - flashTime >= 500) {
            disp = false;
            board.drawSelection(mx, my, flash, display);
            flash = !flash;
            flashTime = millis();
            display.display();
          }
          if (millis() - generalTimer >= 100) { //buttons
            if (digitalRead(5)) {
              mx = (mx + board.XDim - 1) % board.XDim; //+ 7 because mod can output negative numbers
              generalTimer = millis();
              disp = true;
            } else if (digitalRead(4)) {
              mx = (mx + 1) % board.XDim;
              generalTimer = millis();
              disp = true;
            } else if (digitalRead(3)) {
              my = (my + 1) % board.YDim;
              generalTimer = millis();
              disp = true;
            }
          }
          delay(50);
        }
        if (level == 4) {
          mxLast[turn] = mx;
          myLast[turn] = my;
        }
        board.mover(mx, my, turn);
      }
      board.draw(display);
      display.display();
      if (level == 4 or level == 5 and !turn) delay(200);
      boolean count = 1;
      for (uint8_t x = 0; x < board.XDim; x++) { //check for win by counting all of the enemies pieces
        for (uint8_t y = 0; y < board.YDim; y++) {
          if (board.getTeam(x, y) - 1 == !turn) {
            count = 0;
          }
        }
      }
      if (count) { //win display
        delay(1000);
        display.clearDisplay();
        display.fillRect(24, 24, 15, 15, !turn); //dots
        display.fillRect(29, 26, 4, 4, turn);
        display.fillRect(32, 32, 4, 4, turn);
        display.fillRect(26, 32, 4, 4, turn);
        display.setTextSize(2);
        display.setCursor(50, 24);
        display.print("Wins");
        display.display();
        delay(500);
        waitAnyClick();
        delay(500);
        digitalWrite(6, LOW);
      }
      turn = !turn; //change turn
    }
  }



  /*Minesweeper*/

  else if (game == 3) {
    int8_t level = 4;
    unsigned long generalTimer;
    uint8_t nums[3] = {8, 4, 5};
    const String names[6] = {"X", "Y", "Mines", "Start", "Small", "Large"};

    /*Setup*/
    while ((digitalRead(2) == 0 and digitalRead(3) == 0) or level != 4) { //choose size and number of mines
      if (disp) { //only display if something changes
        disp = false;
        display.clearDisplay();
        if (level > 0) {
          display.setTextSize(1);
          display.setCursor(30, 0);
          display.print("Minesweeper");
          for (uint8_t i = 0; i < 4; i++) {
            uint8_t y = 1;
            for (uint8_t j = 0; j < min(i, 3); j++) y += (names[j].length() + 1) * 6 + (j == 0) * 18 + (j == 1) * 6;
            display.setCursor(y, 15);
            display.setTextSize(1);
            display.print(names[i]);
            display.setCursor(y, 30);
            if (i < 3) {
              display.setTextSize(2);
              display.print(nums[i]);
            } else {
              display.print(names[4]);
              display.setCursor(y, 45);
              display.print(names[5]);
            }
          }
          display.setTextColor(BLACK);
          uint8_t y = 1;
          for (uint8_t j = 0; j < min(level - 1, 3); j++) y += (names[j].length() + 1) * 6 + (j == 0) * 18 + (j == 1) * 6;
          display.fillRect(y - 1, 15 * max(level, 4) - 46, names[level - 1].length() * 6 + 1, 9, WHITE);
          display.setCursor(y, 15 * max(level, 4) - 45);
          display.setTextSize(1);
          display.print(names[level - 1]);
          display.setTextColor(WHITE);
        } else gameChangerDisplay();
        display.display();
      }
      if (millis() - generalTimer >= 100) {
        if (digitalRead(4)) {
          level = min(level + 1, 6);
          generalTimer = millis();
          disp = true;
        }
        if (level > 0) {
          if (digitalRead(5)) {
            level = max(level - 1, 0);
            generalTimer = millis();
            disp = true;
          } else if (digitalRead(3) and level < 4) {
            nums[level - 1] = max(nums[level - 1] - 1, 1 + 1 * (level != 3 and nums[0] * nums[1] == 2));
            nums[2] = min(nums[0] * nums[1] - 1, nums[2]);
            generalTimer = millis();
            disp = true;
          } else if (digitalRead(2) and level < 4) {
            nums[level - 1] = min(nums[level - 1] + 1, (level == 1) * 16 + (level == 2) * 8 + (level == 3) * (nums[0] * nums[1] - 1));
            generalTimer = millis();
            disp = true;
          } else if (level == 5 and (digitalRead(3) or digitalRead(2))) { //small board
            nums[0] = 8;
            nums[1] = 4;
            nums[2] = 5;
            generalTimer = millis();
            disp = true;
          } else if (level == 6 and (digitalRead(3) or digitalRead(2))) { //large board
            nums[0] = 16;
            nums[1] = 8;
            nums[2] = 19;
            generalTimer = millis();
            disp = true;
          }
        } else if (level == 0) gameChanger();
        delay(50);
      }
    }
    uint8_t mx = 0, my = 0; //play choice
    boolean gen = 1, flash;
    disp = true;
    MinesweeperBoard board = MinesweeperBoard(nums[0], nums[1], nums[2]);
    board.draw(0, display);
    display.display();
    waitAllUnclick();
    unsigned long flashTime = 0, scoreTime = millis(); //timers for flashing selection and score

    /*Game*/
    while (true) {
      while (digitalRead(2) == 0 or (board.data[mx][my] != 9 and board.data[mx][my] != 11)) { //choose place
        if (disp) { //only display if something changes
          board.draw(0, display);
          flash = 0;
        }
        if (disp or millis() - flashTime >= 500) { //flash selection
          disp = false;
          board.drawSelection(mx, my, flash, display);
          flash = !flash;
          flashTime = millis();
          display.display();
        }
        if (millis() - generalTimer >= 100) {
          if (digitalRead(4)) {
            mx = (mx + 1) % board.XDim;
            generalTimer = millis();
            disp = true;
          } else if (digitalRead(5)) {
            my = (my + 1) % board.YDim;
            generalTimer = millis();
            disp = true;
          } else if (digitalRead(3) and board.data[mx][my] > 8) {
            if (board.data[mx][my] < 11) board.data[mx][my] = -board.data[mx][my] + 19; //swap 9 and 10
            else board.data[mx][my] = -board.data[mx][my] + 23; //swap 11 and 12
            disp = true;
          }
        }
        delay(50);
      }
      if (gen) { //generate mines first time through
        board.generateMines(mx, my);
        gen = 0;
      }
      if (board.data[mx][my] == 11) { //check for loss
        board.draw(1, display);
        board.drawSelection(mx, my, 0, display);
        display.display();
        delay(500);
        waitAnyClick();
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(40, 10);
        display.print("Game");
        display.setCursor(40, 31);
        display.print("Over");
        display.display();
        delay(1000);
        digitalWrite(6, LOW);
      }
      board.mover(mx, my);
      board.draw(0, display);
      display.display();
      if (board.winCheck()) {
        scoreTime = min((millis() - scoreTime) / 1000, 255);
        board.draw(1, display);
        display.display();
        delay(500);
        waitAnyClick();
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(45, 0);
        display.print("You");
        display.setCursor(45, 21);
        display.print("Win");
        display.setCursor(45, 42);
        display.print((uint8_t)(scoreTime / 60));
        display.print(":");
        if (scoreTime % 60 < 10) display.print(0);
        display.print(scoreTime % 60);
        display.display();
        delay(500);
        waitAnyClick();
        digitalWrite(6, LOW);
      }
      delay(200);
    }
  }



  /*Random Number Generator*/

  else if (game == 4) {
    int8_t level = 0;
    unsigned long generalTimer;
    const String names[5] = {"Random Number", "Min", "Max", "Dec", "Result"}; //names
    int16_t nums[3] = {0, 1, 0}; //min, max, and number of decimal places
    float result = random(0, 2);
    display.setTextSize(1);
    while (true) {
      if (disp) {
        disp = false;
        display.clearDisplay();
        if (level >= 0) {
          for (uint8_t i = 0; i < 5; i++) { //print names
            display.setCursor(1, 9 * i);
            display.print(names[i]);
            display.setCursor(40, 9 * i);
            if (i > 0 and i < 4) display.print(nums[i - 1]);
            else if (i == 4) display.print(result);
          }
          display.fillRect(0, 9 * (level + 1), names[level + 1].length() * 6 + 1, 8, WHITE); //highlight selection
          display.setTextColor(BLACK);
          display.setCursor(1, 9 * (level + 1));
          display.print(names[level + 1]);
          display.setTextColor(WHITE);
        } else gameChangerDisplay();
        display.display();
      }
      if (millis() - generalTimer >= 100) {
        if (digitalRead(4)) {
          level = min(level + 1, 3);
          generalTimer = millis();
          disp = true;
        }
        if (level >= 0) {
          if (digitalRead(5)) {
            level = max(level - 1, -1);
            generalTimer = millis();
            disp = true;
          } else if ((digitalRead(3) or digitalRead(2)) and level == 3) { //new result
            result = (float) random(nums[0] * pow(10, nums[2]), nums[1] * pow(10, nums[2]) + 1) / pow(10, nums[2]);
            disp = true;
          } else if (digitalRead(3)) { //increase nums
            nums[level]--;
            nums[1] = max(nums[0] + 1, nums[1]);
            nums[2] = min(max(nums[2], 0), 2);
            generalTimer = millis();
            disp = true;
          } else if (digitalRead(2)) { //decrease nums
            nums[level]++;
            nums[0] = min(nums[0], nums[1] - 1);
            nums[2] = min(max(nums[2], 0), 2);
            generalTimer = millis();
            disp = true;
          }
        } else if (level == -1) gameChanger();
        delay(50);
      }
    }
  }



  /*Level*/

  else if (game == 5) {
    int8_t level = 1;
    unsigned long generalTimer;
    float pitch, pitchCorrection; //calibration
    IMU.begin();
    while (true) {
      if (level and IMU.accelerationAvailable()) {
        display.clearDisplay();
        pitch = getPitch();
        int8_t angle = atan(pitch - pitchCorrection) * 180 / M_PI; //degrees
        display.setTextSize(4);
        display.setCursor(64 - 10 * String(angle).length() - 2 * (String(angle).length() - 1), 16);
        display.print(angle);
        for (uint8_t x = 0; x < 128; x++) {
          for (int16_t y = min(max((pitch - pitchCorrection) * (x - 64) + 32, 0), 66) - 2; y < 64; y++) { //for pixels past level line
            display.drawPixel(x, y, !display.getPixel(x, y)); //invert color
          }
        }
        display.display();
      } else if (disp) {
        disp = false;
        display.clearDisplay();
        gameChangerDisplay();
        display.display();
      }
      if (!level) gameChanger();
      if (millis() - generalTimer >= 100) {
        if (digitalRead(4)) { //level
          level = 1;
          generalTimer = millis();
        } else if (digitalRead(5)) { //game selection
          level = 0;
          generalTimer = millis();
          disp = true;
        } else if (digitalRead(3) or digitalRead(2)) { //calibrate
          pitchCorrection = pitch;
          generalTimer = millis();
          disp = true;
        }
      }
      delay(50);
    }
  }



  /*Fireworks*/

  else if (game == 6) {
    int8_t level = 1;
    unsigned long generalTimer, betweenTimer;
    uint8_t betweenDur = 0;
    Firework fireworks[10];// = Firework(random(0, 128), random(0, 64));
    for (uint8_t i = 0; i < 10; i++) {
      fireworks[i] = Firework();
    }
    while (true) {
      if (level) {
        display.clearDisplay();
        if (millis() - betweenTimer >= betweenDur * 10) {
          uint8_t i;
          for (i = 0; i < 10 and !fireworks[i].offScreen; i++) {}
          if (i < 10) {
            fireworks[i].reset();
          }
          betweenDur = random(0, 50);
          betweenTimer = millis();
        }
        for (uint8_t i = 0; i < 10; i++) {
          fireworks[i].moveAndDisplay(display);
        }
        display.display();
      } else if (disp) {
        disp = false;
        display.clearDisplay();
        gameChangerDisplay();
        display.display();
      }
      if (!level) gameChanger();
      if (millis() - generalTimer >= 100) {
        if (digitalRead(4)) { //level
          level = 1;
          generalTimer = millis();
        } else if (digitalRead(5)) { //game selection
          level = 0;
          generalTimer = millis();
          disp = true;
        }
      }
      delay(50);
    }
  }



  /*Game Unknown*/

  else {
    game = (game + 1) % 7;
    while (true) {
      if (disp) {
        disp = false;
        display.clearDisplay();
        gameChangerDisplay();
        display.display();
      }
      gameChanger();
      delay(50);
    }
  }
}

void loop() {}
