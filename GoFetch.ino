#include <Arduboy2.h>
#include <ArduboyPlaytune.h>
#include "Game.h"

Arduboy2 arduboy;
ArduboyPlaytune tunes(arduboy.audio.enabled);

Game game(&arduboy, &tunes);

void setup()
{
    // initialize the arduboy
    arduboy.initRandomSeed();
    arduboy.begin();
    arduboy.audio.on();
    arduboy.setFrameRate(30);

    tunes.initChannel(PIN_SPEAKER_1);
    tunes.initChannel(PIN_SPEAKER_2);
}

void loop()
{
    // pause render until it's time for the next frame
    if (!(arduboy.nextFrame()))
        return;

    arduboy.clear();

    game.update();
    game.draw();

    arduboy.display();
}
