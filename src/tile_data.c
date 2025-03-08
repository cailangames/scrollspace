#pragma bank 1

#include <stdint.h>

// 4 tiles
const uint8_t block_tiles[] = {
  0X10,0XEF,0X42,0XB9,0X08,0XD7,0X44,0XBA,0X10,0XEB,0X49,0XB6,0X02,0XD9,0X10,0XEF,
  0X00,0X7E,0X3C,0XC3,0X5A,0XA5,0X66,0X99,0X66,0X99,0X5A,0XA5,0X3C,0XC3,0X00,0X7E,
  0X00,0X81,0X00,0X5A,0X18,0X24,0X24,0X5A,0X24,0X5A,0X18,0X24,0X00,0X5A,0X00,0X81,
  0X00,0X00,0X00,0X10,0X00,0X44,0X00,0X10,0X00,0X4A,0X00,0X00,0X00,0X14,0X00,0X00
};

// 7 tiles
const uint8_t health_bar_tiles[] = {
  0X7E,0X7E,0X7E,0X42,0X24,0X24,0X18,0X18,0X18,0X18,0X3C,0X24,0X42,0X42,0X7E,0X7E,
  0X00,0X00,0X0F,0X0F,0X10,0X10,0X1F,0X10,0X1F,0X10,0X10,0X1F,0X0F,0X0F,0X00,0X00,
  0X00,0X00,0XFF,0XFF,0X00,0X00,0XFF,0X00,0XFF,0X00,0X00,0XFF,0XFF,0XFF,0X00,0X00,
  0X00,0X00,0XF0,0XF0,0X08,0X08,0XF8,0X08,0XF8,0X08,0X08,0XF8,0XF0,0XF0,0X00,0X00,
  0X00,0X00,0X0F,0X0F,0X10,0X1F,0X10,0X1F,0X10,0X1F,0X10,0X1F,0X0F,0X0F,0X00,0X00,
  0X00,0X00,0XFF,0XFF,0X00,0XFF,0X00,0XFF,0X00,0XFF,0X00,0XFF,0XFF,0XFF,0X00,0X00,
  0X00,0X00,0XF0,0XF0,0X08,0XF8,0X08,0XF8,0X08,0XF8,0X08,0XF8,0XF0,0XF0,0X00,0X00
};

// 9 tiles
const uint8_t powerup_tiles[] = {
  0X00,0X00,0X28,0X28,0X54,0X54,0X7C,0X54,0X7C,0X54,0X7C,0X54,0X7C,0X7C,0X00,0X00,
  0X00,0X00,0X10,0X00,0X38,0X38,0X5C,0X5C,0X7C,0X7C,0X7C,0X7C,0X38,0X38,0X00,0X00,
  0X00,0X00,0X3C,0X7E,0X46,0X5A,0X46,0X6A,0X46,0X7A,0X2C,0X34,0X18,0X18,0X00,0X00,
  0X00,0X00,0X22,0X3E,0X7C,0X46,0X6C,0X02,0X44,0X02,0X6E,0X02,0X7C,0X44,0X00,0X00,
  0X00,0XFF,0X28,0XD7,0X54,0XAB,0X54,0X83,0X54,0X83,0X54,0X83,0X7C,0X83,0X00,0XFF,
  0X00,0XFF,0X00,0XEF,0X38,0XC7,0X5C,0XA3,0X7C,0X83,0X7C,0X83,0X38,0XC7,0X00,0XFF,
  0X00,0XFF,0X7E,0XC3,0X5A,0XB9,0X6A,0XB9,0X7A,0XB9,0X34,0XD3,0X18,0XE7,0X00,0XFF,
  0X00,0XFF,0X3E,0XDD,0X46,0X83,0X02,0X93,0X02,0XBB,0X02,0X91,0X44,0X83,0X00,0XFF,
  0X00,0X81,0X10,0X42,0X18,0X3C,0X4C,0X5C,0X74,0X7C,0X58,0X7C,0X38,0X7A,0X00,0X81
};

// 1 tile
const uint8_t lock_tiles[] = {
  0X3C,0X3C,0X42,0X42,0X42,0X42,0XFF,0XFF,0XB1,0X8F,0XF1,0X8F,0XF1,0X8F,0X7E,0X7E
};

// 5 tiles
const uint8_t font_extras_tiles[] = {
  0X00,0X00,0X00,0X00,0X10,0X10,0X00,0X00,0X00,0X00,0X10,0X10,0X00,0X00,0X00,0X00,
  0X00,0X00,0X60,0X60,0X70,0X70,0X78,0X78,0X7C,0X7C,0X78,0X78,0X70,0X70,0X60,0X60,
  0X00,0X00,0X08,0X08,0X1C,0X1C,0X1C,0X1C,0X08,0X08,0X00,0X00,0X08,0X08,0X00,0X00,
  0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X20,0X20,0X20,0X20,0X40,0X40,
  0X00,0X00,0X1C,0X1C,0X22,0X22,0X0C,0X0C,0X08,0X08,0X00,0X00,0X08,0X08,0X00,0X00
};

