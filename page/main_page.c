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

static button btn_layout[] = {
    {
        .pic =
            {
                .pic_name = "browse_mode.bmp",
                .rgn = {0, 0, 0, 0},
            },
    },
    {
        .pic =
            {
                .pic_name = "continue_mode.bmp",
                .rgn = {0, 0, 0, 0},
            },
    },
    {
        .pic =
            {
                .pic_name = "setting.bmp",
                .rgn = {0, 0, 0, 0},
            },
    },
};

static int show_main_page(button *btn) {
  int ret;
  video_mem *vd_mem;
  disp_ops *dp_ops_que = get_disp_queue();
  disp_ops *dp_ops;
  disp_buff dp_buff;
  disp_buff origin_icon_buff;
  disp_buff icon_buff;
  int icon_width, icon_height;
  int icon_x, icon_y;
  char icon_name[128];

  vd_mem = get_video_mem(ID("main"), 1);
  if (!vd_mem) {
    printf("get video mem err\n");
    goto err_get_video_mem;
  }

  /*
   * draw.
   */
  if (vd_mem->pic_status != PS_GENERATED) {
    for_each_node(dp_ops_que, dp_ops) {
      ret = get_display_buffer(dp_ops, &dp_buff);
      if (ret)
        goto err_get_disp_buff;

      icon_height = dp_buff.yres * 2 / 10;
      icon_width = icon_height * 2;
      icon_x = (dp_buff.xres - icon_width) / 2;
      icon_y = dp_buff.yres / 10;

      /*
       * set icon params.
       */
      setup_disp_buff(&icon_buff, icon_width, icon_height, dp_buff.bpp, NULL);
      icon_buff.buff = malloc(sizeof(icon_buff.total_size));
      if (!icon_buff.buff)
        goto err_malloc;

      /*
       * display each button.
       */
      while (btn) {
        btn->pic.rgn.left_up_x = icon_x;
        btn->pic.rgn.left_up_y = icon_y;
        btn->pic.rgn.width = icon_width;
        btn->pic.rgn.height = icon_height;
        snprintf(icon_name, 128, "%s/%s", ICON_PATH, btn->pic.pic_name);

        set_disp_buff_bpp(&origin_icon_buff, dp_buff.bpp);
        ret = get_disp_buff_for_icon(&origin_icon_buff, icon_name);
        if (ret)
          goto err_get_disp_buff_for_icon;

        ret = pic_zoom(&origin_icon_buff, &icon_buff);
        if (ret)
          goto err_pic_zoom;

        ret = pic_merge(icon_x, icon_y, &icon_buff, &dp_buff);
        if (ret)
          goto err_pic_merge;

        btn++;
        icon_y += dp_buff.yres * 3 / 10;
      }

      /*
       * free buff.
       */
      free_disp_buff_for_icon(&icon_buff);
      free_disp_buff_for_icon(&origin_icon_buff);
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

err_pic_merge:
err_pic_zoom:
  free_disp_buff_for_icon(&origin_icon_buff);
err_get_disp_buff_for_icon:
  free_disp_buff_for_icon(&icon_buff);
err_malloc:
err_get_disp_buff:
err_get_video_mem:
  return -1;
}

/*
 * main page.
 */
int main_page_run(void *params) {
  input_event ievt;
  int ret;

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
    ret = get_input_event(&ievt);
    if (ret)
      continue;

    /*
     * save, start, restore page.
     */
    switch (ievt) {

    case "browse":
      store_page();
      page("browse")->run(NULL);
      restore_page();
      break;

    case "auto":
      store_page();
      page("auto")->run(NULL);
      restore_page();
      break;

    case "setting":
      store_page();
      page("setting")->run(NULL);
      restore_page();
      break;

    default:
      break;
    }
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
