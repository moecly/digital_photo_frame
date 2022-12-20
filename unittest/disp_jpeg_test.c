#include "common.h"
#include "pic_fmt_manger.h"
#include <config.h>
#include <disp_manger.h>
#include <fcntl.h>
#include <font_manger.h>
#include <input_manger.h>
#include <jpeglib.h>
#include <page_manger.h>
#include <pic_operation.h>
#include <render.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ui.h>
#include <unistd.h>

#if 0

int draw_jpeg(char *name) {
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE *infile;
  int row_stride;
  unsigned char *buffer;

  // 分配和初始化一个decompression结构体
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  // 指定源文件
  if ((infile = fopen(name, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", name);
    return -1;
  }
  jpeg_stdio_src(&cinfo, infile);

  // 用jpeg_read_header获得jpg信息
  jpeg_read_header(&cinfo, TRUE);
  /* 源信息 */
  // printf("image_width = %d\n", cinfo.image_width);
  // printf("image_height = %d\n", cinfo.image_height);
  // printf("num_components = %d\n", cinfo.num_components);

  // 设置解压参数,比如放大、缩小
  // printf("enter scale M/N:\n");
  // scanf("%d/%d", &cinfo.scale_num, &cinfo.scale_denom);
  // printf("scale to : %d/%d\n", cinfo.scale_num, cinfo.scale_denom);
  cinfo.scale_denom = cinfo.scale_num = 1;

  // 启动解压：jpeg_start_decompress
  jpeg_start_decompress(&cinfo);

  /* 输出的图象的信息 */
  // printf("output_width = %d\n", cinfo.output_width);
  // printf("output_height = %d\n", cinfo.output_height);
  // printf("output_components = %d\n", cinfo.output_components);

  // 一行的数据长度
  row_stride = cinfo.output_width * cinfo.output_components;
  buffer = malloc(row_stride);

  // 循环调用jpeg_read_scanlines来一行一行地获得解压的数据
  while (cinfo.output_scanline < cinfo.output_height) {
    (void)jpeg_read_scanlines(&cinfo, &buffer, 1);

    // 写到LCD去
    draw_line(0, cinfo.output_width, cinfo.output_scanline, buffer);
  }

  free(buffer);
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  return 0;
}

int main(int argc, char **argv) {
  int ret;
  disp_buff buff;
  disp_ops *fb_ops;
  region rgn;
  disp_buff pixel_data;
  disp_buff pixel_data_small;
  region_cartesian rgn_cart;
  char icon_name[128];
  extern pic_file_parser bmp_parser;

  if (argc != 2) {
    printf("input err\n");
    return -1;
  }

  /*
   * init lcd display.
   */
  display_system_register();

  fb_ops = get_display_ops_from_name("lcd");
  if (!fb_ops) {
    printf("get_display_ops_from_name err\n");
    return -1;
  }

  add_disp_queue(fb_ops);
  if (!fb_ops) {
    printf("get fb ops err\n");
    return -1;
  }

  ret = display_system_init();
  if (ret) {
    printf("display system init err\n");
    return -1;
  }

  ret = get_display_buffer(fb_ops, &buff);
  if (ret) {
    printf("get display buffer err\n");
    return -1;
  }

  /*
   * init network input and lcd input.
   */
  input_system_register();
  input_device_init();

  /*
   * init font.
   */
  fonts_system_register();
  fonts_init("simsun.ttc");
  sel_def_font("freetype");

  /*
   * init page.
   */
  alloc_video_mem(5);
  pages_system_register();

  // draw_jpeg(argv[1]);
  clean_screen(WHITE);
  pic_fmt_system_register();
  get_disp_buff_from_file(argv[1], &pixel_data);
  setup_disp_buff(&pixel_data_small, pixel_data.xres / 2, pixel_data.yres / 2,
                  buff.bpp, NULL);
  // pixel_data_small.buff = malloc(pixel_data_small.total_size);
  // pic_zoom(&pixel_data, &pixel_data_small);
  // pic_merge(0, 0, &pixel_data, &buff);

#if 0
  snprintf(icon_name, 128, "%s/%s", ICON_PATH, argv[1]);
  icon_name[127] = '\0';
  set_disp_buff_bpp(&pixel_data, buff.bpp);
  ret = get_disp_buff_for_icon(&pixel_data, icon_name);
  if (ret)
    goto err_get_pixel_data;

  setup_disp_buff(&pixel_data_small, pixel_data.xres / 2, pixel_data.yres / 2,
                  buff.bpp, NULL);
  pixel_data_small.buff = malloc(pixel_data_small.total_size);
  if (!pixel_data_small.buff)
    goto err_malloc_small;

  clean_screen(0xffffff);
  pic_zoom(&pixel_data, &pixel_data_small);
  rgn.left_up_x = rgn.left_up_y = 0;
  rgn.width = buff.xres;
  rgn.height = buff.yres;
  pic_merge(0, 0, &pixel_data, &buff);
  pic_merge(277, 177, &pixel_data_small, &buff);
  flush_display_region(&rgn, fb_ops, &buff);

  bmp_parser.free_pixel_data(&pixel_data);
  free_disp_buff_for_icon(&pixel_data_small);
#endif

  return 0;

err_malloc_small:
err_get_pixel_data:
  bmp_parser.free_pixel_data(&pixel_data);
  return -1;
}

#endif