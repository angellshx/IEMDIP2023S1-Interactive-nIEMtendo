//LEDs
#include <FastLED.h>
#include <LinkedList.h>
#include <DFRobot_PAJ7620U2.h>
#include <EEPROM.h>
#define NUM_LEDS 560
#define LED_PIN 53

unsigned long previousMillis = 0;
unsigned long previousMillis_reset;
const unsigned long interval = 250;

//reading serial
String str;


int rows = 28;
int columns = 20;
CRGB leds[NUM_LEDS];
int mode1 = true;
boolean newData = false;
const byte numChars = 128;
char receivedChars[numChars];
int gesInt = 0;


// Init gesture sensor
DFRobot_PAJ7620U2 paj;

//BUTTONS
const int buttonPin_A = 26;
const int buttonPin_B = 28;
const int buttonPin_C = 24;
const int buttonPin_D = 22;

int buttonState_A = 0;
int buttonState_B = 0;
int buttonState_C = 0;
int buttonState_D = 0;


// Directions
#define DIR_UP 0
#define DIR_DOWN 1
#define DIR_LEFT 2
#define DIR_RIGHT 3

int resetDelay = 250;

// Game Settings
#define APPLESIZE 10
#define SNAKESIZE 3

// Game State
#define MAX_SPEED 5
#define MIN_SPEED 150
#define SPEED_LOSS 15
int gameSpeed = MIN_SPEED;
int currDirection = DIR_UP;
int timing_bool = 0;
int s = 0;

boolean isGamePaused = false;
boolean isTogglingPause = false;
boolean isGameOver = false;

CRGB targetAppleColour = CRGB(255, 0, 0);  // Red;


//Score
int numScore = 0;    //starting score
int highScore1 = 0;  // Mode 1 score
int highScore2 = 0;  // Mode 2 score

// base address for highscore memory EEPROM library
int baseAddr = 2000;

// class that represents a point on the matrix. Indexed starting at 0
class Point {
private:
  byte x;
  byte y;
public:
  Point(byte x, byte y) {
    this->x = x;
    this->y = y;
  }
  byte getX() {
    return x;
  }
  byte getY() {
    return y;
  }
  boolean isEqual(int x, int y) {
    return this->x == x && this->y == y;
  }
};


// body of the snake, last element representing the tail, first element representing the head
LinkedList<Point *> snakePositions = LinkedList<Point *>();
LinkedList<Point *> applePositions = LinkedList<Point *>();
LinkedList<Point *> applePositionsCopy = LinkedList<Point *>();

LinkedList<int> emptyIndices;

// where the apple is located
Point *applePosition;

// For Style ;)
CRGB appleColor = CRGB(255, 0, 0);
CRGB snakeColor = CRGB(15, 255, 80);
CRGB pausedAppleColor = CRGB(0, 255, 255);
CRGB pausedSnakeColor = CRGB(0, 0, 255);
CRGB emptyColor = CRGB(0, 0, 0);
CRGB solidColor = CRGB(255, 0, 0);
CRGB greenSolidColor = CRGB(0, 255, 0);


// Define arrays for apple colours
CRGB appleColors[] = {
  CRGB(255, 0, 0),    // Red
  CRGB(0, 128, 0),    // Green
  CRGB(0, 0, 255),    // Blue
  CRGB(255, 255, 0),  // Yellow
  CRGB(128, 0, 128)   // Purple

};

// For debug purposes.
String getAppleColorName(CRGB color) {
  if (color == CRGB(255, 0, 0)) {
    return "Red";
  } else if (color == CRGB(0, 128, 0)) {
    return "Green";
  } else if (color == CRGB(0, 0, 255)) {
    return "Blue";
  } else if (color == CRGB(255, 255, 0)) {
    return "Yellow";
  } else if (color == CRGB(128, 0, 128)) {
    return "Purple";
  } else {
    return "Unknown";
  }
}

//OPENCV
void replyToPython() {
  if (newData == true) {
    str = receivedChars;
    Serial.print("<This just in ... ");
    Serial.print(receivedChars);

    Serial.print("   ");
    Serial.print(millis());
    Serial.print('>');
    // change the state of the LED everytime a reply is sent
    newData = false;
  }
}

