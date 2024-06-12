#ifndef GAME
#define GAME

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <deque>
#include <ctime>
#include <vector>
#include "TextureHandler.h"
#include "Timer.h"
#include "Player.h"

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

        void generateBlockIndependent();

        void renderBackgroundLayer(TextureHandler& backgroundTexture, const SDL_Rect& camera);

        bool checkCollision( SDL_Rect a, SDL_Rect b );

        bool init();

        bool loadMedia();

        void close();

        void gameLoop();

};


#endif