#include "Game.h"

// Level and camera constatns
const int LEVEL_HEIGHT = 793;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int GENERATION_SPACE_WIDTH = 100;

// FPS counting constants
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

// Sprite animation constatnts
const int WALKING_ANIMATION_FRAMES = 6;
const int IDLE_ANIMATION_FRAMES = 4;
const int JUMP_ANIMATION_FRAMES = 10;

const int ANIMATION_DELAY = 4;

// SDL types declarations
Mix_Music* music = NULL;
Mix_Chunk* jumpSound = NULL;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// Tables storing sprite clips
SDL_Rect RunningSpriteClips[ WALKING_ANIMATION_FRAMES ];
SDL_Rect IdleSpriteClips[ IDLE_ANIMATION_FRAMES ];
SDL_Rect JumpSpriteClips[ JUMP_ANIMATION_FRAMES ];
SDL_Rect BlockSpriteClip;

// Objects storing sprites data
TextureHandler SpriteSheetTexture(renderer);
TextureHandler blocksTexture(renderer);
TextureHandler backgroundTextures[7];

// Cameras declaration
SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
SDL_Rect camera2 = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
SDL_Rect playerCamera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

// Datatypes for dynamic storage
std::deque<PlayerState> playerHistory;
std::vector<Block> blocks;

Game::Game() : blockGenerationInterval(500) {
    if (!init()) {
        SDL_Log("Failed to initialize!\n");
    } else {
        if (!loadMedia()) {
            SDL_Log("Failed to load media!\n");
        }
    }
    Player();
    blockGenerationTimer.start();
}

