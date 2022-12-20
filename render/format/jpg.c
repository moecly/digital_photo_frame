#include "disp_manger.h"
#include "file.h"
#include "render.h"
#include <config.h>
#include <jpeglib.h>
#include <pic_fmt_manger.h>
#include <pic_operation.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push)
#pragma pack(1)
#pragma pack(pop)

typedef struct err_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
} err_mgr;

static void error_exit(j_common_ptr ptCInfo) {
  static char err_str[JMSG_LENGTH_MAX];

  err_mgr *my_err = (err_mgr *)ptCInfo->err;

  /* Create the message */
  (*ptCInfo->err->format_message)(ptCInfo, err_str);

  longjmp(my_err->setjmp_buffer, 1);
}

/*
 * whether support jpg.
 */
static int jpg_is_support(file_map *file) {
  struct jpeg_decompress_struct tDInfo;

  /* 默认的错误处理函数是让程序退出
   * 我们参考libjpeg里的bmp.c编写自己的错误处理函数
   */
  // struct jpeg_error_mgr tJErr;
  err_mgr jerr;
  int iRet;

  fseek(file->file, 0, SEEK_SET);

  // 分配和初始化一个decompression结构体
  // tDInfo.err = jpeg_std_error(&tJErr);
  tDInfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = error_exit;

  if (setjmp(jerr.setjmp_buffer)) {
    /* 如果程序能运行到这里, 表示JPEG解码出错 */
    jpeg_destroy_decompress(&tDInfo);
    return 0;
  }

  jpeg_create_decompress(&tDInfo);

  // 用jpeg_read_header获得jpg信息
  jpeg_stdio_src(&tDInfo, file->file);

  iRet = jpeg_read_header(&tDInfo, TRUE);
  jpeg_abort_decompress(&tDInfo);

  return (iRet == JPEG_HEADER_OK);
}

/*
 * bpp covert.
 */
static int covert_one_line_from_jpg(int width, int src_bpp, int dst_bpp,
                                    unsigned char *puc_src_data,
                                    unsigned char *pud_dst_data) {
  unsigned int red;
  unsigned int green;
  unsigned int blue;
  unsigned int color;
  int i;
  int pos = 0;
  unsigned short *dst_data_16_bpp = (unsigned short *)pud_dst_data;
  unsigned int *dst_data_32_bpp = (unsigned int *)pud_dst_data;

  if (src_bpp != 24)
    goto err_bpp;

  if (dst_bpp == 24) {
    memcpy(pud_dst_data, puc_src_data, width * 3);
  } else {
    for (i = 0; i < width; i++) {
      red = puc_src_data[pos++];
      green = puc_src_data[pos++];
      blue = puc_src_data[pos++];
      if (dst_bpp == 32) {
        color = (red << 16 | green << 8) | blue;
        *dst_data_32_bpp = color;
        dst_data_32_bpp++;
      } else if (dst_bpp == 16) {
        blue = blue >> 3;
        green = green >> 2;
        red = red >> 3;
        color = (red << 11 | green << 5) | blue;
        *dst_data_16_bpp = color;
        dst_data_16_bpp++;
      }
    }
  }

  return 0;

err_bpp:
  return -1;
}

/*
 * jpg get pixel data.
 * first set th buff->bpp val.
 */
static int jpg_get_pixel_data(file_map *map, disp_buff *buff) {
  struct jpeg_decompress_struct tDInfo;
  int iRowStride;
  unsigned char *aucLineBuffer = NULL;
  unsigned char *pucDest;
  err_mgr jerr;

  fseek(map->file, 0, SEEK_SET);

  // 分配和初始化一个decompression结构体
  // tDInfo.err = jpeg_std_error(&tJErr);

  tDInfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = error_exit;

  if (setjmp(jerr.setjmp_buffer)) {
    /* 如果程序能运行到这里, 表示JPEG解码出错 */
    jpeg_destroy_decompress(&tDInfo);
    if (aucLineBuffer) {
      free(aucLineBuffer);
    }
    if (buff->buff) {
      free(buff->buff);
    }
    return -1;
  }

  jpeg_create_decompress(&tDInfo);

  // 用jpeg_read_header获得jpg信息
  jpeg_stdio_src(&tDInfo, map->file);

  jpeg_read_header(&tDInfo, TRUE);

  // 设置解压参数,比如放大、缩小
  tDInfo.scale_num = tDInfo.scale_denom = 1;

  // 启动解压：jpeg_start_decompress
  jpeg_start_decompress(&tDInfo);

  // 一行的数据长度
  iRowStride = tDInfo.output_width * tDInfo.output_components;
  aucLineBuffer = malloc(iRowStride);

  if (NULL == aucLineBuffer) {
    return -1;
  }

  buff->xres = tDInfo.output_width;
  buff->yres = tDInfo.output_height;
  // ptPixelDatas->iBpp    = iBpp;
  buff->line_byte = buff->xres * buff->bpp / 8;
  buff->total_size = buff->yres * buff->line_byte;
  buff->buff = malloc(buff->total_size);
  if (NULL == buff->buff) {
    return -1;
  }

  pucDest = buff->buff;

  // 循环调用jpeg_read_scanlines来一行一行地获得解压的数据
  while (tDInfo.output_scanline < tDInfo.output_height) {
    /* 得到一行数据,里面的颜色格式为0xRR, 0xGG, 0xBB */
    (void)jpeg_read_scanlines(&tDInfo, &aucLineBuffer, 1);

    // 转到ptPixelDatas去
    covert_one_line_from_jpg(buff->xres, 24, buff->bpp, aucLineBuffer, pucDest);
    pucDest += buff->line_byte;
  }

  free(aucLineBuffer);
  jpeg_finish_decompress(&tDInfo);
  jpeg_destroy_decompress(&tDInfo);

  return 0;
}

/*
 * jpg free pixel data.
 */
static int jpg_free_pixel_data(disp_buff *buff) {
  free(buff->buff);
  return 0;
}

static pic_file_parser jpg_parser = {
    .name = "jpg",
    .free_pixel_data = jpg_free_pixel_data,
    .get_pixel_data = jpg_get_pixel_data,
    .is_support = jpg_is_support,
};

/*
 * register jpg parser.
 */
void jpg_parser_register(void) {
  /*
   * register jpg parser.
   */
  register_pic_file_parser(&jpg_parser);
}