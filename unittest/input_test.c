#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <input_manger.h>

#if 0

int main(int argc, char **argv) {
  input_event ievt;
  int ret;

  input_init();
  input_device_init();
  
  while (1) {
    ret = get_input_event(&ievt);
    if (!ret) {
      if (ievt.type == INPUT_TYPE_TOUCH) {
        printf("type: %d\n", ievt.type);
        printf("x: %d\n", ievt.x);
        printf("y: %d\n", ievt.y);
        printf("pressure: %d\n", ievt.pressure);
        printf("time: %d\n\n", (int)ievt.time.tv_sec);
      } else {
        printf("type: %d\n", ievt.type);
        printf("data: %s\n", ievt.str);
        printf("time: %d\n\n", (int)ievt.time.tv_sec);
      }
    }
  }

  return 0;
}

#endif