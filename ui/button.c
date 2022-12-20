#include "common.h"
#include "font_manger.h"
#include "input_manger.h"
#include <config.h>
#include <disp_manger.h>
#include <page_manger.h>
#include <render.h>
#include <stdio.h>
#include <stdlib.h>
#include <ui.h>

/*
 * button default draw.
 * picture priority.
 * if you don't need a picture, set pic_name = NULL.
 */
int default_on_draw(button *btn, unsigned int color, char *pic_name,
                    video_mem *vd_mem) {
  disp_buff buff;
  disp_ops *dp_ops = get_display_ops_from_name(LCD_NAME);
  int ret;

  if (!dp_ops)
    goto err_get_display_ops_from_name;

  ret = get_display_buffer(dp_ops, &buff);
  if (ret)
    goto err_get_display_buffer;

  /*
   * draw color.
   */
  draw_region(btn->pic.rgn, color);

  /*
   * draw text.
   */
  set_font_size(btn->font_size);
  draw_text_in_region(btn->name, &btn->pic.rgn, BUTTON_TEXT_COLOR);

  /*
   * flush to lcd/web.
   */
  flush_display_region(&btn->pic.rgn, dp_ops, &buff);

  return 0;

err_get_display_buffer:
err_get_display_ops_from_name:
  return -1;
}

/*
 * set button pic.
 */
void setup_button_pic(button *btn, unsigned int x, unsigned int y,
                      unsigned int width, unsigned int height, char *str) {
  btn->pic.rgn.left_up_x = x;
  btn->pic.rgn.left_up_y = y;
  btn->pic.rgn.width = width;
  btn->pic.rgn.height = height;
  if (str)
    btn->pic.pic_name = str;
}

/*
 * default pressed function.
 */
int default_on_pressed(button *btn, input_event *ievt) {
  unsigned int color;
  disp_buff buff;
  disp_ops *dp_ops = get_display_ops_from_name(LCD_NAME);
  int ret;

  ret = get_display_buffer(dp_ops, &buff);
  if (ret)
    goto err_get_display_buffer;

  btn->status = !btn->status;
  if (btn->status)
    color = BUTTON_PRESSED_COLOR;
  else
    color = BUTTON_DEFAULT_COLOR;

  default_on_draw(btn, color, NULL, NULL);

  return 0;

err_get_display_buffer:
  return -1;
}

/*
 * get button rgn data.
 */
int get_button_rgn_data(button *btn, unsigned int *x, unsigned int *y,
                        unsigned int *width, unsigned int *height) {
  return get_rgn_data(&btn->pic.rgn, x, y, width, height);
}

/*
 * get rgn data.
 */
int get_rgn_data(region *rgn, unsigned int *x, unsigned int *y,
                 unsigned int *width, unsigned int *height) {
  if (x)
    *x = rgn->left_up_x;
  if (y)
    *y = rgn->left_up_y;
  if (width)
    *width = rgn->width;
  if (height)
    *height = rgn->height;
  return 0;
}

/*
 * init a button.
 */
int init_button(button *btn, char *name, pic_layout *pic, on_draw draw,
                on_pressed pred) {
  btn->name = name;
  btn->on_draw = draw ? draw : default_on_draw;
  btn->on_pressed = pred ? pred : default_on_pressed;
  if (pic)
    btn->pic = *pic;
  btn->status = 0;
  return 0;
}

/*
 * from input event get btn.
 */
