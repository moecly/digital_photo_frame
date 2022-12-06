#include "font_manger.h"
#include "input_manger.h"
#include <disp_manger.h>
#include <render.h>
#include <stdio.h>
#include <ui.h>

/*
 * button default draw.
 */
void default_on_draw(button *btn, disp_ops *dp_ops, unsigned int color) {
  disp_buff buff;
  int ret;

  ret = get_display_buffer(dp_ops, &buff);
  if (ret)
    return;

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
}

/*
 * default pressed function.
 */
void default_on_pressed(button *btn, disp_ops *dp_ops, input_event *ievt) {
  unsigned int color;
  disp_buff buff;
  int ret;

  ret = get_display_buffer(dp_ops, &buff);
  if (ret)
    return;

  btn->status = !btn->status;
  if (btn->status)
    color = BUTTON_PRESSED_COLOR;
  else
    color = BUTTON_DEFAULT_COLOR;

  default_on_draw(btn, dp_ops, color);
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