void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      } else {
        receivedChars[ndx] = '\0';  // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

//OPENCV END

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(0);
  previousMillis = millis();

  paj.begin();
  paj.setGestureHighRate(true);
  randomSeed(analogRead(0));

  menuSetup();  //set up starting screen button
  //display score setup
  scoreSystem();
  musicSetup();
  sendMP3Command('4');
  timerdisplay(); // Display Timer Screen
  getHighScore();
  // eraseHighScore();

  //setup buttons
  pinMode(buttonPin_A, INPUT);
  pinMode(buttonPin_B, INPUT);
  pinMode(buttonPin_C, INPUT);
  pinMode(buttonPin_D, INPUT);

  // Init gesture sensor
  paj.begin();
  paj.setGestureHighRate(true);

  // pick apple position
  applePosition = getApplePosition();

  resetScore();
  resetSnake();
  resetApple();


  // setup ws2812b leds
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(100);

  Serial.println("<Arduino is ready>");
}

void gameLoop() {
  unsigned long currentMillis = millis();
  // if (getFrameState() == 1) {
  //   mode1 = true;
  // } else if (getFrameState() == 2) {
  //   mode1 = false;
  // }
  // readInput();  //for string wasd input
  //checkForPause(); //i not using atm

 

  // read the state of the pushbutton value:
  buttonState_A = digitalRead(buttonPin_A);
  buttonState_B = digitalRead(buttonPin_B);
  buttonState_C = digitalRead(buttonPin_C);
  buttonState_D = digitalRead(buttonPin_D);

  //OPENCV
  recvWithStartEndMarkers();
  replyToPython();


  if (currentMillis - previousMillis >= gameSpeed) {
    currDirection = getCurrentDirection();
    previousMillis = currentMillis;

    // get next position
    Point *nextPoint = getNextPosition();

    if (mode1) {
      if (isNextPointValid(nextPoint)) {
        playGame(nextPoint);
      } else {
        setHighScoreOne();
        resetGame(true);
      }

      displayHighScore(1);

    } else {
      // check if we are still valid and continue the game, otherwise reset it
      if (isNextPointValid(nextPoint) && isCorrectApple(nextPoint)) {
        playGame(nextPoint);  // V2 continue
        timing_bool = 1;
        if (applePositions.size() <= 0) {
          timing_bool = 0;
          setHighScoreTwo();
          resetGame(false); //V2 Win
          displayHighScore(2);
        }
      } else {
        Serial.println("Reset");
        // playGame(nextPoint);
        timing_bool = 0;
        resetGame(true);  //V2 gameover
        displayHighScore(2);
      }
    }
  }

  // if (true) {
  //   // get next position
  //   Point *nextPoint = getNextPosition();

  //   if (mode1) {
  //     if (isNextPointValid(nextPoint)) {
  //       playGame(nextPoint);
  //     } else {
  //       resetGame(true);
  //     }
  //   } else {
  //     // check if we are still valid and continue the game, otherwise reset it
  //     if (isNextPointValid(nextPoint) && isCorrectApple(nextPoint)) {
  //       playGame(nextPoint);
  //       timing_bool = 1;
  //       if (applePositions.size() <= 0) {
  //         resetGame(false);
  //       }
  //     } else {
  //       Serial.println("Reset");
  //       playGame(nextPoint);
  //       timing_bool = 0;
  //       resetGame(true);
  //     }
  //   }

  // } else {
  //   renderEmptyScreen();
  // }
}

//to read string input
void readInput() {
  if (Serial.available()) {
    str = Serial.readString();
    Serial.println(str);
  }
}

// always start the game in the same spot
Point *getStartingPosition() {
  return new Point(4, 0);
}

// generate the position of the apple so it is not out of bounds or within the snake
Point *getApplePosition() {
  Point *snakeStart = getStartingPosition();
  int x, y;

  do {
    x = random(rows);
    y = random(columns);

    // [is not in snake], [no snakestart pos], [same pos another apple], [same as snake start row]
  } while (snakeContainsPosition(x, y) || snakeStart->isEqual(x, y) || appleContainsPosition(x, y) || x == snakeStart->getX());
  // Serial.print("Coordinates: ");
  // Serial.println(Point(x, y).toString());
  return new Point(x, y);
}

// check if the x y coordinates are covered by a part of the snake
boolean snakeContainsPosition(int x, int y) {
  for (int i = 0; i < snakePositions.size(); i++) {
    if (snakePositions.get(i)->isEqual(x, y)) {
      return true;
    }
  }

  return false;
}

