/*
  Board library implementation for Clonium and Minesweeper
  Written by Tyler Gordon Hill
*/
#include "boards.h"

/*Functions*/

void grid(uint8_t XDim, uint8_t YDim, Adafruit_SSD1306& display) { //draw grid for Clonium or Minesweeper
  const boolean area = XDim > 8 or YDim > 4;
  for (uint8_t i = 0; i <= XDim * (-8 * area + 16); i += 16 / (area + 1)) display.drawLine(i, 0, i, YDim * (-8 * area + 16), WHITE);
  for (uint8_t i = 0; i <= YDim * (-8 * area + 16); i += 16 / (area + 1)) display.drawLine(0, i, XDim * (-8 * area + 16), i, WHITE);
  if (XDim % 8 == 0) display.drawLine(127, 0, 127, YDim * (-8 * area + 16), WHITE); //rightmost line
  if (YDim % 4 == 0) display.drawLine(0, 63, XDim * (-8 * area + 16), 63, WHITE); //bottom line
}

/*Clonium Board Methods*/

cloniumBoard::cloniumBoard(uint8_t setXDim, uint8_t setYDim) {
  data = new uint8_t*[setXDim];
  loopPrevention = new boolean*[setXDim];
  for (uint8_t i = 0; i < setXDim; i++) {
    data[i] = new uint8_t[setYDim];
    loopPrevention[i] = new boolean[setYDim];
  }
  XDim = setXDim;
  YDim = setYDim;
  area = setXDim > 8 or setYDim > 4;
  sixteenOverArea = 16 / (area + 1);
  fourteenOverArea = 14 / (area + 1) - (area + 1) + 1;
  for (uint8_t x = 0; x < setXDim; x++) for (uint8_t y = 0; y < setYDim; y++) data[x][y] = 0;
  boolean spaceFromEdges = XDim > 3 and YDim > 3 and (XDim != 4 or YDim != 4); //is the board big enough to accomidate space away from the edges for starting pieces
  data[min(spaceFromEdges, XDim - 1)][min(spaceFromEdges, YDim - 1)] = 3; //starting posistions
  data[max(XDim - 1 - spaceFromEdges, 0)][max(YDim - 1 - spaceFromEdges, 0)] = 10;
}
cloniumBoard::~cloniumBoard() {
  for (uint8_t i = 0; i < XDim; i++) {
    delete[] data[i];
  }
  delete[] data;
}
uint8_t cloniumBoard::getTeam(uint8_t x, uint8_t y) {
  return (data[x][y] > 0) + (data[x][y] > 7);
}
uint8_t cloniumBoard::getDots(uint8_t x, uint8_t y) {
  return (data[x][y] - 1) % 7 + 1;
}
void cloniumBoard::drawSelection(uint8_t x, uint8_t y, boolean flash, Adafruit_SSD1306& display) { //displays a box around currently selected square
  display.drawRect(x * sixteenOverArea + 1, y * sixteenOverArea + 1, fourteenOverArea + (x != 8 * (area + 1) - 1), fourteenOverArea + (y != 4 * (area + 1) - 1), flash xor getTeam(x, y) != 1); //flash selection choice like a cursor blinks
}
void cloniumBoard::draw(Adafruit_SSD1306& display) {
  display.clearDisplay();
  grid(XDim, YDim, display); //draw grid
  for (uint8_t x = 0; x < XDim; x++) { //dots
    for (uint8_t y = 0; y < YDim; y++) {
      if (getDots(x, y) > 0) {
        display.fillRect(x * sixteenOverArea + 1, y * sixteenOverArea + 1, (16 / (area + 1)) - 1 - (x == 8 * (area + 1) - 1), (16 / (area + 1)) - 1 - (y == 4 * (area + 1) - 1), !(getTeam(x, y) - 1)); //fill with reverse team color
        display.fillRect(x * sixteenOverArea + (6 / (area + 1) + area), y * sixteenOverArea + (3 / (area + 1) + area), (4 / (area + 1)), (4 / (area + 1)), getTeam(x, y) - 1); //1st dot
      }
      if (getDots(x, y) > 1) display.fillRect(x * sixteenOverArea + (9 / (area + 1) + area), y * sixteenOverArea + (9 / (area + 1) + area), (4 / (area + 1)), (4 / (area + 1)), getTeam(x, y) - 1); //2nd dot
      if (getDots(x, y) > 2) display.fillRect(x * sixteenOverArea + (3 / (area + 1) + area), y * sixteenOverArea + (9 / (area + 1) + area), (4 / (area + 1)), (4 / (area + 1)), getTeam(x, y) - 1); //3rd dot
    }
  }
}
uint8_t cloniumBoard::numNextToNumBonus(uint8_t x, uint8_t y, uint8_t nextTo, int8_t bonus, uint8_t turn, uint8_t lastDir) {
  uint8_t c = 0;
  if (y != YDim - 1 and getDots(x, y + 1) == nextTo and lastDir != 1 and getTeam(x, y + 1) - 1 != turn) c += bonus; //bonus for putting a 2 next to an enemy's 1
  if (x != XDim - 1 and getDots(x + 1, y) == nextTo and lastDir != 3 and getTeam(x + 1, y) - 1 != turn) c += bonus;
  if (y != 0 and getDots(x, y - 1) == nextTo and lastDir != 2 and getTeam(x, y - 1) - 1 != turn) c += bonus;
  if (x != 0 and getDots(x - 1, y) == nextTo and lastDir != 4 and getTeam(x - 1, y) - 1 != turn) c += bonus;
  return c;
}
uint8_t cloniumBoard::recursiveEval(uint8_t x, uint8_t y, uint8_t lastDir, uint8_t turn) {
  if (loopPrevention[x][y]) return 0; //stop a looping situation like a square of 3s
  else {
    loopPrevention[x][y] = 1;
    int8_t c = max(-2, -(y == 0) - (y == 3) - (x == 0) - (x == 7)); //value given to choice, devalue edges
    if (getDots(x, y) == 1) {
      c++; //normal 1 dot bonus
      c += numNextToNumBonus(x, y, 2, 1, turn, lastDir);
    } else if (getDots(x, y) == 2) { //if it's a 2
      int8_t cTemp = c;
      c += numNextToNumBonus(x, y, 3, -4, turn, lastDir);
      if (c == cTemp) {
        c += 2; //normal bonus only applied when none of the 4 above are true
        c += numNextToNumBonus(x, y, 2, 2, turn, lastDir);
      }
    } else if (getDots(x, y) == 3) { //if it's a 3
      c += 3; //3 value
      if (y != YDim - 1 and getDots(x, y + 1) == 3 and lastDir != 1) c += recursiveEval(x, y + 1, 2, turn); //call the same function on neighboring places as a 4 expands into them
      if (x != XDim - 1 and getDots(x + 1, y) == 3 and lastDir != 3) c += recursiveEval(x + 1, y, 4, turn);
      if (y != 0 and getDots(x, y - 1) == 3 and lastDir != 2) c += recursiveEval(x, y - 1, 1, turn);
      if (x != 0 and getDots(x - 1, y) == 3 and lastDir != 4) c += recursiveEval(x - 1, y, 3, turn);
    }
    return c;
  }
}
void cloniumBoard::CPUMove(uint8_t turn, Adafruit_SSD1306& display) {
  int8_t c, fc = -128; //evaluation variables
  boolean f[XDim][YDim];
  for (uint8_t i = 0; i < XDim; i++) for (uint8_t j = 0; j < YDim; j++) f[i][j] = 0;
  for (uint8_t x = 0; x < XDim; x++) { //go though each square and give it a score
    for (uint8_t y = 0; y < YDim; y++) {
      if (getTeam(x, y) - 1 == turn) {
        for (uint8_t i = 0; i < XDim; i++) for (uint8_t j = 0; j < YDim; j++) loopPrevention[i][j] = 0;
        c = recursiveEval(x, y, 0, turn); //evaluate choice
        if (c == fc) { //add to list if same evaluation
          f[x][y] = 1;
        } else if (c > fc) { //delete list and add new value if higher than high evaluation
          fc = c;
          for (uint8_t i = 0; i < XDim; i++) for (uint8_t j = 0; j < YDim; j++) f[i][j] = 0;
          f[x][y] = 1;
        }
      }
    }
  }
  uint8_t rx, ry;
  do {
    rx = random(0, XDim);
    ry = random(0, YDim);
  } while (!f[rx][ry]);
  draw(display); //display results of players move and ai choice
  drawSelection(rx, ry, 0, display);
  display.display();
  delay(100);
  while (digitalRead(2) == 0) {}
  delay(200);
  mover(rx, ry, turn);
}
void cloniumBoard::mover(uint8_t x, uint8_t y, uint8_t turn) {
  if (getDots(x, y) < 3) { //add 1 to places
    data[x][y]++;
    data[x][y] = getDots(x, y) + turn * 7; //capture
  } else {
    data[x][y] -= 3;
    if (data[x][y] == 7) data[x][y] = 0;
    if (getDots(x, y) == 0) data[x][y] = 0; //if now empty set team to not p1 or p2
    if (x != 0) mover(x - 1, y, turn); //call the same function on neighboring places as a 4+ expands into them
    if (x != XDim - 1) mover(x + 1, y, turn);
    if (y != 0) mover(x, y - 1, turn);
    if (y != YDim - 1) mover(x, y + 1, turn);
  }
}

