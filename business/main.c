#include "common.h"
#include "pic_fmt_manger.h"
#include <config.h>
#include <disp_manger.h>
#include <font_manger.h>
#include <input_manger.h>
#include <page_manger.h>
#include <stdio.h>
#include <stdlib.h>
#include <ui.h>

#if 1

int main(int argc, char **argv) {
  int ret;
  disp_buff buff;
  disp_ops *fb_ops;

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

  ret = pic_fmt_system_register();

  /*
   * init network input and lcd input.
   */
  input_system_register();
  input_device_init();

  /*
   * init font.
   */
  fonts_system_register();
  fonts_init(FONT_PATH);
  sel_def_font("freetype");

  /*
   * init page.
   */
  alloc_video_mem(5);
  pages_system_register();
  page("main")->run(NULL);
}

#endif
