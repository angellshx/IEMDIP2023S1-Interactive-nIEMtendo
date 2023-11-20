#define PIN 51
#define PIN_NUMBER 51

Adafruit_NeoMatrix timermatrix = Adafruit_NeoMatrix(17, 7, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

bool is_timing = 0; // // 1 means we are timing the interval between triggers; 0 means we are not
unsigned long start_time;   // stores the start of the timing interval


void timerdisplay() {
  timermatrix.begin();
  timermatrix.setTextWrap(false);
  timermatrix.setBrightness(40);
  timermatrix.setTextColor(scoreColors[0]);
  timermatrix.setTextSize(1);
}

void displaytimer(int timing_bool) {
  timermatrix.setCursor(0, 0);
  uint32_t blackColor = timermatrix.Color(0, 0, 0); //to clear the screen after each number change
  timermatrix.fillScreen(blackColor);
  timermatrix.print(countUpTimer(timing_bool));
  timermatrix.show();
  delay(100);
}

void displayHighScore(int mode) {
  // Retrieve highscore in memory
  getHighScore();

  timermatrix.setCursor(0, 0);
  uint32_t blackColor = timermatrix.Color(0, 0, 0); //to clear the screen after each number change
  timermatrix.fillScreen(blackColor);
  if (mode == 1) {
    timermatrix.print(highScore1);

  } else if (mode == 2) {
      if (highScore2 >= 1000) {
        timermatrix.print(0);
      } else {
        timermatrix.print(highScore2);
      }
  }

  timermatrix.show();
  delay(100);
}

int countUpTimer(int timing_bool) {

  is_timing = timing_bool;

  if (!is_timing && !digitalRead(PIN_NUMBER)) {// trigger the start of the timing interval
    is_timing = 1;            // we are now timing the interval
    start_time = millis();   // record the current time as the start of the interval
  }
  if (is_timing) {
    unsigned long elapsed_time = (millis() - start_time) / 1000;
    byte seconds = elapsed_time;
    // byte seconds = elapsed_time % 60;
    // byte hours = (elapsed_time / 3600) % 100;
    // byte minutes = (elapsed_time / 60) % 60;

    s = seconds;
    // int m = minutes;

    if (elapsed_time > 999 && !digitalRead(PIN_NUMBER))is_timing = 0;

    return s;
  }

  return 0;
}
