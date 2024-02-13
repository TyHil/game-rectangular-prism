/*
  Game Rectangular Prism Version 2 Code
  Asteroids, Astro Party, Clonium, and Minesweeper games and Random Number Generator
  and Level utilities on an Arduino Nano 33 IoT with a 64x128 OLED screen, 4 capacitive
  touch buttons, an EEPROM module, a 9v battery, and a power switch all in a black 3D
  printed case.
  Written by Tyler Gordon Hill
  Version 9.0
*/
#include "helper.h"
#include "screen.h"
#include "asteroids.h"
#include "astroParty.h"
#include "clonium.h"
#include "minesweeper.h"
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
    resetFunc();
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
        if (screen.screen == -1) gameChanger();
        if (digitalRead(5) or digitalRead(4) or digitalRead(3) or digitalRead(2)) {
          disp = true;
        }
        if (screen.screen >= 0) {
          if (digitalRead(3) and screen.screen < 2) {
            nums[screen.screen] = max(nums[screen.screen] - 1, 1 + 1 * (nums[0] * nums[1] == 2));
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
    Screen screen = Screen(-1, 5, 3);
    uint8_t nums[3] = {8, 4, 5}; //XDim, YDim, Mines

    /*Setup*/
    while ((digitalRead(2) == 0 and digitalRead(3) == 0) or screen.screen != 3) {
      if (disp) { //only display if something changes
        disp = false;
        display.clearDisplay();
        if (screen.screen == -1) { //app selection
          gameChangerDisplay();
        } else { //start display
          displayMinesweeperSetup(display, screen.screen, nums[0], nums[1], nums[2]);
        }
        display.display();
      }
      if (screen.buttons()) {
        if (screen.screen == -1) gameChanger();
        if (digitalRead(5) or digitalRead(4) or digitalRead(3) or digitalRead(2)) {
          disp = true;
        }
        if (screen.screen >= 0) {
          if (digitalRead(3) and screen.screen < 3) {
            nums[screen.screen] = max(nums[screen.screen] - 1, 1 + 1 * (screen.screen != 2 and nums[0] * nums[1] == 2));
            nums[2] = min(nums[0] * nums[1] - 1, nums[2]);
          } else if (digitalRead(2) and screen.screen < 4) {
            nums[screen.screen] = min(nums[screen.screen] + 1, (screen.screen == 0) * 16 + (screen.screen == 1) * 8 + (screen.screen == 2) * (nums[0] * nums[1] - 1));
          } else if (screen.screen == 4 and (digitalRead(3) or digitalRead(2))) { //small board
            nums[0] = 8;
            nums[1] = 4;
            nums[2] = 5;
          } else if (screen.screen == 5 and (digitalRead(3) or digitalRead(2))) { //large board
            nums[0] = 16;
            nums[1] = 8;
            nums[2] = 19;
          }
        }
      }
      delay(50);
    }
    waitAllUnclick();
    Minesweeper minesweeper = Minesweeper(nums[0], nums[1], nums[2]);

    /*Game*/
    minesweeper.run(display);
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
