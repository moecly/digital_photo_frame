#ifndef _FILE_H
#define _FILE_H

#include <stdio.h>
typedef struct file_map {
  // int fd;
  FILE *file;
  char *file_name;
  int file_size;
  unsigned char *file_map_mem;
} file_map;

typedef enum {
  FILETYPE_DIR = 0,
  FILETYPE_FILE,
} file_type;

typedef struct dir_content {
  char str_name[256];
  file_type ftype;
} dir_content;

int map_file(file_map *file, char *file_name);
int unmap_file(file_map *file);
int get_dir_contents(char *str_dir_name, dir_content ***dir_contents,
                     int *piNumber);
int free_dir_contents(dir_content **dir_content, int number);
int get_files_indir(char *dir_name, int *start_number_to_record,
                    int *cur_file_number, int *file_count_have_get,
                    int file_count_total, char file_names[][256]);

#endif // !_FILE_H
