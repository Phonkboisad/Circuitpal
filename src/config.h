#pragma once

// ─────────────────────────────────────────────
//  CircuitPal — Global Configuration
// ─────────────────────────────────────────────

// Display
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define I2C_ADDRESS    0x3C

// Buttons (active LOW with internal pull-up)
#define BTN_PREV   25   // Previous / Return
#define BTN_NEXT   26   // Next
#define BTN_ENTER  27   // Enter

// Debounce
#define DEBOUNCE_MS  50

// NVS keys
#define NVS_NAMESPACE  "circuitpal"
#define NVS_KEY_SSID   "ssid"
#define NVS_KEY_PASS   "password"

// Soft-AP
#define AP_SSID      "CircuitPal-Setup"
#define AP_PASSWORD  "12345678"

// NTP
#define NTP_SERVER   "pool.ntp.org"
#define GMT_OFFSET    21600   // UTC+6 (Bangladesh)
#define DST_OFFSET    0

// OpenWeatherMap
#define OWM_API_KEY   "Your_API_KEY_HERE"
#define OWM_CITY      "Chittagong"
#define OWM_COUNTRY   "BD"
#define OWM_URL       "http://api.openweathermap.org/data/2.5/weather?q=" OWM_CITY "," OWM_COUNTRY "&appid=" OWM_API_KEY "&units=metric"

// Flip-clock animation speed (ms per frame)
#define FLIP_SPEED_MS  60

// Menus
#define MENU_COUNT   5
#define GIF_COUNT_STATIC    3

// Pages
enum Page {
  PAGE_INTRO = 0,
  PAGE_WIFI_PORTAL,
  PAGE_HOME,
  PAGE_CLOCK,
  PAGE_TIMER,
  PAGE_SANDTIMER,
  PAGE_WEATHER,
  PAGE_GIF
};
