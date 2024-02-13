/*
  Game Rectangular Prism Version 2 Code
  Asteroids, Astro Party, Clonium, and Minesweeper games and Random Number Generator
  and Level utilities on an Arduino Nano 33 IoT with a 64x128 OLED screen, 4 capacitive
  touch buttons, an EEPROM module, a 9v battery, and a power switch all in a black 3D
  printed case.
  Written by Tyler Gordon Hill
  Version 7.0
*/
#include "helper.h"
#include "screen.h"
#include "shipAsteroidLaser.h"
#include "asteroids.h"
#include "boards.h"
#include "randomNum.h"
#include "level.h"
#include "fireworks.h"
#include "thermometer.h"
#include "cube.h"
enum App { asteroids, astroParty, clonium, minesweeper, randomNum, level, fireworks, thermometer, cube, NUM_APPS };
Adafruit_SSD1306 display(128, 64, &Wire, -1);
App app; //which app is being used
bool disp = true; //limits display refreshes when nothing has changed



/* App Selection Functions */

void gameChangerDisplay() { //displays menu to change between different games
  const String names[NUM_APPS + 1] = {"Asteroids", "Astro Party", "Clonium", "Minesweeper", "Random Num", "Level", "Fireworks", "Temp", "Cube"}; //app names
  display.setTextSize(1);
  display.setCursor(30, 0);
  display.print("Switch App");
  for (uint8_t i = 0; i < NUM_APPS; i++) { //app list
    display.setCursor((i < 6) ? 1 : 68, 9 * (i % 6) + 9);
    display.print(names[i]);
  }
  display.fillRect((app < 6) ? 0 : 67, 9 * (app % 6 + 1) - 1, 67, 9, WHITE); //highlight current selection
  display.setCursor((app < 6) ? 1 : 68, 9 * (app % 6 + 1));
  display.setTextColor(BLACK);
  display.print(names[app]);
  display.setTextColor(WHITE);
}

void gameChanger() { //update app selection and restart arduino on choice
  if (digitalRead(5)) {
    app = static_cast<App>((app + 1) % NUM_APPS);
    disp = true;
  } else if (digitalRead(2) or digitalRead(3)) {
    updateEEPROM(25, app);
    digitalWrite(6, LOW);
  }
}



/* Device Start */

