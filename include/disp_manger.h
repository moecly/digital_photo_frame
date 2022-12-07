#ifndef _DISP_MANGER_H
#define _DISP_MANGER_H

#define True 0
#define False -1

#include <font_manger.h>

typedef struct disp_buff {
  int xres;
  int yres;
  int bpp;
  unsigned char *buff;
  unsigned int line_byte;
  unsigned int pixel_width;
  unsigned int total_size;
} disp_buff;

typedef enum {
  VMS_FREE = 0,
  VMS_USED_FOR_PREPARE,
  VMS_USED_FOR_CUR,
} video_mem_status;

typedef enum {
  PS_BLANK = 0,
  PS_GENERATING,
  PS_GENERATED,
} pic_status;

typedef struct video_mem {
  int id;
  int is_dev_framebuffer;
  video_mem_status mem_status;
  pic_status pic_status;
  disp_buff disp_buff;
  struct video_mem *next;
} video_mem;

typedef struct display_operations {
  char *name;
  int (*device_init)(void);
  void (*device_exit)(void);
  int (*get_buffer)(disp_buff *dp_buff);
  int (*flush_region)(region *rgn, disp_buff *dp_buff);
  int (*show_page)(disp_buff *dp_buff);
  int (*show_pixel)(unsigned int x, unsigned int y, unsigned int color);
  struct display_operations *next;
} disp_ops;

disp_ops *get_disp_queue(void);
int flush_video_mem_to_dev(video_mem *vd_mem);
int add_disp_queue(disp_ops *dp_ops);
int flush_one_video_mem_to_dev_from_ops(disp_ops *dp_ops, video_mem *vd_mem);
void register_display(disp_ops *dops);
void display_system_register(void);
void free_video_mem(void);
int alloc_video_mem(int num);
void put_video_mem(video_mem *vd_mem);
int display_system_init(void);
int flush_display_region(region *rgn, disp_ops *dp_ops, disp_buff *dp_buff);
disp_ops *get_display_ops_from_name(const char *name);
int get_display_buffer(disp_ops *dp_ops, disp_buff *dp_buff);
video_mem *get_video_mem(int id, int cur);

#if 0
int put_pixel(int x, int y, unsigned int color, disp_buff *dp_buff);
void draw_region(region reg, unsigned int color, disp_buff *dp_buff);
int draw_text_in_region(char *str, region *rgn, unsigned int color,
                        disp_buff *dp_buff);
int clean_screen(unsigned int color, disp_buff *dp_buff);
int draw_jpeg(char *name);
int draw_line(int x_start, int x_end, int y, unsigned char *rgb_color_array,
              disp_buff *dp_buff);
void draw_from_bit_map(font_bit_map fb_map, unsigned int color,
                       disp_buff *dp_buff);
#endif

#if 0
int draw_text_in_region(char *name, region *rgn, unsigned int color);
int flush_display_region(region *rgn, disp_buff *buffer);
int draw_line(int x_start, int x_end, int y, unsigned char *rgb_color_array);
void draw_from_bit_map(font_bit_map fb_map, unsigned int color);
int alloc_video_mem(int num);
int clean_screen(unsigned int color);
int default_display_init(void);
int put_pixel(int x, int y, unsigned int color);
int select_default_display(char *name);
void draw_region(region reg, unsigned int color);
disp_buff *get_display_buffer(void);
#endif

#endif