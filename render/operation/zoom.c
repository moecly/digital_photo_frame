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

/*
 * pic merge region.
 */
int pic_merge_region(int iStartXofNewPic, int iStartYofNewPic,
                     int iStartXofOldPic, int iStartYofOldPic, int iWidth,
                     int iHeight, disp_buff *ptNewPic, disp_buff *ptOldPic) {
  int i;
  unsigned char *pucSrc;
  unsigned char *pucDst;
  int iLineBytesCpy = iWidth * ptNewPic->bpp / 8;

  if ((iStartXofNewPic < 0 || iStartXofNewPic >= ptNewPic->xres) ||
      (iStartYofNewPic < 0 || iStartYofNewPic >= ptNewPic->yres) ||
      (iStartXofOldPic < 0 || iStartXofOldPic >= ptOldPic->xres) ||
      (iStartYofOldPic < 0 || iStartYofOldPic >= ptOldPic->yres)) {
    return -1;
  }

  pucSrc = ptNewPic->buff + iStartYofNewPic * ptNewPic->line_byte +
           iStartXofNewPic * ptNewPic->bpp / 8;
  pucDst = ptOldPic->buff + iStartYofOldPic * ptOldPic->line_byte +
           iStartXofOldPic * ptOldPic->bpp / 8;
  for (i = 0; i < iHeight; i++) {
    memcpy(pucDst, pucSrc, iLineBytesCpy);
    pucSrc += ptNewPic->line_byte;
    pucDst += ptOldPic->line_byte;
  }
  return 0;
}
