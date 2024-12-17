#include "hUGEDriver.h"

void mute_all_channels(void) {
  hUGE_mute_channel(HT_CH1, HT_CH_MUTE);
  hUGE_mute_channel(HT_CH2, HT_CH_MUTE);
  hUGE_mute_channel(HT_CH4, HT_CH_MUTE);
}

void play_all_channels(void) {
  hUGE_mute_channel(HT_CH1, HT_CH_PLAY);
  hUGE_mute_channel(HT_CH2, HT_CH_PLAY);
  hUGE_mute_channel(HT_CH4, HT_CH_PLAY);
}

void play_gun_sound(void) {
  // Stop channel before playing sound effect.
  hUGE_mute_channel(HT_CH1, HT_CH_MUTE);

  NR12_REG = 0x0;
  NR14_REG = 0x0;

  // Play sound effect.
  NR10_REG = 0x4D;
  NR11_REG = 0xC1;
  NR12_REG = 0xF2;
  NR13_REG = 0x9B;
  NR14_REG = 0x87;

  // Restart channel.
  hUGE_mute_channel(HT_CH1, HT_CH_PLAY);
}

void play_bomb_sound(void) {
  // Stop channel before playing sound effect.
  hUGE_mute_channel(HT_CH4, HT_CH_MUTE);

  NR42_REG = 0x0;
  NR44_REG = 0x0;

  // Play sound effect.
  NR41_REG = 0x00;
  NR42_REG = 0xF7;
  NR43_REG = 0x71;
  NR44_REG = 0x80;

  // Restart channel.
  hUGE_mute_channel(HT_CH4, HT_CH_PLAY);
}

void play_health_sound(void) {
  // Stop channel before playing sound effect.
  hUGE_mute_channel(HT_CH1, HT_CH_MUTE);

  NR12_REG = 0x0;
  NR14_REG = 0x0;

  // Play sound effect.
  NR10_REG = 0x75;
  NR11_REG = 0x86;
  NR12_REG = 0x5F;
  NR13_REG = 0x62;
  NR14_REG = 0x86;

  // Restart channel.
  hUGE_mute_channel(HT_CH1, HT_CH_PLAY);
}

void play_shield_sound(void) {
  // Stop channel before playing sound effect.
  hUGE_mute_channel(HT_CH4, HT_CH_MUTE);

  NR42_REG = 0x0;
  NR44_REG = 0x0;

  // Play sound effect.
  NR41_REG = 0x3F;
  NR42_REG = 0x74;
  NR43_REG = 0x2C;
  NR44_REG = 0xC0;

  // Restart channel.
  hUGE_mute_channel(HT_CH4, HT_CH_PLAY);
}

void play_gameover_sound(void) {
  // Stop channel before playing sound effect.
  hUGE_mute_channel(HT_CH1, HT_CH_MUTE);

  NR12_REG = 0x0;
  NR14_REG = 0x0;

  // Play sound effect.
  NR10_REG = 0x1C;
  NR11_REG = 0x89;
  NR12_REG = 0xF7;
  NR13_REG = 0x75;
  NR14_REG = 0x86;

  // Restart channel.
  hUGE_mute_channel(HT_CH1, HT_CH_PLAY);
}
