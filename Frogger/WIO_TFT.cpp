#include "WIO_TFT.h"

// Extend the Sprite for bitmap blit

TFT_eSPI_extended::TFT_eSPI_extended(TFT_eSPI *tft, int16_t w, int16_t h) : TFT_eSprite(tft) { 
  createSprite(w, h); 
}

void TFT_eSPI_extended::drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h) {        
    for(int16_t j=0; j<h; j++, y++) {
        for(int16_t i=0; i<w; i++ ) {
            drawPixel(x+i, y, pgm_read_word(&bitmap[j * w + i]));
        }
    }    
}