// check if the x y coordinates are covered by another apple
boolean appleContainsPosition(int x, int y) {
  for (int i = 0; i < applePositions.size(); i++) {
    if (applePositions.get(i)->isEqual(x, y)) {
      return true;
    }
  }

  return false;
}

int getCurrentDirection() {
  int dir = currDirection;

  int xPosition = 0;
  int yPosition = 0;

  DFRobot_PAJ7620U2::eGesture_t gesture = paj.getGesture();
  gesInt = gesture;
  // FOR Gesture sensor
  if (gesInt == paj.eGestureUp) {
    dir = DIR_LEFT;
  } else if (gesture == paj.eGestureDown) {
    dir = DIR_RIGHT;
  } else if (gesture == paj.eGestureLeft) {
    dir = DIR_DOWN;
  } else if (gesture == paj.eGestureRight) {
    dir = DIR_UP;
  } else if (gesture == paj.eGestureForward) {
    // gesInt = 0;
    // directionIndex = 0;
    // currDirection = DIR_UP;
    isGameOver = false;
    // dir = DIR_UP;
  }

  // //FOR STRING INPUT
  if (str == "C") {
    dir = DIR_UP;
  }

  if (str == "D") {
    dir = DIR_RIGHT;
  }

  if (str == "B") {
    dir = DIR_LEFT;
  }

  if (str == "A") {
    dir = DIR_DOWN;
  }


  //FOR button INPUT
  if (buttonState_A == HIGH) {
    dir = DIR_DOWN;
    isGameOver = false;
    Serial.println("s pressed");
  }

  if (buttonState_B == HIGH) {
    dir = DIR_LEFT;
  }

  if (buttonState_C == HIGH) {
    dir = DIR_UP;
    Serial.println("w pressed");
  }

  if (buttonState_D == HIGH) {
    dir = DIR_RIGHT;
    Serial.println("d pressed");
  }


  //ensure you can't go the direction you just came
  switch (dir) {
    case DIR_UP:
      dir = currDirection == DIR_DOWN ? DIR_DOWN : dir;
      break;
    case DIR_DOWN:
      dir = currDirection == DIR_UP ? DIR_UP : dir;
      break;
    case DIR_LEFT:
      dir = currDirection == DIR_RIGHT ? DIR_RIGHT : dir;
      break;
    case DIR_RIGHT:
      dir = currDirection == DIR_LEFT ? DIR_LEFT : dir;
      break;
    default:
      break;
  }

  return dir;
}

Point *getHead() {
  return snakePositions.get(0);
}

Point *getTail() {
  return snakePositions.get(snakePositions.size() - 1);
}

void addToBeginning(Point *p) {
  snakePositions.add(0, p);
}

void removeTail() {
  delete (snakePositions.pop());
}

// calculate the next position based on the current head position and the current direction
Point *getNextPosition() {
  Point *head = getHead();
  switch (currDirection) {
    case DIR_UP:
      return new Point(head->getX(), head->getY() + 1);
    case DIR_DOWN:
      return new Point(head->getX(), head->getY() - 1);
    case DIR_LEFT:
      return new Point(head->getX() - 1, head->getY());
    case DIR_RIGHT:
      return new Point(head->getX() + 1, head->getY());
    default:
      return new Point(-9, -9);
  }
}

// make sure the next point for the head of the snake is in a valid position
boolean isNextPointValid(Point *p) {
  int x = p->getX();
  int y = p->getY();

  // check if within boundary or if we are in the snake
  if (x < 0 || x >= rows || y < 0 || y >= columns || snakeContainsPosition(x, y)) {
    return false;
  }

  return true;
}

boolean isCorrectApple(Point *nextPoint) {
  // if we land on an apple, add score, delete apple and spawn a new one
  for (int i = 0; i <= applePositions.size(); i++) {
    int snakeIndex = getIndexForPoint(nextPoint);
    int appleIndex = getIndexForPoint(applePositions.get(i));

    //Land on apple
    if (applePositions.get(i)->isEqual(nextPoint->getX(), nextPoint->getY())) {
      Serial.print("red value:");
      Serial.println(leds[appleIndex].r);
      Serial.print("blue value:");
      Serial.println(leds[appleIndex].b);
      Serial.print("green value:");
      Serial.println(leds[appleIndex].g);
      // Correct colour
      if (leds[appleIndex] == targetAppleColour) {
        emptyIndices.add(appleIndex);
        deleteApple(i);
        // chooseTargetAppleColor();
        Serial.println("appleindex: ");
        Serial.println(appleIndex);
        addScore();
        return true;
      }
      return false;
    }
  }
  return true;
}

