#include "common.h"
#include "font_manger.h"
#include "input_manger.h"
#include "sys/time.h"
#include <config.h>
#include <disp_manger.h>
#include <math.h>
#include <page_manger.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ui.h>

/*
 * main page.
 */
int browse_page_run(void *params) {
  input_event ievt;
  int ret;

  /*
   * display interface.
   */

  /*
   * create prepare thread.
   */

  /*
   * input processing.
   */
  while (1) {
    ret = get_input_event(&ievt);
    if (ret)
      continue;

    /*
     * save, start, restore page.
     */
    switch (ievt) {

    case "back":
      break;

    /*
     * small.
     */
    case "narrow ":
      break;

    /*
     * big.
     */
    case "enlarge":
      break;

    case "up_picture":
      break;

    case "down_picture":
      break;

    case "auto":
      page("auto")->run(NULL);
      break;

    case "pressure":
      /*
       * display move picture.
       */
      break;

    default:
      break;
    }
  }

  return 0;
}

/*
 * pre pare.
 */
static int browse_page_prepare(void) { return 0; }

static page_action browse_page_atn = {
    .name = "explore",
    .run = browse_page_run,
    .prepare = browse_page_prepare,
};

/*
 * register main page.
 */
void browse_page_register(void) { register_page(&browse_page_atn); }
