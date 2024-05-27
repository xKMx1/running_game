#include <SDL.h>
#include <iostream>

// Stałe dotyczące ekranu
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Stałe dotyczące postaci
const float PLAYER_WIDTH = 50.0f;
const float PLAYER_HEIGHT = 50.0f;
const float PLAYER_VELOCITY = 5.0f;
const float GRAVITY = 1.0f;
const float JUMP_VELOCITY = -15.0f;

// Struktura reprezentująca gracza
struct Player {
    SDL_Rect rect;
    float velocityX;
    float velocityY;
    bool isJumping;
	float accelerationX = 0.3f; // Przyspieszenie w kierunku X
	float maxVelocityX = 10.0f; // Maksymalna prędkość w kierunku X
	float maxVelocityY = 10.0f; // Maksymalna prędkość w kierunku X

    Player(float x, float y) {
        rect = {x, y, PLAYER_WIDTH, PLAYER_HEIGHT};
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
        rect.y += velocityY;
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

int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Player player(100.0f, SCREEN_HEIGHT - PLAYER_HEIGHT);

    bool quit = false;
    SDL_Event e;

    // Kontrola liczby klatek na sekundę
    const int TARGET_FPS = 60;
    const int FRAME_DELAY = 1000 / TARGET_FPS;
    Uint32 frameStart, frameTime;

    while (!quit) {
        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                        player.moveLeft();
                        break;
                    case SDLK_RIGHT:
                        player.moveRight();
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
                    case SDLK_RIGHT:
                        player.stopMoving();
                        break;
                    default:
                        break;
                }
            }
        }

        // Aktualizacja pozycji gracza na podstawie prędkości
        player.rect.x += static_cast<int>(player.velocityX);

        // Zastosowanie grawitacji
        player.applyGravity();

        // Wyczyszczenie ekranu
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);

        // Narysowanie gracza
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderFillRect(renderer, &player.rect);

        // Wyświetlenie renderowanej grafiki
        SDL_RenderPresent(renderer);

        // Kontrola liczby klatek na sekundę
        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
