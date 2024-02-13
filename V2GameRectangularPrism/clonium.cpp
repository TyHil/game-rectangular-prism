/*
  Clonium app implementation
  Written by Tyler Gordon Hill
*/
#include "clonium.h"
#include "helper.h"
const String names[7] = {"X", "Y", "Small", "Large", "2P", "1P", "0P"}; //gamemodes

void displayCloniumSetup(Adafruit_SSD1306& display, int16_t screen, uint8_t XDim, uint8_t YDim) {
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
    if (i == 0 or i == 1) {
      display.setTextSize(2);
    }
    if (i == 0) {
      display.print(XDim);
    } else if (i == 1) {
      display.print(YDim);
    } else {
      display.print(names[i + 1 + (i == 3)]);
    }
    if (i == 3) {
      display.setCursor(y, 45);
      display.print(names[6]);
    }
  }
  display.setTextColor(BLACK);
  uint8_t y = 16;
  for (uint8_t j = 0; j < min(screen - (screen == 3), 3); j++) y += (names[j].length() + 1) * 6 + (j == 0) * 18 + (j == 1) * 6;
  display.fillRect(y - 1, 15 * max(screen + 2 * (screen == 3), 4) - 46, names[screen].length() * 6 + 1, 9, WHITE);
  display.setCursor(y, 15 * max(screen + 2 * (screen == 3), 4) - 45);
  display.setTextSize(1);
  display.print(names[screen]);
  display.setTextColor(WHITE);
}



/* Board */

CloniumBoard::CloniumBoard(uint8_t setXDim, uint8_t setYDim) {
  data = new uint8_t*[setXDim];
  loopPrevention = new bool*[setXDim];
  for (uint8_t i = 0; i < setXDim; i++) {
    data[i] = new uint8_t[setYDim];
    loopPrevention[i] = new bool[setYDim];
  }
  XDim = setXDim;
  YDim = setYDim;
  area = setXDim > 8 or setYDim > 4;
  sixteenOverArea = 16 / (area + 1);
  fourteenOverArea = 14 / (area + 1) - (area + 1) + 1;
  for (uint8_t x = 0; x < setXDim; x++) for (uint8_t y = 0; y < setYDim; y++) data[x][y] = 0;
  bool spaceFromEdges = XDim > 3 and YDim > 3 and (XDim != 4 or YDim != 4); //is the board big enough to accomidate space away from the edges for starting pieces
  data[min(spaceFromEdges, XDim - 1)][min(spaceFromEdges, YDim - 1)] = 3; //starting posistions
  data[max(XDim - 1 - spaceFromEdges, 0)][max(YDim - 1 - spaceFromEdges, 0)] = 10;
}

CloniumBoard::~CloniumBoard() {
  for (uint8_t i = 0; i < XDim; i++) {
    delete[] data[i];
  }
  delete[] data;
}

uint8_t CloniumBoard::getTeam(uint8_t x, uint8_t y) {
  return (data[x][y] > 0) + (data[x][y] > 7);
}

uint8_t CloniumBoard::getDots(uint8_t x, uint8_t y) {
  return (data[x][y] - 1) % 7 + 1;
}

//displays a box around currently selected square
void CloniumBoard::drawSelection(uint8_t x, uint8_t y, bool flash, Adafruit_SSD1306& display) {
  display.drawRect(x * sixteenOverArea + 1, y * sixteenOverArea + 1, fourteenOverArea + (x != 8 * (area + 1) - 1), fourteenOverArea + (y != 4 * (area + 1) - 1), flash xor getTeam(x, y) != 1); //flash selection choice like a cursor blinks
}

void CloniumBoard::draw(Adafruit_SSD1306& display) {
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

uint8_t CloniumBoard::numNextToNumBonus(uint8_t x, uint8_t y, uint8_t nextTo, int8_t bonus, uint8_t turn, uint8_t lastDir) {
  uint8_t c = 0;
  if (y != YDim - 1 and getDots(x, y + 1) == nextTo and lastDir != 1 and getTeam(x, y + 1) - 1 != turn) c += bonus; //bonus for putting a 2 next to an enemy's 1
  if (x != XDim - 1 and getDots(x + 1, y) == nextTo and lastDir != 3 and getTeam(x + 1, y) - 1 != turn) c += bonus;
  if (y != 0 and getDots(x, y - 1) == nextTo and lastDir != 2 and getTeam(x, y - 1) - 1 != turn) c += bonus;
  if (x != 0 and getDots(x - 1, y) == nextTo and lastDir != 4 and getTeam(x - 1, y) - 1 != turn) c += bonus;
  return c;
}

uint8_t CloniumBoard::recursiveEval(uint8_t x, uint8_t y, uint8_t lastDir, uint8_t turn) {
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

void CloniumBoard::CPUMove(uint8_t turn, Adafruit_SSD1306& display) {
  int8_t c, fc = -128; //evaluation variables
  bool f[XDim][YDim];
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

void CloniumBoard::mover(uint8_t x, uint8_t y, uint8_t turn) {
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



/* Clonium */

Clonium::Clonium(uint8_t setXDim, uint8_t setYDim, uint8_t setPlayers): board(setXDim, setYDim) {
  players = setPlayers;
  bool spaceFromEdges = setXDim > 3 and setYDim > 3 and (setXDim != 4 or setYDim != 4); //is the board big enough to accomidate space away from the edges for starting pieces
  mx = min(spaceFromEdges, setXDim - 1);
  my = min(spaceFromEdges, setYDim - 1);
  mxLast[0] = mx;
  mxLast[1] = (uint8_t)max(setXDim - 1 - spaceFromEdges, 0);
  myLast[0] = my;
  myLast[1] = (uint8_t)max(setYDim - 1  - spaceFromEdges, 0);
  turn = 0;
}

void Clonium::takeTurn(Adafruit_SSD1306& display) {
  bool disp = false, flash = false;
  uint64_t buttonTime = millis(), flashTime = millis();
  if ((players == 1 and turn) or players == 0) { //CPU
    board.CPUMove(turn, display);
  } else { //player
    disp = true;
    if (players == 2) { //move selection back to old choice in 2 player for ease of use
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
      if (millis() - buttonTime >= 100) { //buttons
        if (digitalRead(5)) {
          mx = (mx + board.XDim - 1) % board.XDim; //+ 7 because mod can output negative numbers
          buttonTime = millis();
          disp = true;
        } else if (digitalRead(4)) {
          mx = (mx + 1) % board.XDim;
          buttonTime = millis();
          disp = true;
        } else if (digitalRead(3)) {
          my = (my + 1) % board.YDim;
          buttonTime = millis();
          disp = true;
        }
      }
      delay(50);
    }
    if (players == 2) {
      mxLast[turn] = mx;
      myLast[turn] = my;
    }
    board.mover(mx, my, turn);
  }
}

void Clonium::winCheck(Adafruit_SSD1306& display) {
  bool count = 1;
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
    resetFunc();
  }
}


void Clonium::run(Adafruit_SSD1306& display) {
  while (true) {
    takeTurn(display);
    board.draw(display);
    display.display();
    if (players == 2 or (players == 1 and !turn)) delay(200);
    winCheck(display);
    turn = !turn; //change turn
  }
}
