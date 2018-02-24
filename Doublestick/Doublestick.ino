#include <FastLED.h>

#define LED_PIN     3//6
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
#define NUM_LEDS    128

//#define CENTER_LED  27
#define CENTER_LED  8

#define BRIGHTNESS  20
#define FRAMES_PER_SECOND 200

bool gReverseDirection = false;//true;

CRGB Ring_A[24]; // D2
CRGB Ring_B[32]; // D3
CRGB Ring_C[16]; // D4

CRGBPalette16 gPal;

void setup() {
  delay(3000); // sanity delay

  // tell FastLED there's 8 NEOPIXEL leds on pin 2
  FastLED.addLeds<NEOPIXEL, 2>(Ring_A, 24);//.setCorrection( TypicalLEDStrip );

  // tell FastLED there's 8 NEOPIXEL leds on pin 3
  FastLED.addLeds<NEOPIXEL, 3>(Ring_B, 32);//.setCorrection( TypicalLEDStrip );

  // tell FastLED there's 8 NEOPIXEL leds on pin 4
  FastLED.addLeds<NEOPIXEL, 4>(Ring_C, 16);//.setCorrection( TypicalLEDStrip );

  //FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );

  
  FastLED.setBrightness( BRIGHTNESS );

  // This first palette is the basic 'black body radiation' colors,
  // which run from black to red to bright yellow to white.
  //gPal = HeatColors_p;
  
  // These are other ways to set up the color palette for the 'fire'.
  // First, a gradient from black to red to yellow to white -- similar to HeatColors_p
     gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Orange, CRGB(255,255,100));
  
  // Second, this palette is like the heat colors, but blue/aqua instead of red/yellow
     //gPal = CRGBPalette16( CRGB::Black, CRGB::Orange, CRGB::Purple, CRGB::Blue);
  
  // Third, here's a simpler, three-step gradient, from black to red to white
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::White);

}

void loop()
{
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy( random());

  Simulate();
  
  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}

void Simulate(void) {
  static uint8_t color_wheel1 = 0;
  static uint8_t color_wheel2 = 0;
  static uint8_t color_wheel3 = 0;

  static uint32_t color_timer1 = 0;
  static uint32_t color_timer2 = 0;
  static uint32_t color_timer3 = 0;
  uint8_t x;

  if (millis() > color_timer1) {
    color_timer1 += 15;
    color_wheel1++;
    gPal = CRGBPalette16(CRGB::Red, CHSV(color_wheel1,250,250), CRGB::Black);
    for (x=0; x<24; x++) {
      Ring_A[x] = ColorFromPalette( gPal, x*11 + color_wheel1);
    }
  }
  if (millis() > color_timer2) {
    color_timer2 += 19;
    color_wheel2++;
    gPal = CRGBPalette16(CRGB::Green, CHSV(color_wheel2,200,200), CRGB::Black);
    for (x=0; x<32; x++) {
      Ring_B[x] = ColorFromPalette( gPal, x*8 + color_wheel2);
    }
  }
  if (millis() > color_timer3) {
    color_timer3 += 17;
    color_wheel3++;
    gPal = CRGBPalette16(CRGB::Blue, CHSV(color_wheel3,200,200), CRGB::Orange);
    for (x=0; x<16; x++) {
      Ring_C[x] = ColorFromPalette( gPal, x*16 - color_wheel3);  
    }
  }
}




