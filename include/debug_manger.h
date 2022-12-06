#ifndef _DEBUG_MANGER_H
#define _DEBUG_MANGER_H

#ifndef NULL
#define NULL (void *)0
#endif // !NULL

/* 信息的调试级别,数值起小级别越高 */
#define APP_EMERG "<0>"   /* system is unusable			*/
#define APP_ALERT "<1>"   /* action must be taken immediately	*/
#define APP_CRIT "<2>"    /* critical conditions			*/
#define APP_ERR "<3>"     /* error conditions			*/
#define APP_WARNING "<4>" /* warning conditions			*/
#define APP_NOTICE "<5>"  /* normal but significant condition	*/
#define APP_INFO "<6>"    /* informational			*/
#define APP_DEBUG "<7>"   /* debug-level messages			*/

#define DEFAULT_DBGLEVEL 4

typedef struct debug_opr {
  char *name;
  int (*debug_init)(void);
  int (*debug_exit)(void);
  int (*debug_print)(char *str);
  int be_use;
  struct debug_opr *next;
} debug_opr;

#endif // !_DEBUG_MANGER_H