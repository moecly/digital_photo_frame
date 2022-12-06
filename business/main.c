#include "common.h"
#include <disp_manger.h>
#include <font_manger.h>
#include <input_manger.h>
#include <page_manger.h>
#include <stdio.h>
#include <stdlib.h>
#include <ui.h>

#if 0

int main(int argc, char **argv) {
  int ret;
  disp_buff buff;
  region rgn;

  /*
   * init lcd display.
   */
  display_system_register();
  ret = select_default_display("lcd");
  if (ret) {
    printf("select default display err\n");
    return -1;
  }
  ret = default_display_init();
  if (ret) {
    printf("init default display err\n");
  }

  ret = get_display_buffer(&buff);
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
  pages_system_register();

  /*
   * draw text.
   */
  rgn.left_up_x = rgn.left_up_y = 0;
  rgn.width = buff.xres;
  rgn.height = buff.yres;
  set_font_size(77);
  draw_region(rgn, 0xffffff);
  draw_text_in_region("hello world", &rgn, 0x000000);
  // draw_jpeg("4.jpg");
  flush_display_region(rgn, &buff);

  return 0;
}

#endif
