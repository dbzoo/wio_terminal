// Simplistic Etchasketch
// Joystick moves cursor
// left button - choose color
// middle button - clear picture
// right button - toggle draw or movement mode.
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

#define WIDTH 320
#define HEIGHT 220

static int x = WIDTH/2;
static int y = HEIGHT/2;
static int xp = x;
static int yp = y;
int color = 0;
int colors[] = {TFT_RED, TFT_WHITE, TFT_NAVY, TFT_DARKGREEN, TFT_DARKCYAN, TFT_MAROON, TFT_PURPLE, TFT_OLIVE,
  TFT_LIGHTGREY, TFT_DARKGREY, TFT_BLUE, TFT_GREEN, TFT_CYAN, TFT_MAGENTA, 
  TFT_YELLOW, TFT_ORANGE, TFT_GREENYELLOW, TFT_PINK};
const int color_len = sizeof colors / sizeof colors[0];
bool mode = true;

void setup() {
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  
  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);
  pinMode(WIO_KEY_A, INPUT_PULLUP);

  drawMode();
}

void drawMode() {
  tft.fillRect(0,0,10,10, colors[color]);
  if (mode) {
    tft.fillRect(15,0,10,10, TFT_WHITE);
  } else {
    tft.fillRect(15,0,10,10, TFT_BLACK);
    tft.drawRect(15,0,10,10, TFT_WHITE);
  }
}

void loop() {
   // Joystick
   if (digitalRead(WIO_5S_UP) == LOW) {
    y = yp - 1;
    if(y < 0) y = yp = HEIGHT;    
   }
   if (digitalRead(WIO_5S_DOWN) == LOW) {
    y = yp + 1;
    if(y > HEIGHT) y = yp = 0;
   }
   if (digitalRead(WIO_5S_LEFT) == LOW) {
    x = xp - 1;
    if(x < 0) x = xp = WIDTH;
   }
   if (digitalRead(WIO_5S_RIGHT) == LOW) {
    x = xp + 1;
    if(x > WIDTH) x = xp = 0;
   }
   if (digitalRead(WIO_KEY_B) == LOW) { // middle button
    tft.fillScreen(TFT_BLACK);
    drawMode();
   }
   if (digitalRead(WIO_KEY_C) == LOW) { // left
    color = (color + 1) % color_len;
    drawMode();
    delay(200); // primative debounce
   }
   if (digitalRead(WIO_KEY_A) == LOW) { //right
    mode = !mode;
    drawMode();
    delay(200);
   }

   // Draw delta
   if (mode) {
      tft.drawLine(x,y,xp,yp,colors[color]);    
   } else {
      tft.drawPixel(xp,yp, TFT_BLACK);
      tft.drawPixel(x,y, TFT_WHITE);
   }
   xp = x;
   yp = y;

   delay(10);
}
