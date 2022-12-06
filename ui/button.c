#include "font_manger.h"
#include "input_manger.h"
#include <config.h>
#include <disp_manger.h>
#include <render.h>
#include <stdio.h>
#include <ui.h>

/*
 * button default draw.
 * picture priority.
 * if you don't need a picture, set pic_name = NULL.
 */
int default_on_draw(button *btn, unsigned int color, char *pic_name) {
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

  default_on_draw(btn, color, NULL);

  return 0;

err_get_display_buffer:
  return -1;
}

/*
 * get button rgn data.
 */
int get_button_rgn_data(button *btn, unsigned int *x, unsigned int *y,
                        unsigned int *width, unsigned int *height) {
  if (x)
    *x = btn->pic.rgn.left_up_x;
  if (y)
    *y = btn->pic.rgn.left_up_y;
  if (width)
    *width = btn->pic.rgn.width;
  if (height)
    *height = btn->pic.rgn.height;
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