button *from_input_event_get_btn(button *btn, input_event *ievt) {
  /*
   * detection fb input.
   */
  while (btn->pic.pic_name) {
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
 * from input event get btn.
 */
int from_input_event_get_btn_index(button *btn, input_event *ievt) {
  int i = 0;
  /*
   * detection fb input.
   */
  while (btn->pic.pic_name) {
    if (ievt->x >= btn->pic.rgn.left_up_x &&
        ievt->x <= btn->pic.rgn.left_up_x + btn->pic.rgn.width &&
        ievt->y >= btn->pic.rgn.left_up_y &&
        ievt->y <= btn->pic.rgn.left_up_y + btn->pic.rgn.height) {
      return i;
    }
    i++;
    btn++;
  }

  return -1;
}

/*
 * button jump page.
 */
int invert_jump_page_on_pressed(struct button *btn, input_event *ievt,
                                char *page_name, void *params,
                                int (*show)(void *), void *show_params) {
  if (ievt->pressure == INPUT_RELEASE) {
    release_button(btn);
    page(page_name)->run(params);
    show(show_params);
  } else
    press_button(btn);
  return 0;
}

/*
 * button func.
 */
int on_pressed_func(struct button *btn, input_event *ievt,
                    void (*func)(void *params), void *params) {
  if (ievt->pressure == INPUT_RELEASE) {
    release_button(btn);
    func(params);
  } else
    press_button(btn);
  return 0;
}

/*
 * display button.
 */
int show_button(button *btn, unsigned int color, char *pic_name,
                video_mem *vd_mem) {
  while (btn->pic.pic_name) {
    btn->on_draw(btn, color, pic_name, vd_mem);
    btn++;
  }

  return 0;
}

/*
 * clean button invert.
 */
void clean_button_invert(button *btn) {
  while (btn->pic.pic_name) {
    if (btn->status == BUTTON_PRESSED) {
      release_button(btn);
      btn->status = BUTTON_RELEASE;
    }
    btn++;
  }
}

/*
 * invert button.
 */
int invert_button(button *btn) {
  unsigned char *buff;
  int ret;
  unsigned int x, y;
  unsigned int btn_x, btn_y;
  unsigned int btn_width, btn_height;
  disp_buff dp_buff;
  disp_ops *dp_ops = get_display_ops_from_name(LCD_NAME);

  if (!dp_ops)
    goto err_get_display_ops_from_name;

  /*
   * get button data.
   */
  ret = get_display_buffer(dp_ops, &dp_buff);
  if (ret)
    goto err_get_display_buffer;

  get_button_rgn_data(btn, &btn_x, &btn_y, &btn_width, &btn_height);
  buff = dp_buff.buff;
  buff += btn_y * dp_buff.line_byte + btn_x * dp_buff.pixel_width;

  /*
   * invert buffer.
   */
  for (y = 0; y < btn_height; y++) {
    for (x = 0; x < btn_width * dp_buff.pixel_width; x++)
      buff[x] = ~buff[x];
    buff += dp_buff.line_byte;
  }

  return 0;

err_get_display_buffer:
err_get_display_ops_from_name:
  return -1;
}

/*
 * press button.
 */
int press_button(button *btn) {
  if (btn->status != BUTTON_PRESSED) {
    btn->status = BUTTON_PRESSED;
    invert_button(btn);
  }
  return 0;
}

/*
 * release button.
 */
int release_button(button *btn) {
  if (btn->status != BUTTON_RELEASE) {
    btn->status = BUTTON_RELEASE;
    invert_button(btn);
  }
  return 0;
}

/*
 * main page on draw.
 * priority picture.
 */
int icon_on_draw(struct button *btn, unsigned int color, char *pic_name,
                 video_mem *vd_mem) {
  disp_buff origin_icon_buff;
  disp_buff icon_buff;
  disp_buff *dp_buff;
  char icon_name[128];
  char *name;
  unsigned int icon_width, icon_height;
  unsigned int icon_x, icon_y;
  int ret;

  /*
   * get display buffer.
   */
  dp_buff = &vd_mem->disp_buff;

  /*
   * set buff params.
   */
  name = pic_name != NULL ? pic_name : btn->pic.pic_name;
  snprintf(icon_name, 128, "%s/%s", ICON_PATH, name);
  icon_name[127] = '\0';

  /*
   * get icon display buffer.
   */
  set_disp_buff_bpp(&origin_icon_buff, dp_buff->bpp);
  ret = get_disp_buff_for_icon(&origin_icon_buff, icon_name);
  if (ret)
    goto err_get_disp_buff_for_icon;

  /*
   * zoom picture.
   */
  get_button_rgn_data(btn, &icon_x, &icon_y, &icon_width, &icon_height);
  setup_disp_buff(&icon_buff, icon_width, icon_height, dp_buff->bpp, NULL);
  icon_buff.buff = malloc(icon_buff.total_size);
  if (!icon_buff.buff)
    goto err_malloc;

  ret = pic_zoom(&origin_icon_buff, &icon_buff);
  if (ret)
    goto err_pic_zoom;

  /*
   * merge picture.
   */
  ret = pic_merge(icon_x, icon_y, &icon_buff, dp_buff);
  if (ret)
    goto err_pic_merge;

  return 0;

err_pic_merge:
err_pic_zoom:
  free_disp_buff_for_icon(&icon_buff);
err_malloc:
  free_disp_buff_for_icon(&origin_icon_buff);
err_get_disp_buff_for_icon:
  return -1;
}

/*
 * set pic name.
 */
int set_pic_disp_pic(pic_layout *pic, char *pic_name) {
  pic->pic_name = pic_name;
  return 0;
}

/*
 * get button from index.
 */
button *get_button_from_index(button *btn, int index) { return &btn[index]; }