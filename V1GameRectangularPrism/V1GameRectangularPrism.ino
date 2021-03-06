/*
  Game Rectangular Prism Version 1 Code
  Astro Party on an Arduino Nano Every with a 64x128 OLED screen, 4 buttons, a 9v
  battery, and a power switch all in a white 3D printed case.
  Written by Tyler Hill
  Version 1.0
  Differences from the Astro Party of Version 2 include:
    Simplification of start display due to lack of need to change between games.
    digitalRead operation in refrence to buttons is reversed.
    Uses reset function instead of writing pin 6 (wired to the reset pin) low.
*/
#include "shipAsteroidLaser.h"
Adafruit_SSD1306 display(128, 64, &Wire, -1);

/*Functions*/

void waitAnyClick() {//waits until any button is pressed
  while (digitalRead(2) == 1 and digitalRead(3) == 1 and digitalRead(4) == 1 and digitalRead(5) == 1) {}
}
void waitAllUnclick() {//waits until none of the buttons are clicked
  while (digitalRead(2) == 0 or digitalRead(3) == 0 or digitalRead(4) == 0 or digitalRead(5) == 0) {}
}
void(* resetFunc) (void) = 0;

/*Device Start*/

void setup() {
  //Serial.begin(9600);//Serial.println("");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Wire.begin();
  randomSeed(analogRead(1));//better random
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
  unsigned long laserButtonTiming[2], shipTurnTiming[2];//timers for button presses
  for (uint8_t i = 0; i < 2; i++) laserList[i] = new laser[2];
  shipList[0] = ship(32, 32, 0, 0);
  shipList[1] = ship(96, 32, (3 / 2) * PI, 1);
  for (uint8_t i = 0; i < 2; i++) asteroidList[i] = asteroid(8, 1);
  for (uint8_t i = 2; i < 7; i++) asteroidList[i] = asteroid(16, 1);
  for (uint8_t i = 7; i < 12; i++) asteroidList[i] = asteroid(0, 1);

  /*Game*/
  while (true) {
    display.clearDisplay();
    for (uint8_t i = 0; i < 12; i++) {//asteroid movement and display
      asteroidList[i].moveAndDisplay(display);
    }
    for (uint8_t z = 0; z < 2; z++) {//z is used to diferentiate between ships
      if (millis() - shipTurnTiming[z] >= 50) {//turning
        if (digitalRead(-3 * z + 5) == 0) {
          shipList[z].CWTurn();
          shipTurnTiming[z] = millis();
        }
      }
      shipList[z].moveAndDisplay(1, new bool[2] {laserList[z][0].readyToShoot(), laserList[z][1].readyToShoot()}, display);//always moving
      for (uint8_t i = 0; i < 2; i++) {//shoot laser on button press
        if (digitalRead(-z + 4) == 0 and laserList[z][i].readyToShoot() and millis() - laserButtonTiming[z] > 100) {
          laserList[z][i].setUp(shipList[z].dir, shipList[z].X + sin(shipList[z].dir) * 3, shipList[z].Y + cos(shipList[z].dir) * 3, shipList[z].XVelocity, shipList[z].YVelocity);
          laserButtonTiming[z] = millis();
        }
        if (laserList[z][i].readyToMove()) {
          laserList[z][i].moveAndDisplay(display);
        }
      }
      for (uint8_t i = 0; i < 12; i++) {//asteroid laser collision
        if (asteroidList[i].Size != 0) {
          for (uint8_t m = 0; m < 2; m++) {//every laser
            if (laserList[z][m].readyToMove()) {
              for (uint8_t n = 0; n < 2; n++) {//every point on every laser
                for (uint8_t o = 0; o < 2; o++) {
                  if (asteroidList[i].pointInAsteroid(laserList[z][m].X + n, laserList[z][m].Y + o)) {
                    laserList[z][m].hit = true;
                    if (asteroidList[i].hit(laserList[z][m].dir)) {
                      uint8_t l;
                      for (l = 0; asteroidList[l].Size != 0; l++) {}
                      asteroidList[l].Size = 8;
                      asteroidList[l].X = asteroidList[i].X + 8;
                      asteroidList[l].Y = asteroidList[i].Y + 8;
                      asteroidList[l].dir = asteroidList[i].dir - PI;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    for (uint8_t z = 0; z < 2; z++) {//each ship    laser ship collision (point in traingle)
      for (uint8_t i = 0; i < 2; i++) {//each laser
        if (laserList[-z + 1][i].readyToMove()) {
          for (uint8_t j = 0; j < 2; j++) {//every point...
            for (uint8_t k = 0; k < 2; k++) {//...on each laser
              if (shipList[z].pointInShip(laserList[-z + 1][i].X, laserList[-z + 1][i].Y)) {//game won and over
                display.display();
                delay(1000);
                display.fillRect(28, 22, 14, 16, BLACK);
                if (z) display.drawTriangle(35, 24, 30, 36, 40, 36, WHITE);
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
            }
          }
        }
      }
    }
    display.display();
    while (millis() - generalTimer < 40) {}//regulate to 25 fps
    generalTimer = millis();
  }
}
void loop() {}
