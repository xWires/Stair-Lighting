#include <FastLED.h>

#include "Secrets.h"

const CRGB themeDefault(int);
const CRGB themeChristmas(int);
const CRGB themeHalloween(int);

const int ECHO_PIN = 26;
const int TRIG_PIN = 27;

const int LED_PIN = 33;
const int NUM_LEDS = 228;
const float LED_DIST = 1.66; // cm
const int BRIGHTNESS = 128; // 1-255 only
const int NUM_SURROUND = 5;

const CRGB (*getCol)(int) = themeChristmas; // Use this to choose a theme

CRGB leds[NUM_LEDS];

void setup()
{
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  float distCm = duration * 0.034 / 2;
  Serial.print("duration: ");
  Serial.println(duration);
  Serial.print("distCm: ");
  Serial.println(distCm);

  FastLED.clear();

  int led = round(distCm / LED_DIST);
  if (led < 0 || led >= NUM_LEDS)
  {
    Serial.println("led OOB");
    FastLED.clear();
    FastLED.show();
    return;
  }

  for (int i = 0; i <= NUM_SURROUND; i++)
  {
    int before = led - i;
    if (before > 0 && before < NUM_LEDS)
    {
      leds[before] = getCol(before);
    }
    
    int after = led + i;
    if (after > 0 && after < NUM_LEDS)
    {
      leds[after] = getCol(after);
    }
  }
  leds[led] = getCol(led);
  FastLED.show();
  delay(60);
}

const CRGB themeDefault(int led)
{
  return CRGB::White;
}

const CRGB themeChristmas(int led)
{
  if (led % 2 == 0)
  {
    return CRGB::Red;
  }
  else
  {
    return CRGB::Green;
  }
}

const CRGB themeHalloween(int led)
{
  if (led % 2 == 0)
  {
    return CRGB::Orange;
  }
  else
  {
    return CRGB::Purple;
  }
}
