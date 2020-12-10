#include <stdlib.h>
#include <lvgl.h>

LV_FONT_DECLARE(lv_font_montserrat_28_compr_az);  // lowercase only.

#define LV_TRAFFIC_TITLE_HEIGHT 50
#define LV_TRAFFIC_TITLE_PAD 10

#define LV_TRAFFIC_WHITE           lv_color_hex(0xffffff)
#define LV_TRAFFIC_LIGHT           lv_color_hex(0xf3f8fe)
#define LV_TRAFFIC_BLUE            lv_color_hex(0x2f3243)

#define LV_TRAFFIC_POLL_RATE_MS   1000

static lv_style_t style_title;
static lv_style_t style_tx;
static lv_style_t style_rx;

static lv_obj_t * chart;
static lv_chart_series_t * ser1;
static lv_chart_series_t * ser2;

void traffic_task(lv_task_t * task)
{
  lv_chart_set_next(chart, ser1, rand() % 400);
  lv_chart_set_next(chart, ser2, rand() % 400);
}

void traffic(void) {
  // Title
  lv_style_set_text_color(&style_title, LV_STATE_DEFAULT, LV_TRAFFIC_WHITE);
  lv_style_set_text_font(&style_title, LV_STATE_DEFAULT, &lv_font_montserrat_28_compr_az);

  // Rx
  lv_style_set_text_color(&style_rx, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  lv_style_set_text_font(&style_rx, LV_STATE_DEFAULT, &lv_font_montserrat_28_compr_az);

  // Tx
  lv_style_set_text_color(&style_tx, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_style_set_text_font(&style_tx, LV_STATE_DEFAULT, &lv_font_montserrat_28_compr_az);

  // Create Title bar
  lv_obj_t * bar = lv_obj_create(lv_scr_act(), NULL);
  lv_obj_set_size(bar, LV_HOR_RES, LV_TRAFFIC_TITLE_HEIGHT);
  lv_obj_set_style_local_bg_color(bar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_TRAFFIC_BLUE);

  lv_obj_t * title = lv_label_create(bar, NULL);
  lv_label_set_text(title, "internet");
  lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_LEFT, 20, LV_TRAFFIC_TITLE_PAD);
  lv_obj_add_style(title, LV_LABEL_PART_MAIN, &style_title);

  lv_obj_t * tx = lv_label_create(bar, NULL);
  lv_label_set_text(tx, "tx");
  lv_obj_align(tx, NULL, LV_ALIGN_IN_TOP_RIGHT, -30, LV_TRAFFIC_TITLE_PAD);
  lv_obj_add_style(tx, LV_LABEL_PART_MAIN, &style_tx);

  lv_obj_t * rx = lv_label_create(bar, NULL);
  lv_label_set_text(rx, "rx");
  lv_obj_align(rx, NULL, LV_ALIGN_IN_TOP_RIGHT, -70, LV_TRAFFIC_TITLE_PAD);
  lv_obj_add_style(rx, LV_LABEL_PART_MAIN, &style_rx);
  
  // Create a chart
  chart = lv_chart_create(lv_scr_act(), NULL);

  lv_obj_set_pos(chart, 0, LV_TRAFFIC_TITLE_HEIGHT);
  lv_obj_set_size(chart, LV_HOR_RES, LV_VER_RES - LV_TRAFFIC_TITLE_HEIGHT);
  lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
  lv_chart_set_range(chart, 0, 400);

  const char *ytick="400\n300\n200\n100\n0"; // 0 - 400 Mbps
  lv_chart_set_y_tick_texts(chart,ytick, 0, LV_CHART_AXIS_DRAW_LAST_TICK);
  lv_chart_set_y_tick_length(chart, LV_CHART_TICK_LENGTH_AUTO, LV_CHART_TICK_LENGTH_AUTO);

  lv_obj_set_style_local_pad_left(chart,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 60);
  lv_obj_set_style_local_pad_right(chart,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 10);

  lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);

  static lv_style_t styleStats;
  lv_style_set_text_color(&styleStats, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_add_style(chart, LV_CHART_PART_BG, &styleStats);

  /*Add two data series*/
  ser1 = lv_chart_add_series(chart, LV_COLOR_RED);
  ser2 = lv_chart_add_series(chart, LV_COLOR_GREEN);

  // Zero the points - probably not required.
  for(int i=0; i<10; i++) {
    ser1->points[i] = 0;
    ser2->points[i] = 0;
  }
  lv_chart_refresh(chart); /*Required after direct set*/

  // Get new data task
  lv_task_create(traffic_task, LV_TRAFFIC_POLL_RATE_MS, LV_TASK_PRIO_MID, NULL);
}
