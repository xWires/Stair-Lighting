#include <FastLED.h>

const CRGB themeDefault(int);
const CRGB themeChristmas(int);

const int ECHO_PIN = 26;
const int TRIG_PIN = 27;
const int BUF_LEN = 5;

const int LED_PIN = 33;
const int NUM_LEDS = 228;
const float LED_DIST = 1.66; // cm
const int BRIGHTNESS = 128; // 1-255 only
const int NUM_SURROUND = 5;

const CRGB (*getCol)(int) = themeChristmas; // Use this to choose a theme

float buf[BUF_LEN];
CRGB leds[NUM_LEDS];

void setup()
{
  Serial.begin(9600);

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

  long duration = pulseIn(ECHO_PIN, HIGH);

  float distCm = duration * 0.034 / 2;

  FastLED.clear();

  int led = round(distCm / LED_DIST);

  for (int i = 0; i <= NUM_SURROUND; i++)
  {
    leds[led - i] = getCol(led - i);
    leds[led + i] = getCol(led + i);
  }
  leds[led] = getCol(led);
  FastLED.show();
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
