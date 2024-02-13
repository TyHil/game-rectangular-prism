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
#include "asteroids.h"
#include "astroParty.h"
#include "clonium.h"
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
    bool tiltToTurn = false;

    /*Setup*/
    while ((digitalRead(2) == 0 and digitalRead(3) == 0) or screen.screen == 0 or screen.screen == -1 or screen.screen == -2) { //menu and level choice
      if (disp) { //only display if something changes
        disp = false;
        display.clearDisplay();
        if (screen.screen == -2) { //app selection
          gameChangerDisplay();
        } else if (screen.screen == -1) { //highscores
          displayAsteroidsHighScores(display);
        } else if (screen.screen == 0) { //settings
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
        } else { //level choice
          displayAsteroidsLevel(display, screen.screen);
        }
        display.display();
      }
      if (screen.buttons()) {
        if (screen.screen == -2) gameChanger();
        if (digitalRead(5) or digitalRead(4) or digitalRead(3) or digitalRead(2)) {
          disp = true;
        }
        if (screen.screen == 0 and (digitalRead(2) or digitalRead(3))) {
          tiltToTurn = !tiltToTurn;
        }
      }
      delay(50);
    }
    waitAllUnclick();
    Asteroids asteroids = Asteroids(tiltToTurn, screen.screen);

    /*Game*/
    asteroids.run(display);
  }



  /* Astro Party */

  else if (app == astroParty) {
    Screen screen = Screen(1, 1);

    /*Setup*/
    while ((digitalRead(2) == 0 and digitalRead(3) == 0) or screen.screen == 0) {
      if (disp) { //only display if something changes
        disp = false;
        display.clearDisplay();
        if (screen.screen == 0) { //app selection
          gameChangerDisplay();
        } else if (screen.screen == 1) { //start display
          displayAstroPartyStart(display);
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
    AstroParty astroParty = AstroParty();

    /*Game*/
    astroParty.run(display);
  }



  /* Clonium */

  else if (app == clonium) {
    Screen screen = Screen(-1, 6, 4);
    uint8_t nums[2] = {8, 4}; //XDim, YDim

    /*Setup*/
    while ((digitalRead(2) == 0 and digitalRead(3) == 0) or (screen.screen != 4 and screen.screen != 5 and screen.screen != 6)) {
      if (disp) { //only display if something changes
        disp = false;
        display.clearDisplay();
        if (screen.screen == -1) { //app selection
          gameChangerDisplay();
        } else { //start display
          displayCloniumSetup(display, screen.screen, nums[0], nums[1]);
        }
        display.display();
      }
      if (screen.buttons()) {
        if (screen.screen == 0) gameChanger();
        if (digitalRead(5) or digitalRead(4) or digitalRead(3) or digitalRead(2)) {
          disp = true;
        }
        if (screen.screen >= 0) {
          if (digitalRead(3) and screen.screen < 2) {
            nums[screen.screen] = max(nums[level] - 1, 1 + 1 * (nums[0] * nums[1] == 2));
          } else if (digitalRead(2) and screen.screen < 2) {
            nums[screen.screen] = min(nums[screen.screen] + 1, !screen.screen * 16 + screen.screen * 8);
          } else if (screen.screen == 2 and (digitalRead(3) or digitalRead(2))) { //small board
            nums[0] = 8;
            nums[1] = 4;
          } else if (screen.screen == 3 and (digitalRead(3) or digitalRead(2))) { //large board
            nums[0] = 16;
            nums[1] = 8;
          }
        }
      }
      delay(50);
    }
    waitAllUnclick();
    Clonium clonium = Clonium(nums[0], nums[1], screen.screen == 6 ? 0 : (screen.screen == 5 ? 1 : 2));

    /*Game*/
    clonium.run(display);
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
