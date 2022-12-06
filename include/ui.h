#ifndef _UI_H
#define _UI_H

#include "disp_manger.h"
#include "input_manger.h"

#define WHITE 0xFFFFFF
#define BUTTON_DEFAULT_COLOR 0xFF0000
#define BUTTON_PRESSED_COLOR 0x00FF00
#define BUTTON_PERCENT_COLOR 0x0000FF
#define BUTTON_TEXT_COLOR 0x000000

typedef struct pic_layout {
  region rgn;
  char *pic_name;
} pic_layout;

typedef struct button {
  char *name;
  char status;
  int font_size;
  pic_layout pic;
  void (*on_draw)(struct button *btn, disp_ops *dp_ops, unsigned int color);
  void (*on_pressed)(struct button *btn, disp_ops *dp_ops, input_event *ievt);
} button;

typedef void (*on_draw)(button *btn, disp_ops *dp_ops, unsigned int color);
typedef void (*on_pressed)(button *btn, disp_ops *dp_ops, input_event *ievt);

void default_on_draw(button *btn, disp_ops *dp_ops, unsigned int color);
void default_on_pressed(button *btn, disp_ops *dp_ops, input_event *ievt);
int init_button(button *btn, char *name, pic_layout *pic, on_draw draw,
                on_pressed pred);

#if 0
int init_button(button *btn, char *name, region *rgn, on_draw draw,
                on_pressed pred);

#endif

#endif