#ifndef GAME
#define GAME

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <deque>
#include <SDL_mixer.h>
#include <ctime>
#include <vector>

#include "TextureHandler.h"
#include "Timer.h"
#include "Player.h"

struct Block {
    SDL_Rect rect;
};

struct PlayerState {
    SDL_Rect hitBox;
    Vec2f velocity;
    bool isJumping;
    Uint32 timestamp;
};

class Game{
    public:
        Player player;
        Timer blockGenerationTimer;
        Timer rewindTimer;
        int blockGenerationInterval;
        const Uint32 rewindDuration = 4000; // duration in milliseconds
        bool isRewinding = false;
        

        Game();

        void generateBlock();

        void renderBackgroundLayer(TextureHandler& backgroundTexture, const SDL_Rect& camera);

        bool checkCollision( SDL_Rect a, SDL_Rect b );

        void handleKeyDown(SDL_Event &e, bool &movingLeft, bool &movingRight);

        void handleKeyUp(SDL_Event &e, bool &movingLeft, bool &movingRight);

        PlayerState saveCurrentPlayerState();

        void handlePlayerMovement(bool movingLeft, bool movingRight, SDL_RendererFlip &flipType, bool isOnBlock);

        void handlePlayerCollision(bool &isPlayerOnBlock);

        void renderGame(int &animationFrameDelay, int frame, SDL_RendererFlip &flipType, int width, float &portion, int &multiplier, bool movingLeft, bool movingRight);

        SDL_Rect* getCurrentClip(int frame, bool movingLeft, bool movingRight);

        bool init();

        bool loadMedia();

        void close();

        void gameLoop();

};


#endif