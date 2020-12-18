// WIO_Sprite
//
// Example sketch which shows how to display an
// animated GIF image stored in FLASH memory
// Uses the library: https://github.com/bitbank2/AnimatedGIF

// Load GIF library
#include <AnimatedGIF.h>
AnimatedGIF gif;

// Example AnimatedGIF library images
#include "badgers.h"
#define GIF_IMAGE ucBadgers

#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

// Draw a line of image to a Sprite buffer and then render it
void GIFDraw(GIFDRAW *pDraw)
{
  uint8_t *s;
  uint16_t *d, *usPalette, usTemp[320];
  int x, y;

  usPalette = pDraw->pPalette;
  y = pDraw->iY + pDraw->y; // current line

  s = pDraw->pPixels;
  if (pDraw->ucDisposalMethod == 2) // restore to background color
  {
    for (x = 0; x < pDraw->iWidth; x++)
    {
      if (s[x] == pDraw->ucTransparent)
        s[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }
  // Apply the new pixels to the main image
  if (pDraw->ucHasTransparency) // if transparency used
  {
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    int x, iCount;
    pEnd = s + pDraw->iWidth;
    x = 0;
    iCount = 0; // count non-transparent pixels
    while (x < pDraw->iWidth)
    {
      c = ucTransparent - 1;
      d = usTemp;
      while (c != ucTransparent && s < pEnd)
      {
        c = *s++;
        if (c == ucTransparent) // done, stop
        {
          s--; // back up to treat it like transparent
        }
        else // opaque
        {
          *d++ = usPalette[c];
          iCount++;
        }
      } // while looking for opaque pixels
      if (iCount) // any opaque pixels?
      {
        for (int xOffset = 0; xOffset < iCount; xOffset++ ) {
          spr.drawPixel(x + xOffset, 0, usTemp[xOffset]);
        }
        x += iCount;
        iCount = 0;
      }
      // no, look for a run of transparent pixels
      c = ucTransparent;
      while (c == ucTransparent && s < pEnd)
      {
        c = *s++;
        if (c == ucTransparent)
          iCount++;
        else
          s--;
      }
      if (iCount)
      {
        x += iCount; // skip these
        iCount = 0;
      }
    }
  }
  else
  {
    s = pDraw->pPixels;
    // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
    for (x = 0; x < pDraw->iWidth; x++)
    {
      spr.drawPixel(x, 0, usPalette[*s++]);
    }
  }

  int cx = (DISPLAY_WIDTH - pDraw->iWidth) / 2;
  int cy = (DISPLAY_HEIGHT - pDraw->iHeight) / 2 + pDraw->y;    
  spr.pushSprite(cx, cy);
} /* GIFDraw() */

void setup() {  
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  gif.begin(LITTLE_ENDIAN_PIXELS);
  spr.createSprite(DISPLAY_WIDTH, 1);
}

void loop()
{
  // Put your main code here, to run repeatedly:
  if (gif.open((uint8_t *)GIF_IMAGE, sizeof(GIF_IMAGE), GIFDraw))
  {
    while (gif.playFrame(true, NULL)) {
      yield();
    }    
    gif.close();
  }
}
