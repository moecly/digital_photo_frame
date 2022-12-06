#include <fcntl.h>
#include <file.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * get file map.
 */
int map_file(file_map *file, char *file_name) {
  struct stat bmp_stat;
  file->fd = open(file_name, O_RDWR);
  if (file->fd < 0)
    goto err_open;

  fstat(file->fd, &bmp_stat);
  file->file_map_mem = (unsigned char *)mmap(
      NULL, bmp_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file->fd, 0);

  if (file->file_map_mem == (unsigned char *)-1)
    goto err_mmap;

  file->file_name = file_name;
  file->file_size = bmp_stat.st_size;

  return 0;

err_mmap:
  close(file->fd);
err_open:
  return -1;
}

/*
 * unmap file.
 */
int unmap_file(file_map *file) {
  close(file->fd);
  munmap(file->file_map_mem, file->file_size);
  return 0;
}