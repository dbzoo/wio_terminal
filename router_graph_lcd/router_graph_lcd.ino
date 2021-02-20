/*
 * Monitor router internet bandwidth usage with SNMP and display on a chart.
 * Runs the WIO Terminal
*/
#include <TFT_eSPI.h>
#include <rpcWiFi.h>
#include "traffic.h"

#define LVGL_TICK_PERIOD 5 // ms

const char *ssid = "your-ssid";
const char *password = "your-password";

static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];
TFT_eSPI tft = TFT_eSPI(); /* TFT instance */

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
  Serial.begin(115200);
  //while(!Serial); // Wait for Serial to be ready  
  lv_init();

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor((320 - tft.textWidth("Connecting to Wi-Fi.."))/2, 120);
  tft.print("Connecting to Wi-Fi..");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
    WiFi.begin(ssid, password);
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
    
  tft.fillScreen(TFT_BLACK);

  String myip = WiFi.localIP().toString();
  int t_size = tft.textWidth("Connected ") + tft.textWidth(myip);
  tft.setCursor((320 - t_size)/2, 120);
  tft.print("Connected ");
  tft.print(myip);
  
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
  static unsigned long curMillis;
  static unsigned long startMillis = 0;

  traffic_loop();
  curMillis = millis();
  if (curMillis - startMillis >= LVGL_TICK_PERIOD) {      
      lv_tick_inc(LVGL_TICK_PERIOD);
      startMillis = curMillis;
  }
  lv_task_handler();
}
