#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "buttons.h"

// ─────────────────────────────────────────────
//  CircuitPal — Weather Page
//  Fetches real-time weather for Chittagong
//  via OpenWeatherMap free API
//  Prev → back | Next/Enter → nothing
// ─────────────────────────────────────────────

namespace PageWeather {

static bool    _exitReq     = false;
static bool    _fetching    = false;
static bool    _hasFailed   = false;
static uint32_t _lastFetch  = 0;
static uint32_t _refreshMs  = 300000; // refresh every 5 min

// Cached data
struct WeatherData {
  char   city[24]    = "";
  char   desc[32]    = "";
  float  tempC       = 0;
  float  feelsLike   = 0;
  int    humidity    = 0;
  float  windKph     = 0;
  int    pressure    = 0;
  int    visibility  = 0;   // metres
  char   icon[8]     = "";  // e.g. "01d"
  bool   valid       = false;
};

static WeatherData _data;

void reset() { _exitReq = false; }
bool exitRequested() { return _exitReq; }

// ── Small weather icon glyphs (12×12 pixels) ──
// We represent icons as simple drawings based on OWM icon code prefix
static void _drawIcon(Adafruit_SH1106G& display, int16_t x, int16_t y, const char* icon) {
  char c = icon[0]; // '0'=clear/few, '0'=clouds, '0'=shower, etc.
  uint8_t id = atoi(icon); // numeric part

  if (id >= 200 && id < 300) {
    // Thunderstorm — cloud + lightning bolt
    display.fillRoundRect(x, y, 14, 8, 3, SH110X_WHITE);
    display.drawLine(x+5, y+9, x+3, y+13, SH110X_WHITE);
    display.drawLine(x+3, y+13, x+7, y+13, SH110X_WHITE);
    display.drawLine(x+7, y+13, x+5, y+17, SH110X_WHITE);
  } else if (id >= 300 && id < 600) {
    // Rain — cloud with drops
    display.fillRoundRect(x, y, 14, 8, 3, SH110X_WHITE);
    display.drawPixel(x+3,  y+10, SH110X_WHITE);
    display.drawPixel(x+7,  y+11, SH110X_WHITE);
    display.drawPixel(x+11, y+10, SH110X_WHITE);
    display.drawPixel(x+5,  y+13, SH110X_WHITE);
    display.drawPixel(x+9,  y+13, SH110X_WHITE);
  } else if (id >= 600 && id < 700) {
    // Snow — asterisks
    for (int8_t sx = x; sx < x+14; sx += 5) {
      display.drawLine(sx,   y+8, sx,   y+16, SH110X_WHITE);
      display.drawLine(sx-3, y+12, sx+3, y+12, SH110X_WHITE);
    }
  } else if (id >= 700 && id < 800) {
    // Fog / mist — horizontal dashes
    for (int8_t row = 0; row < 4; row++)
      display.drawFastHLine(x, y+7+row*3, 14, SH110X_WHITE);
  } else if (id == 800) {
    // Clear sky — circle sun
    display.drawCircle(x+7, y+9, 4, SH110X_WHITE);
    // Rays
    display.drawLine(x+7, y,   x+7, y+2,  SH110X_WHITE);
    display.drawLine(x+7, y+16, x+7, y+18, SH110X_WHITE);
    display.drawLine(x,   y+9,  x+2, y+9,  SH110X_WHITE);
    display.drawLine(x+12, y+9, x+14, y+9, SH110X_WHITE);
  } else {
    // Cloudy — overlapping circles
    display.fillCircle(x+4,  y+10, 4, SH110X_WHITE);
    display.fillCircle(x+10, y+10, 4, SH110X_WHITE);
    display.fillRect(x+4,   y+6,  7,  6, SH110X_WHITE);
    display.fillCircle(x+7,  y+7,  4, SH110X_WHITE);
  }
}

// ── HTTP fetch ────────────────────────────────
static void _fetchWeather() {
  HTTPClient http;
  http.begin(OWM_URL);
  int code = http.GET();

  if (code == 200) {
    String payload = http.getString();
    StaticJsonDocument<1024> doc;
    DeserializationError err = deserializeJson(doc, payload);

    if (!err) {
      strlcpy(_data.city,      doc["name"]                        | "—",   sizeof(_data.city));
      strlcpy(_data.desc,      doc["weather"][0]["description"]   | "—",   sizeof(_data.desc));
      strlcpy(_data.icon,      doc["weather"][0]["icon"]          | "01d", sizeof(_data.icon));
      _data.tempC     = doc["main"]["temp"]       | 0.0f;
      _data.feelsLike = doc["main"]["feels_like"] | 0.0f;
      _data.humidity  = doc["main"]["humidity"]   | 0;
      _data.pressure  = doc["main"]["pressure"]   | 0;
      _data.windKph   = (doc["wind"]["speed"] | 0.0f) * 3.6f; // m/s → kph
      _data.visibility= doc["visibility"]         | 0;
      _data.valid     = true;
      _hasFailed      = false;
    }
  } else {
    _hasFailed = true;
  }
  http.end();
}

void draw(Adafruit_SH1106G& display) {
  _exitReq = false;

  if (btn_prev())  { _exitReq = true; return; }
  btn_next();   // do nothing
  btn_enter();  // do nothing

  // Auto-refresh
  if (!_data.valid || millis() - _lastFetch > _refreshMs) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(12, 28);
    display.print("Fetching weather...");
    display.display();
    _fetchWeather();
    _lastFetch = millis();
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  if (_hasFailed) {
    display.setCursor(4, 20);
    display.print("Weather fetch failed");
    display.setCursor(4, 34);
    display.print("Check API key/WiFi");
    display.display();
    return;
  }

  // ── Layout (128×64) ──
  // Row 0 (y=0):  City + icon
  // Row 1 (y=12): Temp (large) + feels like
  // Row 2 (y=32): Humidity | Wind
  // Row 3 (y=44): Pressure | Visibility
  // Row 4 (y=56): Description

  // City name (top left)
  display.setCursor(0, 0);
  display.print(_data.city);

  // Icon (top right, 16px wide, y=0)
  _drawIcon(display, SCREEN_WIDTH - 18, 0, _data.icon);

  // Temperature — large text
  display.setTextSize(2);
  display.setCursor(0, 12);
  display.print((int)round(_data.tempC));
  display.print((char)247); // degree symbol
  display.print("C");

  // Feels like — small, to the right
  display.setTextSize(1);
  display.setCursor(68, 14);
  display.print("Fl:");
  display.print((int)round(_data.feelsLike));
  display.print((char)247);

  // Separator
  display.drawFastHLine(0, 30, SCREEN_WIDTH, SH110X_WHITE);

  // Humidity | Wind
  display.setCursor(0, 33);
  display.print("H:");
  display.print(_data.humidity);
  display.print("%");
  display.setCursor(64, 33);
  display.print("W:");
  display.print((int)round(_data.windKph));
  display.print("kph");

  // Pressure | Visibility
  display.setCursor(0, 44);
  display.print("P:");
  display.print(_data.pressure);
  display.print("hPa");
  display.setCursor(64, 44);
  display.print("V:");
  if (_data.visibility >= 1000)
    display.print(String(_data.visibility / 1000) + "km");
  else
    display.print(String(_data.visibility) + "m");

  // Separator
  display.drawFastHLine(0, 54, SCREEN_WIDTH, SH110X_WHITE);

  // Description (truncated to fit)
  display.setCursor(0, 56);
  display.print(_data.desc);

  display.display();
}

} // namespace PageWeather
