#include <font_manger.h>
#include <stdio.h>
#include <string.h>

extern void freetype_register(void);

font_ops *ft_ops;
font_ops *def_ft;

/*
 * register font ops.
 */
void register_font(font_ops *fops) {
  fops->next = ft_ops;
  ft_ops = fops;
}

/*
 * set font size.
 */
int set_font_size(unsigned int size) { return def_ft->set_font_size(size); }

/*
 * get font bit map.
 */
int get_font_bit_map(unsigned int code, font_bit_map *fb_map) {
  return def_ft->get_font_bit_map(code, fb_map);
}

/*
 * select default font.
 */
int sel_def_font(char *name) {
  font_ops *tmp = ft_ops;

  while (tmp) {
    if (strcmp(tmp->name, name) == 0) {
      def_ft = tmp;
      return 0;
    }
    tmp = tmp->next;
  }

  return -1;
}

/*
 * register all font.
 */
void fonts_system_register(void) { freetype_register(); }

/*
 * all font init.
 */
int fonts_init(char *font_name) {
  int ret;
  font_ops *tmp = ft_ops;

  while (tmp) {
    ret = tmp->font_init(font_name);
    if (ret) {
      return -1;
    }

    tmp = tmp->next;
  }

  return 0;
}

/*
 * get string region car.
 */
int get_string_region_car(char *str, region_cartesian *rgn) {
  return def_ft->get_string_region_car(str, rgn);
}
