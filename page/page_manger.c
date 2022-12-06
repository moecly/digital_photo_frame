#include <page_manger.h>
#include <string.h>

extern void main_page_register(void);

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
void pages_system_register(void) { main_page_register(); }