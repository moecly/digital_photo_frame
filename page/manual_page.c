#include "common.h"
#include "file.h"
#include "font_manger.h"
#include "input_manger.h"
#include "page_manger.h"
#include "pic_fmt_manger.h"
#include <config.h>
#include <disp_manger.h>
#include <render.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ui.h>

#define ZOOM_RATIO (0.9)
#define SLIP_MIN_DISTANCE (2 * 2)

static disp_buff *get_zoomed_pic_pixel_datas(disp_buff *ptOriginPicPixelDatas,
                                             int iZoomedWidth,
                                             int iZoomedHeight);
static int is_picture_file_supported(char *strFileName);
static void show_zoomed_picture_in_layout(disp_buff *ptZoomedPicPixelDatas,
                                          video_mem *ptVideoMem);

static int show_manual_page(page_layout *layout, char *file_name);
static int show_picture_in_manual_page(video_mem *ptVideoMem,
                                       char *strFileName);

static button btn_layout[];
static page_layout manual_page_layout;

static int is_exit = 0;
static int pic_file_index;
static char full_path_name[256];
static char dir_name[256];
static char file_name[256];
static dir_content **content;
static int dir_number;
static int xof_zoomed_pic_show_in_center = 0;
static int yof_zoomed_pic_show_in_center = 0;
static page_params page_pms;
static page_params *pms;
static button manual_picture_layout;

static input_event pic_ievt;
static int first_move = 0;

static disp_buff origin_pic_pixel_datas;
static disp_buff zoomed_pic_pixel_datas;

static void set_exit(int val) { is_exit = val; }

static void zoomout_func(void *args) {
  /* 获得缩小后的数据 */
  int zoomed_width;
  int zoomed_height;
  video_mem *vd_mem;
  disp_buff *dp_buff = &zoomed_pic_pixel_datas;

  vd_mem = get_dev_video_mem();
  zoomed_width = (float)dp_buff->xres * ZOOM_RATIO;
  zoomed_height = (float)dp_buff->yres * ZOOM_RATIO;
  dp_buff = get_zoomed_pic_pixel_datas(&origin_pic_pixel_datas, zoomed_width,
                                       zoomed_height);

  /* 重新计算中心点 */
  xof_zoomed_pic_show_in_center =
      (float)xof_zoomed_pic_show_in_center * ZOOM_RATIO;
  yof_zoomed_pic_show_in_center =
      (float)yof_zoomed_pic_show_in_center * ZOOM_RATIO;

  show_zoomed_picture_in_layout(dp_buff, vd_mem);
  put_video_mem(vd_mem);
}

static void zoomin_func(void *args) {
  /* 获得缩小后的数据 */
  int zoomed_width;
  int zoomed_height;
  video_mem *vd_mem;
  disp_buff *dp_buff = &zoomed_pic_pixel_datas;

  vd_mem = get_dev_video_mem();
  zoomed_width = (float)dp_buff->xres / ZOOM_RATIO;
  zoomed_height = (float)dp_buff->yres / ZOOM_RATIO;
  dp_buff = get_zoomed_pic_pixel_datas(&origin_pic_pixel_datas, zoomed_width,
                                       zoomed_height);

  /* 重新计算中心点 */
  xof_zoomed_pic_show_in_center =
      (float)xof_zoomed_pic_show_in_center / ZOOM_RATIO;
  yof_zoomed_pic_show_in_center =
      (float)yof_zoomed_pic_show_in_center / ZOOM_RATIO;

  show_zoomed_picture_in_layout(dp_buff, vd_mem);
  put_video_mem(vd_mem);
}

static void next_pic_func(void *args) {
  video_mem *vd_mem;

  vd_mem = get_dev_video_mem();
  while (pic_file_index < dir_number - 1) {
    pic_file_index++;
    snprintf(full_path_name, 256, "%s/%s", dir_name,
             content[pic_file_index]->str_name);
    full_path_name[255] = '\0';

    if (is_picture_file_supported(full_path_name)) {
      show_picture_in_manual_page(vd_mem, full_path_name);
      break;
    }
  }

  put_video_mem(vd_mem);
}

static void pre_pic_func(void *args) {
  video_mem *vd_mem;

  vd_mem = get_dev_video_mem();
  while (pic_file_index > 0) {
    pic_file_index--;
    snprintf(full_path_name, 256, "%s/%s", dir_name,
             content[pic_file_index]->str_name);
    full_path_name[255] = '\0';

    if (is_picture_file_supported(full_path_name)) {
      show_picture_in_manual_page(vd_mem, full_path_name);
      break;
    }
  }

  put_video_mem(vd_mem);
}

