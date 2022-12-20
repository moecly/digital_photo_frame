#ifndef _UI_H
#define _UI_H

#include <disp_manger.h>
#include <input_manger.h>

#define WHITE 0xFFFFFF
#define BACKGROUND WHITE
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
  int (*on_draw)(struct button *btn, unsigned int color, char *pic_name,
                 video_mem *vd_mem);
  int (*on_pressed)(struct button *btn, input_event *ievt);
} button;

typedef int (*on_draw)(button *btn, unsigned int color, char *pic_name,
                       video_mem *vd_mem);
typedef int (*on_pressed)(button *btn, input_event *ievt);

int default_on_draw(button *btn, unsigned int color, char *pic_name,
                    video_mem *vd_mem);
int default_on_pressed(button *btn, input_event *ievt);
void setup_button_pic(button *btn, unsigned int x, unsigned int y,
                      unsigned int width, unsigned int height, char *str);
int init_button(button *btn, char *name, pic_layout *pic, on_draw draw,
                on_pressed pred);
int get_button_rgn_data(button *btn, unsigned int *x, unsigned int *y,
                        unsigned int *width, unsigned int *height);
int invert_button(button *btn);
void clean_button_invert(button *btn);
int release_button(button *btn);
int set_pic_disp_pic(pic_layout *pic, char *pic_name);
int press_button(button *btn);
int draw_button_pic_from_vd_mem(button *btn, char *pic_name, video_mem *vd_mem);
button *from_input_event_get_btn(button *btn, input_event *ievt);
int invert_jump_page_on_pressed(struct button *btn, input_event *ievt,
                                char *page_name, void *params,
                                int (*show)(void *), void *show_params);
int on_pressed_func(struct button *btn, input_event *ievt, void (*func)(void *),
                    void *params);
int get_rgn_data(region *rgn, unsigned int *x, unsigned int *y,
                 unsigned int *width, unsigned int *height);
int show_button(button *btn, unsigned int color, char *pic_name,
                video_mem *vd_mem);
int icon_on_draw(struct button *btn, unsigned int color, char *pic_name,
                 video_mem *vd_mem);
button *get_button_from_index(button *btn, int index);
int from_input_event_get_btn_index(button *btn, input_event *ievt);

#if 0
int init_button(button *btn, char *name, region *rgn, on_draw draw,
                on_pressed pred);

#endif

#endif