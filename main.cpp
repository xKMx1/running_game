#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

// Stałe dotyczące ekranu
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Stałe dotyczące postaci
const float PLAYER_WIDTH = 200.0f;
const float PLAYER_HEIGHT = 200.0f;
const float PLAYER_VELOCITY = 5.0f;
const float GRAVITY = 1.0f;
const float JUMP_VELOCITY = -20.0f;

// Struktura reprezentująca gracza
struct Player {
    SDL_Rect rect;
    float velocityX;
    float velocityY;
    bool isJumping;
    float accelerationX = 0.7f; // Przyspieszenie w kierunku X
    float maxVelocityX = 10.0f; // Maksymalna prędkość w kierunku X
    float maxVelocityY = 10.0f; // Maksymalna prędkość w kierunku Y

    Player(float x, float y) {
        rect = {static_cast<int>(x), static_cast<int>(y), static_cast<int>(PLAYER_WIDTH), static_cast<int>(PLAYER_HEIGHT)};
        velocityX = 0.0f;
        velocityY = 0.0f;
        isJumping = false;
    }

    void moveLeft() {
        velocityX -= accelerationX;
        if (velocityX < -maxVelocityX) {
            velocityX = -maxVelocityX;
        }
    }

    void moveRight() {
        velocityX += accelerationX;
        if (velocityX > maxVelocityX) {
            velocityX = maxVelocityX;
        }
    }

    void stopMoving() {
        velocityX = 0.0f;
    }

    void jump() {
        if (!isJumping) {
            velocityY = JUMP_VELOCITY;
            isJumping = true;
        }
    }

    void applyGravity() {
        if (isJumping) {
            rect.y += static_cast<int>(velocityY);
            velocityY += GRAVITY;
            if (rect.y >= SCREEN_HEIGHT - PLAYER_HEIGHT) {
                rect.y = SCREEN_HEIGHT - PLAYER_HEIGHT;
                isJumping = false;
            }
        }

        // Ograniczenie prędkości w kierunku Y
        if (velocityY > maxVelocityY) {
            velocityY = maxVelocityY;
        }
    }
};

SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Surface* tempSurface = IMG_Load(path);
    if (tempSurface == nullptr) {
        std::cerr << "Failed to load image: " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    if (texture == nullptr) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
    }
    return texture;
}

int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Ładowanie tekstur gracza
    SDL_Texture* playerStandingTexture = loadTexture("player.png", renderer);
    SDL_Texture* playerMovingLeftTexture = loadTexture("run_left_1.png", renderer);
    SDL_Texture* playerMovingRightTexture = loadTexture("run_right_1.png", renderer);
    SDL_Texture* playerJumpingTexture = loadTexture("player.png", renderer);

    if (!playerStandingTexture || !playerMovingLeftTexture || !playerMovingRightTexture || !playerJumpingTexture) {
        std::cerr << "Failed to load all textures." << std::endl;
        return 1;
    }

    Player player(100.0f, SCREEN_HEIGHT - PLAYER_HEIGHT);

    bool quit = false;
    SDL_Event e;

    // Kontrola liczby klatek na sekundę
    const int TARGET_FPS = 120;
    const int FRAME_DELAY = 1000 / TARGET_FPS;
    Uint32 frameStart, frameTime;

    bool movingLeft = false;
    bool movingRight = false;

    while (!quit) {
        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                        movingLeft = true;
                        break;
                    case SDLK_RIGHT:
                        movingRight = true;
                        break;
                    case SDLK_SPACE:
                        player.jump();
                        break;
                    default:
                        break;
                }
            }
            else if (e.type == SDL_KEYUP) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                        movingLeft = false;
                        break;
                    case SDLK_RIGHT:
                        movingRight = false;
                        break;
                    default:
                        break;
                }
            }
        }

        // Aktualizacja prędkości gracza na podstawie klawiszy kierunkowych
        if (movingLeft && !movingRight) {
            player.moveLeft();
        }
        else if (movingRight && !movingLeft) {
            player.moveRight();
        }
        else {
            player.stopMoving();
        }

        // Aktualizacja pozycji gracza na podstawie prędkości
        player.rect.x += static_cast<int>(player.velocityX);

        // Zastosowanie grawitacji
        player.applyGravity();

        // Wyczyszczenie ekranu
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);

        // Wybór odpowiedniego sprite'a
        SDL_Texture* currentTexture = playerStandingTexture;
        if (player.isJumping) {
            currentTexture = playerJumpingTexture;
        } else if (movingLeft) {
            currentTexture = playerMovingLeftTexture;
        } else if (movingRight) {
            currentTexture = playerMovingRightTexture;
        }

        // Narysowanie gracza
        SDL_RenderCopy(renderer, currentTexture, nullptr, &player.rect);

        // Wyświetlenie renderowanej grafiki
        SDL_RenderPresent(renderer);

        // Kontrola liczby klatek na sekundę
        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    SDL_DestroyTexture(playerStandingTexture);
    SDL_DestroyTexture(playerMovingLeftTexture);
    SDL_DestroyTexture(playerMovingRightTexture);
    SDL_DestroyTexture(playerJumpingTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
