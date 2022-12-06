#include <pic_operation.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

/*
 * zoom picture.
 */
int pic_zoom(disp_buff *origin_desc, disp_buff *zoom_desc) {
  unsigned long dst_width = zoom_desc->xres;
  unsigned long x;
  unsigned long y;
  unsigned long src_y;
  unsigned long *srcx_table = malloc(sizeof(unsigned long) * dst_width);
  unsigned char *puc_dest;
  unsigned char *puc_src;
  unsigned long bytes = origin_desc->bpp / 8;

  if (!srcx_table)
    goto err_table;

  if (origin_desc->bpp != zoom_desc->bpp)
    goto err_bpp;

  for (x = 0; x < dst_width; x++)
    srcx_table[x] = (x * origin_desc->xres / zoom_desc->xres);

  for (y = 0; y < zoom_desc->yres; y++) {
    src_y = (y * origin_desc->yres / zoom_desc->yres);
    puc_src = origin_desc->buff + src_y * origin_desc->line_byte;
    puc_dest = zoom_desc->buff + y * zoom_desc->line_byte;

    for (x = 0; x < dst_width; x++)
      memcpy(puc_dest + x * bytes, puc_src + srcx_table[x] * bytes, bytes);
  }

  free(srcx_table);

  return 0;

err_bpp:
err_table:
  free(srcx_table);
  return -1;
}