// draw the apple
void renderApple() {
  if (mode1) {
    leds[getIndexForPoint(applePosition)] = appleColor;
  } else {
    Point *p;
    
    for (int i = 0; i < applePositionsCopy.size(); i++) {
      p = applePositionsCopy.get(i);
      int index = getIndexForPoint(p);
      int x = p->getX();
      int y = p->getY();

      // Check if the index matches any value in the emptyIndices linked list
      bool isIndexEmpty = false;
      for (int j = 0; j < emptyIndices.size(); j++) {
        if (index == emptyIndices.get(j)) {
          isIndexEmpty = true;
          break;  // No need to continue checking
        }
      }

      if (isIndexEmpty) {
        leds[index] = emptyColor;
      } else {

        int colorIndex = i % (sizeof(appleColors) / sizeof(appleColors[0]));
        leds[index] = isGamePaused ? pausedAppleColor : appleColors[colorIndex];

        // leds[index] = isGamePaused ? pausedAppleColor : appleColors[i % sizeof(appleColors) / sizeof(appleColors[0])];
      }
      
    }
  }
}

// draw the snake
void renderSnake() {
  Point *p;
  for (int i = 0; i < snakePositions.size(); i++) {
    p = snakePositions.get(i);
    int index = getIndexForPoint(p);
    int x = p->getX();
    int y = p->getY();
    leds[index] = isGamePaused ? pausedSnakeColor : snakeColor;
  }
}

// for a point in the matrix, map it to the index in the string
int getIndexForPoint(Point *p) {
  int x = p->getX();
  int y = p->getY();
  boolean oddRow = x % 2 == 1;

  // handle serpentine pattern
  if (oddRow) {
    return (x + 1) * columns - y - 1;
  }

  return x * columns + y;
}

void renderEmptyScreen() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = emptyColor;
  }
}

void renderSolidScreen(bool gameover) {

  for (int i = 0; i < NUM_LEDS; i++) {

    if (!gameover) {
      leds[i] = greenSolidColor;
    } else {
      leds[i] = solidColor;
    }
  }
}

void playGame(Point *nextPoint) {
  // clear screen
  renderEmptyScreen();
  renderApple();

  if (mode1) {
    displayScore();  //SCORE SYSTEN DISPLAY LOOP
    if (applePosition->isEqual(nextPoint->getX(), nextPoint->getY())) {
      growSnake(nextPoint);
    } else {
      moveSnake(nextPoint);
    }
  } else {
    displaytimer(timing_bool);
    
    Serial.print("Target apple colour: ");
    Serial.println(getAppleColorName(targetAppleColour));
    Serial.println("================================");
    displayColor(targetAppleColour.r, targetAppleColour.g, targetAppleColour.b);
    moveSnake(nextPoint);
  }

  renderSnake();

  FastLED.show();

  // delay(gameSpeed);
}

void deleteApple(int i) {
  delete (applePositions.remove(i));
  targetAppleColour = chooseTargetAppleColor();
  sendMP3Command('2');
}

CRGB chooseTargetAppleColor() {
  int index = random(applePositions.size());
  Serial.print("index :");
  Serial.println(index);
  int chosenIndex = getIndexForPoint(applePositions.get(index));

  Serial.print("Chosen index: ");
  Serial.println(chosenIndex);
  return leds[chosenIndex];
}

void moveSnake(Point *p) {
  addToBeginning(p);
  removeTail();
}

void growSnake(Point *p) {
  sendMP3Command('2');
  addToBeginning(p);
  resetApple();
  increaseSpeed();

  //SCORE SYSTEM
  addScore();
}

void increaseSpeed() {
  gameSpeed -= SPEED_LOSS;
  Serial.println(gameSpeed);
  if (gameSpeed <= MAX_SPEED) {
    gameSpeed = 20;  //so that speed will not go to negative or zero after minusing 15
  }

  // //FOR OPENCV
  // interval -= SPEED_LOSS;
  // Serial.println(interval);
  // if (interval <= MAX_SPEED) {
  //   interval = 20; //so that speed will not go to negative or zero after minusing 15
  // }
}

