// Etchasketch using the Joystick
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

static int x = tft.width()/2;
static int y = tft.height()/2;
static int xp = x;
static int yp = y;

void setup() {
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  
  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);
}

void loop() {
   // Joystick
   if (digitalRead(WIO_5S_UP) == LOW) {
    y = yp - 1;
   }
   if (digitalRead(WIO_5S_DOWN) == LOW) {
    y = yp + 1;
   }
   if (digitalRead(WIO_5S_LEFT) == LOW) {
    x = xp - 1;
   }
   if (digitalRead(WIO_5S_RIGHT) == LOW) {
    x = xp + 1;
   }
   if (digitalRead(WIO_5S_PRESS) == LOW) {
    tft.fillScreen(TFT_BLACK);
   }
   // Range check
   if(x < 0) x = 0;
   else if(x > tft.width()) x = tft.width();   
   else if(y < 0) y = 0;
   else if(y > tft.height()) y = tft.height();

   // Draw delta
   tft.drawLine(x,y,xp,yp,TFT_WHITE);    
   xp = x;
   yp = y;

   delay(10);
}
