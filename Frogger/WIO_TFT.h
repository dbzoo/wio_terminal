#ifndef WIO_TFT
#define WIO_TFT

#include <TFT_eSPI.h>

// Add extra method that 1.8.1 libaries don't include
class TFT_eSPI_extended : public TFT_eSprite {
  public:
     TFT_eSPI_extended(TFT_eSPI *tft, int16_t w, int16_t h);
     void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h);
};

#endif
