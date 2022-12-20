#include "common.h"
#include "font_manger.h"
#include "page_manger.h"
#include <config.h>
#include <disp_manger.h>
#include <render.h>
#include <stdio.h>
#include <ui.h>

#define MAX_SECOND 60
#define DEC_SECOND(x) (x == 0 ? MAX_SECOND - 1 : x)
#define INC_SECOND(x) (x == MAX_SECOND ? 1 : x)

static int generate_interval_page_special_icon(int dw_number,
                                               video_mem *vd_mem);
static int show_interval_page(button *btn);

static int is_exit = 0;

static page_params page_pms;
static button g_tIntervalNumberLayout;
static int interval_second = 1;
static int display_interval_second = 1;

static void set_exit(int val) { is_exit = val; }

static int inc_func(int *second) {
  *second -= 1;
  *second = DEC_SECOND(*second);
  return 0;
}

static int dec_func(int *second) {
  *second += 1;
  *second = INC_SECOND(*second);
  return 0;
}

static int inc_on_pressed(struct button *btn, input_event *ievt) {
  video_mem *vd_mem;

  if (ievt->pressure) {
    vd_mem = get_dev_video_mem();
    inc_func(&display_interval_second);
    generate_interval_page_special_icon(display_interval_second, vd_mem);
    put_video_mem(vd_mem);
  }

  return 0;
}

static int time_on_pressed(struct button *btn, input_event *ievt) { return 0; }

static int dec_on_pressed(struct button *btn, input_event *ievt) {
  video_mem *vd_mem;

  if (ievt->pressure) {
    vd_mem = get_dev_video_mem();
    dec_func(&display_interval_second);
    generate_interval_page_special_icon(display_interval_second, vd_mem);
    put_video_mem(vd_mem);
  }

  return 0;
}

static int cancel_on_pressed(struct button *btn, input_event *ievt) {
  return on_pressed_func(btn, ievt, (void *)set_exit, (void *)1);
}

static int ok_on_pressed(struct button *btn, input_event *ievt) {
  interval_second = display_interval_second;
  return cancel_on_pressed(btn, ievt);
}

static button btn_layout[] = {
    {
        .pic =
            {
                .pic_name = "inc.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = inc_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = "time.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = time_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = "dec.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = dec_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = "ok.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = ok_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = "cancel.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = cancel_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = NULL,
            },
    },
};

static page_layout g_tSettingPageLayout = {
    .iMaxTotalBytes = 0,
    .atLayout = btn_layout,
};

/*
 * get interval second.
 */
int get_interval_second(void) { return interval_second; }

/*
 * generate interval page special icon.
 */
static int generate_interval_page_special_icon(int dw_number,
                                               video_mem *vd_mem) {
  unsigned int font_size;
  char str_num[3];
  region *rgn = &g_tIntervalNumberLayout.pic.rgn;

  if (dw_number > 59)
    goto err_dw_number;

  /*
   * set font size.
   */
  font_size = rgn->height;
  set_font_size(font_size);

  /*
   * refresh number to video mem.
   */
  snprintf(str_num, 3, "%02d", dw_number);
  merger_string_to_center_of_rectangle_in_video_mem(
      rgn->left_up_x, rgn->left_up_y, rgn->left_up_x + rgn->width,
      rgn->left_up_y + rgn->height, str_num, vd_mem);

  return 0;

err_dw_number:
  return -1;
}

