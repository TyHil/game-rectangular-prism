/*
  Minesweeper app header
  Written by Tyler Gordon Hill
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

void displayMinesweeperSetup(Adafruit_SSD1306& display, int16_t screen, uint8_t XDim, uint8_t YDim, uint8_t mines);

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

class Minesweeper {
  public:
    uint8_t mx, my; //play choice
    bool gen;
    uint64_t scoreTime; //timers for flashing selection and score
    MinesweeperBoard board;
    Minesweeper(uint8_t setXDim, uint8_t setYDim, uint8_t setPlayers);
    void takeTurn(Adafruit_SSD1306& display);
    void loseCheck(Adafruit_SSD1306& display);
    void winCheck(Adafruit_SSD1306& display);
    void run(Adafruit_SSD1306& display);
};
