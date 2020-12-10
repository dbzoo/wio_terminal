/*
 * Demonstration of simulated Internet traffic monitor
 * Uses the LVGL lv_chart object, font, style and background tasks.
 * Runs the WIO Terminal
*/

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <stdlib.h>
#include "traffic.h"

#define LVGL_TICK_PERIOD 5

TFT_eSPI tft = TFT_eSPI(); /* TFT instance */
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

/* Interrupt driven periodic handler, being polled. */
static void lv_tick_handler(void) {
  lv_tick_inc(LVGL_TICK_PERIOD);
}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint16_t c;

  tft.startWrite(); /* Start new TFT transaction */
  tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1)); /* set the working window */
  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      c = color_p->full;
      tft.writeColor(c, 1);
      color_p++;
    }
  }
  tft.endWrite(); /* terminate TFT transaction */
  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

void setup() {
  lv_init();

  tft.begin(); /* TFT init */
  tft.setRotation(3); /* Landscape orientation */

  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 320;
  disp_drv.ver_res = 240;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  traffic();
}

void loop() {
  lv_tick_handler();
  lv_task_handler(); /* let the GUI do its work */
  delay(LVGL_TICK_PERIOD);
}
