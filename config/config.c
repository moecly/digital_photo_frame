#include <config.h>
#include <stdio.h>
#include <string.h>

static item_cfg item[ITEM_CFG_NUM];
static int item_len;

/*
 * parse config file.
 */
int parse_config_file() {
  FILE *fp;
  char buf[100];
  char *p = buf;

  fp = fopen(CFG_FILE, "r");
  if (!fp) {
    return -1;
  }

  while (fgets(buf, 100, fp)) {
    buf[99] = '\0';
    p = buf;
    while (*p == ' ' || *p == '\t')
      p++;

    if (*p == '#')
      continue;
    
    item[item_len].command[0] = '\0';
    item[item_len].index = item_len;
    sscanf(p, "%s %d %s", item[item_len].name, &item[item_len].can_be_touch,
           item[item_len].command);
    item_len++;
  }

  return 0;
}

/*
 * get item config count.
 */
int get_item_cfg_count(void) { return item_len; }

/*
 * get item config by index.
 */
item_cfg *get_item_cfg_by_index(int index) {
  if (index > item_len) {
    return NULL;
  }

  return &item[index];
}

/*
 * get item cfg by name.
 */
item_cfg *get_item_cfg_by_name(char *name) {
  int i = 0;
  for (; i < item_len; i++) {
    if (strcmp(item[i].name, name) == 0) {
      return &item[i];
    }
  }

  return NULL;
}