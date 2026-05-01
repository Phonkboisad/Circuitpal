#pragma once
#include <Arduino.h>

// Descriptor for each GIF
struct GifDescriptor {
  const char* name;                     // Menu name
  const uint8_t* const* frames;         // For simple GIFs
  const unsigned char (*videoFrames)[1024]; // For video GIFs
  const uint16_t* delays;               // Per-frame delays
  uint16_t frameCount;                  // Number of frames
  uint16_t frameDelayMs;                // Global delay (for video GIFs)
  bool isVideo;                         // true = uses videoFrames
};

// Registry container
extern GifDescriptor* GIFS[];
extern uint8_t GIF_COUNT;

// Function to register new GIFs
void registerGif(GifDescriptor* gif);
