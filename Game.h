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
        void updateInGame();
        void drawInGame();
        void increaseScoreAndDifficulty();
        void resetGame();
};


struct Entity {
    bool alive;
    int16_t x;
    int16_t y;
    uint8_t speed;
};
