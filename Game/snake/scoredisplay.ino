#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
#define PSTR  // Make Arduino Due happy
#endif

#define PIN 49

int x = 0;
int pass = 0;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(10, 7, PIN,
                                               NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
                                               NEO_GRB + NEO_KHZ800);

const uint16_t scoreColors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255)
};

void scoreSystem() {
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(scoreColors[0]);
  // matrix.setTextSize(1); //old static score
  matrix.setTextColor(colors[0]);
}

void displayScore() {
  
  //OLD STATIC SCORE
  // matrix.setCursor(0, 0);
  // uint32_t blackColor = matrix.Color(0, 0, 0);  //to clear the screen after each number change
  // matrix.fillScreen(blackColor);
  // matrix.print(numScore);
  // matrix.show();
  // delay(100);

  matrix.fillScreen(0);
  
  if(numScore > 9){
     matrix.setCursor(x, 0);
  } else {
    matrix.setCursor(3,0);
  }
  matrix.print(numScore);
  if (x < 0) {
    x++;
  } else {
    x--;
  }
  //if(x > 10) {
  // x = -10;
  if (pass >= 3) pass = 0;
  matrix.setTextColor(colors[pass]);
  matrix.show();
  // delay(400);

}

// Display Snake V2 Next Color
void displayColor(int r, int g, int b) {
  uint32_t color = matrix.Color(r, g, b);
  matrix.fillScreen(color);
  matrix.show();
}