static int continue_mod_small_func(button *btn, input_event *ievt) {
  if (ievt->pressure) {
    if (btn->status == BUTTON_RELEASE) {
      press_button(btn);
      btn->status = BUTTON_PRESSED;
    }
    goto done;
  }

  if (btn->status == BUTTON_PRESSED) {
    release_button(btn);
    btn->status = BUTTON_RELEASE;
    if (pms->page_id == ID("browse")) {
      strcpy(page_pms.str_cur_picture_file, full_path_name);
      page("auto")->run(&page_pms);
      show_manual_page(&manual_page_layout, page_pms.str_cur_picture_file);
    } else {
      set_exit(1);
    }
  }

done:
  return 0;
}

static int return_on_pressed(struct button *btn, input_event *ievt) {
  return on_pressed_func(btn, ievt, (void *)set_exit, (void *)1);
}

static int zoomout_on_pressed(struct button *btn, input_event *ievt) {
  return on_pressed_func(btn, ievt, (void *)zoomout_func, NULL);
}

static int zoomin_on_pressed(struct button *btn, input_event *ievt) {
  return on_pressed_func(btn, ievt, (void *)zoomin_func, NULL);
}

static int continue_mod_small_on_pressed(struct button *btn,
                                         input_event *ievt) {
  return continue_mod_small_func(btn, ievt);
}

static int next_pic_on_pressed(struct button *btn, input_event *ievt) {
  return on_pressed_func(btn, ievt, (void *)next_pic_func, NULL);
}

static int pre_pic_on_pressed(struct button *btn, input_event *ievt) {
  return on_pressed_func(btn, ievt, (void *)pre_pic_func, NULL);
}

static button btn_layout[] = {
    {
        .pic =
            {
                .pic_name = "return.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = return_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = "zoomout.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = zoomout_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = "zoomin.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = zoomin_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = "pre_pic.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = pre_pic_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = "next_pic.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = next_pic_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = "continue_mod_small.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = continue_mod_small_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = NULL,
            },
    },
};

static page_layout manual_page_layout = {
    .iMaxTotalBytes = 0,
    .atLayout = btn_layout,
};

/*
 * is picture file supported.
 */
static int is_picture_file_supported(char *strFileName) {
  file_map tFileMap;
  int iError;

  iError = map_file(&tFileMap, strFileName);
  if (iError) {
    return 0;
  }

  if (get_parser(&tFileMap) == NULL) {
    return 0;
  }

  unmap_file(&tFileMap);
  return 1;
}

/*
 * show zoomed picture in layout.
 */
