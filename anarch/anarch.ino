/**
  @file anarch_wio_terminal.ino

  by Miloslav Ciz (drummyfish), 2019   https://gitlab.com/drummyfish/anarch

  Released under CC0 1.0 (https://creativecommons.org/publicdomain/zero/1.0/)
  plus a waiver of all other intellectual property. The goal of this work is to
  be and remain completely in the public domain forever, available for any use
  whatsoever.

  WIO mapping DBZOO
*/
#define SFG_CAN_EXIT 0
#define SFG_FPS 36
#define SFG_TIME_MULTIPLIER 900    /* Without this the game seems too fast. This
                                      also achieves an effective FPS of about
                                      17. */
#define SFG_SCREEN_RESOLUTION_X 320
#define SFG_SCREEN_RESOLUTION_Y 240
#define SFG_RESOLUTION_SCALEDOWN 1
#define SFG_RAYCASTING_MAX_STEPS 18
#define SFG_RAYCASTING_MAX_HITS 6
#define SFG_RAYCASTING_SUBSAMPLE 1
#define SFG_DIMINISH_SPRITES 1
#define SFG_DITHERED_SHADOW 0
#define SFG_BACKGROUND_BLUR 1
#define SFG_PLAYER_TURN_SPEED 135

#include "game.h"
//#include "sounds.h" //TODO
#include <TFT_eSPI.h>

TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft);

void SFG_setPixel(uint16_t x, uint16_t y, uint8_t colorIndex)
{
  // use palette.h
  spr.drawPixel(x, y, paletteRGB565[colorIndex]);
}

void SFG_sleepMs(uint16_t timeMs) {
  delay(timeMs);
}

int8_t SFG_keyPressed(uint8_t key)
{
  uint8_t button = 0;
  switch (key)
  {
    case SFG_KEY_UP: button = WIO_5S_UP; break;
    case SFG_KEY_RIGHT: button = WIO_5S_RIGHT; break;
    case SFG_KEY_DOWN: button = WIO_5S_DOWN; break;
    case SFG_KEY_LEFT: button = WIO_5S_LEFT; break;
    case SFG_KEY_A: button = WIO_KEY_A; break; // right
    case SFG_KEY_B: button = WIO_KEY_B; break; //middle
    case SFG_KEY_C: button = WIO_KEY_C; break; //left
    default: return 0; break;
  }
  return digitalRead(button) == LOW;
}

void SFG_processEvent(uint8_t event, uint8_t value) {}
void SFG_getMouseOffset(int16_t *x, int16_t *y) {}
void SFG_setMusic(uint8_t value) {}
void SFG_playSound(uint8_t soundIndex, uint8_t volume) {}
void SFG_save(uint8_t data[SFG_SAVE_SIZE]) {}

uint8_t SFG_load(uint8_t data[SFG_SAVE_SIZE]) {
  return 0;
}


uint32_t SFG_getTimeMs() {
  return millis();
}

void setup()
{
  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);
  pinMode(WIO_BUZZER, OUTPUT);

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  spr.createSprite(SFG_SCREEN_RESOLUTION_X, SFG_SCREEN_RESOLUTION_Y);

  SFG_init();
}

void loop()
{
  SFG_mainLoopBody();
  spr.pushSprite(0, 0);
}
