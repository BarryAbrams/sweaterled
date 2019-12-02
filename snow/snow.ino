#include <FastLED.h>

const uint8_t kMatrixWidth = 7; // The width of your grid
const uint8_t kMatrixHeight = 10; // The height of your grid

#define LED_PIN  10 // The pin on the arduino you're using for Data

#define COLOR_ORDER RGB
#define CHIPSET     WS2811

#define BRIGHTNESS 64

#define UPDATES_PER_SECOND 200
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds[ NUM_LEDS ];
#define LAST_VISIBLE_LED 69
uint8_t XY (uint8_t x, uint8_t y) {
  // any out of bounds address maps to the first hidden pixel
  if ( (x >= kMatrixWidth) || (y >= kMatrixHeight) ) {
    return (LAST_VISIBLE_LED + 1);
  }

  const uint8_t XYTable[] = {
    63,  64,  65,  66,  67,  68,  69,
    62,  61,  60,  59,  58,  57,  56,
    49,  50,  51,  52,  53,  54,  55,
    48,  47,  46,  45,  44,  43,  42,
    35,  36,  37,  38,  39,  40,  41,
    34,  33,  32,  31,  30,  29,  28,
    21,  22,  23,  24,  25,  26,  27,
    20,  19,  18,  17,  16,  15,  14,
     7,   8,   9,  10,  11,  12,  13,
     6,   5,   4,   3,   2,   1,   0
  };

  uint8_t i = (y * kMatrixWidth) + x;
  uint8_t j = XYTable[i];
  return j;
}

// This is the storage grid for the pixels. It gets modified through code, but this is the inital arrangement.
int grid[kMatrixHeight][kMatrixWidth+4] = {
  {0,1,0,1,0,1,0,1,0,1,0},
  {1,0,1,0,1,0,1,0,1,0,1},
  {0,1,0,1,0,1,0,1,0,1,0},
  {1,0,1,0,1,0,1,0,1,0,1},
  {0,1,0,1,0,1,0,1,0,1,0},
  {1,0,1,0,1,0,1,0,1,0,1},
  {0,1,0,1,0,1,0,1,0,1,0},
  {1,0,1,0,1,0,1,0,1,0,1},
  {0,1,0,1,0,1,0,1,0,1,0},
  {1,0,1,0,1,0,1,0,1,0,1},
};

void setup() {
  delay( 3000 );
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness( BRIGHTNESS );
}

void loop()
{
  FastLED.delay(1000 / UPDATES_PER_SECOND);

  displayFrame();
    
  EVERY_N_MILLISECONDS(200) {
    loadFrame();
  }

  FastLED.show();

}

void loadFrame() {
  // this is triggered 5 times per second. it shifts everything down once row, potentially ads new snow at the top, and potentially pushes things to the left or right to simulate wind
  int newGrid[kMatrixHeight][kMatrixWidth+4] = {};
  
  long randomWindDir = random(-100, 100);

  for (int i =0; i < kMatrixHeight; i++) {
      bool randomNumberHit = false;
      for (int j =0; j < kMatrixWidth + 4; j++) {
          if (i == 0 && !randomNumberHit) {
             long randomNum = random(200);
             if (randomNum >= 180 && randomNum <= 185) {
              newGrid[i][j] = 3;
              randomNumberHit = true;
             } else if (randomNum > 185 && randomNum <= 190 ) {
              newGrid[i][j] = 2;
               randomNumberHit = true;
             } else if (randomNum > 190) {
              randomNumberHit = true;
              newGrid[i][j] = 1;
             }
          
          } else if (i >= 1) {
          

            int windOffset = 0;
            if (randomWindDir < -80) {
              // if the random wind number is less than -80, then shift everything to the left
              windOffset = -1;
            } else if (randomWindDir > 80) {
              // if the random wind number is more than +80 then shift everything to the right
              windOffset = +1;
            } 

            newGrid[i][j] = grid[i-1][j+windOffset];
          }
      }       
  }

  for (int i =0; i < kMatrixHeight; i++) {
      for (int j =0; j < kMatrixWidth+4; j++) {
          grid[i][j] = newGrid[i][j];
      }       
  }
}


void displayFrame()
{
    int count = 0;
    for (int i =0; i < kMatrixHeight; i++) {
          for (int j =0; j < kMatrixWidth; j++) {
            CRGB color = 0x000000;
            int pixel = grid[i][j+2];
            if (pixel == 1) {
              color = 0xFFFFFF; // some snow is white
            } else if (pixel == 2) {
              color = 0x009999; // some snow is cyan
            } else if (pixel == 3) {
              color = 0xFFFF00; // some snow is yellow. :)
            }
            if (count >= 50) {
              // I used a different strip for LEDs 50-69 and the R and G leds are switched. 
              // Use a bit of code to fix this.
              CRGB oldcolor= color;
              color.red = oldcolor.green;
              color.green = oldcolor.red;
            }
          
            fadeTowardColor(leds[ XY(j, i)], color, 15);
            count++;
          }
    }
}

// These functions help to fade the colors.
CRGB fadeTowardColor( CRGB& cur, const CRGB& target, uint8_t amount)
{
  nblendU8TowardU8( cur.red,   target.red,   amount);
  nblendU8TowardU8( cur.green, target.green, amount);
  nblendU8TowardU8( cur.blue,  target.blue,  amount);
  return cur;
}

void nblendU8TowardU8( uint8_t& cur, const uint8_t target, uint8_t amount)
{
  if( cur == target) return;
 
  if( cur < target ) {
    uint8_t delta = target - cur;
    delta = scale8_video( delta, amount);
    cur += delta;
  } else {
    uint8_t delta = cur - target;
    delta = scale8_video( delta, amount);
    cur -= delta;
  }
}
