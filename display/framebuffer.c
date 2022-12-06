#include <config.h>
#include <disp_manger.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <render.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int fd_fb;
static struct fb_var_screeninfo var;
static int screen_size;
static unsigned char *fb_base;
static unsigned int line_width;
static unsigned int pixel_width;

/*
 * init lcd device.
 */
static int lcd_device_init(void) {
  int ret;

  fd_fb = open(FB_DEV_NAME, O_RDWR);
  if (fd_fb < 0) {
    printf("can't open %s\n", FB_DEV_NAME);
    return False;
  }

  ret = ioctl(fd_fb, FBIOGET_VSCREENINFO, &var);
  if (ret < 0) {
    printf("ioctl fd_fb err\n");
    close(fd_fb);
    return False;
  }

  /*
   * get and save lcd screen info.
   */
  pixel_width = var.bits_per_pixel / 8;
  line_width = pixel_width * var.xres;
  screen_size = var.xres * var.yres * pixel_width;

  /*
   * map address to framebuffer.
   */
  fb_base =
      mmap(NULL, screen_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd_fb, 0);
  if (fb_base == MAP_FAILED) {
    printf("mmap fd_fb err\n");
    close(fd_fb);
    return False;
  }

  return True;
}

/*
 * unmap framebuffer and close fd.
 */
static void lcd_device_exit(void) {
  munmap(fb_base, screen_size);
  close(fd_fb);
}

/*
 * get lcd framebuffer base address.
 */
static int lcd_get_buffer(disp_buff *dp_buff) {
  setup_disp_buff(dp_buff, var.xres, var.yres, var.bits_per_pixel, fb_base);
  return True;
}

/*
 * lcd show page.
 */
static int lcd_show_page(disp_buff *dp_buff) {
  memcpy(fb_base, dp_buff, dp_buff->total_size);
  return 0;
}

/*
 * no do something.
 */
// TODO
static int lcd_flush_region(region *rgn, disp_buff *buffer) { return 0; }

/*
 * show pixel.
 */
static int lcd_show_pixel(unsigned int x, unsigned int y, unsigned int color) {
  unsigned char *pen_8 = fb_base + y * line_width + x * pixel_width;
  unsigned short *pen_16;
  unsigned int *pen_32;

  unsigned int red, green, blue;

  pen_16 = (unsigned short *)pen_8;
  pen_32 = (unsigned int *)pen_8;

  switch (var.bits_per_pixel) {
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
    printf("can't surport %dbpp\n", var.bits_per_pixel);
    break;
  }
  }

  return 0;
}

static disp_ops lcd_disp_ops = {
    .name = "lcd",
    .device_init = lcd_device_init,
    .flush_region = lcd_flush_region,
    .get_buffer = lcd_get_buffer,
    .device_exit = lcd_device_exit,
    .show_page = lcd_show_page,
    .show_pixel = lcd_show_pixel,
};

/*
 * register framebuffer to linked list.
 */
void frame_buffer_register(void) { register_display(&lcd_disp_ops); }