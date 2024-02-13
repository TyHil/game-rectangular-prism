/*
  Astro Party app implementation
  Written by Tyler Gordon Hill
*/
#include "astroParty.h"
#include "helper.h"
const String powers[] = {"Reverse", "Laser"};

void displayAstroPartyStart(Adafruit_SSD1306& display) {
  display.setTextSize(1);
  display.setCursor(30, 0);
  display.print("Astro Party");
  display.setTextSize(4);
  display.setCursor(5, 20);
  display.print("Start");
}



/* Astro Party */

AstroParty::AstroParty() {
  ships[0] = Ship(32, 32, 0, 0);
  ships[1] = Ship(96, 32, (3 / 2) * M_PI, 1);
  for (uint8_t i = 0; i < 4; i++) {
    asteroids[i] = Asteroid(8, 0, 1);
  }
  for (uint8_t i = 4; i < 8; i++) {
    asteroids[i] = Asteroid(16, i == 4 or i == 5, 1);
  }
  for (uint8_t i = 8; i < 12; i++) {
    asteroids[i] = Asteroid(0, 0, 1);
  }
  for (uint8_t i = 0; i < 2; i++) {
    shipTurnTiming[i] = millis();
    lastNoTurn[i] = millis();
    lastTurn[i] = millis();
    secondLastNoTurn[i] = millis();
  }
  asteroidSpawn = millis();
  textDisplay = -501;
  turnDir = 1;
  win = false;
  winner = 0;
  textDisplayNum = 0;
  frameTimer = millis();
}

void AstroParty::turning() {
  for (uint8_t z = 0; z < 2; z++) {
    if (millis() - shipTurnTiming[z] >= 50) { //turning
      if (digitalRead(z ? 2 : 5) == 0) {
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
  }
}

void AstroParty::move(Adafruit_SSD1306& display) {
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
  for (uint8_t z = 0; z < 2; z++) {
    bool shoot = digitalRead(z ? 3 : 4) == 0;
    if (shoot and millis() - ships[z].shotTime > 100 and ships[z].power) {
      shoot = false;
      ships[z].shotTime = millis();
      if (ships[z].power == 2) {
        for (int8_t x = -1; x < 2; x++) {
          for (int8_t y = -1; y < 2; y++) {
            display.drawLine(ships[z].XPoints[0][0] + x, ships[z].YPoints[0][0] + y, (int16_t) (ships[z].XPoints[0][0] + sin(ships[z].dir) * 142) + x, (uint16_t) (ships[z].YPoints[0][0] + cos(ships[z].dir) * 142) + y, WHITE);
          }
        }
        display.display();
        for (uint8_t i = 0; i <= 142; i++) {
          if (ships[z ? 0 : 1].pointInShip(ships[z].XPoints[0][0] + sin(ships[z].dir) * i, ships[z].YPoints[0][0] + cos(ships[z].dir) * i)) { //game won and over
            win = 1;
            winner = !z;
          }
        }
      }
      ships[z].power = 0;
    }
    ships[z].moveAndDisplay(1, shoot, display); //always moving
  }
}

void AstroParty::asteroidLaserCollision() {
  for (uint8_t z = 0; z < 2; z++) {
    for (uint8_t i = 0; i < 12; i++) {
      if (asteroids[i].Size != 0) {
        for (uint8_t m = 0; m < 2; m++) { //every laser
          if (ships[z].lasers[m].readyToMove()) {
            for (uint8_t n = 0; n < 2; n++) { //every point on every laser
              for (uint8_t o = 0; o < 2; o++) {
                if (asteroids[i].pointInAsteroid(ships[z].lasers[m].X + n, ships[z].lasers[m].Y + o)) {
                  ships[z].lasers[m].hit = true;
                  if (asteroids[i].hit(ships[z].lasers[m].dir)) {
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
}

void AstroParty::winCheck(Adafruit_SSD1306& display) { //laser ship collision (point in traingle)
  if (win != 1) { //can already be 1 from laser power up
    for (uint8_t z = 0; z < 2; z++) { //each ship
      for (uint8_t i = 0; i < 2; i++) { //each laser
        if (ships[z ? 0 : 1].lasers[i].readyToMove()) {
          for (uint8_t j = 0; j < 2; j++) { //every point...
            for (uint8_t k = 0; k < 2; k++) { //...on each laser
              if (ships[z].pointInShip(ships[z ? 0 : 1].lasers[i].X, ships[z ? 0 : 1].lasers[i].Y)) { //game won and over
                win = 1;
                winner = z;
              }
            }
          }
        }
      }
    }
  }
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
}

void AstroParty::powerUpDisplay(Adafruit_SSD1306& display) {
  if (millis() - textDisplay < 500) {
    display.setCursor((128 - powers[textDisplayNum].length() * 12) / 2, 24);
    display.setTextSize(2);
    display.print(powers[textDisplayNum]);
  }
}

void AstroParty::run(Adafruit_SSD1306& display) {
  while (true) {
    display.clearDisplay();
    turning();
    move(display);
    asteroidLaserCollision();
    winCheck(display);
    powerUpDisplay(display);
    display.display();
    while (millis() - frameTimer < 40) {} //regulate to 25 fps on different levels
    frameTimer = millis();
  }
}
