#ifndef _PIC_OPERATION_H
#define _PIC_OPERATION_H

#include <disp_manger.h>

// typedef struct pixel_data_desc {
//   int width;
//   int height;
//   int bpp;
//   int line_byte;
//   unsigned char *auc_pixel_data;
// } pixel_data_desc;

typedef struct pic_file_parser {
  char *name;
  int (*is_support)(unsigned char *file_head);
  // int (*get_pixel_data)(unsigned char *file_head, pixel_data_desc
  // *pixel_desc); int (*free_pixel_data)(pixel_data_desc *pixel_desc);
  int (*get_pixel_data)(unsigned char *file_head, disp_buff *buff);
  int (*free_pixel_data)(disp_buff *buff);
} pic_file_parser;

#endif // !_PIC_OPERATION_H