static void calc_interval_page_layout(page_layout *ptPageLayout) {
  int iStartY;
  int iWidth;
  int iHeight;
  int iXres, iYres, iBpp;
  int iTmpTotalBytes;
  button *atLayout;
  disp_ops *dp_ops = get_display_ops_from_name(LCD_NAME);
  disp_buff dp_buff;

  get_display_buffer(dp_ops, &dp_buff);
  iBpp = dp_buff.bpp;
  iXres = dp_buff.xres;
  iYres = dp_buff.yres;
  atLayout = ptPageLayout->atLayout;
  ptPageLayout->iBpp = iBpp;

  /*
   *    ----------------------
   *                          1/2 * iHeight
   *          inc.bmp         iHeight * 28 / 128
   *         time.bmp         iHeight * 72 / 128
   *          dec.bmp         iHeight * 28 / 128
   *                          1/2 * iHeight
   *    ok.bmp     cancel.bmp 1/2 * iHeight
   *                          1/2 * iHeight
   *    ----------------------
   */
  iHeight = iYres / 3;
  iWidth = iHeight;
  iStartY = iHeight / 2;

  /* inc图标 */
  atLayout[0].pic.rgn.left_up_y = iStartY;
  atLayout[0].pic.rgn.height = iHeight * 28 / 128 - 1;
  atLayout[0].pic.rgn.left_up_x = (iXres - iWidth * 52 / 128) / 2;
  atLayout[0].pic.rgn.width = iWidth * 52 / 128 - 1;

  iTmpTotalBytes = (atLayout[0].pic.rgn.left_up_x + atLayout[0].pic.rgn.width -
                    atLayout[0].pic.rgn.left_up_x + 1) *
                   (atLayout[0].pic.rgn.left_up_y + atLayout[0].pic.rgn.height -
                    atLayout[0].pic.rgn.left_up_y + 1) *
                   iBpp / 8;
  if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes) {
    ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
  }

  /* time图标 */
  atLayout[1].pic.rgn.left_up_y =
      atLayout[0].pic.rgn.left_up_y + atLayout[0].pic.rgn.height + 1;
  atLayout[1].pic.rgn.height = iHeight * 72 / 128 - 1;
  atLayout[1].pic.rgn.left_up_x = (iXres - iWidth) / 2;
  atLayout[1].pic.rgn.width = iWidth - 1;
  iTmpTotalBytes = (atLayout[1].pic.rgn.left_up_x + atLayout[1].pic.rgn.width -
                    atLayout[1].pic.rgn.left_up_x + 1) *
                   (atLayout[1].pic.rgn.left_up_y + atLayout[1].pic.rgn.height -
                    atLayout[1].pic.rgn.left_up_y + 1) *
                   iBpp / 8;
  if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes) {
    ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
  }

  /* dec图标 */
  atLayout[2].pic.rgn.left_up_y =
      atLayout[1].pic.rgn.left_up_y + atLayout[1].pic.rgn.height + 1;
  atLayout[2].pic.rgn.height = iHeight * 28 / 128 - 1;
  atLayout[2].pic.rgn.left_up_x = (iXres - iWidth * 52 / 128) / 2;
  atLayout[2].pic.rgn.width = iWidth * 52 / 128 - 1;
  iTmpTotalBytes = (atLayout[2].pic.rgn.left_up_x + atLayout[2].pic.rgn.width -
                    atLayout[2].pic.rgn.left_up_x + 1) *
                   (atLayout[2].pic.rgn.left_up_y + atLayout[2].pic.rgn.height -
                    atLayout[2].pic.rgn.left_up_y + 1) *
                   iBpp / 8;
  if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes) {
    ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
  }

  /* ok图标 */
  atLayout[3].pic.rgn.left_up_y = atLayout[2].pic.rgn.left_up_y +
                                  atLayout[2].pic.rgn.height + iHeight / 2 + 1;
  atLayout[3].pic.rgn.height = iHeight / 2 - 1;
  atLayout[3].pic.rgn.left_up_x = (iXres - iWidth) / 3;
  atLayout[3].pic.rgn.width = iWidth / 2 - 1;
  iTmpTotalBytes = (atLayout[3].pic.rgn.left_up_x + atLayout[3].pic.rgn.width -
                    atLayout[3].pic.rgn.left_up_x + 1) *
                   (atLayout[3].pic.rgn.left_up_y + atLayout[3].pic.rgn.height -
                    atLayout[3].pic.rgn.left_up_y + 1) *
                   iBpp / 8;
  if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes) {
    ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
  }

  /* ok图标 */
  atLayout[4].pic.rgn.left_up_y = atLayout[3].pic.rgn.left_up_y;
  atLayout[4].pic.rgn.height = atLayout[3].pic.rgn.height;
  atLayout[4].pic.rgn.left_up_x =
      atLayout[3].pic.rgn.left_up_x * 2 + iWidth / 2;
  atLayout[4].pic.rgn.width = iWidth / 2 - 1;
  iTmpTotalBytes = (atLayout[4].pic.rgn.left_up_x + atLayout[4].pic.rgn.width -
                    atLayout[4].pic.rgn.left_up_x + 1) *
                   (atLayout[4].pic.rgn.left_up_y + atLayout[4].pic.rgn.height -
                    atLayout[4].pic.rgn.left_up_y + 1) *
                   iBpp / 8;
  if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes) {
    ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
  }

  /* 用来显示数字的区域比较特殊, 单独处理
   * time.bmp原图大小为128x72, 里面的两个数字大小为52x40
   * 经过CalcIntervalPageLayout后有所缩放
   */
  iWidth = atLayout[1].pic.rgn.left_up_x + atLayout[1].pic.rgn.width -
           atLayout[1].pic.rgn.left_up_x + 1;
  iHeight = atLayout[1].pic.rgn.left_up_y + atLayout[1].pic.rgn.height -
            atLayout[1].pic.rgn.left_up_y + 1;

  g_tIntervalNumberLayout.pic.rgn.left_up_x =
      atLayout[1].pic.rgn.left_up_x + (128 - 52) / 2 * iWidth / 128;
  g_tIntervalNumberLayout.pic.rgn.width =
      atLayout[1].pic.rgn.left_up_x + atLayout[1].pic.rgn.width -
      (128 - 52) / 2 * iWidth / 128 + 1 -
      g_tIntervalNumberLayout.pic.rgn.left_up_x;

  g_tIntervalNumberLayout.pic.rgn.left_up_y =
      atLayout[1].pic.rgn.left_up_y + (72 - 40) / 2 * iHeight / 72;
  g_tIntervalNumberLayout.pic.rgn.height =
      atLayout[1].pic.rgn.left_up_y + atLayout[1].pic.rgn.height -
      (72 - 40) / 2 * iHeight / 72 + 1 -
      g_tIntervalNumberLayout.pic.rgn.left_up_y;
}