static void show_zoomed_picture_in_layout(disp_buff *ptZoomedPicPixelDatas,
                                          video_mem *ptVideoMem) {
  int iStartXofNewPic, iStartYofNewPic;
  int iStartXofOldPic, iStartYofOldPic;
  int iWidthPictureInPlay, iHeightPictureInPlay;
  int iPictureLayoutWidth, iPictureLayoutHeight;
  int iDeltaX, iDeltaY;

  iPictureLayoutWidth = manual_picture_layout.pic.rgn.width + 1;
  iPictureLayoutHeight = manual_picture_layout.pic.rgn.height + 1;

  /* 显示新数据 */
  iStartXofNewPic = xof_zoomed_pic_show_in_center - iPictureLayoutWidth / 2;
  if (iStartXofNewPic < 0) {
    iStartXofNewPic = 0;
  }
  if (iStartXofNewPic > ptZoomedPicPixelDatas->xres) {
    iStartXofNewPic = ptZoomedPicPixelDatas->xres;
  }

  /*
   * g_iXofZoomedPicShowInCenter - iStartXofNewPic = PictureLayout中心点X坐标 -
   * iStartXofOldPic
   */
  iDeltaX = xof_zoomed_pic_show_in_center - iStartXofNewPic;
  iStartXofOldPic =
      (manual_picture_layout.pic.rgn.left_up_x + iPictureLayoutWidth / 2) -
      iDeltaX;
  if (iStartXofOldPic < manual_picture_layout.pic.rgn.left_up_x) {
    iStartXofOldPic = manual_picture_layout.pic.rgn.left_up_x;
  }
  if (iStartXofOldPic > manual_picture_layout.pic.rgn.left_up_x +
                            manual_picture_layout.pic.rgn.width) {
    iStartXofOldPic = manual_picture_layout.pic.rgn.left_up_x +
                      manual_picture_layout.pic.rgn.width + 1;
  }

  if ((ptZoomedPicPixelDatas->xres - iStartXofNewPic) >
      (manual_picture_layout.pic.rgn.left_up_x +
       manual_picture_layout.pic.rgn.width - iStartXofOldPic + 1))
    iWidthPictureInPlay =
        (manual_picture_layout.pic.rgn.left_up_x +
         manual_picture_layout.pic.rgn.width - iStartXofOldPic + 1);
  else
    iWidthPictureInPlay = (ptZoomedPicPixelDatas->xres - iStartXofNewPic);

  iStartYofNewPic = yof_zoomed_pic_show_in_center - iPictureLayoutHeight / 2;
  if (iStartYofNewPic < 0) {
    iStartYofNewPic = 0;
  }
  if (iStartYofNewPic > ptZoomedPicPixelDatas->yres) {
    iStartYofNewPic = ptZoomedPicPixelDatas->yres;
  }

  /*
   * g_iYofZoomedPicShowInCenter - iStartYofNewPic = PictureLayout中心点Y坐标 -
   * iStartYofOldPic
   */
  iDeltaY = yof_zoomed_pic_show_in_center - iStartYofNewPic;
  iStartYofOldPic =
      (manual_picture_layout.pic.rgn.left_up_y + iPictureLayoutHeight / 2) -
      iDeltaY;

  if (iStartYofOldPic < manual_picture_layout.pic.rgn.left_up_y) {
    iStartYofOldPic = manual_picture_layout.pic.rgn.left_up_y;
  }
  if (iStartYofOldPic > manual_picture_layout.pic.rgn.left_up_y +
                            manual_picture_layout.pic.rgn.height) {
    iStartYofOldPic = manual_picture_layout.pic.rgn.left_up_y +
                      manual_picture_layout.pic.rgn.height + 1;
  }

  if ((ptZoomedPicPixelDatas->yres - iStartYofNewPic) >
      (manual_picture_layout.pic.rgn.left_up_y +
       manual_picture_layout.pic.rgn.height - iStartYofOldPic + 1)) {
    iHeightPictureInPlay =
        (manual_picture_layout.pic.rgn.left_up_y +
         manual_picture_layout.pic.rgn.height - iStartYofOldPic + 1);
  } else {
    iHeightPictureInPlay = (ptZoomedPicPixelDatas->yres - iStartYofNewPic);
  }

  clear_video_mem_region(ptVideoMem, &manual_picture_layout, BACKGROUND);
  pic_merge_region(iStartXofNewPic, iStartYofNewPic, iStartXofOldPic,
                   iStartYofOldPic, iWidthPictureInPlay, iHeightPictureInPlay,
                   ptZoomedPicPixelDatas, &ptVideoMem->disp_buff);
}

/*
 * get origin picture file pixel datas.
 */
static disp_buff *get_origin_picture_file_pixel_datas(char *strFileName) {
  int ret;

  if (origin_pic_pixel_datas.buff) {
    free_disp_buff_for_icon(&origin_pic_pixel_datas);
    origin_pic_pixel_datas.buff = NULL;
  }

  ret = get_disp_buff_from_file(strFileName, &origin_pic_pixel_datas);
  if (ret)
    goto err_get_disp_buff_from_file;

  return &origin_pic_pixel_datas;

err_get_disp_buff_from_file:
  return NULL;
}

static int distance_between_two_point(input_event *ptInputEvent1,
                                      input_event *ptInputEvent2) {
  return (ptInputEvent1->x - ptInputEvent2->x) *
             (ptInputEvent1->x - ptInputEvent2->x) +
         (ptInputEvent1->y - ptInputEvent2->y) *
             (ptInputEvent1->y - ptInputEvent2->y);
}

