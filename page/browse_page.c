#include "common.h"
#include "file.h"
#include "font_manger.h"
#include "input_manger.h"
#include "render.h"
#include "sys/time.h"
#include <config.h>
#include <disp_manger.h>
#include <math.h>
#include <page_manger.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ui.h>

#define DIR_FILE_ICON_WIDTH 127
#define DIR_FILE_ICON_HEIGHT DIR_FILE_ICON_WIDTH
#define DIR_FILE_NAME_HEIGHT 20
#define DIR_FILE_NAME_WIDTH (DIR_FILE_ICON_HEIGHT + DIR_FILE_NAME_HEIGHT)
#define DIR_FILE_ALL_WIDTH DIR_FILE_NAME_WIDTH
#define DIR_FILE_ALL_HEIGHT DIR_FILE_ALL_WIDTH

static int show_browse_page(button *btn);
static int calc_browse_page_dir_and_files_layout(void);
static int generate_dir_and_file_icons(page_layout *ptPageLayout);
static int show_dir_and_file_icons(int iStartIndex, int iDirContentsNumber,
                                   dir_content **aptDirContents,
                                   video_mem *ptVideoMem);
static button btn_layout[];
static page_layout g_tBrowsePageDirAndFileLayout;
static page_params page_pms;
static int g_iDirFileNumPerCol, g_iDirFileNumPerRow;
static disp_buff g_tDirClosedIconPixelDatas;
static disp_buff g_tDirOpenedIconPixelDatas;
static disp_buff g_tFileIconPixelDatas;
static button *g_atDirAndFileLayout;
static button *select_btn;
static char *g_strDirClosedIconName = "fold_closed.bmp";
static char *g_strDirOpenedIconName = "fold_opened.bmp";
static char *g_strFileIconName = "file.bmp";

static int is_exit = 0;
static int is_use_select = 0;
static int g_iStartIndex = 0;
static int open_from_setting = 0;
static dir_content *
    *g_aptDirContents; /* 数组:存有目录下"顶层子目录","文件"的名字 */
static int g_iDirContentsNumber; /* g_aptDirContents数组有多少项 */

static char g_strCurDir[256] = DEFAULT_DIR;
static char g_strSelectedDir[256] = DEFAULT_DIR;

static void jump_to_up_dir(void) {
  char *dir_path;

  /*
   * is root dir.
   */
  if (!strcmp("/", g_strCurDir))
    goto done;

  /*
   * get new dir contents.
   */
  dir_path = strrchr(g_strCurDir, '/');
  *dir_path = '\0';
  free_dir_contents(g_aptDirContents, g_iDirContentsNumber);
  get_dir_contents(g_strCurDir, &g_aptDirContents, &g_iDirContentsNumber);

  /*
   * refresh display.
   */
  show_browse_page(btn_layout);

done:
  return;
}

static void pre_page_func(void) {
  video_mem *vd_mem = get_dev_video_mem();

  if (!vd_mem)
    goto err_get_dev_video_mem;

  g_iStartIndex -= g_iDirFileNumPerCol * g_iDirFileNumPerRow;
  if (g_iStartIndex >= 0) {
    show_dir_and_file_icons(g_iStartIndex, g_iDirContentsNumber,
                            g_aptDirContents, vd_mem);
  } else {
    g_iStartIndex += g_iDirFileNumPerCol * g_iDirFileNumPerRow;
  }

  put_video_mem(vd_mem);
err_get_dev_video_mem:
  return;
}

static void next_page_func(void) {
  video_mem *vd_mem = get_dev_video_mem();

  if (!vd_mem)
    goto err_get_dev_video_mem;

  g_iStartIndex += g_iDirFileNumPerCol * g_iDirFileNumPerRow;
  if (g_iStartIndex < g_iDirContentsNumber) {
    show_dir_and_file_icons(g_iStartIndex, g_iDirContentsNumber,
                            g_aptDirContents, vd_mem);
  } else {
    g_iStartIndex -= g_iDirFileNumPerCol * g_iDirFileNumPerRow;
  }

  put_video_mem(vd_mem);
err_get_dev_video_mem:
  return;
}

