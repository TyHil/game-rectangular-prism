/*
  Thermometer app implementation
  Written by Tyler Gordon Hill
*/
#include "thermometer.h"
#include <Arduino_LSM6DS3.h>



/* Thermometer */

Thermometer::Thermometer() {
  settingsStart = true;
  settingsSelect = 0;
  historyTimer = millis();
  tempTimer = -1001;
  historyScreenPlace = -1;
  for (uint8_t i = 0; i < 128; i++) history[i] = -1;
  historyRecordPlace = 0;
  temperature = 0;
  humidity = 0;
  runningSum = 0;
  sumCount = 0;
  recordDuration = 1;
  stop = false;
  shtc3 = Adafruit_SHTC3();
  shtc3.begin();
}

void Thermometer::historyScreenMin() {
  historyScreenPlace = 0;
}

void Thermometer::historyScreenMax() {
  historyScreenPlace = max(historyRecordPlace - 1, 0);
}

bool Thermometer::measure(int16_t screen) {
  bool setDispTrue = false;

  //run every second on display page and in background when recording history
  if ((!stop or screen == 1) and millis() - tempTimer >= 1000) {
    tempTimer = millis();
    sensors_event_t _humidity, _temperature;
    shtc3.getEvent(&_humidity, &_temperature);
    temperature = _temperature.temperature * (9.0/5.0) + 32;
    humidity = _humidity.relative_humidity;
    if (!stop) {
      runningSum += temperature;
      sumCount++;
    }
    if (screen == 1 or (screen == 3 and !stop)) {
      setDispTrue = true;
    }
  }

  //record every recordDuration seconds, keep (millis() - historyTimer) at 0 while stopped
  if (stop or millis() - historyTimer >= recordDuration * 1000) {
    historyTimer = millis();
    if (!stop) {
      if (historyRecordPlace == 128) { //shift and add
        for (uint8_t i = 0; i < 127; i++) history[i] = history[i + 1];
        history[127] = runningSum / sumCount;
      } else {
        history[historyRecordPlace] = runningSum / sumCount;
        if (screen == 3 and historyScreenPlace + 1 == historyRecordPlace) {
          historyScreenPlace++;
        }
        historyRecordPlace++;
      }
      runningSum = 0;
      sumCount = 0;
      if (screen == 3) {
        setDispTrue = true;
      }
    }
  }

  return setDispTrue;
}

void Thermometer::prepDisplay() {
  tempTimer = -1001;
}

void Thermometer::prepSettings() {
  settingsStart = true;
  settingsSelect = 0;
}

void Thermometer::prepHistory(bool inc) {
  if (inc) {
    historyScreenPlace = min(historyScreenPlace + 1, historyRecordPlace - 1);
  } else {
    historyScreenPlace = max(historyScreenPlace - 1, -1);
  }
}

void Thermometer::clearHistory() {
  for (uint8_t i = 0; i < 128; i++) history[i] = -1;
  historyRecordPlace = 0;
}

void Thermometer::toggleStop() {
  stop = !stop;
  if (!stop) { //clear
    clearHistory();
  }
}

void Thermometer::settingsChoice3() {
  if (settingsStart) { //select setting to change
    settingsSelect = (settingsSelect + 1) % 2;
  } else if (settingsSelect == 0) { //recordDuration change
    recordDuration = max(recordDuration - 1, 1);
    clearHistory();
  } else if (settingsSelect == 1) { //stop change
    toggleStop();
  }
}

void Thermometer::settingsChoice2() {
  if (settingsStart) { //confirm setting to change
    settingsStart = false;
  } else if (settingsSelect == 0) { //recordDuration change
    recordDuration = min(recordDuration + 1, 300);
    clearHistory();
  } else if (settingsSelect == 1) { //stop change
    toggleStop();
  }
}

void Thermometer::mainDisplay(Adafruit_SSD1306& display) {
  String tempString = String(temperature, 1);
  if (temperature < 100) {
    display.setTextSize(4);
    display.setCursor(64 - 10 * (tempString.length() + 1) - 2 * tempString.length(), 8);
  } else {
    display.setTextSize(3);
    display.setCursor(64 - 7.5 * (tempString.length() + 1) - 1.5 * tempString.length(), 12);
  }
  display.print(tempString);
  display.write(0xF8); //degree symbol
  String humidString = String(humidity, 0) + '%';
  display.setTextSize(2);
  if (temperature >= 80) {
    display.setCursor(2, 48);
  } else {
    display.setCursor(64 - 5 * humidString.length() - (humidString.length() - 1), 48);
  }
  display.print(humidString);
  if (temperature >= 80) { //heat index only valid for temp >= 80
    //From https://meteor.geol.iastate.edu/~ckarsten/bufkit/apparent_temperature.html
    String heatIndex = String(-42.38 + 2.049*temperature + 10.14*humidity + -0.2248*temperature*humidity + -0.006838*temperature*temperature + -0.05482*humidity*humidity + 0.001228*temperature*temperature*humidity + 0.0008528*temperature*humidity*humidity + -0.00000199*temperature*temperature*humidity*humidity, 1);
    display.setCursor(128 - 10 * (heatIndex.length() + 1) - 2 * heatIndex.length() - 2, 48);
    display.print(heatIndex);
    display.write(0xF8);
  }
}

void Thermometer::printTime(Adafruit_SSD1306& display, int seconds) {
  uint16_t minutes = seconds / 60;
  if (minutes > 0) {
    display.print(minutes);
    display.print('m');
  }
  display.print(seconds % 60);
  display.print('s');
}

void Thermometer::settingsDisplay(Adafruit_SSD1306& display) {
  display.setTextSize(1);
  display.setCursor(40, 0);
  display.print("Settings");
  if (settingsSelect == 0) {
    display.fillRect(0, 8, 127, 18, WHITE);
  } else {
    display.fillRect(0, 26, 127, 9, WHITE);
  }
  display.setCursor(2, 9); //sample duration
  display.setTextColor(settingsSelect == 0 ? BLACK : WHITE);
  display.print("Sample time: ");
  display.print(recordDuration);
  display.print('s');
  display.setCursor(7, 18);
  display.print("Total time: ");
  printTime(display, 128 * recordDuration);
  display.setCursor(2, 27); //stop recording
  display.setTextColor(settingsSelect == 1 ? BLACK : WHITE);
  display.print(stop ? "Restart" : "Stop");
  display.setTextColor(WHITE);
}

void Thermometer::historyDisplay(Adafruit_SSD1306& display) {
  for (uint8_t i = 0; i < historyRecordPlace; i++) {
    if (i == historyScreenPlace) { //selected
      display.drawFastVLine(i, 0, 55, WHITE);
      display.drawPixel(i, min(54, max(0, history[i] / 100 * -54 + 54)), BLACK);
    } else { //normal
      display.drawPixel(i, min(54, max(0, history[i] / 100 * -54 + 54)), WHITE);
    }
  }
  display.drawFastHLine(0, 55, 128, WHITE);
  display.setTextSize(1);
  display.setCursor(0, 57); //recordDuration period
  printTime(display, (historyRecordPlace - historyScreenPlace) * recordDuration + (int)((millis() - historyTimer) / 1000));
  display.print("-");
  printTime(display, (historyRecordPlace - (historyScreenPlace + 1)) * recordDuration + (int)((millis() - historyTimer) / 1000));
  display.print(": ");
  display.print(String(history[historyScreenPlace], 1));
  display.write(0xF8);
}
