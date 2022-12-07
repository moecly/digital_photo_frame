#ifndef _FILE_H
#define _FILE_H

typedef struct file_map {
  int fd;
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

#endif // !_FILE_H
