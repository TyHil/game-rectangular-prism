/*
  Board library header for Clonium and Minesweeper
  Written by Tyler Gordon Hill
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class MinesweeperBoard {
  void recursiveMover(uint8_t x, uint8_t y);
  public:
    uint8_t XDim, YDim, mines, sixteenOverArea, fourteenOverArea; //board dimensions, number of mines, common math
    uint8_t** data; //board data: 0-8 mean discovered with corresponing neighbor mines, 9 is unknown, 10 is flagged, 11 is an unknown mine, and 12 is a flagged mine
    boolean** loopPrevention; //prevents looping
    boolean area; //16x16 or 8x8 squares
    MinesweeperBoard(uint8_t setXDim, uint8_t setYDim, uint8_t setMines);
    ~MinesweeperBoard();
    void generateMines(uint8_t mx, uint8_t my);
    void drawNumber(uint8_t x, uint8_t y, Adafruit_SSD1306& display);
    void drawSelection(uint8_t x, uint8_t y, boolean flash, Adafruit_SSD1306& display);
    void draw(boolean drawMines, Adafruit_SSD1306& display);
    void mover(uint8_t x, uint8_t y);
    boolean winCheck();
};
