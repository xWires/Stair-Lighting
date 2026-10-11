#include <FastLED.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include "Secrets.h"

CRGB themeDefault(int);
CRGB themeChristmas(int);
CRGB themeHalloween(int);

const int ECHO_PIN = 26;
const int TRIG_PIN = 27;

const int LED_PIN = 33;
const int NUM_LEDS = 228;
const float LED_DIST = 1.66; // cm
const int NUM_SURROUND = 5;

CRGB leds[NUM_LEDS];
AsyncWebServer server(80);
String currentTheme = "default";
String currentMode = "follow";
int currentBrightness = 128; // 1-255 only

void setup()
{
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(currentBrightness);

  fill_solid(leds, NUM_LEDS, CRGB::Yellow);
  FastLED.show();

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.print("Connected, IP: ");
  Serial.println(WiFi.localIP());

  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();

  server.on("/", HTTP_GET, handleIndex);
  server.on("/updateTheme", HTTP_POST, handleUpdateTheme);
  server.on("/updateMode", HTTP_POST, handleUpdateMode);
  server.on("/updateBrightness", HTTP_POST, handleUpdateBrightness);
  server.begin();

  delay(1000);

  FastLED.clear();
  FastLED.show();
}

void loop()
{
  if (currentMode == "follow") modeFollow();
  else if (currentMode == "static") modeStatic();
  else if (currentMode == "off") modeOff();
}

void handleIndex(AsyncWebServerRequest *req)
{
  req->send(200, "text/html", R"EOF(
    <!DOCTYPE html>
    <html>
      <head>
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
        <style>
          * { font-family: Arial; }
        </style>
      </head>
      <body>
        <script>
          window.onload = function() {
            document.getElementById("mode").value = "%CURRENT_MODE%";
            document.getElementById("theme").value = "%CURRENT_THEME%";
            document.getElementById("brightness").value = "%CURRENT_BRIGHTNESS%";
          }

          function updateTheme() {
            const themeDropdown = document.getElementById("theme");
            const theme = themeDropdown.options[themeDropdown.selectedIndex].value;

            const req = new XMLHttpRequest();
            req.open("POST", `/updateTheme?theme=${theme}`);
            req.send();
          }

          function updateMode() {
            const modeDropdown = document.getElementById("mode");
            const mode = modeDropdown.options[modeDropdown.selectedIndex].value;

            const req = new XMLHttpRequest();
            req.open("POST", `/updateMode?mode=${mode}`);
            req.send();
          }

          function updateBrightness() {
            const brightness = document.getElementById("brightness").value;

            const req = new XMLHttpRequest();
            req.open("POST", `/updateBrightness?brightness=${brightness}`);
            req.send();
          }
        </script>
        <h1>Stair Lighting Control</h1>

        <label for="mode">Mode</label>
        <select id="mode" onchange="updateMode()">
          <option value="follow">Follow</option>
          <option value="static">Static</option>
          <option value="off">Off</option>
        </select>
        <br />
        <br />
        <label for="theme">Theme</label>
        <select id="theme" onchange="updateTheme()">
          <option value="default">Default</option>
          <option value="christmas">Christmas</option>
          <option value="halloween">Halloween</option>
        </select>
        <br />
        <br />
        <label for="brightness">Brightness</label>
        <input type="range" id="brightness" min="1" max="255" onchange="updateBrightness()" />
      </body>
    </html>
  )EOF", htmlProcessor);
}

void handleUpdateTheme(AsyncWebServerRequest *req)
{
  if (!req->hasParam("theme")) return;

  currentTheme = req->getParam("theme")->value();

  req->send(200, "text/plain", "Theme updated to " + currentTheme);
}

void handleUpdateMode(AsyncWebServerRequest *req)
{
  if (!req->hasParam("mode")) return;

  currentMode = req->getParam("mode")->value();

  req->send(200, "text/plain", "Mode updated to " + currentMode);
}

void handleUpdateBrightness(AsyncWebServerRequest *req)
{
  if (!req->hasParam("brightness")) return;

  int brightness = req->getParam("brightness")->value().toInt();
  if (brightness > 255)
  {
    req->send(400, "text/plain", "Brightness cannot be more than 255");
    return;
  }
  else if (brightness < 1)
  {
    req->send(400, "text/plain", "Brightness cannot be less than 1");
    return;
  }

  currentBrightness = brightness;
  FastLED.setBrightness(currentBrightness);
  FastLED.show();

  req->send(200, "text/plain", "Brightness updated to " + currentBrightness);
}

String htmlProcessor(const String& placeholder)
{
  if (placeholder == "CURRENT_THEME") return currentTheme;
  else if (placeholder == "CURRENT_MODE") return currentMode;
  else if (placeholder == "CURRENT_BRIGHTNESS") return String(currentBrightness);
}

CRGB getCol(int led)
{
  if (currentTheme == "default") return themeDefault(led);
  else if (currentTheme == "christmas") return themeChristmas(led);
  else if (currentTheme == "halloween") return themeHalloween(led);
}

void modeFollow()
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

void modeStatic()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = getCol(i);
  }
  FastLED.show();
  delay(1000);
}

void modeOff()
{
  FastLED.clear();
  FastLED.show();
  delay(1000);
}

CRGB themeDefault(int led)
{
  return CRGB::White;
}

CRGB themeChristmas(int led)
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

CRGB themeHalloween(int led)
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