/*
 * select dir.
 */
static int select_dir_func(input_event *ievt) {
  char dir_name[256];
  char dir_path[256];
  int index;

  /*
   * get open dir index.
   */
  index = from_input_event_get_button_index_from_page_layout(
      &g_tBrowsePageDirAndFileLayout, ievt);
  index /= 2;
  index += g_iStartIndex;
  strcpy(dir_name, g_aptDirContents[index]->str_name);

  /*
   * get and set select dir.
   */
  snprintf(dir_path, 256, "%s/%s", g_strCurDir, dir_name);
  dir_path[255] = '\0';
  strcpy(g_strSelectedDir, dir_path);

  return 0;
}

static void set_exit(int val) { is_exit = val; }

static int return_on_pressed(struct button *btn, input_event *ievt) {
  return on_pressed_func(btn, ievt, (void *)set_exit, (void *)1);
}

static int up_on_pressed(struct button *btn, input_event *ievt) {
  g_iStartIndex = 0;
  is_use_select = 0;
  release_button(select_btn);
  on_pressed_func(btn, ievt, (void *)jump_to_up_dir, &page_pms);
  calc_browse_page_dir_and_files_layout();
  return 0;
}

static int select_on_pressed(struct button *btn, input_event *ievt) {
  if (ievt->pressure) {
    if (btn->status == BUTTON_PRESSED) {
      release_button(btn);
      is_use_select = 0;
    } else {
      press_button(btn);
      is_use_select = 1;
    }
  }
  return 0;
}

static int pre_page_on_pressed(struct button *btn, input_event *ievt) {
  on_pressed_func(btn, ievt, (void *)pre_page_func, &page_pms);
  calc_browse_page_dir_and_files_layout();
  return 0;
}

static int next_page_on_pressed(struct button *btn, input_event *ievt) {
  on_pressed_func(btn, ievt, (void *)next_page_func, &page_pms);
  calc_browse_page_dir_and_files_layout();
  return 0;
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
                .pic_name = "up.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = up_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = "select.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = select_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = "pre_page.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = pre_page_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = "next_page.bmp",
                .rgn = {0, 0, 0, 0},
            },
        .on_draw = icon_on_draw,
        .on_pressed = next_page_on_pressed,
    },
    {
        .pic =
            {
                .pic_name = NULL,
            },
    },
};

static page_layout g_tBrowsePageDirAndFileLayout = {
    .iMaxTotalBytes = 0,
    //.atLayout       = g_atDirAndFileLayout,
};

// static page_layout g_tBrowsePageMenuIconsLayout = {
//     .iMaxTotalBytes = 0,
//     .atLayout = btn_layout,
// };

static int file_pressed(button *btn) {
  if (btn->status != BUTTON_PRESSED) {
    set_pic_disp_pic(&btn->pic, g_strDirOpenedIconName);
    draw_pic(&btn->pic);
    btn->status = BUTTON_PRESSED;
  }
  return 0;
}

static int file_release(button *btn) {
  if (btn->status != BUTTON_RELEASE) {
    set_pic_disp_pic(&btn->pic, g_strDirClosedIconName);
    draw_pic(&btn->pic);
    btn->status = BUTTON_RELEASE;
  }
  return 0;
}

/*
 * open dir.
 */
