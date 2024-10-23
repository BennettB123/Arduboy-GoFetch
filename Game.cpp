#include <Arduboy2.h>
#include "Game.h"
#include "assets/Sounds.h"
#include "assets/BallThrowSprite.h"
#include "assets/DogTailWagSprite.h"
#include "assets/DogRunningSprite.h"
#include "assets/DogBarkSprite.h"
#include "assets/SquirrelSprite.h"
#include "assets/BallSprite.h"
#include "assets/VolumeSprites.h"
#include "assets/GrassSprites.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define STATUS_BAR_HEIGHT 8
#define NUM_GRASS 6
#define GRASS_SPEED 3

template <typename T>
T clamp(T num, T min, T max)
{
    if (num < min)
        return min;
    else if (num > max)
        return max;
    else
        return num;
}

Arduboy2 *_arduboy;
ArduboyPlaytune *_tunes;
bool volume_on = true;
GameState game_state = GameState::StartMenu;
GameState last_game_state = game_state; // for knowing which state to go back to when exiting help menu
uint8_t dog_speed_x = 3;
uint8_t dog_speed_y = 2;

int8_t ball_throw_frame_counter = 0;
int8_t dog_tail_wag_frame_counter = 0;
int8_t dog_tail_wag_frame_incr = 1; // toggles between 1 and -1 to play sprite in forward & reverse
int8_t dog_running_frame_counter = 0;
int8_t dog_bark_frame_counter = 0;
int8_t ball_frame_counter = 0;
int8_t squirrel_frame_counter = 0;

uint8_t dog_bark_duration = 10;
uint8_t max_scroll_speed = 2;
uint8_t squirrel_spawn_chance = 3; // chance (out of 255) to spawn a squirrel per frame
uint8_t ball_spawn_chance = 4;     // chance (out of 255) to spawn a ball per frame

bool ready_to_throw = false;
bool ball_thrown = false;
int16_t dog_x = 0;
int16_t dog_y = (SCREEN_HEIGHT / 2) - (dog_running_sprite_height / 2);
bool dog_barking = false;
Entity squirrels[10];
Entity balls[10];
Grass grass[NUM_GRASS];
bool lost = false;
uint8_t lost_frames = 60;     // when lose, count down to 0 while flashing the dog sprite
uint8_t lost_game_flash = 10; // decrements to 0. when above 5, sprite=on, when below 5, sprite=off
uint16_t score = 0;
uint8_t num_barks = 3;
uint8_t bark_refill_start = 120;
uint8_t bark_refill = bark_refill_start; // counts down every frame. when reaches 0, gain a bark

Game::Game(Arduboy2 *arduboy, ArduboyPlaytune *tunes)
{
    _arduboy = arduboy;
    _tunes = tunes;
    initGrass();
}

void Game::initGrass()
{
    for (int i = 0; i < NUM_GRASS; i++)
    {
        grass[i].x = random(0, SCREEN_WIDTH);
        grass[i].y = random(STATUS_BAR_HEIGHT, SCREEN_HEIGHT - grass_sprite_height);
        grass[i].frame = random(0, grass_sprite_max_frame + 1);
    }
}

void Game::update()
{
    switch (game_state)
    {
    case GameState::StartMenu:
        updateStartMenu();
        break;
    case GameState::InHelp:
        updateHelpMenu();
        break;
    case GameState::InGame:
        updateGame();
        break;
    }
}

void Game::draw()
{
    switch (game_state)
    {
    case GameState::StartMenu:
        drawStartMenu();
        break;
    case GameState::InHelp:
        drawHelpMenu();
        break;
    case GameState::InGame:
        drawGame();
        break;
    }
}

void Game::resetGame()
{
    game_state = GameState::StartMenu;
    last_game_state = GameState::StartMenu;

    ball_throw_frame_counter = 0;
    dog_tail_wag_frame_counter = 0;
    dog_tail_wag_frame_incr = 1;
    dog_running_frame_counter = 0;
    dog_bark_frame_counter = 0;
    ball_frame_counter = 0;
    squirrel_spawn_chance = 5;
    max_scroll_speed = 2;

    ready_to_throw = false;
    ball_thrown = false;
    dog_x = 0;
    dog_y = (SCREEN_HEIGHT / 2) - (dog_running_sprite_height / 2);
    dog_barking = false;

    for (uint8_t i = 0; i < 10; i++)
    {
        squirrels[i] = Entity();
        balls[i] = Entity();
    }
    initGrass();

    lost = false;
    lost_frames = 60;
    lost_game_flash = 10;
    score = 0;
    num_barks = 3;
    bark_refill_start = 120;
    bark_refill = bark_refill_start;
}

