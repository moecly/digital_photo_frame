#include "file.h"
#include <config.h>
#include <pic_fmt_manger.h>
#include <string.h>

static pic_file_parser *pic_parser;

extern void bmp_parser_register(void);
extern void jpg_parser_register(void);

/*
 * register pic file parser.
 */
void register_pic_file_parser(pic_file_parser *parser) {
  parser->next = pic_parser;
  pic_parser = parser;
}

/*
 * get parser from name.
 */
pic_file_parser *parser(char *name) {
  pic_file_parser *tmp;

  for_each_linked_node(pic_parser, tmp) {
    if (!strcmp(tmp->name, name))
      return tmp;
  }

  return NULL;
}

/*
 * get parser from format.
 */
pic_file_parser *get_parser(file_map *file_map) {
  pic_file_parser *tmp;

  for_each_linked_node(pic_parser, tmp) {
    if (tmp->is_support(file_map))
      return tmp;
  }

  return NULL;
}

/*
 * register pic fmt system.
 */
int pic_fmt_system_register(void) {
  bmp_parser_register();
  jpg_parser_register();
  return 0;
}
