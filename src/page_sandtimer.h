// page_sandtimer.h
#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "config.h"
#include "buttons.h"
#include "flipclock.h"

// ─────────────────────────────────────────────
//  CircuitPal — Sand Timer Page
//  Countdown timer: 3 presets → 5m / 10m / 15m
//  Prev  → back to Home
//  Next  → cycle preset (5 → 10 → 15 → 5…)
//  Enter → start / stop selected preset
// ─────────────────────────────────────────────

namespace PageSandTimer {

static const uint16_t PRESETS[]  = { 5, 10, 15 };  // minutes
static const uint8_t  PRESET_CNT = 3;

static bool     _exitReq    = false;
static uint8_t  _presetIdx  = 0;
static bool     _running    = false;
static uint32_t _endMs      = 0;     // millis() when timer ends
static bool     _done       = false; // countdown complete

void reset() {
  _exitReq   = false;
  _presetIdx = 0;
  _running   = false;
  _done      = false;
}

bool exitRequested() { return _exitReq; }

// ── Hourglass icon (16×24 px, drawn at x,y) ──
static void _drawHourglass(Adafruit_SH1106G& display, int16_t x, int16_t y,
                            uint8_t pct) {
  // Outer frame
  display.drawLine(x,    y,    x+14, y,    SH110X_WHITE);  // top
  display.drawLine(x,    y+22, x+14, y+22, SH110X_WHITE);  // bottom
  display.drawLine(x,    y,    x+7,  y+11, SH110X_WHITE);  // top-left
  display.drawLine(x+14, y,    x+7,  y+11, SH110X_WHITE);  // top-right
  display.drawLine(x,    y+22, x+7,  y+11, SH110X_WHITE);  // bot-left
  display.drawLine(x+14, y+22, x+7,  y+11, SH110X_WHITE);  // bot-right

  // Sand in top half (fills down as time runs)
  // pct = 100 → full top, 0 → empty top
  uint8_t topFill = map(pct, 0, 100, 0, 10);
  for (uint8_t row = 0; row < topFill; row++) {
    uint8_t halfW = (uint8_t)(7.0f * (1.0f - (float)row / 10.0f));
    display.drawFastHLine(x + 7 - halfW, y + 1 + row, halfW * 2, SH110X_WHITE);
  }

  // Sand in bottom half (fills up as time runs)
  uint8_t botFill = map(pct, 100, 0, 0, 10);
  for (uint8_t row = 0; row < botFill; row++) {
    uint8_t hw = (uint8_t)(7.0f * ((float)row / 10.0f));
    display.drawFastHLine(x + 7 - hw, y + 21 - row, hw * 2 + 1, SH110X_WHITE);
  }
}

void draw(Adafruit_SH1106G& display) {
  _exitReq = false;

  if (btn_prev()) { _exitReq = true; _running = false; return; }

  if (btn_next()) {
    if (!_running) {
      _presetIdx = (_presetIdx + 1) % PRESET_CNT;
      _done = false;
    }
  }

  if (btn_enter()) {
    if (!_running && !_done) {
      // Start countdown
      _endMs   = millis() + (uint32_t)PRESETS[_presetIdx] * 60000UL;
      _running = true;
    } else if (_running) {
      // Stop / cancel
      _running = false;
      _done    = false;
    } else if (_done) {
      // Acknowledge done, reset
      _done    = false;
      _running = false;
    }
  }

  // Compute remaining
  uint32_t remaining = 0;
  uint8_t  sandPct   = 100;

  if (_running) {
    uint32_t now = millis();
    if (now >= _endMs) {
      _running   = false;
      _done      = true;
      remaining  = 0;
      sandPct    = 0;
    } else {
      remaining = _endMs - now;
      uint32_t total = (uint32_t)PRESETS[_presetIdx] * 60000UL;
      sandPct = (uint8_t)((remaining * 100UL) / total);
    }
  } else if (!_done) {
    remaining = (uint32_t)PRESETS[_presetIdx] * 60000UL;
    sandPct   = 100;
  }

  uint8_t dispMin = (uint8_t)(remaining / 60000);
  uint8_t dispSec = (uint8_t)((remaining % 60000) / 1000);

  display.clearDisplay();

  if (_done) {
    // Flash "Done!" text
    if ((millis() / 400) % 2 == 0) {
      display.setTextSize(2);
      display.setTextColor(SH110X_WHITE);
      display.setCursor(26, 22);
      display.print("Done!");
    }
    display.setTextSize(1);
    display.setCursor(12, 54);
    display.print("[ENTER] to dismiss");
    display.display();
    return;
  }

  // Hourglass at left
  _drawHourglass(display, 4, 18, sandPct);

  // Clean digital clock at right — slightly smaller to avoid overlap with hourglass
  // Use textSize=3 to shrink for hourglass layout
  fc_render_clean(display, dispMin, dispSec, true, 18, 3);

  // Status / preset selector
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Preset selector dots
  display.setCursor(0, 2);
  display.print("Preset:");
  for (uint8_t i = 0; i < PRESET_CNT; i++) {
    if (i == _presetIdx)
      display.fillCircle(60 + i * 16, 6, 4, SH110X_WHITE);
    else
      display.drawCircle(60 + i * 16, 6, 4, SH110X_WHITE);
    display.setTextColor(i == _presetIdx ? SH110X_BLACK : SH110X_WHITE);
    display.setCursor(57 + i * 16, 3);
    display.print(PRESETS[i]);
    display.setTextColor(SH110X_WHITE);
  }

  // Status hint
  display.setCursor(0, 56);
  if (_running)
    display.print("[ENTER]=stop");
  else
    display.print("[NEXT]=preset [ENTER]=go");

  display.display();
}

} // namespace PageSandTimer