/*
 * show interval page.
 */
static int show_interval_page(button *btn) {
  video_mem *vd_mem;

  /*
   * get video mem.
   */
  vd_mem = get_video_mem(ID("interval"), 1);
  if (!vd_mem)
    goto err_get_video_mem;

  /*
   * draw.
   */
  if (vd_mem->pic_status != PS_GENERATED) {
    /*
     * display each button.
     */
    clean_screen_from_vd(BACKGROUND, vd_mem);
    if (btn[0].pic.rgn.left_up_x == 0)
      calc_interval_page_layout(&g_tSettingPageLayout);

    /*
     * display menu.
     */
    show_button(btn, 0, NULL, vd_mem);
    generate_interval_page_special_icon(interval_second, vd_mem);

    /*
     * display file and dir.
     */
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

err_get_video_mem:
  return -1;
}

/*
 * interval page.
 */
int interval_page_run(void *params) {
  input_event ievt;
  int ret;
  button *btn;

  page_pms.page_id = ID("interval");
  display_interval_second = interval_second;
  set_exit(0);
  /*
   * create prepare thread.
   */

  /*
   * display interface.
   */
  show_interval_page(btn_layout);

  /*
   * input processing.
   */
  for (;;) {
    if (is_exit)
      goto done;

    ret = lcd_page_get_input_event(&ievt);
    if (ret)
      continue;

    /*
     * execute button function.
     */
    btn = from_input_event_get_btn(btn_layout, &ievt);

    /*
     * if isn't menu button.
     */
    if (!btn) {
      clean_button_invert(btn_layout);
      continue;
    }
    btn->on_pressed(btn, &ievt);
  }

done:
  return 0;
}

/*
 * pre pare.
 */
static int interval_page_prepare(void) { return 0; }

static page_action interval_page_atn = {
    .name = "interval",
    .run = interval_page_run,
    .prepare = interval_page_prepare,
};

/*
 * register main page.
 */
void interval_page_register(void) { register_page(&interval_page_atn); }
