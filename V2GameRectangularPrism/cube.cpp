/*
  Cube app implementation
  Written by Tyler Gordon Hill
*/
#include "cube.h"
const float r = 18;
const float px[16] = {-r, r, r, -r, -r, -r, r, r, r, r, r, -r, -r, -r, -r, r};
const float py[16] = {r, r, r, r, r, -r, -r, r, r, -r, -r, -r, -r, r, -r, -r};
const float pz[16] = {r, r, -r, -r, r, r, r, r, -r, -r, r, r, -r, -r, -r, -r};



/* Cube */

Cube::Cube() {
  rx = 0;
  ry = 0;
  rz = 0;
}

float Cube::matrix(Dimension dimension, float a, float x, float y, float z) {
  if (dimension == xx) return x;
  else if (dimension == xy) return y * cos(a) - z * sin(a);
  else if (dimension == xz) return y * sin(a) + z * cos(a);
  else if (dimension == yx) return x * cos(a) + z * sin(a);
  else if (dimension == yy) return y;
  else if (dimension == yz) return z * cos(a) - x * sin(a);
  else if (dimension == zx) return x * cos(a) - y * sin(a);
  else if (dimension == zy) return x * sin(a) + y * cos(a);
  else if (dimension == zz) return z;
  return 0;
}

void Cube::display(Adafruit_SSD1306 & display) {
  float pa1x[16], pa1y[16], pa1z[16];
  for (uint8_t i = 0; i < 16; i++) {
    pa1x[i] = matrix(xx, rx, px[i], py[i], pz[i]);
    pa1y[i] = matrix(xy, ry, px[i], py[i], pz[i]);
    pa1z[i] = matrix(xz, rz, px[i], py[i], pz[i]);
  }
  float pa2x[16], pa2y[16], pa2z[16];
  for (uint8_t i = 0; i < 16; i++) {
    pa2x[i] = matrix(yx, rx, pa1x[i], pa1y[i], pa1z[i]);
    pa2y[i] = matrix(yy, ry, pa1x[i], pa1y[i], pa1z[i]);
    pa2z[i] = matrix(yz, rz, pa1x[i], pa1y[i], pa1z[i]);
  }
  //reuse 1
  for (uint8_t i = 0; i < 16; i++) {
    pa1x[i] = matrix(zx, rx, pa2x[i], pa2y[i], pa2z[i]);
    pa1y[i] = matrix(zy, ry, pa2x[i], pa2y[i], pa2z[i]);
    //pa1z[i] = matrix(zz, rz, pa2x[i], pa2y[i], pa2z[i]);
  }
  for (uint8_t i = 0; i < 15; i++)
    display.drawLine(pa1x[i] + 64, pa1y[i] + 32, pa1x[i + 1] + 64, pa1y[i + 1] + 32, WHITE);
}

void Cube::increment() {
  rx = fmod(rx + M_PI / 24, 2 * M_PI);
  ry = fmod(ry + M_PI / 24, 2 * M_PI);
  rz = fmod(rz + M_PI / 24, 2 * M_PI);
}

void Cube::run(Adafruit_SSD1306 & _display) {
  increment();
  display(_display);
}
