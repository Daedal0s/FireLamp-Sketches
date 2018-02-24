#include <FastLED.h>

#define LED_PIN     3
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
#define NUM_LEDS    16

#define BRIGHTNESS  150
#define FRAMES_PER_SECOND 200

bool gReverseDirection = true;

CRGB leds[NUM_LEDS];

void setup() {
  delay(3000); // sanity delay
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
}

void loop()
{
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy( random());

  Fire2012(); // run simulation frame
  
  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}


// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
//// 
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation, 
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking. 
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
#define COOLING  75

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 40


void Fire2012()
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];
  static byte newheat[NUM_LEDS];
  static byte counter = 0;
  int booster;
  
  if (counter-- == 0) {
    counter = 10;

    // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      if (i < 8) {
        booster = i*8;
      } else {
        booster = (15 - i)*8;
      }
      newheat[i] = qsub8( heat[i],  booster + random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
    
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= 8; k < 15; k++) {
      newheat[k] = (heat[k] + heat[k+1]) / 2;
    }
    for( int k= 7; k >= 1; k--) {
      newheat[k] = (heat[k] + heat[k-1]) / 2;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      if (y > 3) {
        newheat[0] = qadd8( heat[0], random8(190,255) );
      } else {
        newheat[15] = qadd8( heat[15], random8(190,255) );        
      }
    }
  }
  
  // Step 4.  Map from heat cells to LED colors
  for( int j = 0; j < NUM_LEDS; j++) {
    heat[j] = ((uint16_t)newheat[j] + (uint16_t)heat[j]*3) / 4; // A little IIR filtering here
    CRGB color = HeatColor( heat[j]);
    int pixelnumber;
    if( gReverseDirection ) {
      pixelnumber = (NUM_LEDS-1) - j;
    } else {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }
}

