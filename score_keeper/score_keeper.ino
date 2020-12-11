/* 2-player score keeper
 *  Left button player 1 increase
 *  Right button player 2 increase
 *  Middle button reset to 0
 */
#include "TFT_eSPI.h"

TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft);

#define RIGHT_BUTTON WIO_KEY_A
#define CENTER_BUTTON WIO_KEY_B
#define LEFT_BUTTON WIO_KEY_C

static int player_1 = 0;
static int player_2 = 0;

void updateScore() {
  // Blank scores and draw over to avoid a full screen refresh.
  spr.createSprite(110, 40);
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_WHITE);
  spr.setFreeFont(&FreeSansBold24pt7b); 
  spr.drawString(String(player_1), 20, 0);
  spr.pushSprite(30,120);
  spr.fillSprite(TFT_BLACK);
  spr.drawString(String(player_2), 20, 0);  
  spr.pushSprite(190,120);  
  spr.deleteSprite();
}

void drawScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.drawRoundRect(10, 10, 300, 220, 10, TFT_BLUE);
  tft.drawFastVLine(160, 10, 220, TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&FreeSansBold12pt7b); 
  tft.drawString("Player 1", 30, 30);
  tft.drawString("Player 2", 190, 30);
}

void setup() {
  tft.begin();
  tft.setRotation(3);
  
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(CENTER_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);

  drawScreen();
  updateScore();  
}

void loop() {
   if (digitalRead(RIGHT_BUTTON) == LOW) {
    player_2 ++;
    updateScore();
   }
   else if (digitalRead(CENTER_BUTTON) == LOW) {
    player_1 = player_2 = 0;
    updateScore();
   }
   else if (digitalRead(LEFT_BUTTON) == LOW) {
    player_1 ++;
    updateScore();
   }
   delay(200);
}
