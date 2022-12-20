#include "file.h"
#include "input_manger.h"
#include "page_manger.h"
#include "unistd.h"
#include <config.h>
#include <disp_manger.h>
#include <pthread.h>
#include <render.h>
#include <stdio.h>
#include <string.h>
#include <ui.h>

static pthread_t auto_play_thread_id;
static pthread_mutex_t auto_play_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
static int auto_play_thread_should_exit = 0;

static page_params page_pms;
static page_cfg page_browse_cfg;

/* 以深度优先的方式获得目录下的文件
 * 即: 先获得顶层目录下的文件, 再进入一级子目录A
 *     先获得一级子目录A下的文件, 再进入二级子目录AA, ...
 *     处理完一级子目录A后, 再进入一级子目录B
 *
 * "连播模式"下调用该函数获得要显示的文件
 * 有两种方法获得这些文件:
 * 1. 事先只需要调用一次函数,把所有文件的名字保存到某个缓冲区中
 * 2. 要使用文件时再调用函数,只保存当前要使用的文件的名字
 * 第1种方法比较简单,但是当文件很多时有可能导致内存不足.
 * 我们使用第2种方法:
 * 假设某目录(包括所有子目录)下所有的文件都给它编一个号
 * g_iStartNumberToRecord : 从第几个文件开始取出它们的名字
 * g_iCurFileNumber       : 本次函数执行时读到的第1个文件的编号
 * g_iFileCountHaveGet    : 已经得到了多少个文件的名字
 * g_iFileCountTotal      : 每一次总共要取出多少个文件的名字
 * g_iNextProcessFileIndex: 在g_apstrFileNames数组中即将要显示在LCD上的文件
 *
 */
#define FILE_COUNT 10
static int start_number_to_record = 0;
static int cur_file_number = 0;
static int file_count_have_get = 0;
static int file_count_total = 0;
static int next_process_file_index = 0;
static char file_names[FILE_COUNT][256];

/*
 * get next auto play file.
 */
static int get_next_auto_play_file(char *file_name) {
  int ret;
  /*
   * if have next file.
   */
  if (next_process_file_index < file_count_have_get) {
    strncpy(file_name, file_names[next_process_file_index], 256);
    next_process_file_index++;
    goto done;
  } else {
    cur_file_number = 0;
    file_count_have_get = 0;
    file_count_total = FILE_COUNT;
    next_process_file_index = 0;

    /*
     * get files index.
     */
    ret = get_files_indir(page_browse_cfg.select_dir, &start_number_to_record,
                          &cur_file_number, &file_count_have_get,
                          file_count_total, file_names);

    if (ret || next_process_file_index >= file_count_have_get) {
      start_number_to_record = 0;
      cur_file_number = 0;
      file_count_have_get = 0;
      file_count_total = FILE_COUNT;
      next_process_file_index = 0;
      ret = get_files_indir(page_browse_cfg.select_dir, &start_number_to_record,
                            &cur_file_number, &file_count_have_get,
                            file_count_total, file_names);
    }

    /*
     * if get success.
     */
    if (!ret) {
      if (next_process_file_index < file_count_have_get) {
        strncpy(file_name, file_names[next_process_file_index], 256);
        next_process_file_index++;
        goto done;
      }
    }
  }

  return -1;

done:
  return 0;
}

/*
 * 0 is necessary to return video mem.
 * 1 is must to return video mem.
 */
