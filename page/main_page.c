#include "common.h"
#include "font_manger.h"
#include "input_manger.h"
#include "render.h"
#include "sys/time.h"
#include <config.h>
#include <disp_manger.h>
#include <math.h>
#include <page_manger.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ui.h>

static int show_main_page(button *btn);
static int main_page_on_draw(struct button *btn, unsigned int color,
                             char *pic_name);
static int browse_mode_on_pressed(struct button *btn, input_event *ievt);
static int continue_mode_on_pressed(struct button *btn, input_event *ievt);
static int setting_on_pressed(struct button *btn, input_event *ievt);

/*
 * button config.
 */
static button btn_layout[] = {
    {
        .pic =
            {
                .pic_name = "browse_mode.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = main_page_on_draw,
        .on_pressed = browse_mode_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = "continue_mod.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = main_page_on_draw,
        .on_pressed = continue_mode_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = "setting.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = main_page_on_draw,
        .on_pressed = setting_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = NULL,
            },
    },
};

/*
 * main page on draw.
 */
static int main_page_on_draw(struct button *btn, unsigned int color,
                             char *pic_name) {
  disp_buff origin_icon_buff;
  disp_buff icon_buff;
  disp_ops *dp_ops = get_display_ops_from_name(LCD_NAME);
  disp_buff dp_buff;
  char icon_name[128];
  char *name;
  unsigned int icon_width, icon_height;
  unsigned int icon_x, icon_y;
  int ret;

  if (!dp_ops)
    goto err_get_display_ops_from_name;

  /*
   * get display buffer.
   */
  ret = get_display_buffer(dp_ops, &dp_buff);
  if (ret)
    goto err_get_display_buffer;

  /*
   * set buff params.
   */
  name = pic_name != NULL ? pic_name : btn->pic.pic_name;
  snprintf(icon_name, 128, "%s/%s", ICON_PATH, name);
  icon_name[127] = '\0';

  /*
   * get icon display buffer.
   */
  set_disp_buff_bpp(&origin_icon_buff, dp_buff.bpp);
  ret = get_disp_buff_for_icon(&origin_icon_buff, icon_name);
  if (ret)
    goto err_get_disp_buff_for_icon;

  /*
   * zoom picture.
   */
  get_button_rgn_data(btn, &icon_x, &icon_y, &icon_width, &icon_height);
  setup_disp_buff(&icon_buff, icon_width, icon_height, dp_buff.bpp, NULL);
  icon_buff.buff = malloc(icon_buff.total_size);
  if (!icon_buff.buff)
    goto err_malloc;

  ret = pic_zoom(&origin_icon_buff, &icon_buff);
  if (ret)
    goto err_pic_zoom;

  /*
   * merge picture.
   */
  ret = pic_merge(icon_x, icon_y, &icon_buff, &dp_buff);
  if (ret)
    goto err_pic_merge;

  return 0;

err_pic_merge:
err_pic_zoom:
  free_disp_buff_for_icon(&icon_buff);
err_malloc:
  free_disp_buff_for_icon(&origin_icon_buff);
err_get_disp_buff_for_icon:
err_get_display_buffer:
err_get_display_ops_from_name:
  return -1;
}

static int browse_mode_on_pressed(struct button *btn, input_event *ievt) {
  if (ievt == INPUT_RELEASE)
    press_button(btn);
  else
    release_button(btn);
  // page("browse")->run(NULL);
  // show_main_page(btn_layout);
  return 0;
}

static int continue_mode_on_pressed(struct button *btn, input_event *ievt) {
  page("auto")->run(NULL);
  show_main_page(btn_layout);
  return 0;
}

static int setting_on_pressed(struct button *btn, input_event *ievt) {
  page("setting")->run(NULL);
  show_main_page(btn_layout);
  return 0;
}

/*
 * from input event get btn.
 */
static button *from_input_event_get_btn(button *btn, input_event *ievt) {
  /*
   * detection fb input.
   */
  while (btn) {
    if (ievt->x >= btn->pic.rgn.left_up_x &&
        ievt->x <= btn->pic.rgn.left_up_x + btn->pic.rgn.width &&
        ievt->y >= btn->pic.rgn.left_up_y &&
        ievt->y <= btn->pic.rgn.left_up_y + btn->pic.rgn.height) {
      return btn;
    }

    btn++;
  }

  return NULL;
}

/*
 * get main page input event.
 */
static int main_page_get_input_event(button *btn, input_event *ievt) {
  int ret;

  /*
   * filter event.
   */
  ret = get_input_event(ievt);

  if (ret)
    goto err_get_input_event;

  if (ievt->type != INPUT_TYPE_TOUCH)
    goto err_input_event;

  return 0;

err_input_event:
err_get_input_event:
  return -1;
}

/*
 * show main page.
 */
static int show_main_page(button *btn) {
  int ret;
  video_mem *vd_mem;
  disp_ops *dp_ops;
  disp_buff dp_buff;
  int icon_width, icon_height;
  int icon_x, icon_y;

  vd_mem = get_video_mem(ID("main"), 1);
  if (!vd_mem) {
    printf("get video mem err\n");
    goto err_get_video_mem;
  }

  /*
   * draw.
   */
  if (vd_mem->pic_status != PS_GENERATED) {
    /*
     * only lcd.
     */
    dp_ops = get_display_ops_from_name(LCD_NAME);
    if (!dp_ops)
      goto err_get_display_ops_from_name;

    ret = get_display_buffer(dp_ops, &dp_buff);
    if (ret)
      goto err_get_disp_buff;

    icon_height = dp_buff.yres * 2 / 10;
    icon_width = icon_height * 2;
    icon_x = (dp_buff.xres - icon_width) / 2;
    icon_y = dp_buff.yres / 10;

    /*
     * display each button.
     */
    while (btn->pic.pic_name) {
      setup_button_pic(btn, icon_x, icon_y, icon_width, icon_height, NULL);
      btn->on_draw(btn, 0, NULL);
      btn++;
      icon_y += dp_buff.yres * 3 / 10;
    }

    vd_mem->pic_status = PS_GENERATED;
  }

  /*
   * flush.
   */
  flush_video_mem_to_dev(vd_mem);

  /*
   * free video mem.
   */
  put_video_mem(vd_mem);

  return 0;

err_get_disp_buff:
err_get_display_ops_from_name:
err_get_video_mem:
  printf("err\n");
  return -1;
}

/*
 * main page.
 */
int main_page_run(void *params) {
  input_event ievt;
  int ret;
  button *btn;

  /*
   * display interface.
   */
  show_main_page(btn_layout);

  /*
   * create prepare thread.
   */

  /*
   * input processing.
   */
  while (1) {
    ret = main_page_get_input_event(btn_layout, &ievt);
    if (ret)
      continue;

    /*
     * if input release.
     */
    if (ievt.pressure == 0) {
      btn = btn_layout;
      while (btn->pic.pic_name) {
        if (btn->status == BUTTON_PRESSED) {
          btn->status = BUTTON_RELEASE;
          release_button(btn);
        }
        btn++;
      }
      continue;
    }

    /*
     * execute button function.
     */
    btn = from_input_event_get_btn(btn_layout, &ievt);
    if (!btn)
      continue;

    btn->on_pressed(NULL, NULL);
  }

  return 0;
}

/*
 * pre pare.
 */
static int main_page_prepare(void) { return 0; }

static page_action main_page_atn = {
    .name = "main",
    .run = main_page_run,
    .prepare = main_page_prepare,
};

/*
 * register main page.
 */
void main_page_register(void) { register_page(&main_page_atn); }
