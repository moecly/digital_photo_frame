#include "common.h"
#include <config.h>
#include <disp_manger.h>
#include <fcntl.h>
#include <font_manger.h>
#include <input_manger.h>
#include <page_manger.h>
#include <pic_operation.h>
#include <render.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ui.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int ret;
  // int fd_bmp;
  disp_buff buff;
  disp_ops *fb_ops;
  region rgn;
  // unsigned char *puc_bmp;
  // struct stat bmp_stat;
  disp_buff pixel_data;
  disp_buff pixel_data_small;

  char icon_name[128];

  extern pic_file_parser bmp_parser;

  if (argc != 2) {
    printf("input err\n");
    return -1;
  }

  /*
   * init lcd display.
   */
  display_system_register();

  fb_ops = get_display_ops_from_name("lcd");
  add_disp_queue(fb_ops);
  if (!fb_ops) {
    printf("get fb ops err\n");
    return -1;
  }

  ret = display_system_init();
  if (ret) {
    printf("display system init err\n");
    return -1;
  }

  ret = get_display_buffer(fb_ops, &buff);
  if (ret) {
    printf("get display buffer err\n");
    return -1;
  }

  printf("fb_ops name %s\n", fb_ops->name);
  printf("buff val %d\n", buff.bpp);

  /*
   * init network input and lcd input.
   */
  input_system_register();
  input_device_init();

  /*
   * init font.
   */
  fonts_system_register();
  fonts_init("simsun.ttc");
  sel_def_font("freetype");

  /*
   * init page.
   */
  pages_system_register();

  /*
   * open bmp.
   */
  // fd_bmp = open(argv[1], O_RDWR);
  // if (fd_bmp < 0) {
  //   printf("open fd_bmp err\n");
  //   goto err_open;
  // }

  // fstat(fd_bmp, &bmp_stat);
  // puc_bmp = (unsigned char *)mmap(
  //     NULL, bmp_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bmp, 0);
  // if (puc_bmp == (unsigned char *)-1) {
  //   printf("mmap err\n");
  //   goto err_mmap;
  // }

  // ret = bmp_parser.is_support(puc_bmp);
  // if (!ret) {
  //   printf("pic parser is not support\n");
  //   goto err_is_support;
  // }

  // pixel_data.bpp = buff.bpp;
  // ret = bmp_parser.get_pixel_data(puc_bmp, &pixel_data);
  // if (ret) {
  //   printf("get pixel data err\n");
  //   goto err_get_pixel_data;
  // }
  snprintf(icon_name, 128, "%s/%s", ICON_PATH, argv[1]);
  icon_name[127] = '\0';
  set_disp_buff_bpp(&pixel_data, buff.bpp);
  ret = get_disp_buff_for_icon(&pixel_data, icon_name);
  if (ret)
    goto err_get_pixel_data;

  pixel_data_small.bpp = buff.bpp;
  pixel_data_small.xres = pixel_data.xres * 2;
  pixel_data_small.yres = pixel_data.yres * 2;
  pixel_data_small.pixel_width = pixel_data_small.bpp / 8;
  pixel_data_small.line_byte = pixel_data_small.xres * pixel_data_small.bpp / 8;
  pixel_data_small.buff =
      malloc(pixel_data_small.line_byte * pixel_data_small.yres);
  if (!pixel_data_small.buff)
    goto err_malloc_small;

  clean_screen(0xffffff);
  pic_zoom(&pixel_data, &pixel_data_small);
  rgn.left_up_x = rgn.left_up_y = 0;
  rgn.width = buff.xres;
  rgn.height = buff.yres;
  pic_merge(0, 0, &pixel_data, &buff);
  pic_merge(277, 177, &pixel_data_small, &buff);
  flush_display_region(&rgn, fb_ops, &buff);

  bmp_parser.free_pixel_data(&pixel_data);
  free(pixel_data_small.buff);

  return 0;

err_malloc_small:
err_get_pixel_data:
  bmp_parser.free_pixel_data(&pixel_data);
  // err_is_support:
  // err_mmap:
  // err_open:
  return -1;
}