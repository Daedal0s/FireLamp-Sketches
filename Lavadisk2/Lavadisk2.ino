// --------------------------------------------
// Pre-existing stuff
// --------------------------------------------
#include <FastLED.h>

#define CLK_PIN     13
#define DAT_PIN     12
#define COLOR_ORDER BGR
#define CHIPSET     APA102
#define NUM_LEDS    255

//#define CENTER_LED  27
#define CENTER_LED  127

#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 500

bool gReverseDirection = false;//true;


// --------------------------------------------
// Data structures
// --------------------------------------------
struct RING_STRUCT;

typedef struct REGION_STRUCT {
  uint8_t Start_Pos;
  uint8_t End_Pos;
  uint8_t Direction_Req;
  RING_STRUCT* New_Ring_Ptr;
  uint8_t New_Pos;
  uint8_t New_Direction;
} REGION_STRUCT_T;

typedef struct RING_STRUCT {
  uint8_t LED_Count;
  CRGB* LED_Data_Ptr;
  uint8_t Energy;
  uint8_t Glow_Hue; // glow
  uint8_t Glow_Intensity; // glow

  uint8_t Fade_Rate;

  REGION_STRUCT_T Switch_Region[4]; // Only 4 switch regions for now
} RING_STRUCT_T;

typedef struct PIXI_STRUCT {
  RING_STRUCT_T* Ring_Ptr;
  uint8_t Delay_Timer;
  uint8_t Delay_Period;
  uint8_t Ring_Pos;
  uint8_t Hue;
  uint8_t Intensity;
  uint8_t Size; // length
  int8_t Speed;
  uint8_t Energy; // Die if too low, mitosis if high enough
} PIXI_STRUCT_T;

/*
typedef struct RING_STRUCT RING_STRUCT_T;
typedef struct REGION_STRUCT REGION_STRUCT_T;*/

// --------------------------------------------
// Variable declarations
// --------------------------------------------
// Allocate data space for up to x LEDs
//#define MAX_LED_COUNT  100
CRGB leds[NUM_LEDS];

#define MAX_RING_COUNT  5
RING_STRUCT_T Ring[MAX_RING_COUNT];

#define MAX_PIXI_COUNT  10
PIXI_STRUCT_T Pixi[MAX_PIXI_COUNT];

//CRGB leds[NUM_LEDS];

CRGBPalette16 gPal;

// --------------------------------------------
// SETUP
// --------------------------------------------
void setup() {
  delay(1000); // sanity delay
  FastLED.addLeds<CHIPSET, DAT_PIN, CLK_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  //FastLED.setBrightness( BRIGHTNESS );

  // This first palette is the basic 'black body radiation' colors,
  // which run from black to red to bright yellow to white.
  //gPal = HeatColors_p;
  
  // These are other ways to set up the color palette for the 'fire'.
  // First, a gradient from black to red to yellow to white -- similar to HeatColors_p
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);
  
  // Second, this palette is like the heat colors, but blue/aqua instead of red/yellow
     gPal = CRGBPalette16( CRGB(0,0,0), CRGB(100,0,0), CRGB(60,40,0), CRGB(0,10,20));
  
  // Third, here's a simpler, three-step gradient, from black to red to white
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::White);

  // Zero out the LED display buffer
  memset(leds, 0, sizeof(leds));
  // Zero out the rings
  memset(Ring, 0, sizeof(Ring));
  // Zero out the pixis
  memset(Pixi, 0, sizeof(Pixi));

  Ring[0].LED_Data_Ptr = &(leds[0]);

  
  /*Ring[0].Switch_Region[0].Start_Pos = 10;
  Ring[0].Switch_Region[0].End_Pos = 20;
  Ring[0].Switch_Region[0].New_Ring_Ptr = &(Ring[1]);
  Ring[0].Switch_Region[0].New_Pos = 30;*/

}

// --------------------------------------------
// Function definitions
// --------------------------------------------
void Pixi_Process(PIXI_STRUCT_T* p) {
  RING_STRUCT_T* r = p->Ring_Ptr;
  // Is energy low enough to be dead?
  if (p->Energy == 0) {
    //  Die
    //Kill(p);
    return;
  }
  
  // Consume energy from environment
  if (r->Energy > 0) {
    p->Energy = qadd8(p->Energy, r->Energy/16);
    r->Energy = qsub8(r->Energy, r->Energy/16);
  }
  // Is energy high enough to split?
  if (p->Energy > p->Split_Energy) {
    //  Split
    //Split(p);
  }
  // Burn some energy
  p->Energy = qsub16(p->Energy, p->Speed);

  // Are we in a switch region?
  for (x=0; x< RING_REGION_COUNT; x++) {
    // Skip to next one if pixi is not in the limits of this region
    if (r->Region[x].Start_Pos > r->Pos) continue;
    if (r->Region[x].End_Pos < r->Pos) continue;
    // Energy of destination ring must be twice as high as energy in this ring to switch
    if ((r->Region[x].New_Ring_Ptr->Energy) / 2 < r->Energy) continue;
    // OK then, switch to new ring
    p->Pos = r->Region[x].New_Pos;
    p->Ring_Ptr = r->Region[x].New_Ring_Ptr;
  }

  // Move and wrap if needed
  p->Pos += p->Speed;
  if (p->Pos > r->Max_Pos) {
    p->Pos -= r->Max_Pos);
  } else if (p->Pos < 0) {
    p->Pos += r->Max_Pos;
  }
}

