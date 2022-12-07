#include <dirent.h>
#include <fcntl.h>
#include <file.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/*
 * return whether is dir.
 */
int is_dir(char *str_file_path, char *str_file_name) {
  char str_tmp[256];
  struct stat st;

  snprintf(str_tmp, 256, "%s/%s", str_file_path, str_file_name);
  str_tmp[255] = '\0';

  if ((!stat(str_tmp, &st)) && S_ISDIR(st.st_mode)) {
    return 1;
  }

  return 0;
}

/*
 * is reg file.
 */
static int is_reg_file(char *str_file_path, char *str_file_name) {
  char str_tmp[256];
  struct stat st;

  snprintf(str_tmp, 256, "%s/%s", str_file_path, str_file_name);
  str_tmp[255] = '\0';

  if (!(stat(str_tmp, &st)) && S_ISREG(st.st_mode)) {
    return 1;
  }

  return 0;
}

/*
 * get dir contents.
 */
int get_dir_contents(char *str_dir_name, dir_content ***dir_contents,
                     int *piNumber) {
  int number;
  int i, j;
  struct dirent **name_list;
  dir_content **dir_cont;

  /*
   * scan dir and get list and sort.
   */
  number = scandir(str_dir_name, &name_list, 0, alphasort);
  if (number < 0)
    goto err_scandir;

  /*
   * get dir content array.
   */
  dir_cont = malloc(sizeof(dir_content) * number - 2);
  if (!dir_cont)
    goto err_malloc_dir_cont;

  *dir_contents = dir_cont;

  for (i = 0; i < number - 2; i++) {
    dir_cont[i] = malloc(sizeof(dir_content));
    if (!dir_cont)
      goto err_malloc_dir_cont;
  }

  for (i = 0, j = 0; i < number; i++) {
    /* 忽略".",".."这两个目录 */
    if ((0 == strcmp(name_list[i]->d_name, ".")) ||
        (0 == strcmp(name_list[i]->d_name, "..")))
      continue;

    if (is_dir(str_dir_name, name_list[i]->d_name)) {
      strncpy(dir_cont[j]->str_name, name_list[i]->d_name, 256);
      dir_cont[j]->str_name[255] = '\0';
      dir_cont[j]->ftype = FILETYPE_DIR;
      free(name_list[i]);
      name_list[i] = NULL;
      j++;
    }
  }

  /* 再把常规文件挑出来存入aptDirContents */
  for (i = 0; i < number; i++) {
    if (name_list[i] == NULL)
      continue;

    /* 忽略".",".."这两个目录 */
    if ((0 == strcmp(name_list[i]->d_name, ".")) ||
        (0 == strcmp(name_list[i]->d_name, "..")))
      continue;
    /* 并不是所有的文件系统都支持d_type, 所以不能直接判断d_type */
    /* if (aptNameList[i]->d_type == DT_REG) */
    if (is_reg_file(str_dir_name, name_list[i]->d_name)) {
      strncpy(dir_cont[j]->str_name, name_list[i]->d_name, 256);
      dir_cont[j]->str_name[255] = '\0';
      dir_cont[j]->ftype = FILETYPE_FILE;
      free(name_list[i]);
      name_list[i] = NULL;
      j++;
    }
  }

  /* 释放aptDirContents中未使用的项 */
  for (i = j; i < number - 2; i++) {
    free(dir_cont[i]);
  }

  /* 释放scandir函数分配的内存 */
  for (i = 0; i < number; i++) {
    if (name_list[i]) {
      free(name_list[i]);
    }
  }

  free(name_list);
  *piNumber = j;

  return 0;

err_malloc_dir_cont:
err_scandir:
  return -1;
}

/*
 * free dir contents.
 */
int free_dir_contents(dir_content *dir_content, int number) {
  int i;

  for (i = 0; i < number; i++)
    free(&dir_content[i]);

  free(dir_content);

  return 0;
}