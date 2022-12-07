#ifndef _CONFIG_H
#define _CONFIG_H

#define ITEM_CFG_NUM 30
#define CFG_FILE "/etc/test_gui/gui.conf"
#define FB_DEV_NAME "/dev/fb0"
#define ICON_PATH "/etc/digital/icons"
#define DEFAULT_DIR "/"
#define LCD_NAME "lcd"

#define BLACK 0x000000
#define white 0xffffff

#ifndef NULL
#define NULL (void *)0
#endif

typedef struct item_cfg {
  int index;
  char name[100];
  int can_be_touch;
  char command[100];
} item_cfg;

int parse_config_file();
int get_item_cfg_count(void);
item_cfg *get_item_cfg_by_index(int index);
item_cfg *get_item_cfg_by_name(char *name);

#endif // !_CONFIG_H