// 8 tiles
const uint8_t tutorial_screen_tiles[] = {
  0X10,0XEF,0X42,0XB9,0X08,0XD7,0X44,0XBA,0X10,0XEB,0X49,0XB6,0X02,0XD9,0X10,0XEF,
  0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
  0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X7E,0X7E,0X81,0X81,
  0X01,0X01,0X02,0X02,0X02,0X02,0X02,0X02,0X02,0X02,0X02,0X02,0X02,0X02,0X01,0X01,
  0X80,0X80,0X40,0X40,0X40,0X40,0X40,0X40,0X40,0X40,0X40,0X40,0X40,0X40,0X80,0X80,
  0X81,0X81,0X7E,0X7E,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
  0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0XFF,0XFF,
  0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00
};

// 20x17 tile map
const uint8_t tutorial_screen_map[] = {
  0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,
  0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,
  0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,
  0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,
  0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X40,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,
  0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X41,0X3F,0X42,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,
  0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X43,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,
  0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X40,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,
  0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X41,0X3F,0X42,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,
  0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X43,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,
  0X3F,0X3F,0X3F,0X44,0X44,0X44,0X44,0X44,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,
  0X3F,0X3F,0X41,0X3F,0X3F,0X3F,0X3F,0X3F,0X42,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,
  0X3F,0X3F,0X3F,0X45,0X45,0X45,0X45,0X45,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,
  0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,
  0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,
  0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,0X3F,
  0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E,0X3E
};