static disp_buff *get_zoomed_pic_pixel_datas(disp_buff *ptOriginPicPixelDatas,
                                             int iZoomedWidth,
                                             int iZoomedHeight) {
  float k;
  int iBpp;
  disp_ops *fb_ops;
  disp_buff fb_buff;

  fb_ops = get_display_ops_from_name(LCD_NAME);
  get_display_buffer(fb_ops, &fb_buff);
  iBpp = fb_buff.bpp;

  if (zoomed_pic_pixel_datas.buff) {
    free_disp_buff_for_icon(&zoomed_pic_pixel_datas);
    zoomed_pic_pixel_datas.buff = NULL;
  }

  k = (float)ptOriginPicPixelDatas->yres / ptOriginPicPixelDatas->xres;
  zoomed_pic_pixel_datas.xres = iZoomedWidth;
  zoomed_pic_pixel_datas.yres = iZoomedWidth * k;
  if (zoomed_pic_pixel_datas.yres > iZoomedHeight) {
    zoomed_pic_pixel_datas.xres = iZoomedHeight / k;
    zoomed_pic_pixel_datas.yres = iZoomedHeight;
  }

  zoomed_pic_pixel_datas.bpp = iBpp;
  zoomed_pic_pixel_datas.line_byte =
      zoomed_pic_pixel_datas.xres * zoomed_pic_pixel_datas.bpp / 8;
  zoomed_pic_pixel_datas.pixel_width = iBpp / 8;
  zoomed_pic_pixel_datas.total_size =
      zoomed_pic_pixel_datas.line_byte * zoomed_pic_pixel_datas.yres;
  zoomed_pic_pixel_datas.buff = malloc(zoomed_pic_pixel_datas.total_size);
  if (zoomed_pic_pixel_datas.buff == NULL) {
    return NULL;
  }

  pic_zoom(ptOriginPicPixelDatas, &zoomed_pic_pixel_datas);
  return &zoomed_pic_pixel_datas;
}

/*
 * show picture in manual page.
 */
static int show_picture_in_manual_page(video_mem *ptVideoMem,
                                       char *strFileName) {
  disp_buff *ptOriginPicPixelDatas;
  disp_buff *ptZoomedPicPixelDatas;
  int iPictureLayoutWidth;
  int iPictureLayoutHeight;
  int iTopLeftX, iTopLeftY;

  /* 获得图片文件的原始数据 */
  ptOriginPicPixelDatas = get_origin_picture_file_pixel_datas(strFileName);
  if (!ptOriginPicPixelDatas) {
    return -1;
  }

  /* 把图片按比例缩放到LCD屏幕上, 居中显示 */
  iPictureLayoutWidth = manual_picture_layout.pic.rgn.width + 1;
  iPictureLayoutHeight = manual_picture_layout.pic.rgn.height + 1;

  ptZoomedPicPixelDatas = get_zoomed_pic_pixel_datas(
      &origin_pic_pixel_datas, iPictureLayoutWidth, iPictureLayoutHeight);

  if (!ptZoomedPicPixelDatas) {
    return -1;
  }

  /* 算出居中显示时左上角坐标 */
  iTopLeftX = manual_picture_layout.pic.rgn.left_up_x +
              (iPictureLayoutWidth - ptZoomedPicPixelDatas->xres) / 2;
  iTopLeftY = manual_picture_layout.pic.rgn.left_up_y +
              (iPictureLayoutHeight - ptZoomedPicPixelDatas->yres) / 2;
  xof_zoomed_pic_show_in_center = ptZoomedPicPixelDatas->xres / 2;
  yof_zoomed_pic_show_in_center = ptZoomedPicPixelDatas->yres / 2;

  clear_video_mem_region(ptVideoMem, &manual_picture_layout, BACKGROUND);
  pic_merge(iTopLeftX, iTopLeftY, ptZoomedPicPixelDatas,
            &ptVideoMem->disp_buff);

  return 0;
}

/*
 * move picture function.
 */
static int move_picture(input_event *ievt) {
  int distance;
  video_mem *vd_mem;

  if (ievt->pressure == INPUT_RELEASE) {
    first_move = 0;
    goto done;
  }

  if (!first_move) {
    pic_ievt = *ievt;
    first_move = 1;
    goto done;
  }

  vd_mem = get_dev_video_mem();
  if (!vd_mem)
    goto err_get_dev_video_mem;

  distance = distance_between_two_point(ievt, &pic_ievt);
  if (distance > SLIP_MIN_DISTANCE) {
    /* 重新计算中心点 */
    xof_zoomed_pic_show_in_center -= (ievt->x - pic_ievt.x);
    yof_zoomed_pic_show_in_center -= (ievt->y - pic_ievt.y);

    /* 显示新数据 */
    show_zoomed_picture_in_layout(&zoomed_pic_pixel_datas, vd_mem);

    /* 记录滑动点 */
    pic_ievt = *ievt;
  }

  put_video_mem(vd_mem);
done:
  return 0;

err_get_dev_video_mem:
  return -1;
}

