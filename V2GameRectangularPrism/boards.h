/*
  Board library header for Clonium and Minesweeper
  Written by Tyler Gordon Hill
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class cloniumBoard {
    uint8_t numNextToNumBonus(uint8_t x, uint8_t y, uint8_t nextTo, int8_t bonus, uint8_t turn, uint8_t lastDir);
    uint8_t recursiveEval(uint8_t x, uint8_t y, uint8_t lastDir, uint8_t turn);
  public:
    uint8_t XDim, YDim, sixteenOverArea, fourteenOverArea; //board dimensions, common math
    uint8_t** data; //represents dots and team: 0 is empty, 0-7 is dots for team 1, and 8-14 is dots for team 2: (i-1)%7+1 gets dot number and (i>0)+(i>7) gets team
    boolean** loopPrevention; //prevents looping
    boolean area; //16x16 or 8x8 squares
    cloniumBoard(uint8_t setXDim, uint8_t setYDim);
    ~cloniumBoard();
    uint8_t getTeam(uint8_t x, uint8_t y);
    uint8_t getDots(uint8_t x, uint8_t y);
    void drawSelection(uint8_t x, uint8_t y, boolean flash, Adafruit_SSD1306& display);
    void draw(Adafruit_SSD1306& display);
    void CPUMove(uint8_t turn, Adafruit_SSD1306& display);
    void mover(uint8_t x, uint8_t y, uint8_t turn);
};

class minesweeperBoard {
  void recursiveMover(uint8_t x, uint8_t y);
  public:
    uint8_t XDim, YDim, mines, sixteenOverArea, fourteenOverArea; //board dimensions, number of mines, common math
    uint8_t** data; //board data: 0-8 mean discovered with corresponing neighbor mines, 9 is unknown, 10 is flagged, 11 is an unknown mine, and 12 is a flagged mine
    boolean** loopPrevention; //prevents looping
    boolean area; //16x16 or 8x8 squares
    minesweeperBoard(uint8_t setXDim, uint8_t setYDim, uint8_t setMines);
    ~minesweeperBoard();
    void generateMines(uint8_t mx, uint8_t my);
    void drawNumber(uint8_t x, uint8_t y, Adafruit_SSD1306& display);
    void drawSelection(uint8_t x, uint8_t y, boolean flash, Adafruit_SSD1306& display);
    void draw(boolean drawMines, Adafruit_SSD1306& display);
    void mover(uint8_t x, uint8_t y);
    boolean winCheck();
};
