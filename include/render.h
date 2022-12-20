#ifndef _RENDER_H
#define _RENDER_H

#include "disp_manger.h"
#include <pic_operation.h>
#include <ui.h>

#if 0
int put_pixel(int x, int y, unsigned int color, disp_buff *dp_buff);
#endif

int pic_merge(int x, int y, disp_buff *small_pic, disp_buff *big_pic);
int pic_zoom(disp_buff *dst_desc, disp_buff *zoom_desc);

int pic_merge_region(int iStartXofNewPic, int iStartYofNewPic,
                     int iStartXofOldPic, int iStartYofOldPic, int iWidth,
                     int iHeight, disp_buff *ptNewPic, disp_buff *ptOldPic);
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
int clean_screen_from_ops(unsigned int color, disp_ops *dp_ops);
int clean_screen_from_buff(unsigned int color, disp_buff *dp_buff);
int clean_screen_from_vd(unsigned int color, video_mem *vd_mem);
int draw_pic(pic_layout *pic);
int merger_string_to_center_of_rectangle_in_video_mem(
    int iTopLeftX, int iTopLeftY, int iBotRightX, int iBotRightY,
    char *pucTextString, video_mem *ptVideoMem);

int get_disp_buff_from_file(char *file_name, disp_buff *dp_buff);
void draw_from_bit_map_from_buffer(font_bit_map fb_map, unsigned int color,
                                   disp_buff *dp_buff);

int draw_text_in_vd_mem(char *str, region *rgn, unsigned int color,
                        video_mem *vd_mem);

int show_pixel(unsigned int x, unsigned int y, unsigned int color,
               disp_buff *dp_buff);

int clear_video_mem_region(video_mem *ptVideoMem, button *ptLayout,
                           unsigned int dwColor);

int clear_rectangle_from_vd(video_mem *dv_mem, unsigned int x, unsigned int y,
                            unsigned int width, unsigned int height,
                            unsigned int color);

#endif // !_RENDER_H
