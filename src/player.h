#include <stdint.h>

enum direction{
  UP,
  RIGHT,
  DOWN,
  LEFT,
};

struct Player {
  uint8_t sprite_id;
  uint8_t sprite_tile_id;
  uint8_t x;
  uint8_t y;
  uint8_t speed;
  enum direction dir; 
};

