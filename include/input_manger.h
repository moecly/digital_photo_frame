#ifndef _INPUT_MANGER_H
#define _INPUT_MANGER_H


#ifndef NULL
#define NULL (void *)0
#endif

#define INPUT_TYPE_TOUCH 0
#define INPUT_TYPE_NET 1

#include <sys/time.h>

typedef struct input_event {
  struct timeval time;
  int x;
  int y;
  int type;
  int pressure;
  char str[1024];
} input_event;

typedef struct input_device {
  char *name;
  int (*get_input_event)(input_event *);
  int (*device_init)(void);
  void (*device_exit)(void);
  struct input_device *next;
} input_device;


void register_input_device(input_device *idev);
int select_input_device(char *name);
void input_system_register(void);
int def_input_dev_init(void);
int get_input_event(input_event *ievt);
void input_device_init(void);

#endif