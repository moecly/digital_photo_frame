#include "ui.h"
#include <page_manger.h>
#include <string.h>

extern void main_page_register(void);
extern void browse_page_register(void);

page_action *page_atn;

/*
 * register page action.
 */
void register_page(page_action *page) {
  page->next = page_atn;
  page_atn = page;
}

/*
 * get id.
 */
int get_id(const char *name) {
  return (int)(name[0]) + (int)(name[1]) + (int)(name[2]) + (int)(name[3]);
}

/*
 * select page by name.
 */
page_action *page(char *name) {
  page_action *tmp = page_atn;

  while (tmp) {
    if (strcmp(tmp->name, name) == 0) {
      return tmp;
    }
    tmp = tmp->next;
  }

  return NULL;
}

/*
 * display page.
 */
void pages_system_register(void) {
  main_page_register();
  browse_page_register();
}

/*
 * from input event get file or dir.
 */
button *from_input_event_get_button_from_page_layout(page_layout *layout,
                                                     input_event *ievt) {
  return from_input_event_get_btn(layout->atLayout, ievt);
  // button *btn = layout->atLayout;
  // while (btn->pic.rgn.width) {
  //   printf("test\n");
  //   btn++;
  // }
}
