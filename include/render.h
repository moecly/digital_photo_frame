#ifndef _RENDER_H
#define _RENDER_H

#include <pic_operation.h>

#if 0
int put_pixel(int x, int y, unsigned int color, disp_buff *dp_buff);
#endif

int pic_merge(int x, int y, disp_buff *small_pic, disp_buff *big_pic);
int pic_zoom(disp_buff *dst_desc, disp_buff *zoom_desc);

int draw_line(int x_start, int x_end, int y, unsigned char *rgb_color_array);
void draw_region(region reg, unsigned int color);
int draw_text_in_region(char *str, region *rgn, unsigned int color);
int clean_screen(unsigned int color);
void draw_region_from_ops(region reg, unsigned int color, disp_ops *dp_ops);
int get_disp_buff_for_icon(disp_buff *dp_buff, char *icon_name);
// int draw_jpeg(char *name);
void draw_from_bit_map(font_bit_map fb_map, unsigned int color);
void set_disp_buff_bpp(disp_buff *dp_buff, unsigned int bpp);
void setup_disp_buff(disp_buff *dp_buff, unsigned int xres, unsigned int yres,
                     int bpp, unsigned char *buff);
void free_disp_buff_for_icon(disp_buff *dp_buff);

#endif // !_RENDER_H
