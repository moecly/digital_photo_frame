#include "fcntl.h"
#include "file.h"
#include <common.h>
#include <disp_manger.h>
#include <render.h>
#include <stdio.h>
#include <string.h>

extern pic_file_parser bmp_parser;

#if 0
/*
 * draw pixel.
 */
int put_pixel(int x, int y, unsigned int color, disp_buff *dp_buff) {
  unsigned char *pen_8 =
      dp_buff->buff + y * dp_buff->line_byte + x * dp_buff->pixel_width;
  unsigned short *pen_16;
  unsigned int *pen_32;

  unsigned int red, green, blue;

  pen_16 = (unsigned short *)pen_8;
  pen_32 = (unsigned int *)pen_8;

  switch (dp_buff->bpp) {
  case 8: {
    *pen_8 = color;
    break;
  }
  case 16: {
    /* 565 */
    red = (color >> 16) & 0xff;
    green = (color >> 8) & 0xff;
    blue = (color >> 0) & 0xff;
    color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
    *pen_16 = color;
    break;
  }
  case 32: {
    *pen_32 = color;
    break;
  }
  default: {
    printf("can't surport %dbpp\n", dp_buff->bpp);
    break;
  }
  }

  return True;
}
#endif

/*
 * setup disp buff.
 */
void setup_disp_buff(disp_buff *dp_buff, unsigned int xres, unsigned int yres,
                     int bpp, unsigned char *buff) {
  dp_buff->xres = xres;
  dp_buff->yres = yres;

  if (bpp != -1)
    dp_buff->bpp = bpp;

  dp_buff->pixel_width = dp_buff->bpp / 8;
  dp_buff->line_byte = xres * dp_buff->bpp / 8;
  dp_buff->total_size = dp_buff->line_byte * yres;

  if (buff)
    dp_buff->buff = buff;
}

/*
 * set disp buff bpp.
 */
void set_disp_buff_bpp(disp_buff *dp_buff, unsigned int bpp) {
  dp_buff->bpp = bpp;
}

/*
 * get disp buff for icon.
 * first set th dp_buff->bpp val.
 * need to use func bmp_free_pixel_data to free dp_buff.
 */
int get_disp_buff_for_icon(disp_buff *dp_buff, char *icon_name) {
  int ret;
  file_map file;
  /*
   * map file.
   */
  map_file(&file, icon_name);

  /*
   * get disp buff.
   */
  ret = bmp_parser.is_support(file.file_map_mem);
  if (!ret) {
    printf("%s is not support bmp\n", icon_name);
    return -1;
  }

  printf("bpp = %d\n", dp_buff->bpp);
  ret = bmp_parser.get_pixel_data(file.file_map_mem, dp_buff);
  if (ret) {
    printf("%s get pixel data err\n", icon_name);
    return -1;
  }

  /*
   * unmap file.
   */
  unmap_file(&file);

  return 0;
}

/*
 * free disp buff for icon.
 */
void free_disp_buff_for_icon(disp_buff *dp_buff) {
  bmp_parser.free_pixel_data(dp_buff);
}

/*
 * draw line.
 */
int draw_line(int x_start, int x_end, int y, unsigned char *rgb_color_array) {
  int i = x_start * 3;
  int x;
  unsigned int dwColor;
  int ret;
  disp_ops *dp_ops_queue = get_disp_queue();
  disp_ops *dp_ops;
  disp_buff dp_buff;

  /*
   * each every node.
   */
  for_each_node(dp_ops_queue, dp_ops) {
    ret = get_display_buffer(dp_ops, &dp_buff);
    if (ret)
      return -1;

    if (y >= dp_buff.yres)
      return -1;

    if (x_start >= dp_buff.xres)
      return -1;

    if (x_end >= dp_buff.xres) {
      x_end = dp_buff.xres;
    }

    for (x = x_start; x < x_end; x++) {
      /* 0xRRGGBB */
      dwColor = (rgb_color_array[i] << 16) + (rgb_color_array[i + 1] << 8) +
                (rgb_color_array[i + 2] << 0);
      i += 3;
      dp_ops->show_pixel(x, y, dwColor);
    }
  }
  return 0;
}

