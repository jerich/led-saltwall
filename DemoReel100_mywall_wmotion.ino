#include "FastLED.h"

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif


#define DATA_PIN    12
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 480
CRGB leds[NUM_LEDS];

//#define BRIGHTNESS          96
#define BRIGHTNESS          36
#define DIMMING              8
#define FRAMES_PER_SECOND  120

int pirPin = 4;    //the digital pin connected to the PIR sensor's output
long unsigned int startTime = 0;
int onDelay = 10000;

void setup() {
//setup for PIR sensor
  Serial.begin(9600);
  pinMode(pirPin, INPUT);
  digitalWrite(pirPin, LOW);

  delay(30000); // 3 second delay for recovery

  // tell FastLED about the LED strip configuration
//ORIG  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(UncorrectedColor);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
//SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };
SimplePatternList gPatterns = { whitewall, rainbow, bluegreenfade, verticalfade, verticalfade };
//SimplePatternList gPatterns = { counting };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
void loop()
{
  
  if (millis() < (startTime + onDelay)) {
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 15 ) { nextPattern(); } // change patterns periodically
  }
  else {
    FastLED.setBrightness(DIMMING);
    gPatterns[0]();
    FastLED.show();
    if (digitalRead(pirPin) == HIGH){
      startTime = millis();
      FastLED.setBrightness(BRIGHTNESS);
    }
  }
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
//orig  fill_rainbow( leds, NUM_LEDS, gHue, 7);
  fill_rainbow( leds, NUM_LEDS, gHue, 3);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void counting()
{
  //light every 10th led
  for(int i = 0; i < NUM_LEDS; i=i+10) {
   leds[i] = CRGB::White;
  }
  leds[0] = CRGB::Red;
  leds[110] = CRGB::Red;
  leds[215] = CRGB::Red;
  leds[325] = CRGB::Red;
  leds[440] = CRGB::Red;
  leds[479] = CRGB::Red;
}
  
void whitewall()
{
  //fill_solid( leds, NUM_LEDS, CRGB::Linen );
  for(int i = 0; i<NUM_LEDS;i++){
   leds[i] = CRGB::Linen; 
  }
}

void bluegreenfade()
{
  fill_gradient_RGB( leds, 0, CRGB::Blue, 110, CRGB::Green );
  fill_gradient_RGB( leds, 215, CRGB::Blue, 325, CRGB::Green );
  fill_gradient_RGB( leds, 440, CRGB::Blue, 480, CRGB::Blue );
  fill_gradient_RGB( leds, 111, CRGB::Green, 214, CRGB::Blue );
  fill_gradient_RGB( leds, 326, CRGB::Green, 439, CRGB::Blue );
}

void verticalfade()
{
  fill_gradient( leds, 0, CHSV( gHue, 255, 192), 110, CHSV( gHue+100, 255, 192) );
  fill_gradient( leds, 215, CHSV( gHue, 255, 192), 325, CHSV( gHue+100, 255, 192) );
  fill_gradient( leds, 440, CHSV( gHue, 255, 192), 480, CHSV( gHue+40, 255, 192) );
  fill_gradient( leds, 111, CHSV( gHue+100, 255, 192), 214, CHSV( gHue, 255, 192) );
  fill_gradient( leds, 326, CHSV( gHue+100, 255, 192), 439, CHSV( gHue, 255, 192) );
}


void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

