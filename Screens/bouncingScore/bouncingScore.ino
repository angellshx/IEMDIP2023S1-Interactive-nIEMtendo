

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

#define PIN 52

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(10, 7, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +                         //change to top left if data pin connected topleft
  NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,                    //change to zigzag
  NEO_GRB            + NEO_KHZ800);

const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255) };

void setup() {
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(colors[0]);
}

int x    = 0;
int pass = 0;

void loop() {
  matrix.fillScreen(0);
  matrix.setCursor(x, 0);
  matrix.print(F("00"));
  if(x<0) {
    x++;
  }
  else{
    x--;
  }
  //if(x > 10) {
   // x = -10;
    if(pass >= 3) pass = 0;
    matrix.setTextColor(colors[pass]);
  


  matrix.show();
  delay(400);



}