// Initialize all SDL subsystems, 
bool Game::init() {
    bool success = true;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        SDL_Log("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    } else {
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
            SDL_Log("Warning: Linear texture filtering not enabled!");
        }

        // Create window
        window = SDL_CreateWindow("Running game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

        if (!window) {
            SDL_Log("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        } 
        else {
            // Create renderer
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

            if (!renderer) {
                SDL_Log("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            } 
            else {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

                // Initialize IMG subsystems form image lib
                if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
                    SDL_Log("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
                    success = false;
                }
                
                // Initialize audio device
                if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
                    SDL_Log("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
                    success = false;
                }
            }
        }
    }

    SpriteSheetTexture = TextureHandler(renderer);
    blocksTexture = TextureHandler(renderer);
    for (int i = 0; i < 7; ++i) {
        backgroundTextures[i] = TextureHandler(renderer);
    }

    return success;
}

// Load sprite sheats and audio files
bool Game::loadMedia() {
    bool success = true;

    if (!SpriteSheetTexture.loadFromFile("sprites/main_hero/running_man.png")) {
        SDL_Log("Failed to load hero sprite sheet texture!\n");
        success = false;
    } 
    else {
        // Set sprite clips
        // Running Clips
        for (int i = 0; i < WALKING_ANIMATION_FRAMES; i++) {
            RunningSpriteClips[i] = {63 + i * 50, 37, 24, 37};
        }

        // Idle Clips
        for (int i = 0; i < IDLE_ANIMATION_FRAMES; i++) {
            IdleSpriteClips[i] = {13 + i * 50, 0, 24, 37};
        }

        // Jump Clips   
        for (int i = 0; i < JUMP_ANIMATION_FRAMES; i++) {
            int j = i;
            int k = 0;
            if(i <= 5) {}
            else {j -= 6; k = 1;}
            JumpSpriteClips[j] = {63 + j * 50, 74 + k * 37, 24, 37};

            SDL_Log("%d, %d, %d", JumpSpriteClips[j].x, JumpSpriteClips[j].y, i);
        }

        BlockSpriteClip = {0, 0, 47, 47};
    }

    // Set paths for background images
    const std::string bgPaths[7] = {
        "sprites/background/Layer1.png",
        "sprites/background/Layer2.png",
        "sprites/background/Layer3.png",
        "sprites/background/Layer4.png",
        "sprites/background/Layer5.png",
        "sprites/background/Layer6.png",
        "sprites/background/Layer7.png"
    };

    // Load background textures
    for (int i = 0; i < 7; ++i) {
        if (!backgroundTextures[i].loadFromFile(bgPaths[i])) {
            SDL_Log("Failed to load background texture %d!\n", i + 1);
            success = false;
        }
    }

    // Load block texture
    if (!blocksTexture.loadFromFile("sprites/boxes/Terrain.png")) {
        SDL_Log("Failed to load block texture!\n");
        success = false;
    }

    //Load music
    music = Mix_LoadMUS( "sprites/Music/song.wav" );
    if( music == NULL )
    {
        SDL_Log( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
        success = false;
    }
    
    //Load sound effects
    jumpSound = Mix_LoadWAV( "sprites/boxes/sounds/jump.wav" );
    if( jumpSound == NULL )
    {
        SDL_Log( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
        success = false;
    }

    return success;
}

// AABB check for collision betwen two rectangles
bool Game::checkCollision(SDL_Rect rectA, SDL_Rect rectB) {
    // Calculate sides of rectA
    int leftA = rectA.x;
    int rightA = rectA.x + rectA.w;
    int topA = rectA.y;
    int bottomA = rectA.y + rectA.h;

    // Calculate sides of rectB
    int leftB = rectB.x;
    int rightB = rectB.x + rectB.w;
    int topB = rectB.y;
    int bottomB = rectB.y + rectB.h;

    // Check if there is no overlap
    bool noOverlap = (bottomA <= topB) || (topA >= bottomB) || (rightA <= leftB) || (leftA >= rightB);

    // If any of the sides from A are outside of B there is no collision
    return !noOverlap;
}

// Generate blocks off the screan
void Game::generateBlock() {
    // Choose random position within the 100px wide space off the screan
    int x = rand() % (player.hitBox.x + player.hitBox.w / 2 + SCREEN_WIDTH / 2 + GENERATION_SPACE_WIDTH - (player.hitBox.x + player.hitBox.w / 2 + SCREEN_WIDTH / 2 + 1)) + (player.hitBox.x + player.hitBox.w / 2 + SCREEN_WIDTH / 2);
    int y = rand() % (700 - 400 + 1) + 400;
    SDL_Rect newBlock = {x, y, 43, 47};

    // Check if the new block is not colliding with any former ones
    bool isColliding = false;
    for (const auto& block : blocks) {
        if (checkCollision(newBlock, block.rect)) {
            isColliding = true;
            break;
        }
    }

    if (!isColliding) {
        blocks.push_back({newBlock});
    }
}

// Handling left, right, r and space key presses
void Game::handleKeyDown(SDL_Event &e, bool &movingLeft, bool &movingRight) {
    switch (e.key.keysym.sym) {
        case SDLK_LEFT:
            movingLeft = true;
            break;
        case SDLK_RIGHT:
            movingRight = true;
            break;
        case SDLK_SPACE:
            // SDL_Log("%d", player.isJumping);
            if (!player.isJumping) {
                Mix_PlayChannel( -1, jumpSound, 0 );
                player.jump();
            }
            break;
        case SDLK_r: // Key to rewind time
            if (!isRewinding) {
                isRewinding = true;
            }
            break;
        default:
            break;
    }
}

// Handling letting key off
void Game::handleKeyUp(SDL_Event &e, bool &movingLeft, bool &movingRight) {
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

void Game::handlePlayerMovement(bool movingLeft, bool movingRight, SDL_RendererFlip &flipType, bool isOnBlock) {
    // 
    if (movingLeft && !movingRight && player.hitBox.x > 0) {
        flipType = SDL_FLIP_HORIZONTAL;
        player.moveLeft();
    } else if (movingRight && !movingLeft) {
        flipType = SDL_FLIP_NONE;
        player.moveRight();
    } else {
        player.stopMoving();
    }

    bool isCollidingWithBlock = false;
    for (auto& block : blocks) {
        if(checkCollision(player.hitBox, block.rect)){
            isCollidingWithBlock = true;
        }
    }
    if(!isCollidingWithBlock && !isOnBlock){
        player.isJumping = true;
    }

    player.applyGravity();

    if (player.hitBox.y >=LEVEL_HEIGHT - PLAYER_HEIGHT - 50) {
        player.velocity.y = 0;
        player.hitBox.y = LEVEL_HEIGHT - PLAYER_HEIGHT - 50;
        player.isJumping = false;
    }

    player.hitBox.x += static_cast<int>(player.velocity.x);

    camera.x = (player.hitBox.x + player.hitBox.w / 2) - SCREEN_WIDTH / 2;
    if (camera.x < 0) {
        camera.x = 0;
    }
}

void Game::handlePlayerCollision(bool &isPlayerOnBlock) {
    for (auto& block : blocks) {
        if (checkCollision(player.hitBox, block.rect)) {
            int playerBottom = player.hitBox.y + player.hitBox.h;
            int boxBottom = block.rect.y + block.rect.h;
            int playerRight = player.hitBox.x + player.hitBox.w;
            int boxRight = block.rect.x + block.rect.w;

            int bCollision = boxBottom - player.hitBox.y;
            int tCollision = playerBottom - block.rect.y;
            int lCollision = playerRight - block.rect.x;
            int rCollision = boxRight - player.hitBox.x;

            if (tCollision < bCollision && tCollision < lCollision && tCollision < rCollision) {
                player.hitBox.y = block.rect.y - player.hitBox.h + 1;
                isPlayerOnBlock = true;
                player.velocity.y = 0;
                player.isJumping = false;
            } else if (bCollision < tCollision && bCollision < lCollision && bCollision < rCollision) {
                player.hitBox.y = block.rect.y + block.rect.h;
                player.velocity.y = 0;
            } else if (lCollision < rCollision && lCollision < tCollision && lCollision < bCollision) {
                player.hitBox.x = block.rect.x - player.hitBox.w;
                player.velocity.x = 0;
            } else if (rCollision < lCollision && rCollision < tCollision && rCollision < bCollision) {
                player.hitBox.x = block.rect.x + block.rect.w;
                player.velocity.x = 0;
            }

            // if (!(tCollision < bCollision && tCollision < lCollision && tCollision < rCollision)) {
            //     isPlayerOnBlock = false;
            // }
        }
        else {
            isPlayerOnBlock = false;
        }
    }
}

void Game::renderGame(int &animationFrameDelay, int frame, SDL_RendererFlip &flipType, int width, float &portion, int &multiplier, bool movingLeft, bool movingRight) {
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    playerCamera.x = (player.hitBox.x + PLAYER_WIDTH / 2) - SCREEN_WIDTH / 2;
    playerCamera.y = (player.hitBox.y + PLAYER_HEIGHT - SCREEN_HEIGHT + 50);

    camera.x = (player.hitBox.x + PLAYER_WIDTH / 2) - SCREEN_WIDTH / 2;
    camera.y = (player.hitBox.y + PLAYER_HEIGHT - SCREEN_HEIGHT + 50);

    while (camera.x > (multiplier + 1) * width) {
        multiplier++;
    }

    while (camera.x < multiplier * width) {
        multiplier--;
    }

    if (multiplier > 0) {
        camera.x -= multiplier * width;
        camera.w = SCREEN_WIDTH;
    }

    if (camera.x > (width - SCREEN_WIDTH) && camera.x < width) {
        portion = 1.0f - (width - camera.x) / static_cast<float>(SCREEN_WIDTH);
        if (portion > 0.996) portion = 1;
        if (portion < 0.004) portion = 0;
        camera.w = (1 - portion) * SCREEN_WIDTH;
    }

    camera2.x = 0;
    camera2.y = camera.y;
    camera2.w = SCREEN_WIDTH - camera.w;

    if (playerCamera.x <= 0) {
        playerCamera.x = 0;
        camera.x = 0;
    }

    for(int i = 7; i >= 0; i--){
        backgroundTextures[i].render(0, 0, &camera);
    }

    if (camera.x > (width - SCREEN_WIDTH) && camera.x < width) {
        for(int i = 7; i >= 0; i--){
            backgroundTextures[i].render(camera.w, 0, &camera2);
        }
    }

    for (const auto& block : blocks) {
        blocksTexture.render(block.rect.x - (player.hitBox.x) + SCREEN_WIDTH / 2 - 15, block.rect.y - (player.hitBox.y + PLAYER_HEIGHT - SCREEN_HEIGHT + 50), &BlockSpriteClip);
    }

    SDL_Rect* currentClip = getCurrentClip(frame, movingLeft, movingRight);
    SpriteSheetTexture.render(player.hitBox.x - playerCamera.x, player.hitBox.y - playerCamera.y, currentClip, 0.0, NULL, flipType);

    SDL_RenderPresent(renderer);
}

SDL_Rect* Game::getCurrentClip(int frame, bool movingLeft, bool movingRight) {
    SDL_Rect* currentClip;

    // currentClip = &IdleSpriteClips[frame % IDLE_ANIMATION_FRAMES];

    // if (movingLeft || movingRight) currentClip = &RunningSpriteClips[frame % WALKING_ANIMATION_FRAMES];

    // if (player.isJumping) 
    currentClip = &JumpSpriteClips[frame % JUMP_ANIMATION_FRAMES];
    SDL_Log("%d, %d", currentClip->x, currentClip->y);

    return currentClip;
}

void Game::close()
{
	//Free loaded images
    Mix_FreeChunk( jumpSound );
    jumpSound = NULL;
    Mix_FreeMusic( music );
    music = NULL;

    SpriteSheetTexture.free();
    blocksTexture.free();
    for (int i = 0; i < 7; ++i) {
        backgroundTextures[i].free();
    }

	SpriteSheetTexture.free();
	blocksTexture.free();

	//Destroy window	
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	window = NULL;
	renderer = NULL;

	//Quit SDL subsystems
    Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}

PlayerState Game::saveCurrentPlayerState() {
    PlayerState currentPlayerState;
    currentPlayerState.hitBox = player.hitBox;
    currentPlayerState.velocity = player.velocity;
    currentPlayerState.isJumping = player.isJumping;
    currentPlayerState.timestamp = SDL_GetTicks();
    return currentPlayerState;
}

void Game::gameLoop() {
    bool quit = false;

    SDL_Event e;

    int frame = 0;

    Timer fpsTimer;
    Timer capTimer;
    float countedFrames = 0;
    fpsTimer.start();

    bool movingLeft = false;
    bool movingRight = false;

    float portion = 0;
    SDL_RendererFlip flipType = SDL_FLIP_NONE;

    int width = backgroundTextures[0].getWidth();

    int multiplier = 1;

    bool isPlayerOnBlock = false;

    int animationFrameDelay = 0; // Frame delay counter

    Mix_PlayMusic( music, -1 );
    while (!quit) {
        capTimer.start();
        Uint32 currentTime = SDL_GetTicks();
        
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                handleKeyDown(e, movingLeft, movingRight);
            } else if (e.type == SDL_KEYUP) {
                handleKeyUp(e, movingLeft, movingRight);
            }
        }

        if (blockGenerationTimer.getTicks() > blockGenerationInterval) {
            generateBlock();
            blockGenerationTimer.start();
        }

        PlayerState currentPlayerState = saveCurrentPlayerState();
        // SDL_Log("%d %d %d", currentPlayerState.hitBox.x, currentPlayerState.timestamp, currentPlayerState.isJumping);

        handlePlayerMovement(movingLeft, movingRight, flipType, isPlayerOnBlock);

        handlePlayerCollision(isPlayerOnBlock);

        playerHistory.push_back(currentPlayerState);
        if (playerHistory.size() > 120) {
            playerHistory.pop_front();
        }

        if (isRewinding) {
            PlayerState rewindState = playerHistory.front();
            player.hitBox = rewindState.hitBox;
            player.velocity = rewindState.velocity;
            player.isJumping = rewindState.isJumping;
            isRewinding = false;
            rewindTimer.stop();
        }

        renderGame(animationFrameDelay, frame, flipType, width, portion, multiplier, movingLeft, movingRight);

        if(animationFrameDelay > ANIMATION_DELAY){
            ++frame;
            animationFrameDelay = 0;
        }
        else{
            animationFrameDelay++;
        }

        countedFrames++;
        int frameTicks = capTimer.getTicks();
        if (frameTicks < SCREEN_TICKS_PER_FRAME) {
            SDL_Delay(SCREEN_TICKS_PER_FRAME - frameTicks);
        }
    }
}