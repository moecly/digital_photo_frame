#ifndef _UI_H
#define _UI_H

#include "disp_manger.h"
#include "input_manger.h"

#define WHITE 0xFFFFFF
#define BUTTON_DEFAULT_COLOR 0xFF0000
#define BUTTON_PRESSED_COLOR 0x00FF00
#define BUTTON_PERCENT_COLOR 0x0000FF
#define BUTTON_TEXT_COLOR 0x000000

#define BUTTON_PRESSED 1
#define BUTTON_RELEASE 0

typedef struct pic_layout {
  region rgn;
  char *pic_name;
} pic_layout;

typedef struct button {
  char *name;
  char status;
  int font_size;
  pic_layout pic;
  int (*on_draw)(struct button *btn, unsigned int color, char *pic_name);
  int (*on_pressed)(struct button *btn, input_event *ievt);
} button;

typedef int (*on_draw)(button *btn, unsigned int color, char *pic_name);
typedef int (*on_pressed)(button *btn, input_event *ievt);

int default_on_draw(button *btn, unsigned int color, char *pic_name);
int default_on_pressed(button *btn, input_event *ievt);
void setup_button_pic(button *btn, unsigned int x, unsigned int y,
                      unsigned int width, unsigned int height, char *str);
int init_button(button *btn, char *name, pic_layout *pic, on_draw draw,
                on_pressed pred);
int get_button_rgn_data(button *btn, unsigned int *x, unsigned int *y,
                        unsigned int *width, unsigned int *height);

#if 0
int init_button(button *btn, char *name, region *rgn, on_draw draw,
                on_pressed pred);

#endif

#endif