void Ring_Process(RING_STRUCT_T* r, PIXI_STRUCT_T* p) {
  CRGB* l = r->LED_Data_Ptr;

  // Fade
  fadeToBlackBy(l, r->LED_Count, r->Fade_Rate);
  // Glow

  // Render to buffer the pixis on this ring
  // Check each pixi to see if it's on this ring
  for (x = 0; x < PIXI_COUNT; x++) {
    // Load pointer to next Pixi data structure
    PIXI_STRUCT_T* p = &(Pixi[x]);
    // If the pixi is dead, ignore it
    if (p->Energy == 0) continue;
    // If the pixi is on a different ring, ignore it
    if (p->Ring_Ptr != r->Self_Ptr) continue;
    // Translate pixi position and traits into rgb/hsv modifications
    //   to the buffer
    r->LED_Data_Ptr[p->Pos] += p->Color;
  }
  //  the pixi is on this ring, so paint it's shape, hue, and intensity onto the associated LEDs
}



  // Consider
  //  adding start/end for destination region
  //  setting probability for switch
  //  Setting direction limits and new direction mandates
  // Maybe rings have energy levels that are depleted by pixis and recharge over time
  //  So if a pixi sees a ring with a higher energy value, it'll be more likely to jump
  // Maybe it's a color thing.  Each pixi colors the ring it's on, and then jumps to another ring
  //   with less intensity of that color
  // Pixis of other colors may diminish the intensity of extant colors



  Pixi[0] = CPixi(Green, Wanderer, Slow);

  // For clock function consider
  //   Number depends on hour
  //   Color depends on minute
}

// --------------------------------------------
//       ********** LOOP ****************
// --------------------------------------------
void loop()
{
  // Simulate all the pixis
  for (x = 0; x < PIXI_COUNT; x++) {
    Pixi_Process(Pixi[x]);
  }
  // Simulate and Render all the rings
  for (x = 0; x < RING_COUNT; x++) {
    Ring_Process(Ring[x]);
  }
  
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy( random());

  Fire2012(); // run simulation frame
  
  FastLED.show(); // display this frame
  //FastLED.delay(1000 / FRAMES_PER_SECOND);
}


// --------------------------------------------
//       FIRE 2012 STUFF
// --------------------------------------------
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
#define COOLING  2

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 30


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
    counter = 13;

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
        booster = i;
      } else {
        booster = (NUM_LEDS - i);
      }
      newheat[i] = qsub8( heat[i], ((uint16_t)booster*cooling));
    }
    
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= CENTER_LED; k < (NUM_LEDS-1); k++) {
      newheat[k] = ((uint16_t)heat[k] + (uint16_t)heat[k+1]*3) / 4;
    }
    for( int k= CENTER_LED - 1; k > 0; k--) {
      newheat[k] = ((uint16_t)heat[k] + (uint16_t)heat[k-1]*3) / 4;
    }
    
    // Step 3.  Randomly ignite new 'vents' of heat near the bottom
    if (vent0_timer > 110) {
      vent0_timer--;
      newheat[0] = qadd8( heat[0], vent0_heat );
    } else {
      newheat[0] = heat[0]/2;
      if( random8() < SPARKING ) {
        vent0_timer = random8(100,150);
        vent0_heat =  random8(50,150);
      }
    }

    if (vent1_timer > 100) {
      vent1_timer--;
      newheat[NUM_LEDS-1] = qadd8( heat[NUM_LEDS-1], vent1_heat );
    } else {
      newheat[NUM_LEDS-1] = heat[NUM_LEDS-1]/2;
      if( random8() < SPARKING ) {
        vent1_timer = random8(100,110);
        vent1_heat =  random8(50,150);
      }
    }
  }
  
  // Step 4.  Map from heat cells to LED colors
  for( int j = 0; j < NUM_LEDS; j++) {
    heat[j] = ((uint16_t)newheat[j] + (uint16_t)heat[j]*7) / 8; // A little IIR filtering here
    //CRGB color = HeatColor( heat[j]);

      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      //byte colorindex = scale8( heat[j], 240);
      byte colorindex = heat[j];
      CRGB color = ColorFromPalette( gPal, colorindex);

    int pixelnumber;
    if( gReverseDirection ) {
      pixelnumber = (NUM_LEDS-1) - j;
    } else {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }
}