void checkForPause() {
  //boolean isPressedDown = digitalRead(PAUSE_PIN) == 0;
  boolean isPressedDown = str == '0';
  boolean shouldPause = false;
  if (isPressedDown) {
    // we are trying to pause the game and the button is held down
    isTogglingPause = true;
  } else {
    // the pause button is released, so let's trigger a pause
    shouldPause = isTogglingPause;
    isTogglingPause = false;
  }

  if (shouldPause) {
    isGamePaused = !isGamePaused;
  }
}

// initiate a different view
void pauseGame() {
  renderSnake();
  renderApple();
  FastLED.show();
}

// delete the snake and create a new one
void resetSnake() {
  int size = 3;
  if (mode1) {
    size = 0;
  } else {
    size = SNAKESIZE;
  }
  while (snakePositions.size() > 0) {
    delete (snakePositions.pop());
  }
  snakePositions.add(getStartingPosition());

  // Setting Snake Length

  for (int i = 0; i < size - 1; i++) {
    Point *newPoint;
    newPoint = new Point(getStartingPosition()->getX(), getStartingPosition()->getY() + i);
    snakePositions.add(0, newPoint);
  }
}

// delete the current position and draw a new apple
void resetApple() {
  delete (applePosition);
  applePosition = getApplePosition();
  if (!mode1) {
    emptyIndices.clear();
    applePositions.clear();
    applePositionsCopy.clear();

    for (int i = 0; i < APPLESIZE; i++) {
      Point *newApple = getApplePosition();
      // Add to the main list
      applePositions.add(0, newApple);
      // Add to the copy list
      applePositionsCopy.add(0, new Point(newApple->getX(), newApple->getY()));
    }
  }
}

// show an end screen and reset the game state
void resetGame(bool gameover) {
  if (gameover) {
    sendMP3Command('3');
  } else{
    sendMP3Command('1');
  }
  isGameOver = true;
  resetSnake();
  resetApple();
  resetStr();
  gesInt = 0;
  gameSpeed = MIN_SPEED;
  currDirection = DIR_UP;
  renderSolidScreen(gameover);
  FastLED.show();
  delay(resetDelay);
  renderEmptyScreen();
  FastLED.show();
  delay(resetDelay);
  renderSolidScreen(gameover);
  FastLED.show();
  delay(resetDelay);
  renderEmptyScreen();
  FastLED.show();
  delay(resetDelay);
  renderSolidScreen(gameover);
  FastLED.show();
  delay(resetDelay);
  renderEmptyScreen();
  FastLED.show();
  resetScore();
  resetSnake();
  resetApple();
  renderApple();
  targetAppleColour = CRGB(255, 0, 0);  // Red;
  // targetAppleColour = chooseTargetAppleColor();
  displayColor(0, 0, 0);
  if(!gameover){
    delay(3000);
  }
  sendMP3Command('4');
  setFrameState(0);
}

void addScore() {
  numScore = numScore + 1;
  Serial.println(numScore);
}

void resetScore() {
  if (numScore != 0) {
    numScore = 0;
  }
}

// EEPROM Codes to store values into memory
void setHighScoreOne() {
  if (mode1 && (highScore1 <= numScore)) {
    highScore1 = numScore;
    EEPROM.put(baseAddr, highScore1);
  }
}

void setHighScoreTwo() {
  if (s <= highScore2 && s > 0) {  // still need to add the condition where the number of apples is higher
    highScore2 = s;
    EEPROM.put(baseAddr + 4, highScore2);
    Serial.println("s highscore");
    Serial.print(highScore2);
  }
}

// Get value from memory
void getHighScore() {
  // Retrieve Mode 1 high score
  EEPROM.get(baseAddr, highScore1);

  // Retrieve Mode 2 high score
  EEPROM.get(baseAddr + 4, highScore2);
}

// Erase value in memory (for maintenance)
void eraseHighScore() {
  // Mode 1 highscore
  EEPROM.put(baseAddr, 0);

  // Mode 2 highscore
  EEPROM.put(baseAddr + 4, 1000);
}

void resetStr(){
  str ="";
}
void initGesure(){
  // Init gesture sensor
  paj.begin();
  paj.setGestureHighRate(true);
}
void setGameMode1(bool mode) {
  mode1 = mode;
  resetScore();
  resetSnake();
  resetApple();
}