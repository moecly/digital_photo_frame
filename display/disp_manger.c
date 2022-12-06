#include "common.h"
#include "font_manger.h"
#include "input_manger.h"
#include <config.h>
#include <disp_manger.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static disp_ops *r_dops;
static disp_ops *dsp_ops;
static video_mem *video;
// static disp_ops *disp_dft;
// static disp_buff dp_buff;
// static int line_width;
// static int pixel_width;

extern void frame_buffer_register(void);

/*
 * get display queue.
 */
disp_ops *get_disp_queue(void) { return dsp_ops; }

/*
 * add display queue.
 */
int add_disp_queue(disp_ops *dp_ops) {
  dp_ops->next = dsp_ops;
  dsp_ops = dp_ops;
  return 0;
}

/*
 * flush all video mem to dev.
 */
int flush_video_mem_to_dev(video_mem *vd_mem) {
  int ret = 1;
  disp_ops *dp_ops;
  for_each_linked_node(dsp_ops, dp_ops) {
    ret = flush_one_video_mem_to_dev(dp_ops, vd_mem);
    if (ret)
      return -1;
  }
  return 0;
}

/*
 * flush video mem to dev.
 */
int flush_one_video_mem_to_dev(disp_ops *dp_ops, video_mem *vd_mem) {
  return dp_ops->show_page(&vd_mem->disp_buff);
}

/*
 * get video mem.
 */
video_mem *get_video_mem(int id, int cur) {
  video_mem *tmp = video;

  /*
   * get id equal and state idle.
   */
  while (tmp) {
    if (tmp->id == id && tmp->mem_status == VMS_FREE) {
      tmp->mem_status = cur ? VMS_USED_FOR_CUR : VMS_USED_FOR_PREPARE;
      return tmp;
    }
    tmp = tmp->next;
  }

  /*
   * take out any idle.
   */
  tmp = video;
  while (tmp) {
    if (tmp->mem_status == VMS_FREE) {
      tmp->id = id;
      tmp->pic_status = PS_BLANK;
      tmp->mem_status = cur ? VMS_USED_FOR_CUR : VMS_USED_FOR_PREPARE;
      return tmp;
    }
    tmp = tmp->next;
  }

  return NULL;
}

/*
 * put video mem.
 */
void put_video_mem(video_mem *vd_mem) { vd_mem->mem_status = VMS_FREE; }

/*
 * free video mem.
 */
void free_video_mem(void) {
  video_mem *tmp_video;

  while (video) {
    tmp_video = video;
    video = video->next;
    free(tmp_video);
  }
}

/*
 * alloc video mem.
 */
int alloc_video_mem(int num) {
  int i;
  int ret;
  disp_ops *dp_ops = get_display_ops_from_name(LCD_NAME);
  disp_buff dp_buff;
  unsigned int bpp;
  unsigned int width;
  unsigned int height;
  unsigned int line_byte;
  unsigned int vm_size;
  video_mem *tmp_video;

  if (!dp_ops)
    goto err_get_display_ops_from_name;

  ret = get_display_buffer(dp_ops, &dp_buff);
  if (ret)
    goto err_get_display_buffer;

  bpp = dp_buff.bpp;
  width = dp_buff.xres;
  height = dp_buff.yres;
  line_byte = dp_buff.line_byte;
  vm_size = dp_buff.total_size;

  tmp_video = malloc(sizeof(video_mem));
  if (!tmp_video)
    goto err_tmp_video;

  tmp_video->next = video;
  video = tmp_video;

  /*
   * set video.
   */
  tmp_video->id = 0;
  tmp_video->mem_status = VMS_FREE;
  tmp_video->pic_status = PS_BLANK;
  tmp_video->is_dev_framebuffer = 1;

  /*
   * set buff.
   */
  tmp_video->disp_buff.buff = dp_buff.buff;
  tmp_video->disp_buff.bpp = bpp;
  tmp_video->disp_buff.xres = width;
  tmp_video->disp_buff.yres = height;
  tmp_video->disp_buff.line_byte = line_byte;

  if (num != 0)
    tmp_video->mem_status = VMS_USED_FOR_CUR;

  for (i = 1; i <= num; i++) {
    tmp_video = malloc(sizeof(video_mem) + vm_size);
    if (!tmp_video)
      goto err_tmp_video;

    tmp_video->next = video;
    video = tmp_video;

    /*
     * set video.
     */
    tmp_video->id = i;
    tmp_video->mem_status = VMS_FREE;
    tmp_video->pic_status = PS_BLANK;
    tmp_video->is_dev_framebuffer = 0;

    /*
     * set buff.
     */
    tmp_video->disp_buff.buff = (unsigned char *)(tmp_video + 1);
    tmp_video->disp_buff.bpp = bpp;
    tmp_video->disp_buff.xres = width;
    tmp_video->disp_buff.yres = height;
    tmp_video->disp_buff.line_byte = line_byte;
  }

  return 0;

err_tmp_video:
  free_video_mem();
err_get_display_ops_from_name:
err_get_display_buffer:
  return -1;
}

#if 0

/*
 * draw pixel.
 */
int put_pixel(int x, int y, unsigned int color, disp_buff *dp_buff) {
  unsigned char *pen_8 =
      dp_buff->buff + y * dp_buff->line_byte + x * dp_buff->pixel_width;
  unsigned short *pen_16;
  unsigned int *pen_32;

  unsigned int red, green, blue;

  pen_16 = (unsigned short *)pen_8;
  pen_32 = (unsigned int *)pen_8;

  switch (dp_buff->bpp) {
  case 8: {
    *pen_8 = color;
    break;
  }
  case 16: {
    /* 565 */
    red = (color >> 16) & 0xff;
    green = (color >> 8) & 0xff;
    blue = (color >> 0) & 0xff;
    color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
    *pen_16 = color;
    break;
  }
  case 32: {
    *pen_32 = color;
    break;
  }
  default: {
    printf("can't surport %dbpp\n", dp_buff->bpp);
    break;
  }
  }

  return True;
}

