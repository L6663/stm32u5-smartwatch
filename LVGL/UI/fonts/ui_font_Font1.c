/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --bpp 1 --size 16 --font C:/Users/86178/Documents/SquareLine/assets/AlimamaDaoLiTi.ttf -o C:/Users/86178/Documents/SquareLine/assets\ui_font_Font1.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter
 ******************************************************************************/

#include "../ui.h"

#ifndef UI_FONT_FONT1
#define UI_FONT_FONT1 1
#endif

#if UI_FONT_FONT1

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xff, 0x3c,

    /* U+0022 "\"" */
    0x55,

    /* U+0023 "#" */
    0x11, 0x4, 0xc1, 0x31, 0xff, 0x32, 0x8, 0x8f,
    0xf8, 0x98, 0x26, 0x19, 0x80,

    /* U+0024 "$" */
    0x10, 0xff, 0xf6, 0x8d, 0xe, 0xf, 0xf, 0x16,
    0x2f, 0xf7, 0xc1, 0x0,

    /* U+0025 "%" */
    0x71, 0xd9, 0x32, 0x2c, 0x45, 0x89, 0xe0, 0xe8,
    0x3, 0x70, 0xd1, 0x1a, 0x26, 0x45, 0xc7, 0x0,

    /* U+0026 "&" */
    0x38, 0x26, 0x13, 0x5, 0x83, 0x3, 0x89, 0x67,
    0x9e, 0xc6, 0x77, 0x9e, 0x60,

    /* U+0027 "'" */
    0x50,

    /* U+0028 "(" */
    0x33, 0x99, 0xcc, 0x63, 0x18, 0xc6, 0x18, 0xc3,
    0x0,

    /* U+0029 ")" */
    0x63, 0x8c, 0x61, 0x8c, 0x63, 0x18, 0xcc, 0x66,
    0x0,

    /* U+002A "*" */
    0x25, 0x5d, 0xf2, 0x0,

    /* U+002B "+" */
    0x18, 0x18, 0x18, 0xff, 0x18, 0x18, 0x18,

    /* U+002C "," */
    0x6d, 0x0,

    /* U+002D "-" */
    0xfc,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x6, 0xc, 0x30, 0x60, 0x83, 0x4, 0x18, 0x30,
    0xc1, 0x83, 0x0,

    /* U+0030 "0" */
    0x3c, 0x66, 0x43, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
    0xc2, 0x66, 0x3c,

    /* U+0031 "1" */
    0x7d, 0xb6, 0xdb, 0x6d, 0x80,

    /* U+0032 "2" */
    0x7c, 0xc6, 0x6, 0x6, 0xe, 0xc, 0x1c, 0x38,
    0x70, 0xe1, 0xfe,

    /* U+0033 "3" */
    0xfd, 0x8c, 0x18, 0x71, 0xc7, 0x83, 0x83, 0x7,
    0x9f, 0xe0,

    /* U+0034 "4" */
    0x6, 0xe, 0x1e, 0x16, 0x26, 0x66, 0xff, 0x6,
    0x6, 0x6,

    /* U+0035 "5" */
    0x7e, 0x60, 0x60, 0x60, 0x7e, 0x7, 0x3, 0x3,
    0x3, 0xc6, 0x7c,

    /* U+0036 "6" */
    0xc, 0x18, 0x30, 0x60, 0x7c, 0xe6, 0xc3, 0xc3,
    0xc3, 0x66, 0x3c,

    /* U+0037 "7" */
    0xfe, 0xc, 0x18, 0x60, 0xc1, 0x86, 0xc, 0x18,
    0x60, 0xc0,

    /* U+0038 "8" */
    0x3c, 0xc6, 0xc2, 0xc6, 0x74, 0x7e, 0xc7, 0xc3,
    0xc3, 0xe7, 0x3c,

    /* U+0039 "9" */
    0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0x67, 0x3e, 0x6,
    0xc, 0x18, 0x38,

    /* U+003A ":" */
    0xf0, 0xf,

    /* U+003B ";" */
    0x6c, 0x0, 0x1a, 0xc0,

    /* U+003C "<" */
    0x0, 0x33, 0xb8, 0xc1, 0xc1, 0xc3, 0x0,

    /* U+003D "=" */
    0xff, 0x0, 0x0, 0xff,

    /* U+003E ">" */
    0x3, 0x7, 0x7, 0xc, 0xee, 0x30, 0x0,

    /* U+003F "?" */
    0x7b, 0x20, 0x82, 0x38, 0xc3, 0x0, 0x30, 0xc0,

    /* U+0040 "@" */
    0x1f, 0xc, 0x66, 0x1b, 0x7b, 0xf2, 0xfc, 0xbf,
    0x2f, 0xce, 0x7f, 0x18, 0x41, 0xe0,

    /* U+0041 "A" */
    0xc, 0x1, 0x80, 0x38, 0x7, 0x1, 0x30, 0x26,
    0xf, 0xe1, 0xc, 0x60, 0xd8, 0x18,

    /* U+0042 "B" */
    0xfc, 0xc6, 0xc6, 0xc6, 0xce, 0xfc, 0xc6, 0xc3,
    0xc3, 0xc7, 0xfe,

    /* U+0043 "C" */
    0x1f, 0x31, 0x60, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
    0x60, 0x71, 0x1f,

    /* U+0044 "D" */
    0xfc, 0xc6, 0xc6, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
    0xc6, 0xc6, 0xfc,

    /* U+0045 "E" */
    0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xfe, 0xc0, 0xc0,
    0xc0, 0xc1, 0xff,

    /* U+0046 "F" */
    0xff, 0xc0, 0xc0, 0xc0, 0xfe, 0xc0, 0xc0, 0xc0,
    0xc0, 0xc0,

    /* U+0047 "G" */
    0x1f, 0x31, 0x60, 0xc0, 0xc0, 0xc0, 0xc3, 0xc3,
    0x63, 0x73, 0x1f,

    /* U+0048 "H" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3,
    0xc3, 0xc3,

    /* U+0049 "I" */
    0xff, 0xff, 0xfc,

    /* U+004A "J" */
    0x18, 0xc6, 0x31, 0x8c, 0x63, 0x18, 0xc6, 0xf7,
    0x0,

    /* U+004B "K" */
    0xc4, 0xc6, 0xcc, 0xd8, 0xf0, 0xf0, 0xd0, 0xd8,
    0xcc, 0xc6, 0xc3,

    /* U+004C "L" */
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
    0xc1, 0xff,

    /* U+004D "M" */
    0x60, 0x66, 0x6, 0x70, 0xe7, 0xe, 0x50, 0xa5,
    0x9a, 0xc9, 0x2c, 0xf3, 0xcf, 0x3c, 0x63, 0xc6,
    0x30,

    /* U+004E "N" */
    0xe3, 0xe3, 0xf3, 0xd3, 0xd3, 0xcb, 0xcf, 0xc7,
    0xc7, 0xc3,

    /* U+004F "O" */
    0x1e, 0xc, 0x66, 0x1b, 0x3, 0xc0, 0xf0, 0x3c,
    0xf, 0x3, 0x61, 0x98, 0xc1, 0xe0,

    /* U+0050 "P" */
    0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfc, 0xc0, 0xc0,
    0xc0, 0xc0,

    /* U+0051 "Q" */
    0x1e, 0xc, 0x66, 0x1b, 0x3, 0xc0, 0xf0, 0x3c,
    0xf, 0x3, 0x65, 0x98, 0xc1, 0xf0, 0x6,

    /* U+0052 "R" */
    0xfe, 0xc7, 0xc3, 0xc3, 0xc6, 0xfc, 0xc8, 0xc4,
    0xc6, 0xc3,

    /* U+0053 "S" */
    0x3f, 0xe2, 0xc0, 0xc0, 0xf0, 0x7e, 0xf, 0x3,
    0x3, 0xc2, 0xfc,

    /* U+0054 "T" */
    0xff, 0x8c, 0x6, 0x3, 0x1, 0x80, 0xc0, 0x60,
    0x30, 0x18, 0xc, 0x0,

    /* U+0055 "U" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
    0x66, 0x3c,

    /* U+0056 "V" */
    0xc0, 0xd8, 0x36, 0x19, 0x86, 0x21, 0xc, 0xc1,
    0x30, 0x68, 0x1e, 0x3, 0x0, 0xc0,

    /* U+0057 "W" */
    0xc3, 0x6, 0x86, 0x19, 0x86, 0x33, 0x1c, 0x42,
    0x28, 0x86, 0xd3, 0xd, 0x36, 0xa, 0x28, 0x1c,
    0x70, 0x38, 0xe0,

    /* U+0058 "X" */
    0xc0, 0x98, 0x63, 0x30, 0xc8, 0x1e, 0x3, 0x1,
    0xc0, 0xd8, 0x33, 0x18, 0x64, 0x1c,

    /* U+0059 "Y" */
    0x0, 0xb0, 0x36, 0x18, 0xcc, 0x1e, 0x7, 0x80,
    0xc0, 0x30, 0xc, 0x3, 0x0, 0xc0,

    /* U+005A "Z" */
    0xff, 0x7, 0x6, 0xc, 0x1c, 0x38, 0x30, 0x60,
    0xe0, 0xc1, 0xff,

    /* U+005B "[" */
    0xff, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xff,

    /* U+005C "\\" */
    0x0, 0xc0, 0x60, 0x60, 0x30, 0x30, 0x10, 0x18,
    0xc, 0xc, 0x6, 0x6, 0x4,

    /* U+005D "]" */
    0xff, 0x33, 0x33, 0x33, 0x33, 0x33, 0xff,

    /* U+005E "^" */
    0x30, 0x71, 0xa2, 0x6c, 0x40,

    /* U+005F "_" */
    0xff,

    /* U+0060 "`" */
    0x99, 0x80,

    /* U+0061 "a" */
    0x3b, 0x33, 0xb0, 0xd8, 0x6c, 0x36, 0x19, 0x9c,
    0x76,

    /* U+0062 "b" */
    0xc1, 0x83, 0x7, 0xcc, 0xd8, 0xf1, 0xe3, 0xc7,
    0x9b, 0xf0,

    /* U+0063 "c" */
    0x3d, 0x9c, 0x30, 0xc3, 0x6, 0x4f,

    /* U+0064 "d" */
    0x3, 0x1, 0x80, 0xc7, 0xe6, 0x76, 0x1b, 0xd,
    0x86, 0xc3, 0x33, 0x8e, 0xc0,

    /* U+0065 "e" */
    0x3c, 0xcf, 0x1f, 0xff, 0xf8, 0x19, 0x1e,

    /* U+0066 "f" */
    0x1c, 0xc3, 0x1f, 0x30, 0xc3, 0xc, 0x30, 0xc0,

    /* U+0067 "g" */
    0x3f, 0x67, 0xc3, 0xc3, 0xc3, 0xc3, 0x67, 0x3b,
    0x3, 0x46, 0x7c,

    /* U+0068 "h" */
    0xc1, 0x83, 0x6, 0xee, 0x78, 0xf1, 0xe3, 0xc7,
    0x8f, 0x18,

    /* U+0069 "i" */
    0xf3, 0xff, 0xfc,

    /* U+006A "j" */
    0x33, 0x3, 0x33, 0x33, 0x33, 0x33, 0xe0,

    /* U+006B "k" */
    0xc1, 0x83, 0x6, 0x2c, 0xdb, 0x3c, 0x6c, 0xc9,
    0x9b, 0x18,

    /* U+006C "l" */
    0xff, 0xff, 0xfc,

    /* U+006D "m" */
    0xdc, 0xee, 0x73, 0xc6, 0x3c, 0x63, 0xc6, 0x3c,
    0x63, 0xc6, 0x3c, 0x63,

    /* U+006E "n" */
    0xdd, 0xcf, 0x1e, 0x3c, 0x78, 0xf1, 0xe3,

    /* U+006F "o" */
    0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x3c,

    /* U+0070 "p" */
    0x6e, 0x72, 0x63, 0x63, 0x63, 0x63, 0x66, 0x7c,
    0x60, 0x60, 0x60,

    /* U+0071 "q" */
    0x3f, 0x67, 0xc3, 0xc3, 0xc3, 0xc3, 0x67, 0x3b,
    0x3, 0x3, 0x3,

    /* U+0072 "r" */
    0xdf, 0xf1, 0x8c, 0x63, 0x18,

    /* U+0073 "s" */
    0x7f, 0x2c, 0x38, 0x3c, 0x38, 0xfe,

    /* U+0074 "t" */
    0x30, 0xc7, 0xcc, 0x30, 0xc3, 0xc, 0x1c,

    /* U+0075 "u" */
    0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xce, 0x76,

    /* U+0076 "v" */
    0xc3, 0x42, 0x66, 0x66, 0x24, 0x3c, 0x18, 0x18,

    /* U+0077 "w" */
    0xc6, 0x34, 0x63, 0x66, 0x22, 0x76, 0x25, 0x43,
    0x9c, 0x19, 0xc1, 0x8,

    /* U+0078 "x" */
    0x63, 0x26, 0x3c, 0x18, 0x18, 0x3c, 0x66, 0x43,

    /* U+0079 "y" */
    0xc3, 0x42, 0x66, 0x24, 0x3c, 0x3c, 0x18, 0x18,
    0x30, 0x70, 0x60,

    /* U+007A "z" */
    0xfc, 0x61, 0x8c, 0x63, 0x8c, 0x7f,

    /* U+007B "{" */
    0x1b, 0xd8, 0xc6, 0x33, 0x98, 0x63, 0x18, 0xc7,
    0x9c,

    /* U+007C "|" */
    0xff, 0xff, 0xff, 0xfc,

    /* U+007D "}" */
    0xcf, 0x33, 0x33, 0x31, 0x33, 0x33, 0xfe,

    /* U+007E "~" */
    0x71, 0x64, 0xc1, 0xc0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 77, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 91, .box_w = 2, .box_h = 11, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 4, .adv_w = 79, .box_w = 4, .box_h = 2, .ofs_x = 0, .ofs_y = 8},
    {.bitmap_index = 5, .adv_w = 153, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 18, .adv_w = 151, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 30, .adv_w = 194, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 46, .adv_w = 175, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 59, .adv_w = 46, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 8},
    {.bitmap_index = 60, .adv_w = 97, .box_w = 5, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 69, .adv_w = 97, .box_w = 5, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 78, .adv_w = 98, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 82, .adv_w = 131, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 89, .adv_w = 74, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 91, .adv_w = 106, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 92, .adv_w = 76, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 93, .adv_w = 152, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 104, .adv_w = 137, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 115, .adv_w = 91, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 120, .adv_w = 137, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 131, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 141, .adv_w = 137, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 151, .adv_w = 137, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 162, .adv_w = 137, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 173, .adv_w = 123, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 183, .adv_w = 137, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 194, .adv_w = 137, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 205, .adv_w = 76, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 207, .adv_w = 76, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 211, .adv_w = 126, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 218, .adv_w = 131, .box_w = 8, .box_h = 4, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 222, .adv_w = 125, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 229, .adv_w = 114, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 237, .adv_w = 211, .box_w = 10, .box_h = 11, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 251, .adv_w = 168, .box_w = 11, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 265, .adv_w = 148, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 276, .adv_w = 148, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 287, .adv_w = 148, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 298, .adv_w = 144, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 309, .adv_w = 138, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 319, .adv_w = 150, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 330, .adv_w = 166, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 340, .adv_w = 61, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 343, .adv_w = 92, .box_w = 5, .box_h = 13, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 352, .adv_w = 147, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 363, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 373, .adv_w = 201, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 390, .adv_w = 166, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 400, .adv_w = 177, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 414, .adv_w = 148, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 424, .adv_w = 177, .box_w = 10, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 439, .adv_w = 153, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 449, .adv_w = 139, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 460, .adv_w = 141, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 472, .adv_w = 160, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 482, .adv_w = 165, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 496, .adv_w = 239, .box_w = 15, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 515, .adv_w = 160, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 529, .adv_w = 162, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 543, .adv_w = 158, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 554, .adv_w = 77, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 561, .adv_w = 152, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 574, .adv_w = 77, .box_w = 4, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 581, .adv_w = 135, .box_w = 7, .box_h = 5, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 586, .adv_w = 131, .box_w = 8, .box_h = 1, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 587, .adv_w = 77, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 589, .adv_w = 142, .box_w = 9, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 598, .adv_w = 139, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 608, .adv_w = 114, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 614, .adv_w = 145, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 627, .adv_w = 119, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 634, .adv_w = 104, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 642, .adv_w = 141, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 653, .adv_w = 143, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 663, .adv_w = 56, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 666, .adv_w = 79, .box_w = 4, .box_h = 13, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 673, .adv_w = 123, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 683, .adv_w = 56, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 686, .adv_w = 222, .box_w = 12, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 698, .adv_w = 141, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 705, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 713, .adv_w = 144, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 724, .adv_w = 139, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 735, .adv_w = 94, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 740, .adv_w = 108, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 746, .adv_w = 104, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 753, .adv_w = 143, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 760, .adv_w = 130, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 768, .adv_w = 200, .box_w = 12, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 780, .adv_w = 134, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 788, .adv_w = 131, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 799, .adv_w = 121, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 805, .adv_w = 93, .box_w = 5, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 814, .adv_w = 62, .box_w = 2, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 818, .adv_w = 93, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 825, .adv_w = 145, .box_w = 9, .box_h = 3, .ofs_x = 1, .ofs_y = 3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Map glyph_ids to kern left classes*/
static const uint8_t kern_left_class_mapping[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 2, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 4, 5, 0, 6, 0, 7,
    8, 0, 0, 0, 9, 10, 11, 12,
    13, 14, 6, 15, 16, 17, 12, 18,
    19, 20, 21, 22, 1, 0, 0, 0,
    0, 0, 23, 24, 24, 0, 24, 25,
    0, 26, 0, 0, 27, 0, 26, 26,
    28, 29, 0, 30, 31, 32, 33, 34,
    35, 36, 37, 36, 1, 0, 0, 0
};

/*Map glyph_ids to kern right classes*/
static const uint8_t kern_right_class_mapping[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 1,
    0, 0, 0, 0, 0, 2, 0, 3,
    3, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 4, 0, 5, 0, 0, 0,
    5, 0, 0, 6, 0, 0, 0, 0,
    5, 0, 5, 0, 0, 7, 0, 8,
    9, 10, 11, 12, 0, 0, 0, 0,
    0, 0, 13, 14, 13, 13, 13, 15,
    13, 0, 0, 16, 0, 0, 17, 17,
    13, 17, 13, 18, 19, 20, 21, 22,
    23, 24, 25, 26, 0, 0, 0, 0
};

/*Kern values between classes*/
static const int8_t kern_class_values[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 15,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -8, -5, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -5, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, 0, -19, -23, -8, 0,
    -23, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -8, -3, 0, -3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -2, 0, 0, 0,
    -1, -1, 0, 0, -1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -8,
    -8, -5, 0, -5, -5, -6, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -16,
    0, 0, 0, 0, 0, 0, 0, 0,
    -10, 0, 0, -14, -5, -5, 0, 0,
    -5, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -8, -8, -8, 0, -5,
    -8, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -9, 0, 0, 0,
    0, 0, 0, 0, -15, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -10, 0,
    -26, -26, -13, 0, -26, 0, -8, 0,
    0, 0, 0, 0, 0, 0, 0, -15,
    -13, 0, -15, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -2, -2,
    0, 0, -2, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -8, -8, -8,
    0, -5, -8, -6, 0, 0, 0, 0,
    -1, -1, 0, 0, -1, 0, 0, 0,
    0, -2, 0, 0, 0, -13, 0, 0,
    0, 0, 0, 0, 0, 0, -8, 0,
    0, 0, -2, -2, 0, 0, -2, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -5, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 3, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -19, -8, -15, 0, 0,
    0, 0, 0, 0, -26, 0, 0, -11,
    -24, -24, -24, -11, -24, -16, -16, -16,
    -16, -16, 0, 0, 0, -23, -8, 0,
    0, 0, 0, 0, 0, 0, -18, 0,
    0, -8, -5, -5, -18, -5, -6, 0,
    0, 0, 0, -5, 0, 0, 0, -8,
    0, 0, 0, 0, 0, 0, 0, 0,
    -10, 0, 0, -6, -3, -3, -10, 0,
    -3, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -5, 0, 0, 0, 0, 0,
    0, 0, -8, 0, 0, 0, 0, 0,
    0, -5, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -15, -8, 0, -8, 0,
    0, 0, 0, 0, -26, 0, 0, -8,
    -15, -11, -26, -15, -15, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -6, 0,
    0, 0, 0, 0, 0, 0, -6, 0,
    0, 0, 0, 0, 0, -5, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -4, 0, 0, 0, 0, 0,
    0, 2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -26, -16, -10, -8,
    -26, -6, 0, 0, -5, -3, 0, 0,
    0, -4, 0, -5, -5, -5, -3, -5,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -8, 0, 2, -10,
    0, 0, 0, 4, 0, 2, 2, 0,
    2, 0, 0, 0, 0, 0, 0, 0,
    -4, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -10, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -26, -16, -10, -8,
    -24, -6, 0, 0, -5, -3, 0, 0,
    0, -4, 0, -5, -5, -5, -3, -5,
    0, 0, 0, 0, 0, 0, -25, -15,
    -8, -8, -26, -6, 0, 0, -5, 0,
    0, 0, 0, -3, 0, -5, -5, -5,
    -3, -5, 0, 0, 0, 0, 0, 0,
    0, 0, 2, 0, 0, 0, -5, 0,
    2, 0, 0, 0, -5, 2, 0, 3,
    2, 0, 2, 0, 0, 0, 0, 0,
    0, 0, -22, -18, -10, 0, -20, 0,
    0, 0, 0, 0, 0, 0, 0, -5,
    0, 0, 0, 0, -5, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -3, 0, 0, 0, 0, 0,
    -5, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -3,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -8, 0, 0,
    -16, 0, 0, 0, 0, 0, -5, 0,
    1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -16, 0, 0, 0, 0, 0,
    -5, 0, 0, 0, 0, 0, 0, 0,
    0, 2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -16, 0, 0, 0,
    0, 0, -5, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -16, 0,
    0, 0, 0, 0, -3, 0, 0, 0,
    0, 0, -5, 0, 0, 0, 0, 0,
    0, 0
};


/*Collect the kern class' data in one place*/
static const lv_font_fmt_txt_kern_classes_t kern_classes =
{
    .class_pair_values   = kern_class_values,
    .left_class_mapping  = kern_left_class_mapping,
    .right_class_mapping = kern_right_class_mapping,
    .left_class_cnt      = 37,
    .right_class_cnt     = 26,
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_classes,
    .kern_scale = 16,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 1,
    .bitmap_format = 0,
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t ui_font_Font1 = {
#else
lv_font_t ui_font_Font1 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 16,          /*The maximum line height required by the font*/
    .base_line = 4,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if UI_FONT_FONT1*/

