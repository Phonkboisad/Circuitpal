#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "config.h"

// ─────────────────────────────────────────────
//  CircuitPal — Intro / Boot Animation Page
// ─────────────────────────────────────────────

namespace PageIntro {

// Slide-in animation state
static int16_t  _boxY     = -40;   // starts above screen
static bool     _done     = false;
static uint32_t _lastTick = 0;
static uint8_t  _phase    = 0;     // 0=slide in, 1=hold, 2=fade dots

void reset() {
  _boxY     = -40;
  _done     = false;
  _lastTick = 0;
  _phase    = 0;
}

bool isDone() { return _done; }

// Returns true while still animating; false when intro is complete
bool draw(Adafruit_SH1106G& display) {
  uint32_t now = millis();

  display.clearDisplay();

  if (_phase == 0) {
    // ── Slide in ──
    if (now - _lastTick > 12) {
      _boxY += 3;
      _lastTick = now;
    }
    int16_t targetY = (SCREEN_HEIGHT - 30) / 2;
    if (_boxY >= targetY) { _boxY = targetY; _phase = 1; _lastTick = now; }

    // Draw rounded box
    int16_t bx = (SCREEN_WIDTH - 80) / 2;
    display.drawRoundRect(bx, _boxY, 80, 30, 5, SH110X_WHITE);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(bx + 8, _boxY + 5);
    display.print("  Project");
    display.setCursor(bx + 4, _boxY + 16);
    display.print(" Circuitpal");

  } else if (_phase == 1) {
    // ── Hold ──
    int16_t bx  = (SCREEN_WIDTH - 80) / 2;
    int16_t by  = (SCREEN_HEIGHT - 30) / 2;
    display.drawRoundRect(bx, by, 80, 30, 5, SH110X_WHITE);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(bx + 8, by + 5);
    display.print("  Project");
    display.setCursor(bx + 4, by + 16);
    display.print(" Circuitpal");

    // Animated loading dots at bottom
    uint8_t dot = ((now / 400) % 4);
    display.setCursor(54, 56);
    for (uint8_t i = 0; i < dot; i++) display.print(".");

    if (now - _lastTick > 2000) { _phase = 2; _lastTick = now; }

  } else {
    // ── Phase 2: brief "Connecting…" ──
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(20, 28);
    display.print("Connecting...");
    if (now - _lastTick > 800) { _done = true; }
  }

  display.display();
  return !_done;
}

} // namespace PageIntro
