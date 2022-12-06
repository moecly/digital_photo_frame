#include <common.h>
#include <font_manger.h>
#include <freetype/ftimage.h>
#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_GLYPH_H

static FT_Face face;
static FT_UInt default_font_size = 12;

/*
 * get string region car.
 */
static int fy_get_string_region_car(char *str, region_cartesian *rgn) {
  int i = 0;
  int ret;
  FT_BBox bbox;
  FT_BBox glyph_bbox;
  FT_Vector pen;
  FT_Glyph glyph;
  FT_GlyphSlot slot = face->glyph;

  /*
   * init.
   */
  bbox.xMin = bbox.yMin = 32000;
  bbox.xMax = bbox.yMax = -32000;
  pen.x = 0;
  pen.y = 0;

  for (; i < strlen(str); i++) {
    FT_Set_Transform(face, 0, &pen);
    ret = FT_Load_Char(face, str[i], FT_LOAD_RENDER);
    if (ret) {
      printf("FT load_char err\n");
      return -1;
    }

    ret = FT_Get_Glyph(slot, &glyph);
    if (ret) {
      printf("FT get glyph err\n");
      return -1;
    }

    FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &glyph_bbox);
    if (glyph_bbox.xMin < bbox.xMin)
      bbox.xMin = glyph_bbox.xMin;

    if (glyph_bbox.yMin < bbox.yMin)
      bbox.yMin = glyph_bbox.yMin;

    if (glyph_bbox.xMax > bbox.xMax)
      bbox.xMax = glyph_bbox.xMax;

    if (glyph_bbox.yMax > bbox.yMax)
      bbox.yMax = glyph_bbox.yMax;

    pen.x += slot->advance.x;
    pen.y += slot->advance.y;
  }

  rgn->left_up_x = bbox.xMin;
  rgn->left_up_y = bbox.yMax;
  rgn->width = bbox.xMax - bbox.xMin + 1;
  rgn->height = bbox.yMax - bbox.yMin + 1;

  return 0;
}

/*
 * freetype init.
 */
static int fy_font_init(char *name) {
  FT_Library library;
  int ret;

  ret = FT_Init_FreeType(&library);
  if (ret) {
    printf("freetype init err\n");
    return -1;
  }

  ret = FT_New_Face(library, name, 0, &face);
  if (ret) {
    printf("freetype new face err\n");
    return -1;
  }

  FT_Set_Pixel_Sizes(face, default_font_size, 0);

  return 0;
}

/*
 * set font size.
 */
static int fy_set_font_size(unsigned int size) {
  FT_Set_Pixel_Sizes(face, size, 0);
  return 0;
}

/*
 * get font bit map.
 */
static int fy_get_font_bit_map(unsigned int code, font_bit_map *fb_map) {
  FT_Vector pen;
  FT_GlyphSlot slot = face->glyph;
  int ret;

  pen.x = fb_map->cur_origin_x * 64;
  pen.y = fb_map->cur_origin_y * 64;

  FT_Set_Transform(face, 0, &pen);
  ret = FT_Load_Char(face, code, FT_LOAD_RENDER);
  if (ret) {
    printf("FT_LOAD_CHAR err\n");
    return -1;
  }

  fb_map->buffer = slot->bitmap.buffer;
  fb_map->region.left_up_x = slot->bitmap_left;
  fb_map->region.left_up_y = fb_map->cur_origin_y * 2 - slot->bitmap_top;
  fb_map->region.width = slot->bitmap.width;
  fb_map->region.height = slot->bitmap.rows;
  fb_map->next_origin_x = fb_map->cur_origin_x + slot->advance.x / 64;
  fb_map->next_origin_y = fb_map->cur_origin_y;

  return 0;
}

static font_ops fy_fops = {
    .name = "freetype",
    .font_init = fy_font_init,
    .get_font_bit_map = fy_get_font_bit_map,
    .set_font_size = fy_set_font_size,
    .get_string_region_car = fy_get_string_region_car,
};

/*
 * register freetype font.
 */
void freetype_register(void) { register_font(&fy_fops); }