/*
 * calc manual menu page layout.
 */
static void calc_manual_menu_page_layout(page_layout *page_layout) {
  int iWidth;
  int iHeight;
  int iXres, iYres, iBpp;
  int iTmpTotalBytes;
  button *atLayout;
  disp_ops *fb_ops;
  disp_buff fb_buff;
  int i;

  fb_ops = get_display_ops_from_name(LCD_NAME);
  get_display_buffer(fb_ops, &fb_buff);
  iXres = fb_buff.xres;
  iYres = fb_buff.yres;
  iBpp = fb_buff.bpp;
  atLayout = page_layout->atLayout;
  page_layout->iBpp = iBpp;

  if (iXres < iYres) {
    /*	 iXres/6
     *	  --------------------------------------------------------------
     *	   return	zoomout	zoomin  pre_pic next_pic continue_mod_small
     *
     *
     *
     *
     *
     *
     *	  --------------------------------------------------------------
     */

    iWidth = iXres / 6;
    iHeight = iWidth;

    /* return图标 */
    atLayout[0].pic.rgn.left_up_y = 0;
    atLayout[0].pic.rgn.height = iHeight - 1;
    atLayout[0].pic.rgn.left_up_x = 0;
    atLayout[0].pic.rgn.width = iWidth - 1;

    /* 其他5个图标 */
    for (i = 1; i < 6; i++) {
      atLayout[i].pic.rgn.left_up_y = 0;
      atLayout[i].pic.rgn.height = iHeight - 1;
      atLayout[i].pic.rgn.left_up_x =
          atLayout[i - 1].pic.rgn.left_up_x + atLayout[i - 1].pic.rgn.width + 1;
      atLayout[i].pic.rgn.width = iWidth - 1;
    }

  } else {
    /*	 iYres/6
     *	  ----------------------------------
     *	   up
     *
     *    zoomout
     *
     *    zoomin
     *
     *    pre_pic
     *
     *    next_pic
     *
     *    continue_mod_small
     *
     *	  ----------------------------------
     */

    iHeight = iYres / 6;
    iWidth = iHeight;

    /* return图标 */
    atLayout[0].pic.rgn.left_up_y = 0;
    atLayout[0].pic.rgn.height = iHeight - 1;
    atLayout[0].pic.rgn.left_up_x = 0;
    atLayout[0].pic.rgn.width = iWidth - 1;

    /* 其他5个图标 */
    for (i = 1; i < 6; i++) {
      atLayout[i].pic.rgn.left_up_y = atLayout[i - 1].pic.rgn.left_up_y +
                                      atLayout[i - 1].pic.rgn.height + 1;
      atLayout[i].pic.rgn.height = iHeight - 1;
      atLayout[i].pic.rgn.left_up_x = 0;
      atLayout[i].pic.rgn.width = iWidth - 1;
    }
  }

  i = 0;
  while (atLayout[i].pic.pic_name) {
    iTmpTotalBytes =
        (atLayout[i].pic.rgn.left_up_x + atLayout[i].pic.rgn.width -
         atLayout[i].pic.rgn.left_up_x + 1) *
        (atLayout[i].pic.rgn.left_up_y + atLayout[i].pic.rgn.height -
         atLayout[i].pic.rgn.left_up_y + 1) *
        iBpp / 8;
    if (page_layout->iMaxTotalBytes < iTmpTotalBytes) {
      page_layout->iMaxTotalBytes = iTmpTotalBytes;
    }
    i++;
  }
}

