/*
  Random Number Generator app implementation
  Written by Tyler Gordon Hill
*/
#include "randomNum.h"
const String names[5] = {"Random Number", "Min", "Max", "Dec", "Result"}; //names



/* RandomNum */

RandomNum::RandomNum() : nums{0, 1, 0} {
  result = random(0, 2);
}

void RandomNum::incNums(int16_t screen) {
  nums[screen - 1]++;
  nums[0] = min(nums[0], nums[1] - 1);
  nums[2] = min(max(nums[2], 0), 2);
}

void RandomNum::decNums(int16_t screen) {
  nums[screen - 1]--;
  nums[1] = max(nums[0] + 1, nums[1]);
  nums[2] = min(max(nums[2], 0), 2);
}

void RandomNum::generate() {
  result = (float)random(nums[0] * pow(10, nums[2]), nums[1] * pow(10, nums[2]) + 1) / pow(10, nums[2]);
}

void RandomNum::display(Adafruit_SSD1306 & display, int16_t screen) {
  for (uint8_t i = 0; i < 5; i++) { //print names
    display.setCursor(1, 9 * i);
    display.print(names[i]);
  }
  for (uint8_t i = 1; i < 5; i++) { //print nums
    display.setCursor(40, 9 * i);
    if (i < 4) display.print(nums[i - 1]);
    else display.print(result);
  }
  display.fillRect(0, 9 * (screen), names[screen].length() * 6 + 1, 8, WHITE); //highlight selection
  display.setTextColor(BLACK);
  display.setCursor(1, 9 * (screen));
  display.print(names[screen]);
  display.setTextColor(WHITE);
}
