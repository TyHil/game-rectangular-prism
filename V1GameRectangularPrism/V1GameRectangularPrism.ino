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

/*Functions*/

void waitAnyClick() { //waits until any button is pressed
  while (digitalRead(2) == 1 and digitalRead(3) == 1 and digitalRead(4) == 1 and digitalRead(5) == 1) {}
}
void waitAllUnclick() { //waits until none of the buttons are clicked
  while (digitalRead(2) == 0 or digitalRead(3) == 0 or digitalRead(4) == 0 or digitalRead(5) == 0) {}
}
void(* resetFunc) (void) = 0;

/*Device Start*/

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
  ship* shipList = new ship[2];
  asteroid* asteroidList = new asteroid[12];
  laser** laserList = new laser*[2];
  unsigned long laserButtonTiming[2], shipTurnTiming[2], lastNoTurn[2], lastTurn[2], secondLastNoTurn[2], asteroidSpawn = millis(), textDisplay; //timers for button presses
  bool turnDir = 1, win = 0, winner;
  String powers[] = {"Reverse", "Laser"};
  uint8_t textDisplayNum;
  for (uint8_t i = 0; i < 2; i++) laserList[i] = new laser[2];
  shipList[0] = ship(32, 32, 0, 0);
  shipList[1] = ship(96, 32, (3 / 2) * M_PI, 1);
  for (uint8_t i = 0; i < 4; i++) asteroidList[i] = asteroid(8, 0, 1);
  for (uint8_t i = 4; i < 8; i++) asteroidList[i] = asteroid(16, i == 4 or i == 5, 1);
  for (uint8_t i = 8; i < 12; i++) asteroidList[i] = asteroid(0, 0, 1);

  /*Game*/
  while (true) {
    display.clearDisplay();
    for (uint8_t i = 0; i < 12; i++) { //asteroid movement and display
      asteroidList[i].moveAndDisplay(display);
    }
    if (millis() - asteroidSpawn >= 15000) { //spawn asteroids
      for (uint8_t j = 0; j < 2; j++) {
        uint8_t i, k = 0, count16 = 0, count0 = 0;
        for (i = 0; i < 12; i++) {
          if (asteroidList[k].Size != 0) k++;
          if (asteroidList[i].Size == 16) count16++;
          else if (asteroidList[i].Size == 0) count0++;
        }
        if (asteroidList[k].Size == 0 and count0 - 1 > count16) {
          asteroidList[k].Size = 16;
          asteroidList[k].dir = random(0, 629) / 100.0; //random dir
          asteroidList[k].X = random(0, 128); //random anywhere
          asteroidList[k].Y = random(0, 64);
          asteroidList[k].power = random(0, 2);
        }
      }
      asteroidSpawn = millis();
    }
    for (uint8_t z = 0; z < 2; z++) { //z is used to diferentiate between ships
      if (millis() - shipTurnTiming[z] >= 50) { //turning
        if (digitalRead(-3 * z + 5) == 0) {
          if (millis() - lastNoTurn[z] <= 80 and millis() - lastTurn[z] <= 280 and millis() - secondLastNoTurn[z] <= 280) {
            shipList[z].boost(turnDir);
          } else {
            shipList[z].turn(turnDir);
            lastTurn[z] = millis();
            secondLastNoTurn[z] = lastNoTurn[z];
          }
          shipTurnTiming[z] = millis();
        } else {
          lastNoTurn[z] = millis();
        }
      }
      shipList[z].moveAndDisplay(1, new bool[2] {laserList[z][0].readyToShoot(), laserList[z][1].readyToShoot()}, display); //always moving
      for (uint8_t i = 0; i < 2; i++) { //shoot laser on button press
        if (digitalRead(-z + 4) == 0 and laserList[z][i].readyToShoot() and millis() - laserButtonTiming[z] > 100) {
          if (shipList[z].power) {
            if (shipList[z].power == 2) {
              for (int8_t x = -1; x < 2; x++) {
                for (int8_t y = -1; y < 2; y++) {
                  display.drawLine(shipList[z].XPoints[0][0] + x, shipList[z].YPoints[0][0] + y, (int16_t) (shipList[z].XPoints[0][0] + sin(shipList[z].dir) * 142) + x, (uint16_t) (shipList[z].YPoints[0][0] + cos(shipList[z].dir) * 142) + y, WHITE);
                }
              }
              display.display();
              for (uint8_t i = 0; i <= 142; i++) {
                if (shipList[-z + 1].pointInShip(shipList[z].XPoints[0][0] + sin(shipList[z].dir) * i, shipList[z].YPoints[0][0] + cos(shipList[z].dir) * i)) { //game won and over
                  win = 1;
                  winner = !z;
                }
              }
            }
            shipList[z].power = 0;
          } else {
            laserList[z][i].setUp(shipList[z].dir, shipList[z].X + sin(shipList[z].dir) * 3, shipList[z].Y + cos(shipList[z].dir) * 3, shipList[z].XVelocity, shipList[z].YVelocity);
            laserButtonTiming[z] = millis();
          }
        }
        if (laserList[z][i].readyToMove()) {
          laserList[z][i].moveAndDisplay(display);
        }
      }
      for (uint8_t i = 0; i < 12; i++) { //asteroid laser collision
        if (asteroidList[i].Size != 0) {
          for (uint8_t m = 0; m < 2; m++) { //every laser
            if (laserList[z][m].readyToMove()) {
              for (uint8_t n = 0; n < 2; n++) { //every point on every laser
                for (uint8_t o = 0; o < 2; o++) {
                  if (asteroidList[i].pointInAsteroid(laserList[z][m].X + n, laserList[z][m].Y + o)) {
                    laserList[z][m].hit = true;
                    if (asteroidList[i].hit(laserList[z][m].dir)) {
                      uint8_t power = random(1, 3);
                      textDisplayNum = power - 1;
                      textDisplay = millis();
                      if (power == 1) {
                        turnDir = !turnDir;
                      } else if (power > 1) {
                        shipList[z].power = power;
                      }
                    }
                    if (asteroidList[i].Size == 8) {
                      uint8_t l;
                      for (l = 0; asteroidList[l].Size != 0; l++) {}
                      asteroidList[l].Size = 8;
                      asteroidList[l].X = asteroidList[i].X + 8;
                      asteroidList[l].Y = asteroidList[i].Y + 8;
                      asteroidList[l].dir = asteroidList[i].dir - M_PI;
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
        if (laserList[-z + 1][i].readyToMove()) {
          for (uint8_t j = 0; j < 2; j++) { //every point...
            for (uint8_t k = 0; k < 2; k++) { //...on each laser
              if (shipList[z].pointInShip(laserList[-z + 1][i].X, laserList[-z + 1][i].Y)) { //game won and over
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
