#include <FastLED.h>
#define NUM_LEDS 280
#define DATA_PIN 13
CRGB leds[NUM_LEDS];

void setup() {   
 FastLED.addLeds<WS2812,DATA_PIN,RGB>(leds,NUM_LEDS);
 FastLED.setBrightness(50);

}

CRGB Snake[5][5] {

{0x008000, 0x008000, 0x008000, 0x008000, 0x008000},
{0x000000, 0x008000, 0x008000, 0x008000, 0x000000},
{0xffffff, 0xffffff, 0x008000, 0xffffff, 0xffffff},
{0x000000, 0x008000, 0x008000, 0x008000, 0x000000},
{0x008000, 0x008000, 0x008000, 0x008000, 0x008000},
};


void loop() {  

  for (int i=0;i<5;i++)             // j is row
   for (int j=0;j<5;j++){           // i is col    
    if (j&1) 				//odd column
      leds[j*5+4-i]=Snake[i][j];    //flipping odd columns
    else     				//even column
      leds[j*5+i-1]=Snake[i][j];    //even columns stay same
    delay(150);
    FastLED.show();
  }
}