static int open_dir(input_event *ievt) {
  int index;
  char dir_name[256];
  char dir_path[256];

  /*
   * get open dir index.
   */
  index = from_input_event_get_button_index_from_page_layout(
      &g_tBrowsePageDirAndFileLayout, ievt);
  index /= 2;
  index += g_iStartIndex;
  strcpy(dir_name, g_aptDirContents[index]->str_name);

  /*
   * free cur dir contents.
   */
  free_dir_contents(g_aptDirContents, g_iDirContentsNumber);

  /*
   * get open dir contents from index.
   */
  snprintf(dir_path, 256, "%s/%s", g_strCurDir, dir_name);
  dir_path[255] = '\0';
  get_dir_contents(dir_path, &g_aptDirContents, &g_iDirContentsNumber);
  strcpy(g_strCurDir, dir_path);

  /*
   * refresh display.
   */
  show_browse_page(btn_layout);

  return 0;
}

/*
 * dir button function.
 */
static int dir_button_func(button *btn, input_event *ievt) {
  if (ievt->pressure == INPUT_RELEASE) {
    file_release(btn);
    if (is_use_select) {
      select_dir_func(ievt);
      release_button(select_btn);
      is_use_select = 0;
    } else {
      open_dir(ievt);
    }
  } else {
    file_pressed(btn);
  }

  return 0;
}

/*
 * file button function.
 */
static int file_button_func(button *btn, input_event *ievt, int index) {
  if (ievt->pressure == INPUT_RELEASE) {
    snprintf(page_pms.str_cur_picture_file, 256, "%s/%s", g_strCurDir,
             g_aptDirContents[index]->str_name);
    page("manual")->run(&page_pms);
    show_browse_page(btn_layout);
  }
  return 0;
}

/*
 * calc browse page menus layout.
 */
static int calc_browse_page_menus_layout(button *btn) {
  int iWidth;
  int iHeight;
  int iXres, iYres;
  int i;
  disp_ops *dp_ops = get_display_ops_from_name(LCD_NAME);
  disp_buff dp_buff;

  get_display_buffer(dp_ops, &dp_buff);

  iXres = dp_buff.xres;
  iYres = dp_buff.yres;
  select_btn = &btn[2];

  if (iXres < iYres) {
    /*	 iXres/4
     *	  ----------------------------------
     *	   up	select	pre_page  next_page
     *
     *
     *
     *
     *
     *
     *	  ----------------------------------
     */

    iWidth = iXres / 4;
    iHeight = iWidth;

    /* return图标 */
    btn[0].pic.rgn.left_up_y = 0;
    btn[0].pic.rgn.height = iHeight - 1;
    btn[0].pic.rgn.left_up_x = 0;
    btn[0].pic.rgn.width = iWidth - 1;

    /* up图.pic.rgn. */
    btn[1].pic.rgn.left_up_y = 0;
    btn[1].pic.rgn.height = iHeight - 1;
    btn[1].pic.rgn.left_up_x =
        btn[0].pic.rgn.left_up_x + btn[0].pic.rgn.width + 1;
    btn[1].pic.rgn.width = iWidth - 1;

    /* sel.pic.rgn.ct图标 */
    btn[2].pic.rgn.left_up_y = 0;
    btn[2].pic.rgn.height = iHeight - 1;
    btn[2].pic.rgn.left_up_x =
        btn[1].pic.rgn.left_up_x + btn[1].pic.rgn.width + 1;
    btn[2].pic.rgn.width = iWidth - 1;

    /* pre_page图标 */
    btn[3].pic.rgn.left_up_y = 0;
    btn[3].pic.rgn.height = iHeight - 1;
    btn[3].pic.rgn.left_up_x =
        btn[2].pic.rgn.left_up_x + btn[2].pic.rgn.width + 1;
    btn[3].pic.rgn.width = iWidth - 1;

  } else {
    /*	 iYres/4
     *	  ----------------------------------
     *	   up
     *
     *    select
     *
     *    pre_page
     *
     *   next_page
     *
     *	  ----------------------------------
     */

    iHeight = iYres / 5;
    iWidth = iHeight;

    /* return图标 */
    btn[0].pic.rgn.left_up_y = 0;
    btn[0].pic.rgn.height = iHeight - 1;
    btn[0].pic.rgn.left_up_x = 0;
    btn[0].pic.rgn.width = iWidth - 1;

    /* up图.pic.rgn. */
    btn[1].pic.rgn.left_up_y = btn[0].pic.rgn.height + 1;
    btn[1].pic.rgn.height = iHeight - 1;
    btn[1].pic.rgn.left_up_x = 0;
    btn[1].pic.rgn.width = iWidth - 1;

    /* sel.pic.rgn.ct图标 */
    btn[2].pic.rgn.left_up_y =
        btn[1].pic.rgn.left_up_y + btn[1].pic.rgn.height + 1;
    btn[2].pic.rgn.height = iHeight - 1;
    btn[2].pic.rgn.left_up_x = 0;
    btn[2].pic.rgn.width = iWidth - 1;

    /* pre.pic.rgn.page图标 */
    btn[3].pic.rgn.left_up_y =
        btn[2].pic.rgn.left_up_y + btn[2].pic.rgn.height + 1;
    btn[3].pic.rgn.height = iHeight - 1;
    btn[3].pic.rgn.left_up_x = 0;
    btn[3].pic.rgn.width = iWidth - 1;

    btn[4].pic.rgn.left_up_y =
        btn[3].pic.rgn.left_up_y + btn[3].pic.rgn.height + 1;
    btn[4].pic.rgn.height = iHeight - 1;
    btn[4].pic.rgn.left_up_x = 0;
    btn[4].pic.rgn.width = iWidth - 1;
  }

  i = 0;
  while (btn[i].pic.pic_name) {
    // iTmpTotalBytes = (btn[i].pic.rgn.width - btn[i].pic.rgn.left_up_x + 1) *
    //                  (btn[i].pic.rgn.height - btn[i].pic.rgn.left_up_y + 1) *
    //                  iBpp / 8;
    // TODO
    // if (btn->iMaxTotalBytes < iTmpTotalBytes) {
    //   btn->iMaxTotalBytes = iTmpTotalBytes;
    // }
    i++;
  }

  return 0;
}

