#pragma once
#include "config.h"
#include <Arduino.h>

// ─────────────────────────────────────────────
//  CircuitPal — Button Handler
// ─────────────────────────────────────────────

struct Button {
  const uint8_t pin;
  bool          rawLast;        // last raw reading (for debounce timer reset)
  bool          stableState;    // debounce-confirmed state
  bool          pressed;        // single-shot falling-edge flag
  uint32_t      lastDebounce;
};

static Button btnPrev  = { BTN_PREV,  HIGH, HIGH, false, 0 };
static Button btnNext  = { BTN_NEXT,  HIGH, HIGH, false, 0 };
static Button btnEnter = { BTN_ENTER, HIGH, HIGH, false, 0 };

inline void buttons_init() {
  pinMode(BTN_PREV,  INPUT_PULLUP);
  pinMode(BTN_NEXT,  INPUT_PULLUP);
  pinMode(BTN_ENTER, INPUT_PULLUP);
}

// Call every loop iteration — updates .pressed flags
inline void buttons_update() {
  Button* btns[] = { &btnPrev, &btnNext, &btnEnter };
  for (auto* b : btns) {
    b->pressed = false;   // clear previous event each loop

    bool reading = digitalRead(b->pin);

    // Raw changed → restart debounce timer
    if (reading != b->rawLast) {
      b->rawLast      = reading;
      b->lastDebounce = millis();
    }

    // Signal stable long enough → commit to stableState
    if ((millis() - b->lastDebounce) >= DEBOUNCE_MS) {
      if (reading != b->stableState) {
        b->stableState = reading;
        // Falling edge (HIGH→LOW) = button pressed (active-LOW wiring)
        if (b->stableState == LOW) {
          b->pressed = true;
        }
      }
    }
  }
}

// Convenience — consume flag after reading
inline bool btn_prev()  { if (btnPrev.pressed)  { btnPrev.pressed  = false; return true; } return false; }
inline bool btn_next()  { if (btnNext.pressed)  { btnNext.pressed  = false; return true; } return false; }
inline bool btn_enter() { if (btnEnter.pressed) { btnEnter.pressed = false; return true; } return false; }
