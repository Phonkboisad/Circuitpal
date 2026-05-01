# CircuitPal 🔌

> A feature-rich, multi-function ESP32 gadget with an SH1106 OLED display — NTP clock, stopwatch, sand timer, live weather, and animated GIF player, all navigated with three hardware buttons.

---

## 📸 Overview

CircuitPal is a self-contained embedded device that packs five useful applications into a 128×64 OLED display, powered by an ESP32. On first boot it launches a captive-portal WiFi setup page so you can connect to your network without touching any code. Credentials are saved to NVS flash and reused automatically on every subsequent boot.

---

## ✨ Features

| Page | Description |
|---|---|
| 🌐 **WiFi Portal** | AP-mode captive portal for first-time WiFi setup. Credentials saved to NVS flash. |
| 🕐 **Clock** | NTP-synced 12-hour digital clock with blinking colon, AM/PM, and full date bar. |
| ⏱️ **Timer** | Count-up stopwatch. Start, pause, and reset with two buttons. |
| ⏳ **Sand Timer** | Countdown timer with 5 / 10 / 15-minute presets and an animated hourglass graphic. |
| 🌤️ **Weather** | Real-time weather for Chittagong via OpenWeatherMap — temperature, feels-like, humidity, wind speed, pressure, visibility, and condition icon. Auto-refreshes every 5 minutes. |
| 🎞️ **GIF Player** | Plays animated 128×64 monochrome bitmaps stored in PROGMEM. Supports up to 20 registered GIFs with per-frame or global delay. |

---

## 🔧 Hardware Requirements

| Component | Details |
|---|---|
| **Microcontroller** | ESP32 (any variant with WiFi) |
| **Display** | SH1106 128×64 OLED — I²C, address `0x3C` |
| **Button 1 — PREV** | GPIO 25 → GND (active-LOW, internal pull-up) |
| **Button 2 — NEXT** | GPIO 26 → GND (active-LOW, internal pull-up) |
| **Button 3 — ENTER** | GPIO 27 → GND (active-LOW, internal pull-up) |

> All buttons use the ESP32's internal pull-up resistors — no external resistors required.

---

## 🗂️ Project Structure

```
CircuitPal/
├── CircuitPal.ino              # Main sketch — setup(), loop(), page router
├── README.md
│
└── src/                        # All headers and source files
    ├── config.h                # Pin definitions, NTP, OWM, and display constants
    ├── buttons.h               # Debounced button handler (50 ms)
    ├── flipclock.h             # 7-segment flip-clock and clean digital font renderers
    ├── gif_data.h              # GifDescriptor struct and registry declarations
    ├── gif_registry.cpp        # GIF registry — registerGif() implementation
    │
    ├── page_intro.h            # Boot animation (slide-in box + loading dots)
    ├── page_wifi.h             # WiFi captive portal + NVS credential storage
    ├── page_home.h             # Scrollable home menu
    ├── page_clock.h            # NTP clock page
    ├── page_timer.h            # Stopwatch page
    ├── page_sandtimer.h        # Countdown / sand-timer page
    ├── page_weather.h          # Live weather page (OpenWeatherMap)
    ├── page_gif.h              # GIF browser + player
    │
    ├── anime2.cpp              # GIF frames — anime animation  (~1354 KB)
    ├── spider.cpp              # GIF frames — spider animation   (~61 KB)
    ├── spiral.cpp              # GIF frames — spiral animation   (~49 KB)
    ├── sponge.cpp              # GIF frames — sponge animation    (~7 KB)
    └── Car.cpp                 # GIF frames — car animation       (reserved)
```

---

## 📦 Dependencies

Install the following libraries via the **Arduino Library Manager** (`Sketch → Include Library → Manage Libraries`):