/*
 * calc browse page dir and files layout.
 */
static int calc_browse_page_dir_and_files_layout(void) {
  int iXres, iYres, iBpp;
  int iTopLeftX, iTopLeftY;
  int iTopLeftXBak;
  int iBotRightX, iBotRightY;
  int iIconWidth, iIconHeight;
  int iNumPerCol, iNumPerRow;
  int iDeltaX, iDeltaY;
  int i, j, k = 0;
  disp_ops *dp_ops = get_display_ops_from_name(LCD_NAME);
  disp_buff dp_buff;

  get_display_buffer(dp_ops, &dp_buff);
  iXres = dp_buff.xres;
  iYres = dp_buff.yres;
  iBpp = dp_buff.bpp;
  // GetDispResolution(&iXres, &iYres, &iBpp);

  if (iXres < iYres) {
    /* --------------------------------------
     *    up select pre_page next_page 图标
     * --------------------------------------
     *
     *           目录和文件
     *
     *
     * --------------------------------------
     */
    iTopLeftX = 0;
    iBotRightX = iXres - 1;
    iTopLeftY =
        btn_layout[0].pic.rgn.height + 1 + btn_layout[0].pic.rgn.left_up_y;
    iBotRightY = iYres - 1;
  } else {
    /*	 iYres/4
     *	  ----------------------------------
     *	   up      |
     *             |
     *    select   |
     *             |     目录和文件
     *    pre_page |
     *             |
     *   next_page |
     *             |
     *	  ----------------------------------
     */
    iTopLeftX =
        btn_layout[0].pic.rgn.left_up_x + btn_layout[0].pic.rgn.width + 1;
    iBotRightX = iXres - 1;
    iTopLeftY = 0;
    iBotRightY = iYres - 1;
  }

  /* 确定一行显示多少个"目录或文件", 显示多少行 */
  iIconWidth = DIR_FILE_NAME_WIDTH;
  iIconHeight = iIconWidth;

  /* 图标之间的间隔要大于10个象素 */
  iNumPerRow = (iBotRightX - iTopLeftX + 1) / iIconWidth;
  for (;;) {
    iDeltaX = (iBotRightX - iTopLeftX + 1) - iIconWidth * iNumPerRow;
    if ((iDeltaX / (iNumPerRow + 1)) < 10)
      iNumPerRow--;
    else
      break;
  }

  iNumPerCol = (iBotRightY - iTopLeftY + 1) / iIconHeight;
  for (;;) {
    iDeltaY = (iBotRightY - iTopLeftY + 1) - iIconHeight * iNumPerCol;
    if ((iDeltaY / (iNumPerCol + 1)) < 10)
      iNumPerCol--;
    else
      break;
  }

  /* 每个图标之间的间隔 */
  iDeltaX = iDeltaX / (iNumPerRow + 1);
  iDeltaY = iDeltaY / (iNumPerCol + 1);

  g_iDirFileNumPerRow = iNumPerRow;
  g_iDirFileNumPerCol = iNumPerCol;

  /* 可以显示 iNumPerRow * iNumPerCol个"目录或文件"
   * 分配"两倍+1"的T_Layout结构体: 一个用来表示图标,另一个用来表示名字
   * 最后一个用来存NULL,借以判断结构体数组的末尾
   */
  g_atDirAndFileLayout =
      malloc(sizeof(button) * (2 * iNumPerRow * iNumPerCol + 1));
  if (NULL == g_atDirAndFileLayout)
    return -1;

  /* "目录和文件"整体区域的左上角、右下角坐标 */
  g_tBrowsePageDirAndFileLayout.iTopLeftX = iTopLeftX;
  g_tBrowsePageDirAndFileLayout.iBotRightX = iBotRightX;
  g_tBrowsePageDirAndFileLayout.iTopLeftY = iTopLeftY;
  g_tBrowsePageDirAndFileLayout.iBotRightY = iBotRightY;
  g_tBrowsePageDirAndFileLayout.iBpp = iBpp;
  g_tBrowsePageDirAndFileLayout.atLayout = g_atDirAndFileLayout;
  g_tBrowsePageDirAndFileLayout.iMaxTotalBytes =
      DIR_FILE_ALL_WIDTH * DIR_FILE_ALL_HEIGHT * iBpp / 8;

  /* 确定图标和名字的位置
   *
   * 图标是一个正方体, "图标+名字"也是一个正方体
   *   --------
   *   |  图  |
   *   |  标  |
   * ------------
   * |   名字   |
   * ------------
   */
  iTopLeftX += iDeltaX;
  iTopLeftY += iDeltaY;
  iTopLeftXBak = iTopLeftX;
  for (i = 0; i < iNumPerCol; i++) {
    for (j = 0; j < iNumPerRow; j++) {
      /* 图标 */
      g_atDirAndFileLayout[k].pic.rgn.left_up_x =
          iTopLeftX + (DIR_FILE_NAME_WIDTH - DIR_FILE_ICON_WIDTH) / 2;
      g_atDirAndFileLayout[k].pic.rgn.width = DIR_FILE_ICON_WIDTH - 1;
      g_atDirAndFileLayout[k].pic.rgn.left_up_y = iTopLeftY;
      g_atDirAndFileLayout[k].pic.rgn.height = DIR_FILE_ICON_HEIGHT - 1;
      g_atDirAndFileLayout[k].pic.pic_name = "pic";
      g_atDirAndFileLayout[k].on_draw = NULL;
      g_atDirAndFileLayout[k].on_pressed = dir_button_func;

      /* 名字 */
      g_atDirAndFileLayout[k + 1].pic.rgn.left_up_x = iTopLeftX;
      g_atDirAndFileLayout[k + 1].pic.rgn.width = DIR_FILE_NAME_WIDTH - 1;
      g_atDirAndFileLayout[k + 1].pic.rgn.left_up_y =
          g_atDirAndFileLayout[k].pic.rgn.left_up_y +
          g_atDirAndFileLayout[k].pic.rgn.height + 1;
      g_atDirAndFileLayout[k + 1].pic.rgn.height = DIR_FILE_NAME_HEIGHT - 1;
      g_atDirAndFileLayout[k + 1].pic.pic_name = "name";
      g_atDirAndFileLayout[k + 1].on_draw = NULL;
      g_atDirAndFileLayout[k + 1].on_pressed = NULL;

      iTopLeftX += DIR_FILE_ALL_WIDTH + iDeltaX;
      k += 2;
    }
    iTopLeftX = iTopLeftXBak;
    iTopLeftY += DIR_FILE_ALL_HEIGHT + iDeltaY;
  }

  /* 结尾 */
  g_atDirAndFileLayout[k].pic.rgn.left_up_x = 0;
  g_atDirAndFileLayout[k].pic.rgn.width = 0;
  g_atDirAndFileLayout[k].pic.rgn.left_up_y = 0;
  g_atDirAndFileLayout[k].pic.rgn.height = 0;
  g_atDirAndFileLayout[k].pic.pic_name = NULL;

  return 0;
}

