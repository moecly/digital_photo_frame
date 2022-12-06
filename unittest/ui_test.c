#include "font_manger.h"
#include <disp_manger.h>
#include <input_manger.h>
#include <stdio.h>
#include <ui.h>

#if 0

int main(int argc, char **argv) {
  int ret;
  button btn_ary[15];
//   int i;
//   int j;
//   input_device idev;
//   input_event ievt;
  disp_buff buff;
  region rgn;

  display_init();
  ret = select_default_display("lcd");
  if (ret) {
    printf("select display err\n");
    return -1;
  }
  
  fonts_register();
  sel_def_font("freetype");
  fonts_init("simsun.ttc");
  set_font_size(30);

  get_display_buffer(&buff);
  ret = init_default_display();
  if (ret) {
    printf("init default display err\n");
    return -1;
  }

  input_init();
  input_device_init();

  rgn.width = 270;
  rgn.height = 150;
  rgn.left_up_x = 10;
  rgn.left_up_y = 10;
  init_button(&btn_ary[0], "TEST", rgn, NULL, NULL);
  if (btn_ary[0].on_draw == NULL) {
  }
  btn_ary[0].on_draw(&btn_ary[0], buff, BUTTON_DEFAULT_COLOR);
  //   for (i = 0; i < 3; i++) {
  //     for (j = 0; j < 5; j++) {
  //   init_button(btn_ary[i * 5 + j], "test", (i + 1) * h, NULL, NULL);
  //     }
  //   }

  return 0;
}

#endif