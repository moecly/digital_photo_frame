#ifndef _PIC_FMT_MANGER_H
#define _PIC_FMT_MANGER_H

#include <pic_operation.h>

void register_pic_file_parser(pic_file_parser *parser);
pic_file_parser *parser(char *name);
pic_file_parser *get_parser(file_map *file_map);
int pic_fmt_system_register(void);


#endif // !_PIC_FMT_MANGER_H
