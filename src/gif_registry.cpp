#include "gif_data.h"

GifDescriptor* GIFS[20];   // support up to 20 GIFs
uint8_t GIF_COUNT = 0;

void registerGif(GifDescriptor* gif) {
  if (GIF_COUNT < sizeof(GIFS)/sizeof(GIFS[0])) {
    GIFS[GIF_COUNT++] = gif;
  }
}
