/*
  Game Rectangular Prism Version 1 Code
  Astro Party on an Arduino Nano Every with a 64x128 OLED screen, 4 buttons, a 9v
  battery, and a power switch all in a white 3D printed case.
  Written by Tyler Gordon Hill
  Version 2.0
  Differences from the Astro Party of Version 2 include:
    Simplification of start display due to lack of need to change between games.
    digitalRead operation in refrence to buttons is reversed.
    Uses reset function instead of writing pin 6 (wired to the reset pin) low.
*/
#include "astroParty.h"
#include "helper.h"
Adafruit_SSD1306 display(128, 64, &Wire, -1);



/* Device Start */

void setup() {
  //Serial.begin(9600); //Serial.println("");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Wire.begin();
  randomSeed(analogRead(1)); //better random
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  display.setTextColor(WHITE);



  /* Astro Party */

  /*Setup*/
  display.clearDisplay();
  displayAstroPartyStart(display);
  display.display();
  waitAnyClick();
  waitAllUnclick();
  AstroParty astroParty = AstroParty();

  /*Game*/
  astroParty.run(display);
}



void loop() {}