static void calc_manual_page_picture_layout(void) {
  int iXres, iYres;
  int iTopLeftX, iTopLeftY;
  int iBotRightX, iBotRightY;
  disp_ops *fb_ops;
  disp_buff fb_buff;

  fb_ops = get_display_ops_from_name(LCD_NAME);
  get_display_buffer(fb_ops, &fb_buff);
  iXres = fb_buff.xres;
  iYres = fb_buff.yres;

  if (iXres < iYres) {
    /*	 iXres/6
     *	  --------------------------------------------------------------
     *	   return	zoomout	zoomin  pre_pic next_pic continue_mod_small
     *(图标)
     *	  --------------------------------------------------------------
     *
     *                              图片
     *
     *
     *	  --------------------------------------------------------------
     */
    iTopLeftX = 0;
    iBotRightX = iXres - 1;
    iTopLeftY =
        btn_layout[0].pic.rgn.left_up_y + btn_layout[0].pic.rgn.height + 1;
    iBotRightY = iYres - 1;
  } else {
    /*	 iYres/6
     *	  --------------------------------------------------------------
     *	   up		         |
     *                       |
     *    zoomout	         |
     *                       |
     *    zoomin             |
     *                       |
     *    pre_pic            |                 图片
     *                       |
     *    next_pic           |
     *                       |
     *    continue_mod_small |
     *                       |
     *	  --------------------------------------------------------------
     */
    iTopLeftX =
        btn_layout[0].pic.rgn.left_up_x + btn_layout[0].pic.rgn.width + 1;
    iBotRightX = iXres - 1;
    iTopLeftY = 0;
    iBotRightY = iYres - 1;
  }

  manual_picture_layout.pic.rgn.left_up_x = iTopLeftX;
  manual_picture_layout.pic.rgn.left_up_y = iTopLeftY;
  manual_picture_layout.pic.rgn.width = iBotRightX - iTopLeftX;
  manual_picture_layout.pic.rgn.height = iBotRightY - iTopLeftY;
  manual_picture_layout.pic.pic_name = NULL;
}

/*
 * show manual page.
 */
static int show_manual_page(page_layout *layout, char *file_name) {
  int ret;
  video_mem *vd_mem;
  button *btn = layout->atLayout;

  /*
   * get video mem.
   */
  vd_mem = get_video_mem(ID("manual"), 1);
  if (!vd_mem)
    goto err_get_video_mem;

  /*
   * draw.
   */
  if (vd_mem->pic_status != PS_GENERATED) {
    /*
     * display each button.
     */
    clean_screen_from_vd(BACKGROUND, vd_mem);
    if (btn[0].pic.rgn.left_up_x == 0) {
      calc_manual_menu_page_layout(&manual_page_layout);
      calc_manual_page_picture_layout();
    }

    /*
     * display menu.
     */
    show_button(layout->atLayout, 0, NULL, vd_mem);
    ret = show_picture_in_manual_page(vd_mem, file_name);
    if (ret)
      goto err_show_picture_in_manual_page;

    /*
     * display file and dir.
     */
    // vd_mem->pic_status = PS_GENERATED;
  }

  /*
   * flush.
   */
  flush_video_mem_to_dev(vd_mem);

  /*
   * free video mem.
   */
  put_video_mem(vd_mem);

  return 0;

err_show_picture_in_manual_page:
  // set_exit(1);
  put_video_mem(vd_mem);
err_get_video_mem:
  return -1;
}

/*
 * manual page.
 */
int manual_page_run(void *params) {
  input_event ievt;
  int ret;
  char *tmp;
  button *btn;
  pms = (page_params *)params;

  page_pms.page_id = ID("manual");
  set_exit(0);

  /*
   * get dir file.
   */
  strcpy(full_path_name, pms->str_cur_picture_file);

  /*
   * display interface.
   */
  show_manual_page(&manual_page_layout, pms->str_cur_picture_file);

  /*
   * get cur dir path.
   */
  strcpy(dir_name, pms->str_cur_picture_file);
  tmp = strrchr(dir_name, '/');
  *tmp = '\0';

  /*
   * get file name.
   */
  strcpy(file_name, tmp + 1);

  /*
   * get cur file index.
   */
  ret = get_dir_contents(dir_name, &content, &dir_number);
  if (ret)
    goto err_get_dir_contents;

  for (pic_file_index = 0; pic_file_index < dir_number; pic_file_index++) {
    if (!strcmp(file_name, content[pic_file_index]->str_name))
      break;
  }

  /*
   * input processing.
   */
  for (;;) {
    if (is_exit)
      goto done;

    ret = lcd_page_get_input_event(&ievt);
    if (ret)
      continue;

    /*
     * execute button function.
     */
    btn = from_input_event_get_btn(btn_layout, &ievt);

    /*
     * if isn't menu button.
     */
    if (!btn) {
      clean_button_invert(btn_layout);
      move_picture(&ievt);
      continue;
    }

    first_move = 0;
    btn->on_pressed(btn, &ievt);
  }

done:
  return 0;

err_get_dir_contents:
  return -1;
}

/*
 * pre pare.
 */
static int manual_page_prepare(void) { return 0; }

static page_action manual_page_atn = {
    .name = "manual",
    .run = manual_page_run,
    .prepare = manual_page_prepare,
};

/*
 * register main page.
 */
void manual_page_register(void) { register_page(&manual_page_atn); }
