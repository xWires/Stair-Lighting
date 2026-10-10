#include <FastLED.h>

const int ECHO_PIN = 26;
const int TRIG_PIN = 27;
const int BUF_LEN = 5;

const int LED_PIN = 33;
const int NUM_LEDS = 228;
const float LED_DIST = 1.5; // cm
const int brightness = 128; // 1-255 only
const CRGB colour = CRGB::White;

float buf[BUF_LEN];
CRGB leds[NUM_LEDS];

void setup()
{
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);
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

  int led = round(distCm * LED_DIST);
  
  leds[led - 1] = colour;
  leds[led] = colour;
  leds[led + 1] = colour;
  FastLED.show();
}
