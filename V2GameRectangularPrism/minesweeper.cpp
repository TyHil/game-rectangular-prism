/*
  Minesweeper app implementation
  Written by Tyler Gordon Hill
*/
#include "minesweeper.h"
#include "helper.h"
const String names[6] = {"X", "Y", "Mines", "Start", "Small", "Large"};

void displayMinesweeperSetup(Adafruit_SSD1306& display, int16_t screen, uint8_t XDim, uint8_t YDim, uint8_t mines) {
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
    if (i == 0 or i == 1 or i == 2) {
      display.setTextSize(2);
    }
    if (i == 0) {
      display.print(XDim);
    } else if (i == 1) {
      display.print(YDim);
    } else if (i == 2) {
      display.print(mines);
    } else {
      display.print(names[4]);
      display.setCursor(y, 45);
      display.print(names[5]);
    }
  }
  display.setTextColor(BLACK);
  uint8_t y = 1;
  for (uint8_t j = 0; j < min(screen, 3); j++) y += (names[j].length() + 1) * 6 + (j == 0) * 18 + (j == 1) * 6;
  display.fillRect(y - 1, 15 * max(screen + 1, 4) - 46, names[screen].length() * 6 + 1, 9, WHITE);
  display.setCursor(y, 15 * max(screen + 1, 4) - 45);
  display.setTextSize(1);
  display.print(names[screen]);
  display.setTextColor(WHITE);
}



/* Minesweeper Board */

MinesweeperBoard::MinesweeperBoard(uint8_t setXDim, uint8_t setYDim, uint8_t setMines) {
  data = new uint8_t*[setXDim];
  loopPrevention = new bool*[setXDim];
  for (uint8_t i = 0; i < setXDim; i++) {
    data[i] = new uint8_t[setYDim];
    loopPrevention[i] = new bool[setYDim];
  }
  XDim = setXDim;
  YDim = setYDim;
  mines = setMines;
  area = setXDim > 8 or setYDim > 4;
  sixteenOverArea = 16 / (area + 1);
  fourteenOverArea = 14 / (area + 1) - (area + 1) + 1;
  for (uint8_t x = 0; x < XDim; x++) for (uint8_t y = 0; y < YDim; y++) data[x][y] = 9; //set all to 9 which is unknown
}

MinesweeperBoard::~MinesweeperBoard() {
  for (uint8_t i = 0; i < XDim; i++) {
    delete[] data[i];
  }
  delete[] data;
}

void MinesweeperBoard::generateMines(uint8_t mx, uint8_t my) {
  uint8_t currentMineCount = 0;
  while (currentMineCount < mines) {
    uint8_t x = random(0, XDim), y = random(0, YDim);
    if (data[x][y] == 9 and (x != mx or y != my)) {
      data[x][y] = 11;
      currentMineCount++;
    }
  }
}

void MinesweeperBoard::drawNumber(uint8_t x, uint8_t y, Adafruit_SSD1306& display) { //displays a number in the specified grid location if necessary
  if (data[x][y] < 9 and data[x][y] > 0) { //is a number to be displayed
    display.setCursor(x * sixteenOverArea + 3 - area, y * sixteenOverArea + 2 - area);
    display.setTextSize(-area + 2);
    display.print(data[x][y]);
  }
}

void MinesweeperBoard::drawSelection(uint8_t x, uint8_t y, bool flash, Adafruit_SSD1306& display) { //displays a box around currently selected square
  display.drawRect(x * sixteenOverArea + 1, y * sixteenOverArea + 1, fourteenOverArea + (x != 8 * (area + 1) - 1), fourteenOverArea + (y != 4 * (area + 1) - 1), flash xor data[x][y] < 9); //flash selection choice like a cursor blinks
  if (flash) drawNumber(x, y, display);
}