/*
 * generate dir and file icons.
 */
static int generate_dir_and_file_icons(page_layout *ptPageLayout) {
  disp_buff tOriginIconPixelDatas;
  int iError;
  int iBpp;
  button *atLayout = ptPageLayout->atLayout;
  char icon_name[128];
  disp_ops *dp_ops = get_display_ops_from_name(LCD_NAME);
  disp_buff dp_buff;

  get_display_buffer(dp_ops, &dp_buff);
  iBpp = dp_buff.bpp;
  set_disp_buff_bpp(&tOriginIconPixelDatas, iBpp);

  /* 给目录图标、文件图标分配内存 */
  g_tDirClosedIconPixelDatas.bpp = iBpp;
  g_tDirClosedIconPixelDatas.buff = malloc(ptPageLayout->iMaxTotalBytes);
  if (g_tDirClosedIconPixelDatas.buff == NULL) {
    return -1;
  }

  g_tDirOpenedIconPixelDatas.bpp = iBpp;
  g_tDirOpenedIconPixelDatas.buff = malloc(ptPageLayout->iMaxTotalBytes);
  if (g_tDirOpenedIconPixelDatas.buff == NULL) {
    return -1;
  }

  g_tFileIconPixelDatas.bpp = iBpp;
  g_tFileIconPixelDatas.buff = malloc(ptPageLayout->iMaxTotalBytes);
  if (g_tFileIconPixelDatas.buff == NULL) {
    return -1;
  }

  /* 从BMP文件里提取图像数据 */
  /* 1. 提取"fold_closed图标" */
  snprintf(icon_name, 128, "%s/%s", ICON_PATH, g_strDirClosedIconName);
  icon_name[127] = '\0';
  iError = get_disp_buff_for_icon(&tOriginIconPixelDatas, icon_name);

  if (iError)
    return -1;

  g_tDirClosedIconPixelDatas.yres = atLayout[0].pic.rgn.height + 1;
  g_tDirClosedIconPixelDatas.xres = atLayout[0].pic.rgn.width + 1;
  g_tDirClosedIconPixelDatas.line_byte =
      g_tDirClosedIconPixelDatas.xres * g_tDirClosedIconPixelDatas.bpp / 8;
  g_tDirClosedIconPixelDatas.total_size =
      g_tDirClosedIconPixelDatas.line_byte * g_tDirClosedIconPixelDatas.yres;

  pic_zoom(&tOriginIconPixelDatas, &g_tDirClosedIconPixelDatas);
  free_disp_buff_for_icon(&tOriginIconPixelDatas);

  /* 2. 提取"fold_opened图标" */
  snprintf(icon_name, 128, "%s/%s", ICON_PATH, g_strDirOpenedIconName);
  icon_name[127] = '\0';
  iError = get_disp_buff_for_icon(&tOriginIconPixelDatas, icon_name);
  if (iError)
    return -1;

  g_tDirOpenedIconPixelDatas.yres = atLayout[0].pic.rgn.height + 1;
  g_tDirOpenedIconPixelDatas.xres = atLayout[0].pic.rgn.width + 1;
  g_tDirOpenedIconPixelDatas.line_byte =
      g_tDirOpenedIconPixelDatas.xres * g_tDirOpenedIconPixelDatas.bpp / 8;
  g_tDirOpenedIconPixelDatas.total_size =
      g_tDirOpenedIconPixelDatas.line_byte * g_tDirOpenedIconPixelDatas.yres;
  pic_zoom(&tOriginIconPixelDatas, &g_tDirOpenedIconPixelDatas);
  free_disp_buff_for_icon(&tOriginIconPixelDatas);

  /* 3. 提取"file图标" */
  snprintf(icon_name, 128, "%s/%s", ICON_PATH, g_strFileIconName);
  icon_name[127] = '\0';
  iError = get_disp_buff_for_icon(&tOriginIconPixelDatas, icon_name);
  if (iError)
    return -1;

  g_tFileIconPixelDatas.yres = atLayout[0].pic.rgn.height + 1;
  g_tFileIconPixelDatas.xres = atLayout[0].pic.rgn.width + 1;
  g_tFileIconPixelDatas.line_byte =
      g_tDirClosedIconPixelDatas.xres * g_tDirClosedIconPixelDatas.bpp / 8;
  g_tFileIconPixelDatas.total_size =
      g_tFileIconPixelDatas.line_byte * g_tFileIconPixelDatas.yres;
  pic_zoom(&tOriginIconPixelDatas, &g_tFileIconPixelDatas);
  free_disp_buff_for_icon(&tOriginIconPixelDatas);

  return 0;
}