static video_mem *prepare_next_picture(int cur) {
  int ret;
  float k;
  disp_buff origin_icon_pixel_datas;
  disp_buff pic_pixel_datas;
  video_mem *vd_mem;
  char file_name[256];
  disp_ops *fb_ops;
  disp_buff fb_buff;
  int xres, yres, bpp;
  int top_left_x;
  int top_left_y;

  /*
   * get video mem.
   */
  vd_mem = get_video_mem(-1, cur);
  if (!vd_mem)
    goto err_get_video_mem;

  /*
   * set video mem.
   */
  clean_screen_from_vd(BACKGROUND, vd_mem);
  for (;;) {
    ret = get_next_auto_play_file(file_name);
    if (ret)
      goto err_get_next_auto_play_file;

    /*
     * get display buff from file.
     */
    ret = get_disp_buff_from_file(file_name, &origin_icon_pixel_datas);
    if (!ret)
      break;
  }

  /* 把图片按比例缩放到VideoMem上, 居中显示
   * 1. 先算出缩放后的大小
   */
  fb_ops = get_display_ops_from_name(LCD_NAME);
  get_display_buffer(fb_ops, &fb_buff);
  xres = fb_buff.xres;
  yres = fb_buff.yres;
  bpp = fb_buff.bpp;

  k = (float)origin_icon_pixel_datas.yres / origin_icon_pixel_datas.xres;
  pic_pixel_datas.xres = xres;
  pic_pixel_datas.yres = xres * k;
  if (pic_pixel_datas.yres > yres) {
    pic_pixel_datas.xres = yres / k;
    pic_pixel_datas.yres = yres;
  }
  pic_pixel_datas.bpp = bpp;
  pic_pixel_datas.pixel_width = bpp / 8;
  pic_pixel_datas.line_byte = pic_pixel_datas.xres * pic_pixel_datas.bpp / 8;
  pic_pixel_datas.total_size = pic_pixel_datas.line_byte * pic_pixel_datas.yres;
  pic_pixel_datas.buff = malloc(pic_pixel_datas.total_size);
  if (pic_pixel_datas.buff == NULL)
    goto err_malloc;

  /* 2. 再进行缩放 */
  pic_zoom(&origin_icon_pixel_datas, &pic_pixel_datas);

  /* 3. 接着算出居中显示时左上角坐标 */
  top_left_x = (xres - pic_pixel_datas.xres) / 2;
  top_left_y = (yres - pic_pixel_datas.yres) / 2;

  /* 4. 最后把得到的图片合并入VideoMem */
  pic_merge(top_left_x, top_left_y, &pic_pixel_datas, &vd_mem->disp_buff);

  /* 5. 释放图片原始数据 */
  free_disp_buff_for_icon(&origin_icon_pixel_datas);

  /* 6. 释放缩放后的数据 */
  free_disp_buff_for_icon(&pic_pixel_datas);

  put_video_mem(vd_mem);

  return vd_mem;

err_malloc:
err_get_next_auto_play_file:
  put_video_mem(vd_mem);
err_get_video_mem:
  return NULL;
}

/*
 * auto play picture thread function.
 */
static void *auto_play_thread_function(void *args) {
  int exit;
  int first = 1;
  int sec = 0;
  video_mem *vd_mem;

  for (;;) {
    /*
     * prepare next picture.
     */
    vd_mem = prepare_next_picture(0);

    if (!first) {
      while (sec < page_browse_cfg.interval_second) {
        /*
         * is exit.
         */
        pthread_mutex_lock(&auto_play_thread_mutex);
        exit = auto_play_thread_should_exit;
        pthread_mutex_unlock(&auto_play_thread_mutex);
        if (exit)
          goto done;

        sleep(1);
        sec++;
      }
    }
    sec = 0;
    first = 0;

    if (!vd_mem)
      vd_mem = prepare_next_picture(1);

    /*
     * flush to dev.
     */
    flush_video_mem_to_dev(vd_mem);

    /*
     * put video mem.
     */
    put_video_mem(vd_mem);
  }

done:
  return NULL;
}

/*
 * auto page.
 */
static int auto_page_run(void *params) {
  input_event ievt;
  page_pms.page_id = ID("auto");
  auto_play_thread_should_exit = 0;
  start_number_to_record = 0;
  cur_file_number = 0;
  file_count_have_get = 0;
  file_count_total = 0;
  next_process_file_index = 0;
  page_params *pms = (page_params *)params;
  char *tmp;

  /*
   * get config data.
   */
  get_browse_page_cfg(&page_browse_cfg);
  if (pms->str_cur_picture_file[0] != '\0') {
    strcpy(page_browse_cfg.select_dir, pms->str_cur_picture_file);
    tmp = strrchr(page_browse_cfg.select_dir, '/');
    *tmp = '\0';
  }

  /*
   * display interface.
   */
  pthread_create(&auto_play_thread_id, NULL, auto_play_thread_function, NULL);

  /*
   * input processing.
   */
  for (;;) {
    get_input_event(&ievt);
    /*
     * is exit.
     */
    if (ievt.type == INPUT_TYPE_TOUCH) {
      pthread_mutex_lock(&auto_play_thread_mutex);
      auto_play_thread_should_exit = 1;
      pthread_mutex_unlock(&auto_play_thread_mutex);
      pthread_join(auto_play_thread_id, NULL);
      goto done;
    }
  }

done:
  return 0;
}

/*
 * pre pare.
 */
static int auto_page_prepare(void) { return 0; }

static page_action auto_page_atn = {
    .name = "auto",
    .run = auto_page_run,
    .prepare = auto_page_prepare,
};

/*
 * register auto page.
 */
void auto_page_register(void) { register_page(&auto_page_atn); }
