#include "page_manger.h"
#include <config.h>
#include <disp_manger.h>
#include <render.h>
#include <stdio.h>
#include <ui.h>

static int show_setting_page(button *btn);
static button btn_layout[];
static int is_exit = 0;

static page_params page_pms;

static void set_exit(int val) { is_exit = val; }

static int select_fold_on_pressed(struct button *btn, input_event *ievt) {
  return invert_jump_page_on_pressed(btn, ievt, "browse", &page_pms,
                                     (void *)show_setting_page, btn_layout);
}

static int interval_on_pressed(struct button *btn, input_event *ievt) {
  return invert_jump_page_on_pressed(btn, ievt, "interval", &page_pms,
                                     (void *)show_setting_page, btn_layout);
}

static int return_on_pressed(struct button *btn, input_event *ievt) {
  return on_pressed_func(btn, ievt, (void *)set_exit, (void *)1);
}

static button btn_layout[] = {
    {
        .pic =
            {
                .pic_name = "select_fold.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = select_fold_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = "interval.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = interval_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = "return.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = return_on_pressed,
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
 * calc setting page layout.
 */
static void calc_setting_page_layout(page_layout *ptPageLayout) {
  int iStartY;
  int iWidth;
  int iHeight;
  int iXres, iYres, iBpp;
  int iTmpTotalBytes;
  button *atLayout;
  disp_ops *dp_ops = get_display_ops_from_name(LCD_NAME);
  disp_buff dp_buff;

  atLayout = ptPageLayout->atLayout;
  get_display_buffer(dp_ops, &dp_buff);
  iBpp = dp_buff.bpp;
  iXres = dp_buff.xres;
  iYres = dp_buff.yres;
  ptPageLayout->iBpp = iBpp;

  /*
   *    ----------------------
   *                           1/2 * iHeight
   *          select_fold.bmp  iHeight
   *                           1/2 * iHeight
   *          interval.bmp     iHeight
   *                           1/2 * iHeight
   *          return.bmp       iHeight
   *                           1/2 * iHeight
   *    ----------------------
   */

  iHeight = iYres * 2 / 10;
  iWidth = iHeight;
  iStartY = iHeight / 2;

  /* select_fold图标 */
  atLayout[0].pic.rgn.left_up_y = iStartY;
  atLayout[0].pic.rgn.height = iHeight - 1;
  atLayout[0].pic.rgn.left_up_x = (iXres - iWidth * 2) / 2;
  atLayout[0].pic.rgn.width = iWidth * 2 - 1;

  iTmpTotalBytes = (atLayout[0].pic.rgn.left_up_x + atLayout[0].pic.rgn.width -
                    atLayout[0].pic.rgn.left_up_x + 1) *
                   (atLayout[0].pic.rgn.left_up_y + atLayout[0].pic.rgn.height -
                    atLayout[0].pic.rgn.left_up_y + 1) *
                   iBpp / 8;
  if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes) {
    ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
  }

  /* interval图标 */
  atLayout[1].pic.rgn.left_up_y = atLayout[0].pic.rgn.left_up_y +
                                  atLayout[0].pic.rgn.height + iHeight / 2 + 1;
  atLayout[1].pic.rgn.height = iHeight - 1;
  atLayout[1].pic.rgn.left_up_x = (iXres - iWidth * 2) / 2;
  atLayout[1].pic.rgn.width = iWidth * 2 - 1;

  iTmpTotalBytes = (atLayout[1].pic.rgn.left_up_x + atLayout[0].pic.rgn.width -
                    atLayout[1].pic.rgn.left_up_x + 1) *
                   (atLayout[1].pic.rgn.left_up_y + atLayout[0].pic.rgn.height -
                    atLayout[1].pic.rgn.left_up_y + 1) *
                   iBpp / 8;
  if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes) {
    ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
  }

  /* return图标 */
  atLayout[2].pic.rgn.left_up_y = atLayout[1].pic.rgn.left_up_y +
                                  atLayout[0].pic.rgn.height + iHeight / 2 + 1;
  atLayout[2].pic.rgn.height = iHeight - 1;
  atLayout[2].pic.rgn.left_up_x = (iXres - iWidth) / 2;
  atLayout[2].pic.rgn.width = iWidth - 1;

  iTmpTotalBytes = (atLayout[2].pic.rgn.left_up_x + atLayout[2].pic.rgn.width -
                    atLayout[2].pic.rgn.left_up_x + 1) *
                   (atLayout[2].pic.rgn.left_up_y + atLayout[2].pic.rgn.height -
                    atLayout[2].pic.rgn.left_up_y + 1) *
                   iBpp / 8;
  if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes) {
    ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
  }
}

/*
 * show setting page.
 */
static int show_setting_page(button *btn) {
  video_mem *vd_mem;

  /*
   * get video mem.
   */
  vd_mem = get_video_mem(ID("setting"), 1);
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
      calc_setting_page_layout(&g_tSettingPageLayout);

    /*
     * display menu.
     */
    show_button(btn, 0, NULL, vd_mem);

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
 * setting page.
 */
int setting_page_run(void *params) {
  input_event ievt;
  int ret;
  button *btn;

  page_pms.page_id = ID("setting");
  set_exit(0);
  /*
   * create prepare thread.
   */

  /*
   * display interface.
   */
  show_setting_page(btn_layout);

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
static int setting_page_prepare(void) { return 0; }

static page_action setting_page_atn = {
    .name = "setting",
    .run = setting_page_run,
    .prepare = setting_page_prepare,
};

/*
 * register main page.
 */
void setting_page_register(void) { register_page(&setting_page_atn); }