/*
 * show dir and file icons.
 */
static int show_dir_and_file_icons(int iStartIndex, int iDirContentsNumber,
                                   dir_content **aptDirContents,
                                   video_mem *ptVideoMem) {
  int i, j, k = 0;
  int iDirContentIndex = iStartIndex;
  page_layout *ptPageLayout = &g_tBrowsePageDirAndFileLayout;
  button *atLayout = ptPageLayout->atLayout;

  clear_rectangle_from_vd(
      ptVideoMem, ptPageLayout->iTopLeftX, ptPageLayout->iTopLeftY,
      ptPageLayout->iBotRightX - ptPageLayout->iTopLeftX,
      ptPageLayout->iBotRightY - ptPageLayout->iTopLeftY, BACKGROUND);

  set_font_size(atLayout[1].pic.rgn.height);

  for (i = 0; i < g_iDirFileNumPerCol; i++) {
    for (j = 0; j < g_iDirFileNumPerRow; j++) {
      if (iDirContentIndex < iDirContentsNumber) {
        /* 显示目录或文件的图标 */
        if (aptDirContents[iDirContentIndex]->ftype == FILETYPE_DIR) {
          pic_merge(atLayout[k].pic.rgn.left_up_x,
                    atLayout[k].pic.rgn.left_up_y, &g_tDirClosedIconPixelDatas,
                    &ptVideoMem->disp_buff);
        } else {
          pic_merge(atLayout[k].pic.rgn.left_up_x,
                    atLayout[k].pic.rgn.left_up_y, &g_tFileIconPixelDatas,
                    &ptVideoMem->disp_buff);
        }

        k++;
        /* 显示目录或文件的名字 */
        merger_string_to_center_of_rectangle_in_video_mem(
            atLayout[k].pic.rgn.left_up_x, atLayout[k].pic.rgn.left_up_y,
            atLayout[k].pic.rgn.left_up_x + atLayout[k].pic.rgn.width,
            atLayout[k].pic.rgn.left_up_y + atLayout[k].pic.rgn.height,
            (char *)aptDirContents[iDirContentIndex]->str_name, ptVideoMem);
        k++;

        iDirContentIndex++;
      } else {
        break;
      }
    }
    if (iDirContentIndex >= iDirContentsNumber) {
      break;
    }
  }
  return 0;
}

