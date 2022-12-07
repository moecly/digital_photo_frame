#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h> /* See NOTES */
#include <unistd.h>

#include <input_manger.h>

extern void net_device_register(void);
extern void touchscreen_register(void);

#define BUFFER_SIZE 20
#define NEXT_VAL(x) (x + 1) % BUFFER_SIZE

static input_device *r_idev;
static input_device *def_idev;
static pthread_mutex_t pmutex;
static pthread_cond_t pcond;

static int read_cnt = 0;
static int write_cnt = 0;
static input_event all_ievt[BUFFER_SIZE];

/*
 * save input event.
 */
static void put_input_event_to_buffer(input_event *ievt) {
  all_ievt[write_cnt] = *ievt;
  write_cnt = NEXT_VAL(write_cnt);
}

/*
 * Judge whether the buffer has input event.
 */
static int input_event_is_empty(void) { return read_cnt == write_cnt; }

/*
 * get input event.
 */
static int get_input_event_from_buffer(input_event *ievt) {
  if (input_event_is_empty()) {
    return -1;
  }

  *ievt = all_ievt[read_cnt];
  read_cnt = NEXT_VAL(read_cnt);

  return 0;
}

/*
 * register input device.
 */
void register_input_device(input_device *idev) {
  idev->next = r_idev;
  r_idev = idev;
}

/*
 * select device from name.
 */
int select_input_device(char *name) {
  input_device *tmp = r_idev;
  while (tmp) {
    if (strcmp(name, tmp->name) == 0) {
      def_idev = tmp;
      return 0;
    }
    tmp = tmp->next;
  }
  return -1;
}

/*
 * thread recv data.
 */
void *input_recv_thread_func(void *data) {
  input_device *tmp = (input_device *)data;
  input_event ievt;
  int ret;

  while (1) {
    ret = tmp->get_input_event(&ievt);
    if (!ret) {
      /*
       * add to buffer.
       */
      pthread_mutex_lock(&pmutex);
      put_input_event_to_buffer(&ievt);

      /*
       * wake up thread.
       */
      pthread_cond_signal(&pcond);
      pthread_mutex_unlock(&pmutex);
    }
  }
}

/*
 * input init.
 */
void input_system_register(void) {
  touchscreen_register();
  net_device_register();
}

/*
 * init input device.
 */
void input_device_init(void) {
  int ret;
  input_device *tmp = r_idev;
  pthread_t tid;

  while (tmp) {
    ret = tmp->device_init();
    if (ret) {
      continue;
    }

    ret = pthread_create(&tid, NULL, input_recv_thread_func, tmp);
    tmp = tmp->next;
  }
}

/*
 * init default device.
 * need to use int "select_input_device(char *name)" to select default device.
 */
int def_input_dev_init(void) {
  int ret;

  ret = def_idev->device_init();
  if (ret) {
    return -1;
  }

  return 0;
}

/*
 * get input event.
 */
int get_input_event(input_event *ievt) {
  int ret;
  int res = 0;

  pthread_mutex_lock(&pmutex);
  ret = get_input_event_from_buffer(ievt);
  if (ret) {
    pthread_cond_wait(&pcond, &pmutex);
    ret = get_input_event_from_buffer(ievt);

    if (ret) {
      res = -1;
    } else {
      res = 0;
    }
  }

  pthread_mutex_unlock(&pmutex);

  return res;
}

/*
 * get lcd input event.
 */
int lcd_page_get_input_event(input_event *ievt) {
  int ret;

  /*
   * filter event.
   */
  ret = get_input_event(ievt);

  if (ret)
    goto err_get_input_event;

  if (ievt->type != INPUT_TYPE_TOUCH)
    goto err_input_event;

  return 0;

err_input_event:
err_get_input_event:
  return -1;
}