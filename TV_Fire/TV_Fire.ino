#include <FastLED.h>

#define LED_PIN     3
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
#define NUM_LEDS    120

//#define CENTER_LED  27
#define CENTER_LED  60

#define BRIGHTNESS  250
#define FRAMES_PER_SECOND 200

bool gReverseDirection = false;//true;

CRGB leds[NUM_LEDS];

CRGBPalette16 gPal;

void setup() {
  delay(3000); // sanity delay
  //FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  //FastLED.setBrightness( BRIGHTNESS );

  // This first palette is the basic 'black body radiation' colors,
  // which run from black to red to bright yellow to white.
  //gPal = HeatColors_p;
  
  // These are other ways to set up the color palette for the 'fire'.
  // First, a gradient from black to red to yellow to white -- similar to HeatColors_p
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);
  
  // Second, this palette is like the heat colors, but blue/aqua instead of red/yellow
     gPal = CRGBPalette16( CRGB(50,10,0), CRGB(200,100,150), CRGB(250,250,250));
  
  // Third, here's a simpler, three-step gradient, from black to red to white
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::White);

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
#define COOLING  20//40

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 50


void Fire2012()
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];
  static byte newheat[NUM_LEDS];
  static byte counter = 0;
  int booster = 0;
  static byte vent0_timer = 0;
  static byte vent1_timer = 0;
  static byte vent0_heat = 0;
  static byte vent1_heat = 0;
  static byte cooling = COOLING;
  if (counter-- == 0) {
    counter = 7;

    // Set new value for cooling
    byte wind = random8(0,8);
    if ((wind > 4) && (cooling < COOLING * 4)) {
      cooling += (wind-4);
    }
    if ((wind < 4) && (cooling > COOLING / 4)) {
      cooling -= (4-wind);
    }

    // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      if (i < CENTER_LED) {
        booster = i/2;
      } else {
        booster = (NUM_LEDS - i)/2;
      }
      newheat[i] = qsub8( heat[i], ((uint16_t)booster*cooling)*4);
    }
    
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= CENTER_LED; k < (NUM_LEDS-1); k++) {
      newheat[k] = ((uint16_t)heat[k] + (uint16_t)heat[k+1]*3) / 4;
    }
    for( int k= CENTER_LED - 1; k > 0; k--) {
      newheat[k] = ((uint16_t)heat[k] + (uint16_t)heat[k-1]*3) / 4;
    }
    
    // Step 3.  Randomly ignite new 'vents' of heat near the bottom
    if (vent0_timer > 0) {
      vent0_timer--;
      newheat[0] = qadd8( heat[0], vent0_heat );
    } else {
      newheat[0] = heat[0]/2;
      if( random8() < SPARKING ) {
        vent0_timer = random8(3,15);
        vent0_heat =  random8(50,230);
      }
    }

    if (vent1_timer > 0) {
      vent1_timer--;
      newheat[NUM_LEDS-1] = qadd8( heat[NUM_LEDS-1], vent1_heat );
    } else {
      newheat[NUM_LEDS-1] = heat[NUM_LEDS-1]/2;
      if( random8() < SPARKING ) {
        vent1_timer = random8(3,15);
        vent1_heat =  random8(50,230);
      }
    }
  }
  
  // Step 4.  Map from heat cells to LED colors
  for( int j = 0; j < NUM_LEDS; j++) {
    heat[j] = ((uint16_t)newheat[j] + (uint16_t)heat[j]*3) / 4; // A little IIR filtering here

    if (j <= CENTER_LED) {
      leds[j] = ColorFromPalette( gPal, heat[CENTER_LED-j]);
    } else {
      leds[j] = ColorFromPalette( gPal, heat[NUM_LEDS - (j-CENTER_LED)]);
    }
  }
}

