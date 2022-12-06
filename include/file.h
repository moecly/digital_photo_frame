#ifndef _FILE_H
#define _FILE_H

typedef struct file_map {
  int fd;
  char *file_name;
  int file_size;
  unsigned char *file_map_mem;
} file_map;

int map_file(file_map *file, char *file_name);
int unmap_file(file_map *file);

#endif // !_FILE_H
