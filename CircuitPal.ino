// ╔═══════════════════════════════════════════╗
// ║           Project CircuitPal              ║
// ║  ESP-32 · SH1106 OLED · 3 Push Buttons   ║
// ╚═══════════════════════════════════════════╝
//
//  Hardware:
//    SH1106 128×64 OLED  → I2C (SDA=21, SCL=22 default)
//    Button PREV         → GPIO 25 (active LOW, pull-up)
//    Button NEXT         → GPIO 26 (active LOW, pull-up)
//    Button ENTER        → GPIO 27 (active LOW, pull-up)
//
//  Libraries required (install via Arduino Library Manager):
//    • Adafruit SH110X          (Adafruit)
//    • Adafruit GFX Library     (Adafruit)
//    • ArduinoJson              (Benoit Blanchon)
//    • WiFi / HTTPClient        (built-in ESP32 core)
//    • Preferences              (built-in ESP32 core)
//
//  Board: ESP32 Dev Module (Arduino IDE → Board Manager → esp32 by Espressif)

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Project modules
#include "src/config.h"
#include "src/buttons.h"
#include "src/page_intro.h"
#include "src/page_wifi.h"
#include "src/page_home.h"
#include "src/page_clock.h"
#include "src/page_timer.h"
#include "src/page_sandtimer.h"
#include "src/page_weather.h"
#include "src/page_gif.h"

// ── Display object ────────────────────────────
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ── Global state machine ──────────────────────
static Page currentPage = PAGE_INTRO;
static bool wifiDone    = false;

// ─────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // Display init
  Wire.begin();
  if (!display.begin(I2C_ADDRESS, true)) {
    Serial.println("[ERROR] SH1106 not found. Check wiring.");
    while (true) delay(1000);
  }
  display.setContrast(255);
  display.clearDisplay();
  display.display();

  // Buttons
  buttons_init();

  // Start on intro page
  currentPage = PAGE_INTRO;
  PageIntro::reset();

  Serial.println("[CircuitPal] Boot complete.");
}

// ─────────────────────────────────────────────
void loop() {
  buttons_update();

  switch (currentPage) {

    // ── INTRO ──────────────────────────────
    case PAGE_INTRO:
      if (!PageIntro::draw(display)) {
        // Intro finished → begin WiFi flow
        PageWifi::begin(display);
        if (PageWifi::isConnected()) {
          PageClock::syncTime();  // kick NTP on first connect
          currentPage = PAGE_HOME;
          PageHome::reset();
        } else {
          currentPage = PAGE_WIFI_PORTAL;
        }
      }
      break;

    // ── WIFI PORTAL ────────────────────────
    case PAGE_WIFI_PORTAL:
      if (PageWifi::tick(display)) {
        PageClock::syncTime();
        currentPage = PAGE_HOME;
        PageHome::reset();
      }
      break;

    // ── HOME ───────────────────────────────
    case PAGE_HOME:
      PageHome::draw(display);
      if (PageHome::wasEntered()) {
        Page dest = PageHome::destination();
        currentPage = dest;

        // Reset the destination page on entry
        switch (dest) {
          case PAGE_CLOCK:     PageClock::reset();     break;
          case PAGE_TIMER:     PageTimer::reset();     break;
          case PAGE_SANDTIMER: PageSandTimer::reset();  break;
          case PAGE_WEATHER:   PageWeather::reset();   break;
          case PAGE_GIF:       PageGif::reset();       break;
          default: break;
        }
      }
      break;

    // ── CLOCK ──────────────────────────────
    case PAGE_CLOCK:
      PageClock::draw(display);
      if (PageClock::exitRequested()) {
        currentPage = PAGE_HOME;
        PageHome::reset();
      }
      break;

    // ── TIMER ──────────────────────────────
    case PAGE_TIMER:
      PageTimer::draw(display);
      if (PageTimer::exitRequested()) {
        currentPage = PAGE_HOME;
        PageHome::reset();
      }
      break;

    // ── SAND TIMER ─────────────────────────
    case PAGE_SANDTIMER:
      PageSandTimer::draw(display);
      if (PageSandTimer::exitRequested()) {
        currentPage = PAGE_HOME;
        PageHome::reset();
      }
      break;

    // ── WEATHER ────────────────────────────
    case PAGE_WEATHER:
      PageWeather::draw(display);
      if (PageWeather::exitRequested()) {
        currentPage = PAGE_HOME;
        PageHome::reset();
      }
      break;

    // ── GIF ────────────────────────────────
    case PAGE_GIF:
      PageGif::draw(display);
      if (PageGif::exitRequested()) {
        currentPage = PAGE_HOME;
        PageHome::reset();
      }
      break;

    default:
      currentPage = PAGE_HOME;
      break;
  }
}
