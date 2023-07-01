#include <FastLED.h>
#include "SPIFFS.h"
#include <vector>

using namespace std;

#define LED_PIN  5

#define COLOR_ORDER GRB
#define CHIPSET     WS2811

#define BRIGHTNESS 16


const uint8_t kMatrixWidth = 16;
const uint8_t kMatrixHeight = 16;

const bool    kMatrixSerpentineLayout = true;
const bool    kMatrixVertical = true;

uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;
  
  if( kMatrixSerpentineLayout == false) {
    if (kMatrixVertical == false) {
      i = (y * kMatrixWidth) + x;
    } else {
      i = kMatrixHeight * (kMatrixWidth - (x+1))+y;
    }
  }

  if( kMatrixSerpentineLayout == true) {
    if (kMatrixVertical == false) {
      if( y & 0x01) {
        // Odd rows run backwards
        uint8_t reverseX = (kMatrixWidth - 1) - x;
        i = (y * kMatrixWidth) + reverseX;
      } else {
        // Even rows run forwards
        i = (y * kMatrixWidth) + x;
      }
    } else { // vertical positioning
      if ( x & 0x01) {
        i = kMatrixHeight * (kMatrixWidth - (x+1))+y;
      } else {
        i = kMatrixHeight * (kMatrixWidth - x) - (y+1);
      }
    }
  }
  
  return i;
}

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB* const leds( leds_plus_safety_pixel + 1);

uint16_t XYsafe( uint8_t x, uint8_t y)
{
  if( x >= kMatrixWidth) return -1;
  if( y >= kMatrixHeight) return -1;
  return XY(x,y);
}

void loop()
{
  vector<String> v;
  static int f = 0;
  String nome = "/pixil-frame-";
  nome.concat(f);
  nome.concat(".ppm");
  File file = SPIFFS.open(nome.c_str());
  Serial.println(nome);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  while (file.available()) {
    v.push_back(file.readStringUntil('\n'));
  }
  file.close();
  int i = 4;
  int x = 0;
  int y = 0;
  while (i < v.size()) {
    int corR = v[i].toInt();
    int corG = v[i+1].toInt();
    int corB = v[i+2].toInt();
    //Serial.printf("%d %d %d", corR, corG, corB);
    leds[XY(x, y)] = CRGB(corR, corG, corB);
    i = i + 3;
    x = x + 1;
    if (x == 16) {
      y = y + 1;
      x = 0;
    }
  }
  FastLED.show();
  f = f + 1;
  if (f > 66) {
    f = 0;
  }

  delay(150);
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness( BRIGHTNESS );
  
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
}