// 47 tiles
const uint8_t title_screen_tiles[] = {
  0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
  0X10,0XEF,0X42,0XB9,0X08,0XD7,0X44,0XBA,0X10,0XEB,0X49,0XB6,0X02,0XD9,0X10,0XEF,
  0X00,0X00,0X07,0X07,0X0F,0X0F,0X1F,0X1F,0X1E,0X1E,0X1E,0X1E,0X1F,0X1F,0X1F,0X1F,
  0X00,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X00,0XFF,0XFF,0XFF,0XFF,
  0X00,0X00,0XF8,0XF8,0XF9,0XF9,0XF3,0XF3,0X03,0X03,0X03,0X03,0XC3,0XC3,0XF3,0XF3,
  0X00,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XC0,0XC0,0XC0,0XC0,0XC0,0XC0,0XC0,0XC0,
  0X00,0X00,0XFE,0XFE,0XFE,0XFE,0XFC,0XFC,0X00,0X00,0X00,0X00,0X00,0X00,0X01,0X01,
  0X00,0X00,0X7F,0X7F,0XFF,0XFF,0XFF,0XFF,0XF0,0XF0,0XF0,0XF0,0XFF,0XFF,0XFF,0XFF,
  0X00,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X07,0X07,0X07,0X07,0XFF,0XFF,0XFF,0XFF,
  0X00,0X00,0X0F,0X0F,0X9F,0X9F,0X9F,0X9F,0XBC,0XBC,0XBC,0XBC,0XBC,0XBC,0X3C,0X3C,
  0X00,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
  0X00,0X00,0XF1,0XF1,0XF9,0XF9,0XF9,0XF9,0X79,0X79,0X79,0X79,0X79,0X79,0X7B,0X7B,
  0X00,0X00,0XE0,0XE0,0XE0,0XE0,0XE0,0XE0,0XE0,0XE0,0XE0,0XE0,0XE0,0XE0,0XC0,0XC0,
  0X00,0X00,0X01,0X01,0X01,0X01,0X01,0X01,0X03,0X03,0X03,0X03,0X03,0X03,0X03,0X03,
  0X00,0X00,0XE0,0XE0,0XE0,0XE0,0XE0,0XE0,0XC0,0XC0,0XC0,0XC0,0XC0,0XC0,0XC0,0XC0,
  0X0F,0X0F,0X00,0X00,0X00,0X00,0X00,0X00,0X7F,0X7F,0X7F,0X7F,0X7F,0X7F,0X00,0X00,
  0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,
  0XF7,0XF7,0XF7,0XF7,0XF7,0XF7,0XF7,0XF7,0XE7,0XE7,0XE7,0XE7,0XC3,0XC3,0X00,0X00,
  0X80,0X80,0X80,0X80,0X80,0X80,0X80,0X80,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,
  0X01,0X01,0X01,0X01,0X01,0X01,0X01,0X01,0XF9,0XF9,0XFB,0XFB,0XFB,0XFB,0X00,0X00,
  0XFF,0XFF,0XE0,0XE0,0XE0,0XE0,0XE0,0XE0,0XE0,0XE0,0XC0,0XC0,0XC0,0XC0,0X00,0X00,
  0XFE,0XFE,0X78,0X78,0X3C,0X3C,0X3C,0X3C,0X3C,0X3C,0X1E,0X1E,0X1E,0X1E,0X00,0X00,
  0X38,0X38,0X78,0X78,0X78,0X78,0X78,0X78,0X7F,0X7F,0X7F,0X7F,0X3F,0X3F,0X00,0X00,
  0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,
  0X73,0X73,0XF3,0XF3,0XF3,0XF3,0XF3,0XF3,0XF3,0XF3,0XE7,0XE7,0XC7,0XC7,0X00,0X00,
  0XC0,0XC0,0XC0,0XC0,0XC0,0XC0,0XC0,0XC0,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,
  0X03,0X03,0X03,0X03,0X03,0X03,0X03,0X03,0XF3,0XF3,0XF7,0XF7,0XF7,0XF7,0X00,0X00,
  0XC0,0XC0,0XC0,0XC0,0X80,0X80,0X80,0X80,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,
  0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0XF0,0XF0,0XF0,0XF0,0XF0,0XF0,0X00,0X00,
  0X00,0X00,0X1F,0X1F,0X3F,0X3F,0X7F,0X7F,0X78,0X78,0X78,0X78,0X7F,0X7F,0X7F,0X7F,
  0X00,0X00,0XCF,0XCF,0XCF,0XCF,0X8F,0X8F,0X0F,0X0F,0X0F,0X0F,0X1F,0X1F,0X9F,0X9F,
  0X00,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0X00,0XFF,0XFF,
  0X00,0X00,0XF0,0XF0,0XF8,0XF8,0XF8,0XF8,0X78,0X78,0XF1,0XF1,0XF3,0XF3,0XF3,0XF3,
  0X00,0X00,0X1E,0X1E,0X3E,0X3E,0X7F,0X7F,0XFF,0XFF,0XF7,0XF7,0XE7,0XE7,0XC3,0XC3,
  0X00,0X00,0X00,0X00,0X01,0X01,0X03,0X03,0X03,0X03,0X83,0X83,0X83,0X83,0XC7,0XC7,
  0X00,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XC0,0XC0,0XC0,0XC0,0XC0,0XC0,0X80,0X80,
  0X00,0X00,0XFF,0XFF,0XFF,0XFF,0XFE,0XFE,0X00,0X00,0X00,0X00,0XFC,0XFC,0XFC,0XFC,
  0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X01,0X01,0X01,0X01,0X01,0X01,0X00,0X00,
  0X3F,0X3F,0X00,0X00,0X00,0X00,0X00,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,
  0XFF,0XFF,0X07,0X07,0X07,0X07,0X07,0X07,0XFF,0XFF,0XFF,0XFF,0XFE,0XFE,0X00,0X00,
  0X9F,0X9F,0X9F,0X9F,0X9E,0X9E,0X9E,0X9E,0XBC,0XBC,0X3C,0X3C,0X3C,0X3C,0X00,0X00,
  0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
  0XE7,0XE7,0XCF,0XCF,0X1E,0X1E,0X3E,0X3E,0X7C,0X7C,0XF8,0XF8,0XF0,0XF0,0X00,0X00,
  0X83,0X83,0X01,0X01,0X01,0X01,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
  0XC7,0XC7,0XE7,0XE7,0XE7,0XE7,0XF7,0XF7,0XF7,0XF7,0XFB,0XFB,0X79,0X79,0X00,0X00,
  0XFF,0XFF,0XE0,0XE0,0XE0,0XE0,0XE0,0XE0,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,
  0XFC,0XFC,0X00,0X00,0X00,0X00,0X00,0X00,0XFC,0XFC,0XFC,0XFC,0XFC,0XFC,0X00,0X00
};

// 20x18 tile map
const uint8_t title_screen_map[] = {
  0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,
  0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,
  0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,
  0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA5,0XA5,0XA5,0XA5,0XA5,
  0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA5,0XA5,0XA5,
  0XA4,0XA6,0XA7,0XA8,0XA9,0XAA,0XAB,0XAC,0XAD,0XAE,0XAF,0XB0,0XB1,0XB2,0XA4,0XA4,0XA4,0XA4,0XA4,0XA5,
  0XA4,0XB3,0XB4,0XB5,0XB6,0XB7,0XB8,0XB9,0XBA,0XBB,0XBC,0XBD,0XBE,0XBF,0XC0,0XA4,0XA4,0XA4,0XA4,0XA4,
  0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XC1,0XA7,0XC2,0XC3,0XC4,0XC5,0XC6,0XC7,0XAA,0XAB,0XC8,0XA4,
  0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XC9,0XCA,0XCB,0XCC,0XCD,0XCE,0XCF,0XD0,0XB6,0XB7,0XD1,0XD2,0XA4,
  0XA5,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,
  0XA5,0XA5,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,
  0XA5,0XA5,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,
  0XA5,0XA5,0XA5,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA5,
  0XA5,0XA5,0XA5,0XA5,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA5,0XA5,
  0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA4,0XA5,0XA5,0XA5,0XA5,
  0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA4,0XA4,0XA4,0XA4,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,
  0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,
  0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5,0XA5
};
