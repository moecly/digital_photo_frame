#include <input_manger.h>
#include <stdio.h>
#include <tslib.h>

static struct tsdev *ts;

/*
 * get touchscreen input info.
 */
static int touchscreen_get_input_event(input_event *ievent) {
  struct ts_sample samp;
  int ret;

  ret = ts_read(ts, &samp, 1);
  if (ret != 1) {
    printf("ts_read err\n");
    return -1;
  }

  ievent->x = samp.x;
  ievent->y = samp.y;
  ievent->pressure = samp.pressure;
  ievent->time = samp.tv;
  ievent->type = INPUT_TYPE_TOUCH;

  return 0;
}

/*
 * init touchscreen device.
 */
static int touchscreen_device_init(void) {

  ts = ts_setup(NULL, 0);
  if (!ts) {
    printf("te_setup err\n");
    return -1;
  }

  return 0;
}

/*
 * exit touchscreen device.
 */
static void touchscreen_device_exit(void) {
  ts_close(ts);
}

static input_device touch_dev = {
    .name = "touchscreen",
    .device_init = touchscreen_device_init,
    .device_exit = touchscreen_device_exit,
    .get_input_event = touchscreen_get_input_event,
};

/*
 * register touchscreen.
 */
void touchscreen_register(void) { register_input_device(&touch_dev); }

#if 0

/*
 * test.
 */
int main(int argc, char **argv) {
  input_event ievt;
  int ret;

  touch_dev.device_init();
  while (1) {
    ret = touch_dev.get_input_event(&ievt);
    if (ret) {
      printf("get input event err\n");
      return -1;
    }

    printf("type: %d\n", ievt.type);
    printf("x: %d\n", ievt.x);
    printf("y: %d\n", ievt.y);
    printf("pressure: %d\n", ievt.pressure);
    printf("time: %d\n", (int)ievt.time.tv_sec);
  }

  return 0;
}

#endif