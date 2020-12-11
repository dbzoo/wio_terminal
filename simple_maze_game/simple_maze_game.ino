/* Port of the following to the Wio Terminal
 * https://hackaday.com/2020/05/26/simple-sprite-routines-ease-handheld-gaming-diy/
 */
#include <TFT_eSPI.h>
#include "LIS3DHTR.h"
#include <Wire.h>

LIS3DHTR<TwoWire> LIS; //IIC
#define WIRE Wire1

class TFT_sprite : public TFT_eSPI {
  public:
    TFT_sprite();
    void xorPixel(int32_t x, uint32_t y, uint16_t color);
    void xorSprite(uint16_t x, uint16_t y, uint64_t sprite, uint16_t color);
    bool hitSprite(uint16_t x, uint16_t y, uint64_t sprite, uint16_t color);
    void moveSprite(uint16_t x, uint16_t y, uint64_t sprite, int dx, int dy, uint16_t color);
};

TFT_sprite::TFT_sprite() : TFT_eSPI() {}

// Plot a pixel at x, y by exclusive-ORing the colour with the display
void TFT_sprite::xorPixel(int32_t x, uint32_t y, uint16_t color) {
  uint16_t lastcolor = readPixel(x, y);
  drawPixel(x, y, color ^ lastcolor);
}

// Plot an 8x8 sprite at x, y by exclusive-ORing the colour with the display
void TFT_sprite::xorSprite (uint16_t x0, uint16_t y0, uint64_t sprite, uint16_t color) {
  bool bit;
  if ((x0 >= 0) && (x0 + 7 < width()) && (y0 >= 0) && (y0 + 7 < height())) {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        bit = sprite >> (63 - x - y * 8) & 1;
        if (bit) xorPixel(x0 + x, y0 + y, color);
      }
    }
  }
}

// Collision detection between an 8x8 sprite at x, y with a specified colour
bool TFT_sprite::hitSprite (uint16_t x0, uint16_t y0, uint64_t sprite, uint16_t color) {
  uint16_t row[8];
  uint32_t col16;
  bool bit;
  if ((x0 >= 0) && (x0 + 7 < width()) && (y0 >= 0) && (y0 + 7 < height())) {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        col16 = readPixel(x0 + x, y0 + y);
        bit = sprite >> (63 - x - y * 8) & 1;
        if (bit && (col16 == color)) {
          return true;
        }
      }
    }
  }
  return false;
}

// Move a sprite from x, y by one pixel in any direction by exclusive-ORing only the changed pixels with the display
void TFT_sprite::moveSprite(uint16_t x0, uint16_t y0, uint64_t sprite, int dx, int dy, uint16_t color) {
  int oldbit, newbit;
  if ((x0 >= 0) && (x0 + 7 < width()) && (y0 >= 0) && (y0 + 7 < height())) {
    dx = (dx > 0) - (dx < 0); dy = (dy > 0) - (dy < 0);
    for (int y = 0; y < 10; y++) {
      for (int x = 0; x < 10; x++) {
        // Sprite's previous position
        int xs = x - 1, ys = y - 1;
        if (xs >= 0 && xs <= 7 && ys >= 0 && ys <= 7) {
          oldbit = sprite >> (63 - ys * 8 - xs) & 1;
        } else oldbit = 0;
        // Sprite's new position
        int xn = x - 1 - dx, yn = y - 1 - dy;
        if (xn >= 0 && xn <= 7 && yn >= 0 && yn <= 7) {
          newbit = sprite >> (63 - yn * 8 - xn) & 1;
        } else newbit = 0;
        if (oldbit != newbit) xorPixel(x0 - 1 + x, y0 - 1 + y, color);
      }
    }
  }
}

#define B(x) S_to_binary_(#x)

static inline unsigned long long S_to_binary_(const char *s)
{
        unsigned long long i = 0;
        while (*s) {
                i <<= 1;
                i += *s++ - '0';
        }
        return i;
}

// The rolling ball maze
const int Sens = 1; // Accelerometer sensitivity
const uint16_t Maze = TFT_MAGENTA;
const uint16_t Ball = TFT_WHITE;
const uint16_t Board = TFT_BLACK;
const uint64_t Sprite = B(
                        00111100\
                        01111110\
                        11111111\
                        11111111\
                        11111111\
                        11111111\
                        01111110\
                        00111100);

TFT_sprite tft = TFT_sprite();

void maze() {
  // Hand drawn Maze
  for (int c = 4; c >= 0; c--) {
    tft.fillCircle(160, 110, (c * 14 + 7), Maze);
    if (c) tft.fillCircle(160, 110, (c * 14 + 5), Board);
  }
  // Gaps
  tft.fillCircle(160, 158, 6, Board);
  tft.fillCircle(112, 110, 6, Board);
  tft.fillCircle(136, 134, 6, Board);
  tft.fillCircle(174, 96, 6, Board);
}

void setup() {  
  // lcd init
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  // Accelerometer init.
  LIS.begin(WIRE); //IRC init default: 0x18
  delay(100);
  if (!LIS) {
     tft.setTextColor(TFT_WHITE);
     tft.setTextSize(2);
     tft.drawString("LIS3DHTR didn't connect.",0,10);
     while (1);
     return;
  }
  LIS.setOutputDataRate(LIS3DHTR_DATARATE_25HZ); // 1Hz up to 5KHz
  LIS.setFullScaleRange(LIS3DHTR_RANGE_2G); //Scale range 2g,4g,8g,16g  

  // render maze
  maze();
}

void loop() {
  int x = 157, y = 161, dx, dy, x1, y1;  
  tft.xorSprite(x, y, Sprite, Ball);
  for (;;) {
    // Move ball?
    int sx = LIS.getAccelerationX() * 100;
    int sy = LIS.getAccelerationY() * 100;
   // Set dx and dy to -1, 0, or +1
    dx = (sx > 0) - (sx < 0);
    dy = (sy > 0) - (sy < 0);
    if (tft.hitSprite(x+dx, y, Sprite, Maze)) dx = 0;
    if (tft.hitSprite(x, y+dy, Sprite, Maze)) dy = 0;
    tft.moveSprite(x, y, Sprite, dx, dy, Ball);
    x = x + dx; y = y + dy;
    delay(50);
  }  
}
