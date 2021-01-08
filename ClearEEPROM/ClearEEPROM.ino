#include <Wire.h>
byte readEEPROM(unsigned int location) {//read the value of a spot in EEPROM for highscores and last game played
  Wire.beginTransmission(0x50);
  Wire.write((int)(location >> 8));//MSB
  Wire.write((int)(location & 0xFF));//LSB
  Wire.endTransmission();
  Wire.requestFrom(0x50, 1);
  if (Wire.available()) return Wire.read();
  else return 0;
}
void updateEEPROM(unsigned int location, byte data) {//update the value of a spot in EEPROM for hghscores and last game played
  if (readEEPROM(location) != data) {//avoid writing if data is the same
    Wire.beginTransmission(0x50);
    Wire.write((int)(location >> 8));//MSB
    Wire.write((int)(location & 0xFF));//LSB
    Wire.write(data);
    Wire.endTransmission();
    delay(5);
  }
}
void setup() {
  Wire.begin();
  for (uint8_t i = 0; i < 26; i++) updateEEPROM(i, 0);
}
void loop() {}
