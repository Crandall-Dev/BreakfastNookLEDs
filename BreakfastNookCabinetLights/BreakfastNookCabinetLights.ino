/**
 *  Breakfast Nook Cabinet RGB LEDs
 *
 *  Digispark / ATTiny85 based controller - https://www.microchip.com/en-us/product/ATtiny85
 *  Sharp GP2Y0D810Z0F Digital Distance Sensor - https://www.adafruit.com/product/1927
 *  RGB LED WS2812b strip (21 lights long) - https://amzn.to/3MPkcRv
 *
 *  @author Aaron S. Crandall <crandall@gonzaga.edu> 
 *  @copyright 2022
 *  @license GPL v3.0
 */

// ** ********** Library includes *****************************************
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#include <Adafruit_NeoPixel.h>


// ** ********** Magic number definitions *********************************

// Digispark // ATTiny85 // Arduino Pro Mini
#define LED_STRIP_PIN  PB2 // 3 //  6
#define HAND_SENSOR_PIN PB0 // 4 // 7
#define BRIGHTNESS_DIAL_PIN 2 // A1 // A0
#define LED_ONBOARD_PIN PB1

#define LED_COUNT 21
#define LED_GLOBAL_BRIGHTNESS 100
#define MIN_BRIGHTNESS_VALUE 50
#define MAX_BRIGHTNESS_VALUE 255
#define MIN_DIAL_VALUE 0
#define MAX_DIAL_VALUE 1023

#define HAND_SENSE_TIMEOUT 1000


// ** ********** Type definitions *****************************************
enum LightPattern {
  OFF,
  RAINBOW,
  WHITE
};


// ** ********** Global variables *****************************************
Adafruit_NeoPixel strip(LED_COUNT, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);

long currPixelHue = 0;
unsigned long startHandDetect = 0;
bool handDetected = false;

int currLightPattern = 0;
#define TOTAL_LIGHT_PATTERNS 4
LightPattern patterns[] = {OFF, WHITE, OFF, RAINBOW};


// ******************************************************************************************************
void setup() {
  
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  pinMode(HAND_SENSOR_PIN, INPUT_PULLUP);
  pinMode(LED_ONBOARD_PIN, OUTPUT);

  blinkLEDDeviceOnline();
  setupLEDStrip();
}


// ******************************************************************************************************
void loop() {
  handleHandSensor();
  updateStrip();

  delay(10);
}


// ** **********************************************************
void setupLEDStrip() {
  strip.begin();
  strip.show();
  strip.setBrightness(LED_GLOBAL_BRIGHTNESS); // Set BRIGHTNESS to about 1/5 (max = 255)
}


// ** **********************************************************
void blinkLEDDeviceOnline() {
  for(int i = 0; i < 5; i++) {
    digitalWrite(LED_ONBOARD_PIN, HIGH);
    delay(50);
    digitalWrite(LED_ONBOARD_PIN, LOW);
    delay(75);
  }
}


// ** **********************************************************
void updateStrip() {
  int brightnessValue = getCurrentBrightnessSetting();

  if( patterns[currLightPattern] == OFF ) {
    strip.clear();
    strip.show();
  } else if( patterns[currLightPattern] == RAINBOW ) {
    updateStripRainbow(brightnessValue);      
  } else if( patterns[currLightPattern] == WHITE ) {
    int color[3] = {1, 1, 1};
    updateStripColor(brightnessValue, color);
  }
}


// ** **********************************************************
int getCurrentBrightnessSetting() {
  return map(
    analogRead(BRIGHTNESS_DIAL_PIN),
    MIN_DIAL_VALUE,
    MAX_DIAL_VALUE,
    MIN_BRIGHTNESS_VALUE,
    MAX_BRIGHTNESS_VALUE
    );  
}


// ** **********************************************************
void handleHandSensor() {
  if( digitalRead(HAND_SENSOR_PIN) == LOW ) {
    if( handDetected == false) {
      handDetected = true;
      startHandDetect = millis();
    }
    if( startHandDetect + HAND_SENSE_TIMEOUT < millis() ) {
      currLightPattern ++;
      currLightPattern %= TOTAL_LIGHT_PATTERNS;
      startHandDetect = millis();
    }
  } else {
    handDetected = false;
  }
}


// ** ***********************************************************
void updateStripColor(int brightness, int colorVector[3]) {
  for( int i = 0; i < 3; i++ ) {
    colorVector[i] *= brightness;
  }

  for(int i = 0; i < LED_COUNT; i++ ) {
    strip.setPixelColor(i, colorVector[0], colorVector[1], colorVector[2]);
  }
  strip.show();
}


// ** ***********************************************************
void updateStripRainbow(int brightness) {
  currPixelHue += 256;
  currPixelHue %= 5*65536;  // Reset to 0 if leaving max range

  int repetitions = 1;      // Default settings of "rainbow" interface
  int saturation = 255;
  bool gammify = true;

  strip.rainbow(currPixelHue, repetitions, saturation, brightness, gammify);
  strip.show();
}