/*Minesweeper Board Methods*/

minesweeperBoard::minesweeperBoard(uint8_t setXDim, uint8_t setYDim, uint8_t setMines) {
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
minesweeperBoard::~minesweeperBoard() {
  for (uint8_t i = 0; i < XDim; i++) {
    delete[] data[i];
  }
  delete[] data;
}
void minesweeperBoard::generateMines(uint8_t mx, uint8_t my) {
  uint8_t currentMineCount = 0;
  while (currentMineCount < mines) {
    uint8_t x = random(0, XDim), y = random(0, YDim);
    if (data[x][y] == 9 and (x != mx or y != my)) {
      data[x][y] = 11;
      currentMineCount++;
    }
  }
}
void minesweeperBoard::drawNumber(uint8_t x, uint8_t y, Adafruit_SSD1306& display) { //displays a number in the specified grid location if necessary
  if (data[x][y] < 9 and data[x][y] > 0) { //is a number to be displayed
    display.setCursor(x * sixteenOverArea + 3 - area, y * sixteenOverArea + 2 - area);
    display.setTextSize(-area + 2);
    display.print(data[x][y]);
  }
}
void minesweeperBoard::drawSelection(uint8_t x, uint8_t y, boolean flash, Adafruit_SSD1306& display) { //displays a box around currently selected square
  display.drawRect(x * sixteenOverArea + 1, y * sixteenOverArea + 1, fourteenOverArea + (x != 8 * (area + 1) - 1), fourteenOverArea + (y != 4 * (area + 1) - 1), flash xor data[x][y] < 9); //flash selection choice like a cursor blinks
  if (flash) drawNumber(x, y, display);
}
void minesweeperBoard::draw(boolean drawMines, Adafruit_SSD1306& display) { //displays grid and numbers for Minesweeper
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
void minesweeperBoard::recursiveMover(uint8_t x, uint8_t y) { //make a move for Minesweeper
  if (!loopPrevention[x][y]) {
    loopPrevention[x][y] = 1;
    int8_t neighborMines = 0;
    for (int8_t i = max(0, x - 1); i < min(XDim, x + 2); i++) for (int8_t j = max(0, y - 1); j < min(YDim, y + 2); j++) if (data[i][j] == 11 or data[i][j] == 12) neighborMines++; //calculate number of mines around
    data[x][y] = neighborMines; //set num mines in data
    if (neighborMines == 0) for (int8_t i = max(0, x - 1); i < min(XDim, x + 2); i++) for (int8_t j = max(0, y - 1); j < min(YDim, y + 2); j++) if (data[i][j] == 9) recursiveMover(i, j); //expand to empty spaces for ease of the game
  }
}
void minesweeperBoard::mover(uint8_t x, uint8_t y) { //make a move for Minesweeper
  for (uint8_t i = 0; i < XDim; i++) for (uint8_t j = 0; j < YDim; j++) loopPrevention[i][j] = 0;
  recursiveMover(x, y);
}
boolean minesweeperBoard::winCheck() {
  uint8_t discoveredSquares = 0; //found any mines that have not been flagged
  for (uint8_t x = 0; x < XDim; x++) for (uint8_t y = 0; y < YDim; y++) if (data[x][y] < 9) discoveredSquares++; //check for win
  return XDim * YDim - discoveredSquares == mines;
}
