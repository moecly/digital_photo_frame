#include "fcntl.h"
#include "file.h"
#include "font_manger.h"
#include "pic_operation.h"
#include "ui.h"
#include <common.h>
#include <config.h>
#include <disp_manger.h>
#include <pic_fmt_manger.h>
#include <render.h>
#include <stdio.h>
#include <string.h>

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
  // pic_file_parser *parser;
  /*
   * map file.
   */
  map_file(&file, icon_name);

  /*
   * get disp buff.
   */
  ret = parser("bmp")->is_support(&file);
  if (!ret) {
    printf("%s is not support bmp\n", icon_name);
    return -1;
  }

  ret = parser("bmp")->get_pixel_data(&file, dp_buff);
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
  parser("bmp")->free_pixel_data(dp_buff);
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
  for_each_linked_node(dp_ops_queue, dp_ops) {
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

  for_each_linked_node(dp_ops_queue, dp_ops) {
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
 * clean screen from buff.
 */
int clean_screen_from_buff(unsigned int color, disp_buff *dp_buff) {
  memset(dp_buff->buff, color, dp_buff->total_size);
  return 0;
}

/*
 * clean screen from vd.
 */
int clean_screen_from_vd(unsigned int color, video_mem *vd_mem) {
  clean_screen_from_buff(color, &vd_mem->disp_buff);
  return 0;
}

/*
 * clean screen from ops.
 */
int clean_screen_from_ops(unsigned int color, disp_ops *dp_ops) {
  region rgn;
  disp_buff dp_buff;
  int ret;

  ret = get_display_buffer(dp_ops, &dp_buff);
  if (ret)
    goto err_get_display_buffer;
  rgn.width = dp_buff.xres;
  rgn.height = dp_buff.yres;
  rgn.left_up_x = rgn.left_up_y = 0;
  draw_region_from_ops(rgn, color, dp_ops);

  return 0;

err_get_display_buffer:
  return -1;
}

/*
 * clear rectangle from video mem.
 */
int clear_rectangle_from_vd(video_mem *dv_mem, unsigned int x, unsigned int y,
                            unsigned int width, unsigned int height,
                            unsigned int color) {
  int i, j;
  disp_buff *dp_buff = &dv_mem->disp_buff;
  unsigned char *buff = dp_buff->buff;

  buff += y * dp_buff->line_byte + x * dp_buff->pixel_width;
  for (i = 0; i < height; i++) {
    for (j = 0; j < width * dp_buff->pixel_width; j++)
      buff[j] = color;
    buff += dp_buff->line_byte;
  }

  return 0;
}

/*
 * show pixel.
 */
int show_pixel(unsigned int x, unsigned int y, unsigned int color,
               disp_buff *dp_buff) {
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

  return 0;
}

/*
 * draw font bit map.
 */
void draw_from_bit_map_from_buffer(font_bit_map fb_map, unsigned int color,
                                   disp_buff *dp_buff) {
  int i, j;
  int p, q;
  int x = fb_map.region.left_up_x;
  int y = fb_map.region.left_up_y;
  int x_max = fb_map.region.width;
  int y_max = fb_map.region.height;

  for (j = y, q = 0; q < y_max; j++, q++) {
    for (i = x, p = 0; p < x_max; i++, p++) {
      if (i < 0 || j < 0 || i >= dp_buff->xres || j >= dp_buff->yres) {
        continue;
      }
      if (fb_map.buffer[q * fb_map.region.width + p])
        show_pixel(i, j, color, dp_buff);
    }
  }
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

  for_each_linked_node(dp_ops_queue, dp_ops) {
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

  for_each_linked_node(dp_ops_queue, dp_ops) {
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

/*
 * draw text in buff.
 */
int draw_text_in_buff(char *str, region *rgn, unsigned int color,
                      disp_buff *dp_buff) {
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

    draw_from_bit_map_from_buffer(fb_map, color, dp_buff);
    fb_map.cur_origin_x = fb_map.next_origin_x;
    fb_map.cur_origin_y = fb_map.next_origin_y;
  }

  return 0;
}

/*
 * draw text in video mem.
 */
int draw_text_in_vd_mem(char *str, region *rgn, unsigned int color,
                        video_mem *vd_mem) {
  return draw_text_in_buff(str, rgn, color, &vd_mem->disp_buff);
}

/*
 * merger_string_to_center_of_rectangle_in_video_mem.
 */
int merger_string_to_center_of_rectangle_in_video_mem(
    int iTopLeftX, int iTopLeftY, int iBotRightX, int iBotRightY,
    char *pucTextString, video_mem *ptVideoMem) {
  char *pucBufStart;
  region rgn;

  pucBufStart = pucTextString;

  /* 0. 清除这个区域 */
  clear_rectangle_from_vd(ptVideoMem, iTopLeftX, iTopLeftY,
                          iBotRightX - iTopLeftX, iBotRightY - iTopLeftY,
                          BACKGROUND);

  rgn.left_up_x = iTopLeftX;
  rgn.left_up_y = iTopLeftY;
  rgn.width = iBotRightX - iTopLeftX;
  rgn.height = iBotRightY - iTopLeftY;
  draw_text_in_vd_mem(pucBufStart, &rgn, BLACK, ptVideoMem);

  return 0;
}

/*
 * draw pic from video mem.
 */
int draw_pic_from_vd_mem(char *pic_name, video_mem *vd_mem) { return 0; }

/*
 * draw button pic from.
 */
int draw_pic(pic_layout *pic) {
  int ret;
  disp_ops *dp_ops = get_display_ops_from_name(LCD_NAME);
  disp_buff dp_buff;
  disp_buff icon_buff;
  disp_buff origin_icon_buff;
  unsigned int icon_width, icon_height;
  unsigned int icon_x, icon_y;
  char icon_name[128];

  if (!dp_ops)
    goto err_get_display_ops_from_name;

  /*
   * get display buffer.
   */
  ret = get_display_buffer(dp_ops, &dp_buff);
  if (ret)
    goto err_get_display_buffer;

  /*
   * get icon name.
   */
  snprintf(icon_name, 128, "%s/%s", ICON_PATH, pic->pic_name);
  icon_name[127] = '\0';

  /*
   * get icon display buffer.
   */
  set_disp_buff_bpp(&origin_icon_buff, dp_buff.bpp);
  ret = get_disp_buff_for_icon(&origin_icon_buff, icon_name);
  if (ret)
    goto err_get_disp_buff_for_icon;

  /*
   * zoom picture.
   */
  get_rgn_data(&pic->rgn, &icon_x, &icon_y, &icon_width, &icon_height);
  setup_disp_buff(&icon_buff, icon_width, icon_height, dp_buff.bpp, NULL);
  icon_buff.buff = malloc(icon_buff.total_size);
  if (!icon_buff.buff)
    goto err_malloc;

  ret = pic_zoom(&origin_icon_buff, &icon_buff);
  if (ret)
    goto err_pic_zoom;

  /*
   * merge picture.
   */
  ret = pic_merge(icon_x, icon_y, &icon_buff, &dp_buff);
  if (ret)
    goto err_pic_merge;

  free_disp_buff_for_icon(&icon_buff);
  free_disp_buff_for_icon(&origin_icon_buff);

  return 0;

err_pic_merge:
err_pic_zoom:
  free_disp_buff_for_icon(&icon_buff);
err_malloc:
  free_disp_buff_for_icon(&origin_icon_buff);
err_get_disp_buff_for_icon:
err_get_display_buffer:
err_get_display_ops_from_name:
  return -1;
}

/*
 * get disp buff from file.
 */
int get_disp_buff_from_file(char *file_name, disp_buff *dp_buff) {
  int ret;
  file_map file;
  pic_file_parser *parser;
  disp_ops *dp_ops;
  disp_buff fb_buff;

  ret = map_file(&file, file_name);
  if (ret)
    goto err_map_file;

  parser = get_parser(&file);
  if (!parser)
    goto err_get_parser;

  /*
   * get fb info.
   */
  dp_ops = get_display_ops_from_name(LCD_NAME);
  get_display_buffer(dp_ops, &fb_buff);

  /*
   * set pic bpp.
   */
  set_disp_buff_bpp(dp_buff, fb_buff.bpp);
  ret = parser->get_pixel_data(&file, dp_buff);
  if (ret)
    goto err_get_pixel_data;

  unmap_file(&file);
  return 0;

err_get_pixel_data:
err_get_parser:
  unmap_file(&file);
err_map_file:
  return -1;
}

/*
 * clear video mem region.
 */
int clear_video_mem_region(video_mem *ptVideoMem, button *ptLayout,
                           unsigned int dwColor) {
  unsigned char *pucVM;
  unsigned short *pwVM16bpp;
  unsigned int *pdwVM32bpp;
  unsigned short wColor16bpp; /* 565 */
  int iRed;
  int iGreen;
  int iBlue;
  int iX;
  int iY;
  int iLineBytesClear;
  int i;

  pucVM = ptVideoMem->disp_buff.buff +
          ptLayout->pic.rgn.left_up_y * ptVideoMem->disp_buff.line_byte +
          ptLayout->pic.rgn.left_up_x * ptVideoMem->disp_buff.bpp / 8;
  pwVM16bpp = (unsigned short *)pucVM;
  pdwVM32bpp = (unsigned int *)pucVM;

  iLineBytesClear = (ptLayout->pic.rgn.left_up_x + ptLayout->pic.rgn.width -
                     ptLayout->pic.rgn.left_up_x + 1) *
                    ptVideoMem->disp_buff.bpp / 8;

  switch (ptVideoMem->disp_buff.bpp) {
  case 8: {
    for (iY = ptLayout->pic.rgn.left_up_y;
         iY <= ptLayout->pic.rgn.left_up_y + ptLayout->pic.rgn.height; iY++) {
      memset(pucVM, dwColor, iLineBytesClear);
      pucVM += ptVideoMem->disp_buff.line_byte;
    }
    break;
  }
  case 16: {
    /* 先根据32位的dwColor构造出16位的wColor16bpp */
    iRed = (dwColor >> (16 + 3)) & 0x1f;
    iGreen = (dwColor >> (8 + 2)) & 0x3f;
    iBlue = (dwColor >> 3) & 0x1f;
    wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
    for (iY = ptLayout->pic.rgn.left_up_y;
         iY <= ptLayout->pic.rgn.left_up_y + ptLayout->pic.rgn.height; iY++) {
      i = 0;
      for (iX = ptLayout->pic.rgn.left_up_x;
           iX <= ptLayout->pic.rgn.left_up_x + ptLayout->pic.rgn.width; iX++) {
        pwVM16bpp[i++] = wColor16bpp;
      }
      pwVM16bpp = (unsigned short *)((unsigned int)pwVM16bpp +
                                     ptVideoMem->disp_buff.line_byte);
    }
    break;
  }
  case 32: {
    for (iY = ptLayout->pic.rgn.left_up_y;
         iY <= ptLayout->pic.rgn.left_up_y + ptLayout->pic.rgn.height; iY++) {
      i = 0;
      for (iX = ptLayout->pic.rgn.left_up_x;
           iX <= ptLayout->pic.rgn.left_up_x + ptLayout->pic.rgn.width; iX++) {
        pdwVM32bpp[i++] = dwColor;
      }
      pdwVM32bpp = (unsigned int *)((unsigned int)pdwVM32bpp +
                                    ptVideoMem->disp_buff.line_byte);
    }
    break;
  }
  default: {
    return -1;
  }
  }

  return 0;
}