/*
 * show browse page.
 */
static int show_browse_page(button *btn) {
  video_mem *vd_mem;

  /*
   * get video mem.
   */
  vd_mem = get_video_mem(ID("browse"), 1);
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
      calc_browse_page_menus_layout(btn);
      calc_browse_page_dir_and_files_layout();
    }

    if (!g_tDirClosedIconPixelDatas.buff)
      generate_dir_and_file_icons(&g_tBrowsePageDirAndFileLayout);

    /*
     * display menu.
     */
    show_button(btn, 0, NULL, vd_mem);

    /*
     * display file and dir.
     */
    show_dir_and_file_icons(g_iStartIndex, g_iDirContentsNumber,
                            g_aptDirContents, vd_mem);
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

err_get_video_mem:
  return -1;
}

/*
 * clean file icon.
 */
static int clean_file_icon(button *btn) {
  while (btn->pic.pic_name) {
    if (btn->status == BUTTON_PRESSED) {
      file_release(btn);
      btn->status = BUTTON_RELEASE;
    }
    btn++;
  }
  return 0;
}

/*
 * get select dir.
 */
static void get_select_dir(char *dir) { strcpy(dir, g_strSelectedDir); }

/*
 * get browse page config.
 */
int get_browse_page_cfg(page_cfg *page) {
  get_select_dir(page->select_dir);
  page->interval_second = get_interval_second();
  return 0;
}

