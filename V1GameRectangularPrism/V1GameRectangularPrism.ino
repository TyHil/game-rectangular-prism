/*
  Game Rectangular Prism Version 1 Code
  Astro Party on an Arduino Nano Every with a 64x128 OLED screen, 4 buttons, a 9v
  battery, and a power switch all in a white 3D printed case.
  Written by Tyler Gordon Hill
  Version 1.1
  Differences from the Astro Party of Version 2 include:
    Simplification of start display due to lack of need to change between games.
    digitalRead operation in refrence to buttons is reversed.
    Uses reset function instead of writing pin 6 (wired to the reset pin) low.
*/
#include "shipAsteroidLaser.h"
Adafruit_SSD1306 display(128, 64, &Wire, -1);



/* General */

void waitAnyClick() { //waits until any button is pressed
  while (digitalRead(2) == 1 and digitalRead(3) == 1 and digitalRead(4) == 1 and digitalRead(5) == 1) {}
}
void waitAllUnclick() { //waits until none of the buttons are clicked
  while (digitalRead(2) == 0 or digitalRead(3) == 0 or digitalRead(4) == 0 or digitalRead(5) == 0) {}
}
void(* resetFunc) (void) = 0;



/* Device Start */

void setup() {
  //Serial.begin(9600); //Serial.println("");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Wire.begin();
  randomSeed(analogRead(1)); //better random
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  display.setTextColor(WHITE);
  delay(100);

  /*Astro Party*/

  unsigned long generalTimer;

  /*Setup*/
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(30, 0);
  display.print("Astro Party");
  display.setTextSize(4);
  display.setCursor(5, 20);
  display.print("Start");
  display.display();
  waitAnyClick();
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
        if (digitalRead(-3 * z + 5) == 0) {
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
        if (digitalRead(-z + 4) == 0 and lasers[z][i].readyToShoot() and millis() - laserButtonTiming[z] > 100) {
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
      resetFunc();
    }
    while (millis() - generalTimer < 40) {} //regulate to 25 fps
    generalTimer = millis();
  }
}
void loop() {}
