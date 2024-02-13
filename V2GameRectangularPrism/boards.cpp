/*
  Board library implementation for Clonium and Minesweeper
  Written by Tyler Gordon Hill
*/
#include "boards.h"
#include "helper.h"



/* Minesweeper Board */

MinesweeperBoard::MinesweeperBoard(uint8_t setXDim, uint8_t setYDim, uint8_t setMines) {
  data = new uint8_t*[setXDim];
  loopPrevention = new boolean*[setXDim];
  for (uint8_t i = 0; i < setXDim; i++) {
    data[i] = new uint8_t[setYDim];
    loopPrevention[i] = new boolean[setYDim];
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

void MinesweeperBoard::drawSelection(uint8_t x, uint8_t y, boolean flash, Adafruit_SSD1306& display) { //displays a box around currently selected square
  display.drawRect(x * sixteenOverArea + 1, y * sixteenOverArea + 1, fourteenOverArea + (x != 8 * (area + 1) - 1), fourteenOverArea + (y != 4 * (area + 1) - 1), flash xor data[x][y] < 9); //flash selection choice like a cursor blinks
  if (flash) drawNumber(x, y, display);
}

void MinesweeperBoard::draw(boolean drawMines, Adafruit_SSD1306& display) { //displays grid and numbers for Minesweeper
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

boolean MinesweeperBoard::winCheck() {
  uint8_t discoveredSquares = 0; //found any mines that have not been flagged
  for (uint8_t x = 0; x < XDim; x++) for (uint8_t y = 0; y < YDim; y++) if (data[x][y] < 9) discoveredSquares++; //check for win
  return XDim * YDim - discoveredSquares == mines;
}
