#ifndef _SPRITES_H_
#define _SPRITES_H_

#include <stdbool.h>
#include <stdint.h>

enum direction{
  NONE=0,
  UP=1,
  RIGHT=2,
  DOWN=4,
  LEFT=8,
};

enum sprite_type{
  PLAYER=0,
  BULLET=1,
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
  enum direction dir;
  uint8_t cb_x_offset;
  uint8_t cb_y_offset;
  struct CollisionBox cb;
  int8_t health;
  bool active;
  uint8_t lifespan;
  enum sprite_type type;
  // Whether or not this sprite collided with an object this frame
  bool collided;
  uint8_t collided_row;
  uint8_t collided_col;
};

#endif