/*
 * draw line.
 */
int draw_line(int x_start, int x_end, int y, unsigned char *rgb_color_array,
              disp_buff *dp_buff) {
  int i = x_start * 3;
  int x;
  unsigned int dwColor;

  if (y >= dp_buff->yres)
    return -1;

  if (x_start >= dp_buff->xres)
    return -1;

  if (x_end >= dp_buff->xres) {
    x_end = dp_buff->xres;
  }

  for (x = x_start; x < x_end; x++) {
    /* 0xRRGGBB */
    dwColor = (rgb_color_array[i] << 16) + (rgb_color_array[i + 1] << 8) +
              (rgb_color_array[i + 2] << 0);
    i += 3;
    put_pixel(x, y, dwColor, dp_buff);
  }
  return 0;
}

/*
 * clear screen.
 */
int clean_screen(unsigned int color, disp_buff *dp_buff) {
  region rgn;

  rgn.width = dp_buff->xres;
  rgn.height = dp_buff->yres;
  rgn.left_up_x = rgn.left_up_y = 0;
  draw_region(rgn, color, dp_buff);

  return 0;
}

/*
 * draw font bit map.
 */
void draw_from_bit_map(font_bit_map fb_map, unsigned int color,
                       disp_buff *dp_buff) {
  int p, q;
  int i, j;
  int x = fb_map.region.left_up_x;
  int y = fb_map.region.left_up_y;
  int x_max = fb_map.region.width;
  int y_max = fb_map.region.height;

  for (j = y, q = 0; q < y_max; j++, q++) {
    for (i = x, p = 0; p < x_max; i++, p++) {
      if (i < 0 || j < 0 || i >= dp_buff->xres || j >= dp_buff->yres) {
        continue;
      }
      if (fb_map.buffer[q * fb_map.region.width + p])
        put_pixel(i, j, color, dp_buff);
    }
  }
}

/*
 * draw text in region center.
 */
int draw_text_in_region(char *str, region *rgn, unsigned int color,
                        disp_buff *dp_buff) {
  int ret;
  font_bit_map fb_map;
  region_cartesian rgn_car;

  ret = get_string_region_car(str, &rgn_car);
  if (ret) {
    printf("get string region car err\n");
    return -1;
  }

  fb_map.cur_origin_x =
      rgn->left_up_x + (rgn->width - rgn_car.width) / 2 - rgn_car.left_up_x;
  fb_map.cur_origin_y =
      rgn->left_up_y + (rgn->height - rgn_car.height) / 2 + rgn_car.left_up_y;

  while (*str) {
    ret = get_font_bit_map((unsigned int)*str, &fb_map);
    str++;
    if (ret) {
      return -1;
    }

    draw_from_bit_map(fb_map, color, dp_buff);
    fb_map.cur_origin_x = fb_map.next_origin_x;
    fb_map.cur_origin_y = fb_map.next_origin_y;
  }

  return 0;
}

/*
 * draw region.
 */
void draw_region(region reg, unsigned int color, disp_buff *dp_buff) {
  int x = reg.left_up_x;
  int y = reg.left_up_y;
  int width = reg.width;
  int height = reg.height;
  int i, j;

  for (j = y; j < y + height; j++) {
    for (i = x; i < x + width; i++) {
      if (i < 0 || j < 0 || i >= dp_buff->xres || j >= dp_buff->yres)
        continue;
      put_pixel(i, j, color, dp_buff);
    }
  }
}

#endif

/*
 * display device init.
 */
void display_system_register(void) { frame_buffer_register(); }

/*
 * display system init.
 */
int display_system_init(void) {
  disp_ops *ptmp = dsp_ops;

  while (ptmp) {
    if (ptmp && ptmp->device_init)
      ptmp->device_init();
    ptmp = ptmp->next;
  }

  return 0;
}

/*
 * get display ops.
 */
disp_ops *get_display_ops_from_name(const char *name) {
  disp_ops *tmp = r_dops;

  while (tmp) {
    if (!strcmp(name, tmp->name))
      return tmp;
    tmp = tmp->next;
  }

  return NULL;
}

/*
 * get display buffer.
 */
int get_display_buffer(disp_ops *dp_ops, disp_buff *dp_buff) {
  dp_ops->get_buffer(dp_buff);
  return 0;
}

/*
 * register device linked list.
 */
void register_display(disp_ops *dops) {
  dops->next = r_dops;
  r_dops = dops;
}

/*
 * select default device.
 */
#if 0
 int select_default_display(char *name) {
  disp_ops *ptmp = r_dops;

  while (ptmp) {
    if (strcmp(name, ptmp->name) == 0) {
      disp_dft = ptmp;
      return True;
    }
    ptmp = ptmp->next;
  }

  return False;
}

/*
 * init default display device.
 * need to use int "select_default_display(char *name)" to select default
 * device.
 */
int default_display_init(void) {
  int ret;

  ret = disp_dft->device_init();
  if (ret) {
    printf("device init err\n");
    return False;
  }

  ret = disp_dft->get_buffer(&dp_buff);
  if (ret) {
    printf("device get buffer err\n");
    return False;
  }

  pixel_width = dp_buff.bpp / 8;
  line_width = pixel_width * dp_buff.xres;

  return True;
}


/*
 * draw jpeg picture.
 */
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
#endif

/*
 * flush display region.
 */
int flush_display_region(region *rgn, disp_ops *dp_ops, disp_buff *dp_buff) {
  return dp_ops->flush_region(rgn, dp_buff);
}
