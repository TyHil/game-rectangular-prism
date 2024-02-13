/*
  Thermometer app header
  Written by Tyler Gordon Hill
*/
#include <Adafruit_GFX.h>
#include <Adafruit_SHTC3.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

class Thermometer {
public:
  bool settingsStart; //true on switch to settings page
  int8_t settingsSelect; //option to control in settings
  uint64_t historyTimer; //when to save graph data
  int64_t tempTimer;
  int8_t historyScreenPlace;
  float history[128];
  uint8_t historyRecordPlace;
  float temperature, humidity, runningSum;
  uint16_t sumCount; //temps in running for avg
  uint16_t recordDuration; //duration of recording for history
  bool stop; //stop
  Adafruit_SHTC3 shtc3;
  Thermometer();
  void historyScreenMin();
  void historyScreenMax();
  bool measure(int16_t screen);
  void prepDisplay();
  void prepSettings();
  void prepHistory(bool inc);
  void clearHistory();
  void toggleStop();
  void settingsChoice3();
  void settingsChoice2();
  void mainDisplay(Adafruit_SSD1306 & display);
  void printTime(Adafruit_SSD1306 & display, int seconds);
  void settingsDisplay(Adafruit_SSD1306 & display);
  void historyDisplay(Adafruit_SSD1306 & display);
};
