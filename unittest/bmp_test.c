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

#if 1

int main(int argc, char **argv) {
  int ret;
  disp_buff buff;
  disp_ops *fb_ops;
  region rgn;
  disp_buff pixel_data;
  disp_buff pixel_data_small;
  region_cartesian rgn_cart;
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
  if (!fb_ops) {
    printf("get_display_ops_from_name err\n");
    return -1;
  }

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
  alloc_video_mem(5);
  pages_system_register();
  page("main")->run(NULL);

#if 0
  snprintf(icon_name, 128, "%s/%s", ICON_PATH, argv[1]);
  icon_name[127] = '\0';
  set_disp_buff_bpp(&pixel_data, buff.bpp);
  ret = get_disp_buff_for_icon(&pixel_data, icon_name);
  if (ret)
    goto err_get_pixel_data;

  setup_disp_buff(&pixel_data_small, pixel_data.xres / 2, pixel_data.yres / 2,
                  buff.bpp, NULL);
  pixel_data_small.buff = malloc(pixel_data_small.total_size);
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
  free_disp_buff_for_icon(&pixel_data_small);
#endif

  return 0;

err_malloc_small:
err_get_pixel_data:
  bmp_parser.free_pixel_data(&pixel_data);
  return -1;
}

#endif