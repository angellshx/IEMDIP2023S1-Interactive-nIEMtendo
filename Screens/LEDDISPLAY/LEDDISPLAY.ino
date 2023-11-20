
//My matrix is setup 3 x 35 (3 Rows, 35 LEDs wide
//Text is being flipped in the matrix to scroll from left to right. 

#include <avr/pgmspace.h>                     //Needed to store stuff in Flash using PROGMEM
#include <EEPROM.h>                           //Needed to store variables in memory to persist after power off. 
#include <FastLED.h>                          //Fastled library to control the LEDs

#define MAX_BRIGHT  40
#define LED_PIN     3
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 100
CRGB leds[NUM_LEDS];

// Params for width and height (work to replace)
const uint8_t kMatrixWidth = 11;  //orig 11x27
const uint8_t kMatrixHeight = 27; //(can try 3x25 for 75 leds) or (3*20 for 60)

int Start = 0;
int End = 35;

byte text_r1[] = {0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,1,1,0,0,1,1,1,0,0,0,0,0,0};
byte text_r2[] = {0,0,0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0};
byte text_r3[] = {0,0,0,0,0,0,0,1,0,0,0,1,1,1,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,0};

int array_len = sizeof(text_r1) / sizeof(text_r1[0]); //since all rows are even, only need to do this once


void setup() {
  //Serial.begin(9600); 
  delay(1500);    //Startup power delay. 1.5 seconds.
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setMaxPowerInVoltsAndMilliamps(5,800);
  FastLED.setBrightness(MAX_BRIGHT);
  fill_solid(leds, NUM_LEDS, CRGB::Black); 
  FastLED.show();

//Flip the arrays for the display.
  for(int h=0;h<array_len;h++){
    byte temp;
    temp = text_r1[h];
    text_r1[h] = text_r1[array_len-h-1];
    text_r1[array_len-h-1] = temp;    
  }

    for(int i=0;i<array_len;i++){
    byte temp;
    temp = text_r2[i];
    text_r2[i] = text_r2[array_len-i-1];
    text_r2[array_len-i-1] = temp;    
  }

    for(int j=0;j<array_len;j++){
    byte temp;
    temp = text_r3[j];
    text_r3[j] = text_r3[array_len-j-1];
    text_r3[array_len-j-1] = temp;    
  }
  
}

void loop() {
  text_scroll();
}

void text_scroll(){
  
  FastLED.clear();
  int next = Start;
  int i = 0;
  while(true) {    
    if (text_r1[next]==1)
      leds[i] = CRGB::Red;
    if (text_r2[next]==1)
      leds[i+35] = CRGB::Red;
    if (text_r3[next]==1)
      leds[i+70] = CRGB::Red;

    i++;
    next = next+1;
    if (next == 57)
      next = 0;
    if (next == End)
      break;
  }
  
  if (Start == 0){
    Start = 56;
  }
    else
    {
    Start = Start - 1;
    }
  if (End == 0)
  {
    End = 56;
  }
    else
    {
    End = End-1;
    }
  //Serial.print(Start);
  //Serial.print(" ");
  //Serial.print(End);
  //Serial.print("\n");

  
  delay(130);
  FastLED.show();
}