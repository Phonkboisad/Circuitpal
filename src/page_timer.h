// page_timer.h
#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "config.h"
#include "buttons.h"
#include "flipclock.h"

// ─────────────────────────────────────────────
//  CircuitPal — Timer Page (Count-up Stopwatch)
//  Starts paused at 00:00
//  Prev  → back to Home
//  Next  → Resume / unpause timer
//  Enter → Reset & pause at 00:00
// ─────────────────────────────────────────────

namespace PageTimer {

static bool     _exitReq   = false;
static bool     _running   = false;
static uint32_t _elapsed   = 0;   // ms
static uint32_t _startedAt = 0;
static bool     _colonVis  = true;
static uint32_t _lastBlink = 0;

void reset() {
  _exitReq   = false;
  _running   = false;
  _elapsed   = 0;
  _startedAt = 0;
}

bool exitRequested() { return _exitReq; }

void draw(Adafruit_SH1106G& display) {
  _exitReq = false;

  if (btn_prev())  { _exitReq = true; return; }
  if (btn_next())  {
    // Start / resume
    if (!_running) {
      _startedAt = millis() - _elapsed;
      _running = true;
    }
  }
  if (btn_enter()) {
    // Reset & pause
    _running = false;
    _elapsed = 0;
  }

  if (_running) _elapsed = millis() - _startedAt;

  // Blink colon when paused
  if (!_running && millis() - _lastBlink > 600) { _colonVis = !_colonVis; _lastBlink = millis(); }
  if (_running) _colonVis = true;

  uint32_t totalSec = _elapsed / 1000;
  uint8_t  minutes  = (totalSec / 60) % 100;  // cap at 99:59
  uint8_t  seconds  = totalSec % 60;

  display.clearDisplay();

  // Title
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(40, 2);
  display.print("TIMER");

  // Draw clean digital timer (big, centered)
  fc_render_clean(display, minutes, seconds, _colonVis, 14, 4);

  // Status bar
  display.setCursor(2, 56);
  if (_running) {
    display.print("[NEXT]=stop [ENTER]=rst");
  } else {
    display.print("[NEXT]=start");
  }

  display.display();
}

} // namespace PageTimer
