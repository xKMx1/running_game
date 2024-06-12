#include "Player.h"

Player::Player(){
    hitBox = {PLAYER_START_X, PLAYER_START_Y, PLAYER_WIDTH, PLAYER_HEIGHT};
    velocity.x = 0.0f;
    velocity.y = 0.0f;
    isJumping = false;
    acceleration.x = 1.0f;
    acceleration.y = GRAVITY;
}

void Player::moveLeft(){
    velocity.x -= acceleration.x;
    if (velocity.x < -maxVelocity.x) {
        velocity.x = -maxVelocity.x;
    }
}

void Player::moveRight(){
    velocity.x += acceleration.x;
    if (velocity.x > maxVelocity.x) {
        velocity.x = maxVelocity.x;
    }
}

void Player::stopMoving(){
    velocity.x = 0;
}

void Player::jump(){
    if( !isJumping ){
        velocity.y = JUMP_VELOCITY;
        isJumping = true;
    }
}

void Player::applyGravity(){
    if( isJumping ){
        hitBox.y += static_cast<int>(velocity.y);
        velocity.y += GRAVITY;
    }

    // Ograniczenie prędkości w kierunku Y
    if (velocity.y > maxVelocity.y) {
        velocity.y = maxVelocity.y;
    }
}