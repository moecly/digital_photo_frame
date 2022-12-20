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
  FILE *fp;
  int fd;

  fp = fopen(file_name, "r+");
  if (fp == NULL)
    goto err_open;

  fd = fileno(fp);
  fstat(fd, &bmp_stat);
  file->file_name = file_name;
  file->file_size = bmp_stat.st_size;
  file->file = fp;

  file->file_map_mem = (unsigned char *)mmap(
      NULL, file->file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (file->file_map_mem == (unsigned char *)-1)
    goto err_mmap;

  return 0;

err_mmap:
  fclose(fp);
err_open:
  return -1;
}

/*
 * unmap file.
 */
int unmap_file(file_map *file) {
  munmap(file->file_map_mem, file->file_size);
  fclose(file->file);
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
 * is reg dir.
 */
static int is_reg_dir(char *str_file_path, char *str_file_name) {
  static const char *specail_dirs[] = {"sbin", "bin", "usr", "lib", "proc",
                                       "tmp",  "dev", "sys", NULL};
  int i = 0;
  /* 如果目录名含有"astrSpecailDirs"中的任意一个, 则返回0 */
  if (0 == strcmp(str_file_path, "/")) {
    while (specail_dirs[i]) {
      if (0 == strcmp(str_file_name, specail_dirs[i]))
        return 0;
      i++;
    }
  }
  return 1;
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
int free_dir_contents(dir_content **dir_content, int number) {
  int i;

  for (i = 0; i < number; i++)
    free(dir_content[i]);

  free(dir_content);

  return 0;
}

/*
 * get files indir.
 */
int get_files_indir(char *dir_name, int *start_number_to_record,
                    int *cur_file_number, int *file_count_have_get,
                    int file_count_total, char file_names[][256]) {
  int ret;
  int dir_number;
  int i;
  dir_content **dir_cont;
  char sub_dir_name[256];
  static int dir_deepness = 0;
#define MAX_DIR_DEEPNESS 10

  if (dir_deepness > MAX_DIR_DEEPNESS)
    goto err_dir_deepness;

  dir_deepness++;
  /*
   * get dir contents.
   */
  ret = get_dir_contents(dir_name, &dir_cont, &dir_number);
  if (ret)
    goto err_get_dir_contents;

  /* 先记录文件 */
  for (i = 0; i < dir_number; i++) {
    if (dir_cont[i]->ftype == FILETYPE_FILE) {
      if (*cur_file_number >= *start_number_to_record) {
        snprintf(file_names[*file_count_have_get], 256, "%s/%s", dir_name,
                 dir_cont[i]->str_name);
        (*file_count_have_get)++;
        (*cur_file_number)++;
        (*start_number_to_record)++;
        if (*file_count_have_get >= file_count_total) {
          free_dir_contents(dir_cont, dir_number);
          dir_deepness--;
          goto done;
        }
      } else {
        (*cur_file_number)++;
      }
    }
  }

  /* 递归处理目录 */
  for (i = 0; i < dir_number; i++) {
    if ((dir_cont[i]->ftype == FILETYPE_DIR) &&
        is_reg_dir(dir_name, dir_cont[i]->str_name)) {
      snprintf(sub_dir_name, 256, "%s/%s", dir_name, dir_cont[i]->str_name);
      get_files_indir(sub_dir_name, start_number_to_record, cur_file_number,
                      file_count_have_get, file_count_total, file_names);
      if (*file_count_have_get >= file_count_total) {
        free_dir_contents(dir_cont, dir_number);
        dir_deepness--;
        goto done;
      }
    }
  }

  free_dir_contents(dir_cont, dir_number);
  dir_deepness--;

done:
  return 0;

err_get_dir_contents:
  dir_deepness--;
err_dir_deepness:
  return -1;
}