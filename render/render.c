#include "fcntl.h"
#include "file.h"
#include "font_manger.h"
#include "ui.h"
#include <common.h>
#include <config.h>
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
 * main page on draw.
 */
int icon_on_draw(struct button *btn, unsigned int color, char *pic_name,
                 video_mem *vd_mem) {
  disp_buff origin_icon_buff;
  disp_buff icon_buff;
  disp_buff *dp_buff;
  char icon_name[128];
  char *name;
  unsigned int icon_width, icon_height;
  unsigned int icon_x, icon_y;
  int ret;

  /*
   * get display buffer.
   */
  dp_buff = &vd_mem->disp_buff;

  /*
   * set buff params.
   */
  name = pic_name != NULL ? pic_name : btn->pic.pic_name;
  snprintf(icon_name, 128, "%s/%s", ICON_PATH, name);
  icon_name[127] = '\0';

  /*
   * get icon display buffer.
   */
  set_disp_buff_bpp(&origin_icon_buff, dp_buff->bpp);
  ret = get_disp_buff_for_icon(&origin_icon_buff, icon_name);
  if (ret)
    goto err_get_disp_buff_for_icon;

  /*
   * zoom picture.
   */
  get_button_rgn_data(btn, &icon_x, &icon_y, &icon_width, &icon_height);
  setup_disp_buff(&icon_buff, icon_width, icon_height, dp_buff->bpp, NULL);
  icon_buff.buff = malloc(icon_buff.total_size);
  if (!icon_buff.buff)
    goto err_malloc;

  ret = pic_zoom(&origin_icon_buff, &icon_buff);
  if (ret)
    goto err_pic_zoom;

  /*
   * merge picture.
   */
  ret = pic_merge(icon_x, icon_y, &icon_buff, dp_buff);
  if (ret)
    goto err_pic_merge;

  return 0;

err_pic_merge:
err_pic_zoom:
  free_disp_buff_for_icon(&icon_buff);
err_malloc:
  free_disp_buff_for_icon(&origin_icon_buff);
err_get_disp_buff_for_icon:
  return -1;
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
 * invert button.
 */
int invert_button(button *btn) {
  unsigned char *buff;
  int ret;
  unsigned int x, y;
  unsigned int btn_x, btn_y;
  unsigned int btn_width, btn_height;
  disp_buff dp_buff;
  disp_ops *dp_ops = get_display_ops_from_name(LCD_NAME);

  if (!dp_ops)
    goto err_get_display_ops_from_name;

  /*
   * get button data.
   */
  ret = get_display_buffer(dp_ops, &dp_buff);
  if (ret)
    goto err_get_display_buffer;

  get_button_rgn_data(btn, &btn_x, &btn_y, &btn_width, &btn_height);
  buff = dp_buff.buff;
  buff += btn_y * dp_buff.line_byte + btn_x * dp_buff.pixel_width;

  /*
   * invert buffer.
   */
  for (y = 0; y < btn_height; y++) {
    for (x = 0; x < btn_width * dp_buff.pixel_width; x++)
      buff[x] = ~buff[x];
    buff += dp_buff.line_byte;
  }

  return 0;

err_get_display_buffer:
err_get_display_ops_from_name:
  return -1;
}

/*
 * press button.
 */
int press_button(button *btn) {
  if (btn->status != BUTTON_PRESSED) {
    btn->status = BUTTON_PRESSED;
    invert_button(btn);
  }
  return 0;
}

/*
 * release button.
 */
int release_button(button *btn) {
  if (btn->status != BUTTON_RELEASE) {
    btn->status = BUTTON_RELEASE;
    invert_button(btn);
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
 * clean button invert.
 */
void clean_button_invert(button *btn) {
  while (btn->pic.pic_name) {
    if (btn->status == BUTTON_PRESSED) {
      release_button(btn);
      btn->status = BUTTON_RELEASE;
    }
    btn++;
  }
}

static int get_pre_one_bits(unsigned char ucVal) {
  int i;
  int j = 0;

  for (i = 7; i >= 0; i--) {
    if (!(ucVal & (1 << i)))
      break;
    else
      j++;
  }
  return j;
}

static int get_code_from_buf(unsigned char *pucBufStart,
                             unsigned char *pucBufEnd, unsigned int *pdwCode) {
#if 0
    对于UTF-8编码中的任意字节B，如果B的第一位为0，则B为ASCII码，并且B独立的表示一个字符;
    如果B的第一位为1，第二位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的一个字节，并且不为字符的第一个字节编码;
    如果B的前两位为1，第三位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由两个字节表示;
    如果B的前三位为1，第四位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由三个字节表示;
    如果B的前四位为1，第五位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由四个字节表示;

    因此，对UTF-8编码中的任意字节，根据第一位，可判断是否为ASCII字符;
    根据前二位，可判断该字节是否为一个字符编码的第一个字节; 
    根据前四位（如果前两位均为1），可确定该字节为字符编码的第一个字节，并且可判断对应的字符由几个字节表示;
    根据前五位（如果前四位为1），可判断编码是否有错误或数据传输过程中是否有错误。
#endif

  int i;
  int iNum;
  unsigned char ucVal;
  unsigned int dwSum = 0;

  if (pucBufStart >= pucBufEnd) {
    /* 文件结束 */
    return 0;
  }

  ucVal = pucBufStart[0];
  iNum = get_pre_one_bits(pucBufStart[0]);

  if ((pucBufStart + iNum) > pucBufEnd) {
    /* 文件结束 */
    return 0;
  }

  if (iNum == 0) {
    /* ASCII */
    *pdwCode = pucBufStart[0];
    return 1;
  } else {
    ucVal = ucVal << iNum;
    ucVal = ucVal >> iNum;
    dwSum += ucVal;
    for (i = 1; i < iNum; i++) {
      ucVal = pucBufStart[i] & 0x3f;
      dwSum = dwSum << 6;
      dwSum += ucVal;
    }
    *pdwCode = dwSum;
    return iNum;
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
 * merger_string_to_center_of_rectangle_in_video_mem
 */
int merger_string_to_center_of_rectangle_in_video_mem(
    int iTopLeftX, int iTopLeftY, int iBotRightX, int iBotRightY,
    char *pucTextString, video_mem *ptVideoMem) {
  int iLen;
  int iError;
  char *pucBufStart;
  char *pucBufEnd;
  unsigned int dwCode;
  font_bit_map tFontBitMap;
  region_cartesian rgn_cart;
  region rgn;
  int bHasGetCode = 0;

  int iMinX = 32000, iMaxX = -1;
  int iMinY = 32000, iMaxY = -1;

  int iStrTopLeftX, iStrTopLeftY;

  int iWidth, iHeight;

  tFontBitMap.cur_origin_x = 0;
  tFontBitMap.cur_origin_y = 0;
  pucBufStart = pucTextString;
  pucBufEnd = pucTextString + strlen((char *)pucTextString);

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