/*
 * clear screen.
 */
int clean_screen(unsigned int color) {
  region rgn;
  int ret;
  disp_ops *dp_ops_queue = get_disp_queue();
  disp_ops *dp_ops;
  disp_buff dp_buff;

  for_each_node(dp_ops_queue, dp_ops) {
    ret = get_display_buffer(dp_ops, &dp_buff);
    if (ret)
      return -1;

    rgn.width = dp_buff.xres;
    rgn.height = dp_buff.yres;
    rgn.left_up_x = rgn.left_up_y = 0;
    draw_region_from_ops(rgn, color, dp_ops);
  }

  return 0;
}

/*
 * draw font bit map.
 */
void draw_from_bit_map(font_bit_map fb_map, unsigned int color) {
  int ret;
  disp_ops *dp_ops_queue = get_disp_queue();
  disp_ops *dp_ops;
  disp_buff dp_buff;

  int p, q;
  int i, j;
  int x = fb_map.region.left_up_x;
  int y = fb_map.region.left_up_y;
  int x_max = fb_map.region.width;
  int y_max = fb_map.region.height;

  for_each_node(dp_ops_queue, dp_ops) {
    ret = get_display_buffer(dp_ops, &dp_buff);
    if (ret)
      return;

    for (j = y, q = 0; q < y_max; j++, q++) {
      for (i = x, p = 0; p < x_max; i++, p++) {
        if (i < 0 || j < 0 || i >= dp_buff.xres || j >= dp_buff.yres) {
          continue;
        }
        if (fb_map.buffer[q * fb_map.region.width + p])
          dp_ops->show_pixel(i, j, color);
      }
    }
  }
}

/*
 * draw text in region center.
 */
int draw_text_in_region(char *str, region *rgn, unsigned int color) {
  int ret;
  font_bit_map fb_map;
  region_cartesian rgn_car;

  ret = get_string_region_car(str, &rgn_car);
  if (ret) {
    printf("get string region car err\n");
    return -1;
  }

  fb_map.cur_origin_x =
      rgn->left_up_x + (rgn->width - rgn_car.width) / 2 - rgn_car.left_up_x;
  fb_map.cur_origin_y =
      rgn->left_up_y + (rgn->height - rgn_car.height) / 2 + rgn_car.left_up_y;

  while (*str) {
    ret = get_font_bit_map((unsigned int)*str, &fb_map);
    str++;
    if (ret) {
      return -1;
    }

    draw_from_bit_map(fb_map, color);
    fb_map.cur_origin_x = fb_map.next_origin_x;
    fb_map.cur_origin_y = fb_map.next_origin_y;
  }

  return 0;
}

/*
 * draw region form ops.
 */
void draw_region_from_ops(region reg, unsigned int color, disp_ops *dp_ops) {
  int x = reg.left_up_x;
  int y = reg.left_up_y;
  int width = reg.width;
  int height = reg.height;
  int ret;
  int i, j;
  disp_buff dp_buff;

  ret = get_display_buffer(dp_ops, &dp_buff);
  if (ret)
    return;

  for (j = y; j < y + height; j++) {
    for (i = x; i < x + width; i++) {
      if (i < 0 || j < 0 || i >= dp_buff.xres || j >= dp_buff.yres)
        continue;
      dp_ops->show_pixel(i, j, color);
    }
  }
}

/*
 * draw region.
 */
void draw_region(region reg, unsigned int color) {
  int ret;
  disp_ops *dp_ops_queue = get_disp_queue();
  disp_ops *dp_ops;
  disp_buff dp_buff;

  int x = reg.left_up_x;
  int y = reg.left_up_y;
  int width = reg.width;
  int height = reg.height;
  int i, j;

  for_each_node(dp_ops_queue, dp_ops) {
    ret = get_display_buffer(dp_ops, &dp_buff);
    if (ret)
      return;
    for (j = y; j < y + height; j++) {
      for (i = x; i < x + width; i++) {
        if (i < 0 || j < 0 || i >= dp_buff.xres || j >= dp_buff.yres)
          continue;
        dp_ops->show_pixel(i, j, color);
      }
    }
  }
}
