#include "render.h"
#include <pic_operation.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#pragma pack(push)
#pragma pack(1)

typedef struct tab_bit_map_file_head {
  unsigned short type;
  unsigned long size;
  unsigned short reserved0;
  unsigned short reserved1;
  unsigned long off_bits;
} tab_bit_map_file_head;

typedef struct tab_bit_map_info_head {
  unsigned long size;
  unsigned long width;
  unsigned long height;
  unsigned short planes;
  unsigned short bit_count;
  unsigned long compression;
  unsigned long image_size;
  unsigned long x_pels_per_meter;
  unsigned long y_pels_per_meter;
  unsigned long cir_used;
  unsigned long cir_important;
} tab_bit_map_info_head;

#pragma pack(pop)

/*
 * whether support bmp.
 */
static int bmp_is_support(unsigned char *file_head) {
  if (file_head[0] != 0x42 || file_head[1] != 0x4d)
    return 0;
  else
    return 1;
}

/*
 * bpp covert.
 */
static int covert_one_line_from_bmp(int width, int src_bpp, int dst_bpp,
                                    unsigned char *puc_src_data,
                                    unsigned char *pud_dst_data) {
  unsigned int red;
  unsigned int green;
  unsigned int blue;
  unsigned int color;
  int i;
  int pos = 0;
  unsigned short *dst_data_16_bpp = (unsigned short *)pud_dst_data;
  unsigned int *dst_data_32_bpp = (unsigned int *)pud_dst_data;

  if (src_bpp != 24)
    goto err_bpp;

  if (dst_bpp == 24)
    memcpy(pud_dst_data, puc_src_data, width * 3);
  else {
    for (i = 0; i < width; i++) {
      blue = puc_src_data[pos++];
      green = puc_src_data[pos++];
      red = puc_src_data[pos++];
      if (dst_bpp == 32) {
        color = (red << 16 | green << 8) | blue;
        *dst_data_32_bpp = color;
        dst_data_32_bpp++;
      } else if (dst_bpp == 16) {
        blue = blue >> 3;
        green = green >> 2;
        red = red >> 3;
        color = (red << 11 | green << 5) | blue;
        *dst_data_16_bpp = color;
        dst_data_16_bpp++;
      }
    }
  }

  return 0;

err_bpp:
  return -1;
}

/*
 * bmp get pixel data.
 * first set th buff->bpp val.
 */
static int bmp_get_pixel_data(unsigned char *file_head, disp_buff *buff) {
  int width;
  int height;
  int bpp;
  int i;
  tab_bit_map_file_head *file;
  tab_bit_map_info_head *info;
  unsigned char *puc_src;
  unsigned char *puc_dest;
  int line_width_align;
  int line_width_real;

  file = (tab_bit_map_file_head *)file_head;
  info = (tab_bit_map_info_head *)(file_head + sizeof(tab_bit_map_file_head));

  width = info->width;
  height = info->height;
  bpp = info->bit_count;

  if (bpp != 24)
    goto err_bpp;

  setup_disp_buff(buff, width, height, -1, NULL);

  /*
   * malloc data.
   */
  buff->buff = malloc((width * height * buff->bpp / 8) * sizeof(unsigned char));
  if (!buff->buff)
    goto err_auc_pixel_data;

  /*
   * get data.
   */
  line_width_real = width * bpp / 8;
  line_width_align = (line_width_real + 3) & ~0x3;
  puc_src = file_head + file->off_bits;
  puc_src = puc_src + (height - 1) * line_width_align;
  puc_dest = buff->buff;

  for (i = 0; i < height; i++) {
    covert_one_line_from_bmp(width, bpp, buff->bpp, puc_src, puc_dest);
    puc_src -= line_width_align;
    puc_dest += buff->line_byte;
  }

  return 0;

err_bpp:
err_auc_pixel_data:
  return -1;
}

/*
 * bmp free pixel data.
 */
static int bmp_free_pixel_data(disp_buff *buff) {
  free(buff->buff);
  return 0;
}

pic_file_parser bmp_parser = {
    .name = "bmp",
    .free_pixel_data = bmp_free_pixel_data,
    .get_pixel_data = bmp_get_pixel_data,
    .is_support = bmp_is_support,
};

/*
 * register bmp parser.
 */
void register_bmp_parser(void) {
  /*
   * register bmp parser.
   */
}