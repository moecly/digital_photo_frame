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
int explore_page_run(void *params) {
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

    case "upward":
      if (is_top_level)
        return 0;
      else {
        /*
         * display last page.
         */
      }
      break;

    case "select":
      if (is_select_dir) {
        /*
         * display next dir.
         */
      } else {
        /*
         * display browse.
         */
        store_page();
        page("browse")->run(NULL);
        restore_page();
      }
      break;

    case "up_page":
      /*
       * display up page.
       */
      break;

    case "down_page":
      /*
       * display down page.
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
static int explore_page_prepare(void) { return 0; }

static page_action explore_page_atn = {
    .name = "explore",
    .run = explore_page_run,
    .prepare = explore_page_prepare,
};

/*
 * register main page.
 */
void explore_page_register(void) { register_page(&explore_page_atn); }