| Library | Version | Purpose |
|---|---|---|
| [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library) | Latest | Graphics primitives |
| [Adafruit SH110X](https://github.com/adafruit/Adafruit_SH110X) | Latest | SH1106 OLED driver |
| [ArduinoJson](https://arduinojson.org/) | 6.x | OpenWeatherMap JSON parsing |

The following are **built into the ESP32 Arduino core** — no installation needed:

- `WiFi.h`
- `WebServer.h`
- `HTTPClient.h`
- `Preferences.h` (NVS)

---

## ⚙️ Configuration

All user-configurable constants live in **`config.h`**:

```cpp
// ── Display ─────────────────────────────────
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64
#define I2C_ADDRESS    0x3C

// ── Button Pins ──────────────────────────────
#define BTN_PREV   25
#define BTN_NEXT   26
#define BTN_ENTER  27

// ── NTP ──────────────────────────────────────
#define NTP_SERVER  "pool.ntp.org"
#define GMT_OFFSET  21600     // UTC+6 — change for your timezone

// ── OpenWeatherMap ───────────────────────────
#define OWM_API_KEY  "your_api_key_here"
#define OWM_CITY     "Chittagong"
#define OWM_COUNTRY  "BD"

// ── Soft-AP (first-time WiFi setup) ──────────
#define AP_SSID      "CircuitPal-Setup"
#define AP_PASSWORD  "12345678"
```

> **Important:** Replace `OWM_API_KEY` with your own free key from [openweathermap.org](https://openweathermap.org/api). The default key in the repo is for development only.

---

## 🚀 Getting Started

### 1. Clone the repository

```bash
git clone https://github.com/Phonkboisad/CircuitPal.git
cd CircuitPal
```

### 2. Install the Arduino ESP32 core

Follow the official guide: [ESP32 Arduino Core Installation](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)

### 3. Install dependencies

Open the Arduino Library Manager and install **Adafruit GFX**, **Adafruit SH110X**, and **ArduinoJson**.

### 4. Set your configuration

Open `config.h` and update:
- `OWM_API_KEY` with your OpenWeatherMap API key
- `OWM_CITY` / `OWM_COUNTRY` for your location
- `GMT_OFFSET` for your UTC timezone offset in seconds

### 5. Select your board and upload

In the Arduino IDE Tools menu, set the following:

| Setting | Value |
|---|---|
| **Board** | `ESP32 Dev Module` (or your specific variant) |
| **Upload Speed** | `921600` |
| **Partition Scheme** | **`Huge APP (3MB No OTA/1MB SPIFFS)`** ⚠️ |

> **Why this partition?** CircuitPal stores GIF frame data as large PROGMEM arrays. The default partition leaves insufficient flash for the app to compile. `Huge APP` allocates 3 MB to the application, which is required when multiple GIF files are included.

Upload the sketch and open the Serial Monitor at **115200 baud** to see boot logs.

---

## 📶 First-Time WiFi Setup

On first boot (or when saved credentials fail), CircuitPal automatically opens a WiFi access point:

| Setting | Value |
|---|---|
| **Network name** | `CircuitPal-Setup` |
| **Password** | `12345678` |
| **Portal URL** | `192.168.4.1` |

1. Connect your phone or laptop to `CircuitPal-Setup`
2. Open a browser and navigate to `192.168.4.1`
3. Enter your home WiFi SSID and password, then tap **Connect**
4. Credentials are saved to flash — the device will reconnect automatically on every future boot

---

## 🕹️ Button Controls

| Button | Home Menu | Clock | Timer | Sand Timer | Weather | GIF Player |
|---|---|---|---|---|---|---|
| **PREV** | Scroll up | ← Back | ← Back | ← Back | ← Back | ← Back |
| **NEXT** | Scroll down | — | Start/Resume | Cycle preset | — | Next GIF |
| **ENTER** | Select item | — | Reset | Start/Stop | — | Open/Select |

---

## 🎞️ Adding Custom GIFs

GIFs are stored as PROGMEM byte arrays — **1-bit monochrome, 128×64 pixels, 1024 bytes per frame**.

### Step 1 — Convert your image or video

Use the [ESP32 OLED Video Converter](https://github.com/triwahyu45/ESP32-OLED-Video-Converter) by **triwahyu45** to convert any image or video clip into the correct byte-array format for this display. It outputs the `PROGMEM` C array that you paste directly into your `.cpp` file.

### Step 2 — Create a new `.cpp` file

Add a new file to the project folder (e.g., `mygif.cpp`). Every GIF file follows this exact structure — based on the same pattern used by the built-in GIFs:

```cpp
#include "gif_data.h"

// ── Frame data ────────────────────────────────────────────────────────────
// Each frame = 128×64 pixels ÷ 8 = 1024 bytes exactly.
// Paste the output from the OLED Video Converter here.
const unsigned char video_frames[][1024] PROGMEM = {
  {
    /* frame 0 — paste your byte array here */
    0x00, 0xFF, /* ... 1024 bytes total ... */
  },
  {
    /* frame 1 */
    0x00, 0xFF, /* ... */
  },
  // add more frames as needed
};

// ── Descriptor ────────────────────────────────────────────────────────────
const uint16_t MYGIF_TOTAL_FRAMES   = 2;    // number of frames above
const uint16_t MYGIF_FRAME_DELAY_MS = 100;  // ms between frames (speed)

static GifDescriptor MYGIF = {
  "My GIF",              // name shown in the GIF browser menu
  nullptr,               // simple frames — leave nullptr for video GIFs
  video_frames,          // your PROGMEM frame array
  nullptr,               // per-frame delay array — nullptr = use global delay
  MYGIF_TOTAL_FRAMES,
  MYGIF_FRAME_DELAY_MS,
  true                   // isVideo = true (uses videoFrames path)
};

// ── Auto-registration ─────────────────────────────────────────────────────
// Runs before setup() — no changes to CircuitPal.ino required.
__attribute__((constructor)) void initMyGif() {
  registerGif(&MYGIF);
}
```

### Step 3 — Done

Save the file and re-upload. The GIF will appear automatically in the on-device GIF browser. The registry supports up to **20 GIFs** simultaneously.

> **Flash tip:** Each frame costs 1024 bytes of PROGMEM. A 30-frame animation uses ~30 KB of flash. Keep the **Huge APP** partition scheme selected to avoid running out of program space.

---

## 🏗️ Architecture

CircuitPal uses a simple **page-based state machine** in `loop()`. Each page is a namespace with three standard functions:

```
reset()          — called when entering the page
draw(display)    — called every loop iteration; handles input + renders
exitRequested()  — returns true when the page wants to return to Home
```

The main loop switches between pages using the `Page` enum defined in `config.h`.

---

## 📝 License

This project is open source and available under the [MIT License](LICENSE).

---

## 🙌 Acknowledgements

- [Adafruit Industries](https://www.adafruit.com/) for their excellent GFX and display libraries
- [OpenWeatherMap](https://openweathermap.org/) for the free weather API
- [ArduinoJson](https://arduinojson.org/) by Benoît Blanchon
- [triwahyu45/ESP32-OLED-Video-Converter](https://github.com/triwahyu45/ESP32-OLED-Video-Converter) for the image/video to OLED PROGMEM converter used to create the GIF assets

---


