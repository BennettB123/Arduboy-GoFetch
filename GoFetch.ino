#include <Arduboy2.h>

#include "assets/BallThrowSprite.h"
#include "assets/DogTailWag.h"

Arduboy2 arduboy;

void setup()
{
    // initialize the arduboy
    arduboy.initRandomSeed();
    arduboy.begin();
    arduboy.setFrameRate(30);
}

uint8_t ball_throw_frame_count = 0;
uint8_t dog_tail_wag_frame_count = 0;
uint8_t dog_tail_wag_frame_incr = 1;

void loop()
{
    // pause render until it's time for the next frame
    if (!(arduboy.nextFrame()))
        return;

    arduboy.clear();

    Sprites::drawOverwrite(0, 0, ball_throw_sprite, ball_throw_frame_count);
    ball_throw_frame_count++;
    if (ball_throw_frame_count > ball_throw_num_frames)
        ball_throw_frame_count = 0;

    Sprites::drawOverwrite(60, 32, dog_tail_wag_sprite, dog_tail_wag_frame_count);
    dog_tail_wag_frame_count += dog_tail_wag_frame_incr;
    if (dog_tail_wag_frame_count >= dog_tail_wag_num_frames)
        dog_tail_wag_frame_incr = -1;
    else if (dog_tail_wag_frame_count <= 0)
        dog_tail_wag_frame_incr = 1;

    arduboy.display();
}
