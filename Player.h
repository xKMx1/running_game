#ifndef PLAYER
#define PLAYER

#include <SDL.h>
#include <SDL_image.h>

const int PLAYER_WIDTH = 24;
const int PLAYER_HEIGHT = 37;
const int PLAYER_START_X = 200;
const int PLAYER_START_Y = 700;
const float PLAYER_VELOCITY = 1.0f;
const float GRAVITY = 1.0f;
const float JUMP_VELOCITY = -20.0f;

struct Vec2f{
    float x;
    float y;
};


class Player{
    public: 
        SDL_Rect hitBox;
        Vec2f velocity;
        bool isJumping;
        Vec2f acceleration;
        Vec2f maxVelocity = { 10.0f, 10.0f };

        Player();

        void moveLeft();

        void moveRight();

        void stopMoving();

        void jump();
        
        void applyGravity();
};

#endif
