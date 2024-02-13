/*
  Clonium app header
  Written by Tyler Gordon Hill
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

void displayCloniumSetup(Adafruit_SSD1306& display, int16_t screen, uint8_t XDim, uint8_t YDim);

class CloniumBoard {
    uint8_t numNextToNumBonus(uint8_t x, uint8_t y, uint8_t nextTo, int8_t bonus, uint8_t turn, uint8_t lastDir);
    uint8_t recursiveEval(uint8_t x, uint8_t y, uint8_t lastDir, uint8_t turn);
  public:
    uint8_t XDim, YDim, sixteenOverArea, fourteenOverArea; //board dimensions, common math
    uint8_t** data; //represents dots and team: 0 is empty, 0-7 is dots for team 1, and 8-14 is dots for team 2: (i-1)%7+1 gets dot number and (i>0)+(i>7) gets team
    boolean** loopPrevention; //prevents looping
    boolean area; //16x16 or 8x8 squares
    CloniumBoard(uint8_t setXDim, uint8_t setYDim);
    ~CloniumBoard();
    uint8_t getTeam(uint8_t x, uint8_t y);
    uint8_t getDots(uint8_t x, uint8_t y);
    void drawSelection(uint8_t x, uint8_t y, boolean flash, Adafruit_SSD1306& display);
    void draw(Adafruit_SSD1306& display);
    void CPUMove(uint8_t turn, Adafruit_SSD1306& display);
    void mover(uint8_t x, uint8_t y, uint8_t turn);
};

class Clonium {
  public:
    uint8_t players;
    uint8_t mx, my, mxLast[2], myLast[2]; //play choice, last choice
    bool turn; //turn, flashing selection
    CloniumBoard board;
    Clonium(uint8_t setXDim, uint8_t setYDim, uint8_t setPlayers);
    void takeTurn(Adafruit_SSD1306& display);
    void winCheck(Adafruit_SSD1306& display);
    void run(Adafruit_SSD1306& display);
};