void MinesweeperBoard::draw(bool drawMines, Adafruit_SSD1306& display) { //displays grid and numbers for Minesweeper
  display.clearDisplay();
  grid(XDim, YDim, display); //draw grid
  for (uint8_t x = 0; x < XDim; x++) { //run through grid
    for (uint8_t y = 0; y < YDim; y++) {
      drawNumber(x, y, display);
      if (data[x][y] > 8) { //9 is blank
        display.fillRect(x * sixteenOverArea + 1, y * sixteenOverArea + 1, (14 / (area + 1)) + (x != 8 * (area + 1) - 1), (14 / (area + 1)) + (y != 4 * (area + 1) - 1), WHITE); //blank
        if (data[x][y] == 10 or data[x][y] == 12) { //10 is X is a flag
          display.drawLine(x * sixteenOverArea + 1, y * sixteenOverArea + 1, x * sixteenOverArea + fourteenOverArea + (x != 8 * (area + 1) - 1), y * sixteenOverArea + fourteenOverArea + (y != 4 * (area + 1) - 1), BLACK);
          display.drawLine(x * sixteenOverArea + fourteenOverArea + (x != 8 * (area + 1) - 1), y * sixteenOverArea + 1, x * sixteenOverArea + 1, y * sixteenOverArea + fourteenOverArea + (y != 4 * (area + 1) - 1), BLACK);
        } if (drawMines and (data[x][y] == 11 or data[x][y] == 12)) display.fillCircle(x * sixteenOverArea + (8 / (area + 1)), y * sixteenOverArea + (8 / (area + 1)), (3 / (area + 1)), BLACK); //add a mine if there's a mine there and the function was called with the intent to draw mines
      }
    }
  }
}

void MinesweeperBoard::recursiveMover(uint8_t x, uint8_t y) { //make a move for Minesweeper
  if (!loopPrevention[x][y]) {
    loopPrevention[x][y] = 1;
    int8_t neighborMines = 0;
    for (int8_t i = max(0, x - 1); i < min(XDim, x + 2); i++) for (int8_t j = max(0, y - 1); j < min(YDim, y + 2); j++) if (data[i][j] == 11 or data[i][j] == 12) neighborMines++; //calculate number of mines around
    data[x][y] = neighborMines; //set num mines in data
    if (neighborMines == 0) for (int8_t i = max(0, x - 1); i < min(XDim, x + 2); i++) for (int8_t j = max(0, y - 1); j < min(YDim, y + 2); j++) if (data[i][j] == 9) recursiveMover(i, j); //expand to empty spaces for ease of the game
  }
}

void MinesweeperBoard::mover(uint8_t x, uint8_t y) { //make a move for Minesweeper
  for (uint8_t i = 0; i < XDim; i++) for (uint8_t j = 0; j < YDim; j++) loopPrevention[i][j] = 0;
  recursiveMover(x, y);
}

bool MinesweeperBoard::winCheck() {
  uint8_t discoveredSquares = 0; //found any mines that have not been flagged
  for (uint8_t x = 0; x < XDim; x++) for (uint8_t y = 0; y < YDim; y++) if (data[x][y] < 9) discoveredSquares++; //check for win
  return XDim * YDim - discoveredSquares == mines;
}



/* Clonium */

Minesweeper::Minesweeper(uint8_t setXDim, uint8_t setYDim, uint8_t setMines): board(setXDim, setYDim, setMines) {
  mx = 0;
  my = 0;
  gen = 1;
  scoreTime = millis();
}

void Minesweeper::takeTurn(Adafruit_SSD1306& display) {
  bool disp = true, flash = false;
  uint64_t buttonTime = millis(), flashTime = millis();
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
    if (millis() - buttonTime >= 100) {
      if (digitalRead(4)) {
        mx = (mx + 1) % board.XDim;
        buttonTime = millis();
        disp = true;
      } else if (digitalRead(5)) {
        my = (my + 1) % board.YDim;
        buttonTime = millis();
        disp = true;
      } else if (digitalRead(3) and board.data[mx][my] > 8) {
        if (board.data[mx][my] < 11) board.data[mx][my] = -board.data[mx][my] + 19; //swap 9 and 10
        else board.data[mx][my] = -board.data[mx][my] + 23; //swap 11 and 12
        disp = true;
      }
    }
    delay(50);
  }
}

void Minesweeper::loseCheck(Adafruit_SSD1306& display) {
  if (board.data[mx][my] == 11) {
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
    resetFunc();
  }
}

void Minesweeper::winCheck(Adafruit_SSD1306& display) {
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
    resetFunc();
  }
}


void Minesweeper::run(Adafruit_SSD1306& display) {
  while (true) {
    takeTurn(display);
    if (gen) { //generate mines first time through
      board.generateMines(mx, my);
      gen = 0;
    }
    loseCheck(display);
    board.mover(mx, my);
    board.draw(0, display);
    display.display();
    winCheck(display);
    delay(200);
  }
}
