
#include <pic_operation.h>
#include <stdio.h>
#include <string.h>

/*
 * merge picture.
 */
int pic_merge(int x, int y, disp_buff *small_pic, disp_buff *big_pic) {

  int i;
  unsigned char *puc_small;
  unsigned char *puc_big;

  if (small_pic->xres > big_pic->xres || small_pic->yres > big_pic->yres ||
      small_pic->bpp > big_pic->bpp)
    goto err_pic;

  puc_small = small_pic->buff;
  puc_big = big_pic->buff + y * big_pic->line_byte + x * big_pic->bpp / 8;
  for (i = 0; i < small_pic->yres; i++) {
    memcpy(puc_big, puc_small, small_pic->line_byte);
    puc_big += big_pic->line_byte;
    puc_small += small_pic->line_byte;
  }

  return 0;

err_pic:
  return -1;
}