void setup() {
  digitalWrite(6, HIGH);
  pinMode(6, OUTPUT);
  //Serial.begin(9600); //Serial.println("");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.cp437(true);
  Wire.begin();
  randomSeed(analogRead(1)); //better random
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  display.setTextColor(WHITE);
  app = static_cast<App>(readEEPROM(25)); //read app last used
  delay(100);



  /* Asteroids */

  if (app == asteroids) {
    Screen screen = Screen(-2, MAX_LEVEL, 1);
    Asteroids asteroids = Asteroids();

    /*Setup*/
    while ((digitalRead(2) == 0 and digitalRead(3) == 0) or screen.screen == 0 or screen.screen == -1 or screen.screen == -2) { //menu and level choice
      if (disp) { //only display if something changes
        disp = false;
        display.clearDisplay();
        if (screen.screen == -2) { //app selection
          gameChangerDisplay();
        } else if (screen.screen == -1) { //highscores
          asteroids.displayHighScores(display);
        } else if (screen.screen == 0) { //settings
          asteroids.displaySettings(display);
        } else { //level choice
          asteroids.displayLevel(display, screen.screen);
        }
        display.display();
      }
      if (screen.buttons()) {
        if (screen.screen == -2) gameChanger();
        if (digitalRead(5) or digitalRead(4) or digitalRead(3) or digitalRead(2)) {
          disp = true;
        }
        if (screen.screen == 0 and (digitalRead(2) or digitalRead(3))) {
          asteroids.tiltToTurn = !asteroids.tiltToTurn;
        }
      }
      delay(50);
    }
    waitAllUnclick();
    asteroids.setup(screen.screen);

    /*Game*/
    asteroids.run(display);
  }



  /* Astro Party */

  else if (app == astroParty) { //comments are more sparse as the code is largely similar to Asteroids
    Screen screen = Screen(1, 1);

    /*Setup*/
    while ((digitalRead(2) == 0 and digitalRead(3) == 0) or screen.screen == 0) {
      if (disp) { //only display if something changes
        disp = false;
        display.clearDisplay();
        if (screen.screen == 0) { //app selection
          gameChangerDisplay();
        } else if (screen.screen == 1) { //start display
          display.setTextSize(1);
          display.setCursor(30, 0);
          display.print("Astro Party");
          display.setTextSize(4);
          display.setCursor(5, 20);
          display.print("Start");
        }
        display.display();
      }
      if (screen.buttons()) {
        if (screen.screen == 0) gameChanger();
        if (digitalRead(5) or digitalRead(4)) {
          disp = true;
        }
      }
      delay(50);
    }
    waitAllUnclick();
    Ship* ships = new Ship[2];
    Asteroid* asteroids = new Asteroid[12];
    Laser** lasers = new Laser*[2];
    uint64_t laserButtonTiming[2] = {millis(), millis()}, shipTurnTiming[2] = {millis(), millis()}, lastNoTurn[2] = {millis(), millis()}, lastTurn[2] = {millis(), millis()}, secondLastNoTurn[2] = {millis(), millis()}, asteroidSpawn = millis(); //timers for button presses
    int64_t textDisplay = -501;
    bool turnDir = 1, win = 0, winner = 0;
    String powers[] = {"Reverse", "Laser"};
    uint8_t textDisplayNum = 0;
    for (uint8_t i = 0; i < 2; i++) lasers[i] = new Laser[2];
    ships[0] = Ship(32, 32, 0, 0);
    ships[1] = Ship(96, 32, (3 / 2) * M_PI, 1);
    for (uint8_t i = 0; i < 4; i++) asteroids[i] = Asteroid(8, 0, 1);
    for (uint8_t i = 4; i < 8; i++) asteroids[i] = Asteroid(16, i == 4 or i == 5, 1);
    for (uint8_t i = 8; i < 12; i++) asteroids[i] = Asteroid(0, 0, 1);
    uint64_t frameTimer = millis();

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
      while (millis() - frameTimer < 40) {} //regulate to 25 fps
      frameTimer = millis();
    }
  }



  /* Clonium */

  else if (app == clonium) {
    const String names[7] = {"X", "Y", "Small", "Large", "2P", "1P", "0P"}; //gamemodes
    uint8_t nums[2] = {8, 4};
    int8_t level = 4;
    uint64_t generalTimer = millis();

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
        } else gameChangerDisplay(); //app selection
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
    uint8_t mx = min(spaceFromEdges, nums[0] - 1), my = min(spaceFromEdges, nums[1] - 1), mxLast[2] = {mx, (uint8_t)max(nums[0] - 1 - spaceFromEdges, 0)}, myLast[2] = {my, (uint8_t)max(nums[1] - 1  - spaceFromEdges, 0)}; //play choice, last choice
    boolean turn = 0, flash = 0; //turn, flashing selection
    uint64_t flashTime = 0;
    CloniumBoard board = CloniumBoard(nums[0], nums[1]);
    board.draw(display);
    display.display();
    waitAllUnclick();

    /*Game*/
    while (true) {
      if ((level >= 5 and turn) or level == 6) { //CPU
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
      if (level == 4 or (level == 5 and !turn)) delay(200);
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



  /* Minesweeper */

  else if (app == minesweeper) {
    int8_t level = 4;
    uint64_t generalTimer = millis();
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
    boolean gen = 1, flash = 0;
    disp = true;
    MinesweeperBoard board = MinesweeperBoard(nums[0], nums[1], nums[2]);
    board.draw(0, display);
    display.display();
    waitAllUnclick();
    uint64_t flashTime = 0, scoreTime = millis(); //timers for flashing selection and score

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



  /* Random Number Generator */

  else if (app == randomNum) {
    Screen screen = Screen(4, 1);
    RandomNum randomNum = RandomNum();

    while (true) {
      if (disp) {
        disp = false;
        display.clearDisplay();
        if (screen.screen == 0) {
          gameChangerDisplay();
        } else if (screen.screen > 0) {
          randomNum.display(display, screen.screen);
        }
        display.display();
      }

      if (screen.buttons()) {
        if (screen.screen == 0) gameChanger();
        if (digitalRead(5) or digitalRead(4) or digitalRead(3) or digitalRead(2)) {
          disp = true;
        }
        if (screen.screen > 0) {
          if ((digitalRead(3) or digitalRead(2)) and screen.screen == 4) { //new result
            randomNum.generate();
          } else if (digitalRead(3)) {
            randomNum.decNums(screen.screen);
          } else if (digitalRead(2)) {
            randomNum.incNums(screen.screen);
          }
        }
      }
      delay(50);
    }
  }



  /* Level */

  else if (app == level) {
    Screen screen = Screen(1, 1);
    Level level = Level();

    while (true) {
      if (screen.screen == 0 and disp) {
        disp = false;
        display.clearDisplay();
        gameChangerDisplay();
        display.display();
      } else if (screen.screen == 1) {
        level.display(display);
      }

      if (screen.buttons()) {
        if (screen.screen == 0) gameChanger();
        if (digitalRead(5)) { //app selection
          disp = true;
        } else if (digitalRead(3) or digitalRead(2)) { //calibrate
          level.correctPitch();
        }
      }
      delay(50);
    }
  }



  /* Fireworks */

  else if (app == fireworks) {
    Screen screen = Screen(1, 1);
    Fireworks fireworks = Fireworks();

    while (true) {
      if (screen.screen == 0 and disp) {
        disp = false;
        display.clearDisplay();
        gameChangerDisplay();
        display.display();
      } else if (screen.screen == 1) {
        display.clearDisplay();
        fireworks.display(display);
        display.display();
      }

      if (screen.buttons()) {
        if (screen.screen == 0) gameChanger();
        if (digitalRead(5)) { //app selection
          disp = true;
        }
      }
      delay(50);
    }
  }
  


  /* Thermometer */

  else if (app == thermometer) {
    Screen screen = Screen(3, 1);
    Thermometer thermometer = Thermometer();

    while (true) {
      if (thermometer.measure(screen.screen)) {
        disp = true;
      }

      if (screen.screen == 0 and disp) {
        disp = false;
        display.clearDisplay();
        gameChangerDisplay();
        display.display();
      } else if (screen.screen == 1 and disp) {
        disp = false;
        display.clearDisplay();
        thermometer.mainDisplay(display);
        display.display();
      } else if (screen.screen == 2 and disp) {
        disp = false;
        display.clearDisplay();
        thermometer.settingsDisplay(display);
        display.display();
      } else if (screen.screen > 2 and disp) {
        disp = false;
        display.clearDisplay();
        thermometer.historyDisplay(display);
        display.display();
      }

      if (screen.buttons()) {
        if (screen.screen == 0) gameChanger();
        if (digitalRead(5)) { //app selection
          if (screen.screen == 1 and thermometer.stop) { //remeasure
            thermometer.prepDisplay();
          }
          if (screen.screen == 2 and thermometer.historyScreenPlace != 0) {
            screen.screen = 3;
            thermometer.prepHistory(false);
          }
          if (screen.screen == 2) { //settings prep
            thermometer.prepHistory(false);
            thermometer.prepSettings();
          }
          disp = true;
        } else if (digitalRead(4)) {
          if (screen.screen == 1 and thermometer.stop) { //remeasure
            thermometer.prepDisplay();
          }
          if (screen.screen == 2) { //settings prep
            thermometer.prepSettings();
          }
          if (screen.screen == 3) {
            if (thermometer.historyRecordPlace == 0) {
              screen.screen = 2;
            } else {
              thermometer.prepHistory(true);
            }
          }
          disp = true;
        } else if (digitalRead(3)) {
          if (screen.screen == 2) { //settings
            thermometer.settingsChoice3();
          } else if (screen.screen == 3) { //jump to start
            thermometer.historyScreenMin();
          }
          if (screen.screen > 1) {
            disp = true;
          }
        } else if (digitalRead(2)) {
          if (screen.screen == 2) { //settings
            thermometer.settingsChoice2();
          } else if (screen.screen == 3) { //jump to end
            thermometer.historyScreenMax();
          }
          if (screen.screen > 1) {
            disp = true;
          }
        }
      }
      delay(50);
    }
  }



  /* Cube */

  else if (app == cube) {
    Screen screen = Screen(1, 1);
    Cube cube = Cube();

    while (true) {
      if (screen.screen == 0 and disp) {
        disp = false;
        display.clearDisplay();
        gameChangerDisplay();
        display.display();
      } else if (screen.screen == 1) {
        display.clearDisplay();
        cube.run(display);
        display.display();
      }

      if (screen.buttons()) {
        if (screen.screen == 0) gameChanger();
        if (digitalRead(5)) { //app selection
          disp = true;
        }
      }
      delay(50);
    }
  }



  /* App Unknown */

  else {
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
