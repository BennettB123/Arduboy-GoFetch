#include <Arduboy2.h>
#include "Game.h"

Arduboy2 arduboy;
Game game(arduboy);

void setup()
{
    // initialize the arduboy
    arduboy.initRandomSeed();
    arduboy.begin();
    arduboy.setFrameRate(30);
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
