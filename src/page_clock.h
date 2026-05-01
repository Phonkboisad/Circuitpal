#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <time.h>
#include "config.h"
#include "buttons.h"
#include "flipclock.h"

// ─────────────────────────────────────────────
//  CircuitPal — Clock Page
//  12-hour flip-clock fetched from NTP
//  Prev → back to Home | Next/Enter → nothing
// ─────────────────────────────────────────────

namespace PageClock {

static bool   _exitReq   = false;
static bool   _colonVis  = true;
static uint32_t _lastBlink = 0;
static uint32_t _lastSync  = 0;
static bool   _hasValidTime = false;

void reset() {
  _exitReq = false;
  _hasValidTime = false;
}
bool exitRequested() { return _exitReq; }

void syncTime() {
  configTime(GMT_OFFSET, DST_OFFSET, NTP_SERVER);
}

void draw(Adafruit_SH1106G& display) {
  _exitReq = false;

  if (btn_prev()) { _exitReq = true; return; }
  btn_next();   // consume / do nothing
  btn_enter();  // consume / do nothing

  // Retry quickly until first valid time, then keep normal 5-minute sync.
  uint32_t syncEvery = _hasValidTime ? 300000UL : 10000UL;
  if (millis() - _lastSync > syncEvery) { syncTime(); _lastSync = millis(); }

  // Blink colon every 500 ms
  if (millis() - _lastBlink > 500) { _colonVis = !_colonVis; _lastBlink = millis(); }

  struct tm timeInfo;
  // Use short timeout so draw() stays responsive while waiting for NTP.
  bool gotTime = getLocalTime(&timeInfo, 120);

  display.clearDisplay();

  if (!gotTime) {
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(16, 28);
    display.print("Syncing time...");
    display.display();
    return;
  }
  _hasValidTime = true;

  // Convert to 12h
  uint8_t h = timeInfo.tm_hour % 12;
  if (h == 0) h = 12;
  uint8_t m = timeInfo.tm_min;
  bool    pm = timeInfo.tm_hour >= 12;

  // Draw clean digital clock (big, centred)
  fc_render_clean(display, h, m, _colonVis, 12, 4);

  // Date bar at bottom
  char dateBuf[24];
  const char* days[]   = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  const char* months[] = {"Jan","Feb","Mar","Apr","May","Jun",
                           "Jul","Aug","Sep","Oct","Nov","Dec"};
  snprintf(dateBuf, sizeof(dateBuf), "%s %d %s %d",
           days[timeInfo.tm_wday], timeInfo.tm_mday,
           months[timeInfo.tm_mon], 1900 + timeInfo.tm_year);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(2, 56);
  display.print(pm ? "PM  " : "AM  ");
  display.print(dateBuf);

  display.display();
}

} // namespace PageClock
