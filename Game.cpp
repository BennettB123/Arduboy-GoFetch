#include <Arduboy2.h>
#include "assets/BallThrowSprite.h"
#include "assets/DogTailWag.h"

enum class GameState
{
    StartMenu,
    InGame,
};

class Game
{
private:
    Arduboy2 _arduboy;
    GameState _gameState = GameState::StartMenu;

    uint8_t _ball_throw_frame_counter = 0;
    uint8_t _dog_tail_wag_frame_counter = 0;
    uint8_t _dog_tail_wag_frame_incr = 1; // toggles between 1 and -1 to play sprite in forward & reverse
    bool _ready_to_throw = false;

public:
    Game(Arduboy2 arduboy)
    {
        this->_arduboy = arduboy;
    }

    void update()
    {
        switch (_gameState)
        {
        case GameState::StartMenu:
            updateStartMenu();
            break;
        case GameState::InGame:
            updateInGame();
            break;
        }
    }

    void draw()
    {
        switch (_gameState)
        {
        case GameState::StartMenu:
            drawStartMenu();
            break;
        case GameState::InGame:
            drawInGame();
            break;
        }
    }

private:
    void updateStartMenu()
    {
        // if we reach the last ball throw frame, start the game
        if (_ball_throw_frame_counter >= ball_throw_num_frames)
        {
            _gameState = GameState::InGame;
        }

        if (_arduboy.pressed(A_BUTTON))
        {
            // go through first sequence of throw animation
            if (_ball_throw_frame_counter < 16)
                _ball_throw_frame_counter++;
            else
                _ready_to_throw = true;
        }
        else
        {
            // A_BUTTON has been released, continue the last part of ball throw animation
            if (_ready_to_throw)
            {
                _ball_throw_frame_counter++;
            }
            else if (_ball_throw_frame_counter > 0)
                _ball_throw_frame_counter--;
        }

        // iterate forward/backward through dog tail wag frames
        _dog_tail_wag_frame_counter += _dog_tail_wag_frame_incr;
        if (_dog_tail_wag_frame_counter >= dog_tail_wag_num_frames)
            _dog_tail_wag_frame_incr = -1;
        else if (_dog_tail_wag_frame_counter <= 0)
            _dog_tail_wag_frame_incr = 1;
    }

    void drawStartMenu()
    {
        Sprites::drawOverwrite(0, 0, ball_throw_sprite, _ball_throw_frame_counter);
        Sprites::drawOverwrite(60, 32, dog_tail_wag_sprite, _dog_tail_wag_frame_counter);

        if (!_ready_to_throw)
        {
            _arduboy.setTextSize(1);
            _arduboy.setCursor(42, 0);
            _arduboy.println(F("Hold (A) to"));
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

    void updateInGame()
    {
    }

    void drawInGame()
    {
        _arduboy.setCursor(0, 0);
        _arduboy.setTextSize(2);
        _arduboy.print(F("(Insert\nEpic Ball\nChase\nGameplay)"));
    }
};