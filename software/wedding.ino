/*
  based on the example neo-pixel sketch from Adafruit
  modified by Chris Nelson
  NeoPixel is released under the GNU LGPLv3 license

  requires Adafruit_NeoPixel.cpp and Adafruit_NeoPixel.h
*/


#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN            13

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      16

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 200;

void setup() {
  pixels.begin(); // This initializes the NeoPixel library.
  Serial.begin(1200);
}

//Transitions for state machine
#define RED_TO_BLUE 0
#define BLUE_TO_GREEN 1
#define GREEN_TO_RED 2

//simple rgb struct
struct rgb{
  byte r;
  byte g;
  byte b;
};

//color wheel has a state and an RGB value
struct colorWheel{
  byte state;
  struct rgb rgbLED;
};

//Advances the colorWheel
void advanceColor(struct colorWheel * cw){
  switch (cw->state) {
    case RED_TO_BLUE:
    //color fading from red to blue
      cw->rgbLED.r = cw->rgbLED.r - 1;
      cw->rgbLED.b = cw->rgbLED.b + 1;
      if(cw->rgbLED.r == 0){
        //change to blue to green
        cw->rgbLED.b = 255;
        cw->state = BLUE_TO_GREEN;
      }
      break;
    case BLUE_TO_GREEN:
    //color fading from blue to green
      cw->rgbLED.b = cw->rgbLED.b - 1;
      cw->rgbLED.g  = cw->rgbLED.g  + 1;
      if(cw->rgbLED.b == 0){
        //chnage to green to red
        cw->rgbLED.g = 255;
        cw->state = GREEN_TO_RED;
      }
      break;
    case GREEN_TO_RED:
    //color fading from green to red
      cw->rgbLED.g  = cw->rgbLED.g  - 1;
      cw->rgbLED.r = cw->rgbLED.r + 1;
      if(cw->rgbLED.g  == 0){
        //change to red to blue
        cw->rgbLED.r = 255;
        cw->state = RED_TO_BLUE;
      }
      break;
  }
}

//writes out the rgb color from a color wheel to the physical LEDs
void updateLEDFromCW(byte ledNum, struct colorWheel * cw){
  pixels.setPixelColor(ledNum, pixels.Color(cw->rgbLED.r, cw->rgbLED.b, cw->rgbLED.g));
}

//prints out colorwheel for debugging
void printCW(struct colorWheel * cw){
  Serial.println('\n');

  switch(cw->state){
    case RED_TO_BLUE:
    Serial.println("red to blue");
    break;
    case BLUE_TO_GREEN:
    Serial.println("blue to green");
    break;
    case GREEN_TO_RED:
    Serial.println("green to red");
    break;
  }

  Serial.println(cw->rgbLED.r);
  Serial.println(cw->rgbLED.g);
  Serial.println(cw->rgbLED.b);
}

void loop() {

  //two color wheels one for each side
  struct colorWheel cw1;
  struct colorWheel cw2;

  //a color wheel for white
  struct colorWheel white;

  //initializes one of the color wheels
  cw1.state = RED_TO_BLUE;
  cw1.rgbLED.r = 1;
  cw1.rgbLED.g = 0;
  cw1.rgbLED.b = 0;

  //initializes the other color wheel
  cw2.state = GREEN_TO_RED;
  cw2.rgbLED.r = 127;
  cw2.rgbLED.g = 127;
  cw2.rgbLED.b = 0;

  //set white to white 65 was found through guess and test
  white.rgbLED.r = 65;
  white.rgbLED.g = 65;
  white.rgbLED.b = 65;

  //one color wheel for each led
  struct colorWheel cwArray[16];

  //turn off all LEDs
  for (int i = 0; i < NUMPIXELS; i ++){
    cwArray[i].rgbLED.r = 0;
    cwArray[i].rgbLED.g = 0;
    cwArray[i].rgbLED.b = 0;
  }

  //two outer pixels set to rotating colors
  cwArray[8] = cw1;
  cwArray[15] = cw2;

  while(1){

        //advance the colors in the state machine
        advanceColor(&cwArray[8]);
        advanceColor(&cwArray[15]);

        //copy colors to LEDs set middle as white
        cwArray[14] = cwArray[15];
        cwArray[13] = cwArray[15];
        cwArray[12] = white;
        cwArray[11] = white;
        cwArray[10] = cwArray[8];
        cwArray[9] = cwArray[8];

        //back side same as front
        cwArray[0] = cwArray[15];
        cwArray[1] = cwArray[14];
        cwArray[2] = cwArray[13];
        cwArray[3] = cwArray[12];
        cwArray[4] = cwArray[11];
        cwArray[5] = cwArray[10];
        cwArray[6] = cwArray[9];
        cwArray[7] = cwArray[8];

        //print to terminal for debugging
        printCW(&cwArray[8]);
        printCW(&cwArray[15]);

        //set the colors in the AdaFruit library
        for(int i = 0; i < NUMPIXELS; i++){
          updateLEDFromCW(i, &cwArray[i]);
        }

        // This sends the updated pixel color to the hardware.
        pixels.show();
        // Delay for a period of time (in milliseconds).
        delay(delayval);
  }
}
