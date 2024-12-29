#ifndef _SPRITES_H_
#define _SPRITES_H_

#include <stdint.h>

enum direction{
  UP=1,
  RIGHT=2,
  DOWN=4,
  LEFT=8,
};

enum sprite_type{
  PLAYER=0,
  BULLET=1,
  BOMBS=2,
  LASER=3
};

struct CollisionBox {
  uint8_t x;
  uint8_t y;
  uint8_t w;
  uint8_t h;
};

struct Sprite {
  uint8_t sprite_id;
  uint8_t sprite_tile_id;
  uint8_t x;
  uint8_t y;
  uint8_t speed;
  uint8_t cb_x_offset;
  uint8_t cb_y_offset;
  enum direction dir;
  struct CollisionBox cb;
  int8_t health;
  bool active;
  uint8_t lifespan;
  enum sprite_type type;
};

#endif
