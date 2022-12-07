#ifndef _PAGE_MANGER_H
#define _PAGE_MANGER_H

#include <ui.h>

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

typedef struct page_layout {
  int iTopLeftX; /* 这个区域的左上角、右下角坐标 */
  int iTopLeftY;
  int iBotRightX;
  int iBotRightY;
  int iBpp; /* 一个象素用多少位来表示 */
  int iMaxTotalBytes;
  button *atLayout; /* 数组: 这个区域分成好几个小区域 */
} page_layout;

void register_page(page_action *page);
int get_id(const char *name);
page_action *page(char *name);
void pages_system_register(void);
button *from_input_event_get_button_from_page_layout(page_layout *layout,
                                                     input_event *ievt);
#define ID(name) get_id(name)

#endif