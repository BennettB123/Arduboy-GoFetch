#include <Arduboy2.h>
#include "Game.h"
#include "assets/BallThrowSprite.h"
#include "assets/DogTailWagSprite.h"
#include "assets/DogRunningSprite.h"
#include "assets/DogBarkSprite.h"
#include "assets/SquirrelSprite.h"
#include "assets/BallSprite.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define STATUS_BAR_HEIGHT 8

enum class GameState
{
    StartMenu,
    InGame,
};

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

Arduboy2 _arduboy;
GameState game_state = GameState::StartMenu;
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
uint8_t ball_spawn_chance = 4;    // chance (out of 255) to spawn a ball per frame

bool ready_to_throw = false;
int16_t dog_x = 0;
int16_t dog_y = (SCREEN_HEIGHT / 2) - (dog_running_sprite_height / 2);
bool dog_barking = false;
Entity squirrels[10];
Entity balls[10];
bool lost = false;
uint8_t lost_frames = 60;     // when lose, count down to 0 while flashing the dog sprite
uint8_t lost_game_flash = 10; // decrements to 0. when above 5, sprite=on, when below 5, sprite=off
uint16_t score = 0;
uint8_t num_barks = 3;
uint8_t bark_refill_start = 120;
uint8_t bark_refill = bark_refill_start; // counts down every frame. when reaches 0, gain a bark

Game::Game(Arduboy2 arduboy)
{
    arduboy = arduboy;
}

void Game::update()
{
    switch (game_state)
    {
    case GameState::StartMenu:
        updateStartMenu();
        break;
    case GameState::InGame:
        updateInGame();
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
    case GameState::InGame:
        drawInGame();
        break;
    }
}

void Game::resetGame()
{
    game_state = GameState::StartMenu;

    ball_throw_frame_counter = 0;
    dog_tail_wag_frame_counter = 0;
    dog_tail_wag_frame_incr = 1;
    dog_running_frame_counter = 0;
    dog_bark_frame_counter = 0;
    ball_frame_counter = 0;
    squirrel_spawn_chance = 5;
    max_scroll_speed = 2;

    ready_to_throw = false;
    dog_x = 0;
    dog_y = (SCREEN_HEIGHT / 2) - (dog_running_sprite_height / 2);
    dog_barking = false;

    for (uint8_t i = 0; i < 10; i++)
    {
        squirrels[i] = Entity();
        balls[i] = Entity();
    }

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

    if (_arduboy.pressed(A_BUTTON))
    {
        // go through first sequence of throw animation
        if (ball_throw_frame_counter < 16)
            ball_throw_frame_counter++;
        else
            ready_to_throw = true;
    }
    else
    {
        // A_BUTTON has been released, continue the last part of ball throw animation
        if (ready_to_throw)
        {
            ball_throw_frame_counter++;
        }
        else if (ball_throw_frame_counter > 0)
            ball_throw_frame_counter--;
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
        _arduboy.setTextSize(1);
        _arduboy.setCursor(42, 0);
        _arduboy.println(F("Hold A to"));
        _arduboy.setCursorX(42);
        _arduboy.print(F("Throw the Ball!"));
    }
    else
    {
        _arduboy.setCursor(42, 0);
        _arduboy.setTextSize(2);
        _arduboy.print(F("Let Go!"));
    }
}

void Game::updateInGame()
{
    if (lost)
    {
        if (lost_frames == 0)
        {
            if (_arduboy.anyPressed(A_BUTTON | B_BUTTON | UP_BUTTON | DOWN_BUTTON | LEFT_BUTTON | RIGHT_BUTTON))
            {
                resetGame();
            }
        }

        return;
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

            if (_arduboy.collide(dog_hit_box, entity_hit_box))
                lost = true;

            // check collisions of bark with guys
            if (dog_barking)
            {
                if (_arduboy.collide(bark_hit_box, entity_hit_box))
                    squirrels[i].alive = false;
            }
        }

        if (balls[i].alive)
        {
            entity_hit_box = Rect(balls[i].x, balls[i].y, ball_sprite_width, ball_sprite_height);

            if (_arduboy.collide(dog_hit_box, entity_hit_box))
            {
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
    if (_arduboy.pressed(UP_BUTTON))
        dog_y = clamp((int16_t)(dog_y - dog_speed_y), (int16_t)STATUS_BAR_HEIGHT, (int16_t)(SCREEN_HEIGHT - dog_running_sprite_height));
    if (_arduboy.pressed(DOWN_BUTTON))
        dog_y = clamp((int16_t)(dog_y + dog_speed_y), (int16_t)STATUS_BAR_HEIGHT, (int16_t)(SCREEN_HEIGHT - dog_running_sprite_height));
    if (_arduboy.pressed(LEFT_BUTTON))
        dog_x = clamp((int16_t)(dog_x - dog_speed_x), (int16_t)STATUS_BAR_HEIGHT, (int16_t)(SCREEN_WIDTH - dog_running_sprite_width - dog_bark_sprite_width));
    if (_arduboy.pressed(RIGHT_BUTTON))
        dog_x = clamp((int16_t)(dog_x + dog_speed_x), (int16_t)STATUS_BAR_HEIGHT, (int16_t)(SCREEN_WIDTH - dog_running_sprite_width - dog_bark_sprite_width));
    if (!dog_barking && num_barks > 0)
    {
        if (_arduboy.pressed(A_BUTTON))
        {
            dog_barking = true;
            num_barks--;
        }
    }
}

void Game::increaseScoreAndDifficulty()
{
    score++;

    if (score % 10 == 0) {
        squirrel_spawn_chance++;
        bark_refill_start += 20;
    }

    if (score % 25 == 0)
        max_scroll_speed++;
}

void Game::drawInGame()
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
            _arduboy.setCursor(12, 16);
            _arduboy.setTextSize(2);
            _arduboy.println("Game Over");

            _arduboy.setTextSize(1);
            _arduboy.setCursorX(16);
            _arduboy.println("press any button");
            _arduboy.setCursorX(40);
            _arduboy.print("to retry");

            // still draw score on gameover screen
            _arduboy.setCursor(96, 0);
            _arduboy.print(score);

            return;
        }
    }

    _arduboy.setTextSize(1);
    _arduboy.setCursor(0, 0);
    _arduboy.print("barks:");

    // draw barks as empty or filled in circles
    for (uint8_t i = 1; i <= 3; i++)
    {
        uint8_t x = 30 + (10 * i);
        uint8_t y = (STATUS_BAR_HEIGHT / 2) - 1;
        if (num_barks - i >= 0)
            _arduboy.fillCircle(x, y, (STATUS_BAR_HEIGHT / 2) - 1, WHITE);
        else
            _arduboy.drawCircle(x, y, (STATUS_BAR_HEIGHT / 2) - 1, WHITE);
    }

    _arduboy.setCursor(96, 0);
    _arduboy.print(score);

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
