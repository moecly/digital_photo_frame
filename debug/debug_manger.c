#include <debug_manger.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static debug_opr *debug;
static int print_level = 8;

static int parse_level(char *level) {
  int ret = DEFAULT_DBGLEVEL;
  if (level[0] == '<' && level[2] == '>') {
    ret = level[1] - '0';
    if (ret >= 0 && ret <= 9) {
    } else
      ret = DEFAULT_DBGLEVEL;
  }
  return ret;
}

/*
 * register debug register.
 */
int debug_register(debug_opr *dg_opr) {
  dg_opr->next = debug;
  debug = dg_opr;
  return 0;
}

/*
 * debug system register.
 */
int debug_system_register(void) { return 0; }

/*
 * set print level.
 */
void set_print_level(char *level) { print_level = parse_level(level); }

/*
 * get debug opr from name.
 */
static debug_opr *get_debug_opr_from_name(char *name) {
  debug_opr *tmp = debug;

  while (tmp) {
    if (strcmp(tmp->name, name) == 0)
      return tmp;
    tmp = tmp->next;
  }

  return NULL;
}

/*
 * debug init.
 */
int debug_init(void) {
  debug_opr *tmp = debug;

  while (tmp) {
    if (tmp->be_use && tmp->debug_init)
      tmp->debug_init();
    tmp = tmp->next;
  }

  return 0;
}

/*
 * debug print.
 */
void debug_print(const char *format, ...) {
  int num;
  va_list tArg;
  char str_buf[1000];
  char *str_tmp = str_buf;
  debug_opr *dbg_tmp = debug;
  int dbglevel = DEFAULT_DBGLEVEL;

  va_start(tArg, format);
  num = vsprintf(str_buf, format, tArg);
  va_end(tArg);
  str_buf[num] = '\0';

  if (str_buf[0] == '<' && str_buf[2] == '>') {
    dbglevel = str_buf[1] - '0';
    if (dbglevel >= 0 && dbglevel <= 9)
      str_tmp += 3;
    else
      dbglevel = DEFAULT_DBGLEVEL;
  }

  while (dbg_tmp) {
    if (dbg_tmp->be_use)
      dbg_tmp->debug_print(str_tmp);
    dbg_tmp = dbg_tmp->next;
  }
}

/*
 * set debug channel.
 */
int set_debug_channel(char *str) {
  char name[100];
  char *str_tmp;
  debug_opr *dgb_tmp = debug;

  str_tmp = strchr(str, '=');
  if (!str_tmp)
    return -1;
  else {
    strncpy(name, str, str_tmp - str);
    name[str_tmp - str] = '\0';
    dgb_tmp = get_debug_opr_from_name(name);
    if (!dgb_tmp)
      return -1;

    if (str_tmp[1] == '0')
      dgb_tmp->be_use = 0;
    else
      dgb_tmp->be_use = 1;
  }

  return 0;
}