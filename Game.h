#include <Arduboy2.h>
#include <ArduboyPlaytune.h>

class Game {
    public: 
        Game(Arduboy2*, ArduboyPlaytune*);
        void update();
        void draw();

    private: 
        void updateStartMenu();
        void drawStartMenu();
        void updateHelpMenu();
        void drawHelpMenu();
        void updateGame();
        void drawGame();
        void increaseScoreAndDifficulty();
        void resetGame();
        void toggleVolume();
};

enum class GameState
{
    StartMenu,
    InGame,
    InHelp,
};

struct Entity {
    bool alive;
    int16_t x;
    int16_t y;
    uint8_t speed;
};
