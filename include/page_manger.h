#ifndef _PAGE_MANGER_H
#define _PAGE_MANGER_H

#include "ui.h"
#ifndef NULL
#define NULL (void *)0
#endif

typedef struct page_action {
  char *name;
  int (*run)(void *params);
  int (*prepare)(void);
  // int (*get_input_event)(pic_layout *pic);
  struct page_action *next;
} page_action;

void register_page(page_action *page);
int get_id(const char *name);
page_action *page(char *name);
void pages_system_register(void);

#define ID(name) get_id(name)

#endif