#ifndef _PAGE_MANGER_H
#define _PAGE_MANGER_H

#ifndef NULL
#define NULL (void *)0
#endif

#define ID(name) (int(name[0]) + int(name[1]) + int(name[2]) + int(name[3]))

typedef struct page_action {
  char *name;
  int (*run)(void *params);
  int (*prepare)(void);
  struct page_action *next;
} page_action;

void register_page(page_action *page);
page_action *page(char *name);
void pages_system_register(void);

#endif