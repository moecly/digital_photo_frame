#ifndef _FONT_MANGER_H
#define _FONT_MANGER_H

#ifndef NULL
#define NULL (void *)0
#endif

#include <common.h>

typedef struct font_bit_map {
  region region;
  int cur_origin_x;
  int cur_origin_y;
  int next_origin_x;
  int next_origin_y;
  unsigned char *buffer;
} font_bit_map;

typedef struct font_ops {
  char *name;
  int (*font_init)(char *);
  int (*set_font_size)(unsigned int);
  int (*get_font_bit_map)(unsigned int, font_bit_map *);
  int (*get_string_region_car)(char *, region_cartesian *);
  struct font_ops *next;
} font_ops;

void register_font(font_ops *fops);
void fonts_system_register(void);
int get_font_bit_map(unsigned int code, font_bit_map *fb_map);
int fonts_init(char *font_name);
int sel_def_font(char *name);
int get_string_region_car(char *str, region_cartesian *rgn);
int set_font_size(unsigned int size);

#endif