void Game::updateStartMenu()
{
    // if we reach the last ball throw frame, start the game
    if (ball_throw_frame_counter >= ball_throw_max_frame)
    {
        game_state = GameState::InGame;
    }

    if (_arduboy->justPressed(B_BUTTON))
    {
        game_state = GameState::InHelp;
        last_game_state = GameState::StartMenu;
    }

    if (_arduboy->pressed(A_BUTTON))
    {
        // go through first sequence of throw animation
        if (ball_throw_frame_counter < 16)
            ball_throw_frame_counter++;
        else
            ready_to_throw = true;
    }
    else
    {
        if (!ball_thrown)
        {
            // A_BUTTON has been released, continue the last part of ball throw animation
            if (ready_to_throw)
            {
                ball_thrown = true;
                _tunes->playScore(ball_throw_sound);
            }
            else if (ball_throw_frame_counter > 0)
                ball_throw_frame_counter--;
        }
    }

    if (ball_thrown)
    {
        ball_throw_frame_counter++;
    }

    // iterate forward/backward through dog tail wag frames
    dog_tail_wag_frame_counter += dog_tail_wag_frame_incr;
    if (dog_tail_wag_frame_counter >= dog_tail_wag_max_frame)
        dog_tail_wag_frame_incr = -1;
    else if (dog_tail_wag_frame_counter <= 0)
        dog_tail_wag_frame_incr = 1;
}

void Game::drawStartMenu()
{
    Sprites::drawOverwrite(0, 0, ball_throw_sprite, ball_throw_frame_counter);
    Sprites::drawOverwrite(60, 32, dog_tail_wag_sprite, dog_tail_wag_frame_counter);

    if (!ready_to_throw)
    {
        _arduboy->setTextSize(1);
        _arduboy->setCursor(38, 0);
        _arduboy->println(F("Hold A > Start"));
        _arduboy->setCursorX(38);
        _arduboy->println(F("     B > Help"));
    }
    else
    {
        _arduboy->setCursor(42, 0);
        _arduboy->setTextSize(2);
        _arduboy->print(F("Let Go!"));
    }
}

void Game::updateHelpMenu()
{
    if (_arduboy->justPressed(B_BUTTON))
        game_state = last_game_state;

    if (_arduboy->justPressed(A_BUTTON))
        toggleVolume();
}

void Game::toggleVolume()
{
    volume_on = !volume_on;
    if (volume_on)
    {
        _arduboy->audio.on();
        _tunes->tone(1500, 20);
    }
    else
    {
        _arduboy->audio.off();
    }
}

void Game::drawHelpMenu()
{
    _arduboy->setTextSize(1);
    _arduboy->setCursor(0, 0);
    _arduboy->println(F("- Collect balls"));
    _arduboy->println(F("- Avoid squirrels"));
    _arduboy->println(F("- Press A button to"));
    _arduboy->println(F(" bark squirrels away"));
    _arduboy->println(F("- Barks refill over"));
    _arduboy->println(F(" time"));

    _arduboy->setCursorX(52);
    _arduboy->println(F("A:    B: Back"));

    if (volume_on)
        Sprites::drawOverwrite(64, 46, volume_on_sprite, 0);
    else
        Sprites::drawOverwrite(64, 46, volume_off_sprite, 0);
}