/*
 * browse page.
 */
int browse_page_run(void *params) {
  input_event ievt;
  int ret;
  int index;
  int dir_index;
  button *btn;
  page_params *pms;

  pms = (page_params *)params;
  page_pms.page_id = ID("browse");
  is_use_select = 0;
  set_exit(0);
  if (ID("setting") == pms->page_id) {
    open_from_setting = 1;
  } else {
    open_from_setting = 0;
  }
  /*
   * create prepare thread.
   */

  /*
   * get dir contents.
   */
  ret = get_dir_contents(g_strCurDir, &g_aptDirContents, &g_iDirContentsNumber);
  if (ret)
    goto err_get_dir_contents;

  /*
   * display interface.
   */
  show_browse_page(btn_layout);
  select_btn->status = BUTTON_RELEASE;

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
      /*
       * get file or dir.
       */
      index = from_input_event_get_button_index_from_page_layout(
          &g_tBrowsePageDirAndFileLayout, &ievt);
      dir_index = index / 2 + g_iStartIndex;
      if (index == -1 || dir_index >= g_iDirContentsNumber) {
        clean_button_invert(btn_layout);
        clean_file_icon(g_tBrowsePageDirAndFileLayout.atLayout);
        is_use_select = 0;
        continue;
      }

      if (g_aptDirContents[dir_index]->ftype == FILETYPE_DIR) {
        /*
         * open dir.
         */
        btn = get_button_from_index(g_tBrowsePageDirAndFileLayout.atLayout,
                                    index);

        if (btn && btn->on_pressed) {
          btn->on_pressed(btn, &ievt);
        } else {
        }
      } else {
        /*
         * open picture file.
         */
        file_button_func(btn, &ievt, dir_index);
      }
      continue;
    }

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
static int browse_page_prepare(void) { return 0; }

static page_action browse_page_atn = {
    .name = "browse",
    .run = browse_page_run,
    .prepare = browse_page_prepare,
};

/*
 * register main page.
 */
void browse_page_register(void) { register_page(&browse_page_atn); }
