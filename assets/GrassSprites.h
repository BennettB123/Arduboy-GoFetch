#pragma once

#include <stdint.h>
#include <avr/pgmspace.h>

constexpr uint8_t grass_sprite_width = 2;
constexpr uint8_t grass_sprite_height = 3;
constexpr uint8_t grass_sprite_max_frame = 3;

constexpr uint8_t grass_sprite[] PROGMEM
{
  grass_sprite_width, grass_sprite_height,

  //Frame 0
  0x06, 0x01, 

  //Frame 1
  0x01, 0x06, 

  //Frame 2
  0x03, 0x04, 

  //Frame 3
  0x04, 0x03
};