void Game::updateGame()
{
    if (lost)
    {
        if (lost_frames == 0)
        {
            if (_arduboy->anyPressed(A_BUTTON | B_BUTTON | UP_BUTTON | DOWN_BUTTON | LEFT_BUTTON | RIGHT_BUTTON))
            {
                resetGame();
            }
        }

        return;
    }

    if (_arduboy->justPressed(B_BUTTON))
    {
        game_state = GameState::InHelp;
        last_game_state = GameState::InGame;
    }

    // move entities and un-alive them if off screen
    for (uint8_t i = 0; i < 10; i++)
    {
        if (squirrels[i].alive)
        {
            squirrels[i].x -= squirrels[i].speed;
            if (squirrels[i].x < -16)
            {
                squirrels[i].alive = false;
            }
        }

        if (balls[i].alive)
        {
            balls[i].x -= balls[i].speed;
            if (balls[i].x < -16)
            {
                balls[i].alive = false;
            }
        }
    }

    // update grass
    for (int i = 0; i < NUM_GRASS; i++)
    {
        grass[i].x -= GRASS_SPEED;

        if (grass[i].x < (0 - grass_sprite_width))
        {
            grass[i].x = random(SCREEN_WIDTH, SCREEN_WIDTH * 2); // random off screen x location
            grass[i].y = random(STATUS_BAR_HEIGHT, SCREEN_HEIGHT - grass_sprite_height);
            grass[i].frame = random(0, grass_sprite_max_frame + 1);
        }
    }

    // chance to spawn squirrel
    if (random(0, 255) < squirrel_spawn_chance)
    {
        // find un-alive entity and ressurect them
        for (uint8_t i = 0; i < 10; i++)
        {
            if (squirrels[i].alive)
                continue;

            squirrels[i].alive = true;
            squirrels[i].x = SCREEN_WIDTH;
            squirrels[i].y = random(STATUS_BAR_HEIGHT, SCREEN_HEIGHT - 16);
            squirrels[i].speed = random(1, max_scroll_speed + 1);
            break;
        }
    }

    // chance to spawn ball
    if (random(0, 255) < ball_spawn_chance)
    {
        // find un-alive entity and ressurect them
        for (uint8_t i = 0; i < 10; i++)
        {
            if (balls[i].alive)
                continue;

            balls[i].alive = true;
            balls[i].x = SCREEN_WIDTH;
            balls[i].y = random(STATUS_BAR_HEIGHT, SCREEN_HEIGHT - 16);
            balls[i].speed = random(1, max_scroll_speed + 1);
            break;
        }
    }

    // check collisions with entities
    for (uint8_t i = 0; i < 10; i++)
    {
        // make dog's hitbox a bit smaller than it's sprite
        Rect dog_hit_box = Rect(dog_x + 1, dog_y + 1, dog_running_sprite_width - 2, dog_running_sprite_height - 2);
        Rect bark_hit_box = Rect(dog_x + 26, dog_y - 6, dog_bark_sprite_width, dog_bark_sprite_height + 2);
        Rect entity_hit_box;

        if (squirrels[i].alive)
        {
            entity_hit_box = Rect(squirrels[i].x, squirrels[i].y, 16, 8);

            if (_arduboy->collide(dog_hit_box, entity_hit_box))
            {
                _tunes->playScore(lose_sound);
                lost = true;
            }

            // check collisions of bark with guys
            if (dog_barking)
            {
                if (_arduboy->collide(bark_hit_box, entity_hit_box))
                    squirrels[i].alive = false;
            }
        }

        if (balls[i].alive)
        {
            entity_hit_box = Rect(balls[i].x, balls[i].y, ball_sprite_width, ball_sprite_height);

            if (_arduboy->collide(dog_hit_box, entity_hit_box))
            {
                _tunes->playScore(coin_collected_sound);
                increaseScoreAndDifficulty();
                balls[i].alive = false;
            }
        }
    }

    // regenerate barks over time
    if (num_barks < 3)
    {
        bark_refill--;
        if (bark_refill <= 0)
        {
            bark_refill = bark_refill_start;
            num_barks++;
        }
    }

    // if barking, update barking sprite
    if (dog_barking)
    {
        dog_bark_frame_counter++;
        if (dog_bark_frame_counter > dog_bark_duration)
        {
            dog_barking = false;
            dog_bark_frame_counter = 0;
        }
    }

    // update running sprite
    dog_running_frame_counter++;
    if (dog_running_frame_counter > dog_running_max_frame)
        dog_running_frame_counter = 0;

    // update ball sprite
    ball_frame_counter++;
    if (ball_frame_counter > ball_sprite_max_frame)
        ball_frame_counter = 0;

    // update squirrel sprite
    squirrel_frame_counter++;
    if (squirrel_frame_counter > squirrel_max_frame)
        squirrel_frame_counter = 0;

    // Handle input
    if (_arduboy->pressed(UP_BUTTON))
        dog_y = clamp((int16_t)(dog_y - dog_speed_y), (int16_t)STATUS_BAR_HEIGHT, (int16_t)(SCREEN_HEIGHT - dog_running_sprite_height));
    if (_arduboy->pressed(DOWN_BUTTON))
        dog_y = clamp((int16_t)(dog_y + dog_speed_y), (int16_t)STATUS_BAR_HEIGHT, (int16_t)(SCREEN_HEIGHT - dog_running_sprite_height));
    if (_arduboy->pressed(LEFT_BUTTON))
        dog_x = clamp((int16_t)(dog_x - dog_speed_x), (int16_t)STATUS_BAR_HEIGHT, (int16_t)(SCREEN_WIDTH - dog_running_sprite_width - dog_bark_sprite_width));
    if (_arduboy->pressed(RIGHT_BUTTON))
        dog_x = clamp((int16_t)(dog_x + dog_speed_x), (int16_t)STATUS_BAR_HEIGHT, (int16_t)(SCREEN_WIDTH - dog_running_sprite_width - dog_bark_sprite_width));
    if (!dog_barking && num_barks > 0)
    {
        if (_arduboy->justPressed(A_BUTTON))
        {
            _tunes->playScore(bark_sound);
            dog_barking = true;
            num_barks--;
        }
    }
}

