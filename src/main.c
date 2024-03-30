#include <gb/gb.h>
#include <gbdk/font.h>
#include <stdint.h>
#include <rand.h>

#include "player.h"
#include "player_py.h"
#include "block_tiles_py.h"

#define KEY_PRESSED(K) (current_input & (K))

void main(void){
  /*
   * Load background and sprite data
   */

  // Load font tiles to background map
  font_t min_font;
  font_init();
  min_font = font_load(font_min);
  font_set(min_font);

  // Load background tiles
  set_bkg_data(0,1,block_tiles);

  // Load sprite data
  set_sprite_data(0,1,player_data);

  /*
   * Create a player and display the sprite 
   */
  struct Player player;
  player.sprite_id = 0;
  player.x = 20;
  player.y = 77;
  player.speed = 2;
  player.dir = LEFT;
  move_sprite(player.sprite_id, player.x, player.y);

  /*
   * Turn on display and show sprites and background 
   */
  DISPLAY_ON;
  SHOW_SPRITES;
  SHOW_BKG;


  /*
   * Game Looop
   */
  uint8_t current_input;
  uint8_t dx,dy;
  uint16_t frame_count = 0;

  while(1) {
    current_input = joypad();

    // D-PAD
    if ((player.dir & LEFT) || (player.dir & RIGHT)){
      if (KEY_PRESSED(J_UP)){
        dx += 0;
        dy += -player.speed;
      }
      if (KEY_PRESSED(J_DOWN)){
        dx += 0;
        dy += player.speed;
      }
    }
    // else if ((player.dir & UP) || (player.dir & DOWN)){
    if ((player.dir & UP) || (player.dir & DOWN)){
      if (KEY_PRESSED(J_RIGHT)){
        dx += player.speed;
        dy += 0;
      }
      if (KEY_PRESSED(J_LEFT)){
        dx += -player.speed;
        dy += 0;
      }
    }

    // Update player position
    player.x += dx;
    player.y += dy;
    move_sprite(player.sprite_id, player.x, player.y);

    // Reset displacement values
    dx = 0;
    dy = 0;

    // Wait for frame to finish drawing
    wait_vbl_done();

    // frame_count += 1;
    // if ((frame_count % 255) == 0){
      // player.dir += 1;
      // if (player.dir > 3){
        // player.dir = 0;
      // }
    // }
  }
}