void Game::increaseScoreAndDifficulty()
{
    score++;

    if (score % 10 == 0)
    {
        squirrel_spawn_chance++;
        bark_refill_start += 20;
    }

    if (score % 25 == 0)
        max_scroll_speed++;
}

void Game::drawGame()
{
    if (lost)
    {
        if (lost_frames > 0)
        {
            lost_game_flash--;
            if (lost_game_flash == 0)
                lost_game_flash = 10;

            lost_frames--;
        }
        else
        {
            _arduboy->setCursor(12, 16);
            _arduboy->setTextSize(2);
            _arduboy->println(F("Game Over"));

            _arduboy->setTextSize(1);
            _arduboy->setCursorX(16);
            _arduboy->println(F("press any button"));
            _arduboy->setCursorX(40);
            _arduboy->print(F("to retry"));

            // still draw score on gameover screen
            _arduboy->setCursor(96, 0);
            _arduboy->print(score);

            return;
        }
    }

    _arduboy->setTextSize(1);
    _arduboy->setCursor(0, 0);
    _arduboy->print(F("barks:"));

    // draw barks as empty or filled in circles
    for (uint8_t i = 1; i <= 3; i++)
    {
        uint8_t x = 30 + (10 * i);
        uint8_t y = (STATUS_BAR_HEIGHT / 2) - 1;
        if (num_barks - i >= 0)
            _arduboy->fillCircle(x, y, (STATUS_BAR_HEIGHT / 2) - 1, WHITE);
        else
            _arduboy->drawCircle(x, y, (STATUS_BAR_HEIGHT / 2) - 1, WHITE);
    }

    _arduboy->setCursor(96, 0);
    _arduboy->print(score);

    // Draw grass
    for (auto g : grass)
    {
        Sprites::drawSelfMasked(g.x, g.y, grass_sprite, g.frame);
    }

    // Draw dog
    if (lost_game_flash > 5)
        Sprites::drawSelfMasked(dog_x, dog_y, dog_running_sprite, dog_running_frame_counter);

    if (dog_barking)
        Sprites::drawSelfMasked(dog_x + 26, dog_y - 2, dog_bark_sprite, dog_bark_frame_counter % (dog_bark_max_frame + 1));

    // Draw squirrel guys
    for (auto squirrel : squirrels)
    {
        if (squirrel.alive)
            Sprites::drawSelfMasked(squirrel.x, squirrel.y, squirrel_sprite, squirrel_frame_counter);
    }

    // Draw balls
    for (auto ball : balls)
    {
        if (ball.alive)
            Sprites::drawSelfMasked(ball.x, ball.y, ball_sprite, ball_frame_counter);
    }
}
