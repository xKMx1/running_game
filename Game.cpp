#include "Game.h"

const int LEVEL_HEIGHT = 793;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

Mix_Music* gMusic = NULL;
Mix_Chunk* gJump = NULL;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Current displayed texture
SDL_Texture* gTexture = NULL;

const int WALKING_ANIMATION_FRAMES = 6;
const int IDLE_ANIMATION_FRAMES = 4;
const int JUMP_ANIMATION_FRAMES = 10;

const int ANIMATION_DELAY = 4; // Number of frames to wait before updating the animation

SDL_Rect RunningSpriteClips[ WALKING_ANIMATION_FRAMES ];
SDL_Rect IdleSpriteClips[ IDLE_ANIMATION_FRAMES ];
SDL_Rect JumpSpriteClips[ JUMP_ANIMATION_FRAMES ];
SDL_Rect BlockSpriteClip;

TextureHandler gSpriteSheetTexture( gRenderer );
TextureHandler gBlocksSheetTexture( gRenderer );
TextureHandler gBackgroundTexture1( gRenderer );
TextureHandler gBackgroundTexture2( gRenderer );
TextureHandler gBackgroundTexture3( gRenderer );
TextureHandler gBackgroundTexture4( gRenderer );
TextureHandler gBackgroundTexture5( gRenderer );
TextureHandler gBackgroundTexture6( gRenderer );
TextureHandler gBackgroundTexture7( gRenderer );

SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
SDL_Rect camera2 = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
SDL_Rect playerCamera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

// Nowa struktura do przechowywania bloków
struct Block {
    SDL_Rect rect;
};

struct PlayerState {
    SDL_Rect hitBox;
    Vec2f velocity;
    bool isJumping;
    Uint32 timestamp;
};

std::deque<PlayerState> playerHistory;
// Wektor przechowujący wszystkie bloki
std::vector<Block> blocks;

Game::Game() : blockGenerationInterval(500) { // np. co 2000 milisekund (2 sekundy)
    if (!init()) {
        SDL_Log("Failed to initialize!\n");
    } else {
        if (!loadMedia()) {
            SDL_Log("Failed to load media!\n");
        }
    }
    Player();
    blockGenerationTimer.start(); // Uruchom timer
}

void Game::generateBlockIndependent() {
    int x = rand() % ( (player.hitBox.x + player.hitBox.w / 2 + SCREEN_WIDTH / 2 + 100) - (player.hitBox.x + player.hitBox.w / 2 + SCREEN_WIDTH / 2) + 1) + (player.hitBox.x + player.hitBox.w / 2 + SCREEN_WIDTH / 2);
    int y = rand() % (700 - 400 + 1) + 400;
    SDL_Rect newBlock = { x, y, 43, 47 };

    bool isColliding = false;
    for (auto& block : blocks) {
        if (checkCollision(newBlock, block.rect)) {
            isColliding = true;
            break; // Przerwij pętlę, jeśli wykryto kolizję
        }
    }

    if (!isColliding) {
        blocks.push_back({ newBlock });
    }
}

bool Game::checkCollision( SDL_Rect a, SDL_Rect b ){
	//The sides of the rectangles
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    //Calculate the sides of rect A
    leftA = a.x;
    rightA = a.x + a.w;
    topA = a.y;
    bottomA = a.y + a.h;

    //Calculate the sides of rect B
    leftB = b.x;
    rightB = b.x + b.w;
    topB = b.y;
    bottomB = b.y + b.h;
	//If any of the sides from A are outside of B
    if( bottomA <= topB )
    {
        return false;
    }

    if( topA >= bottomB )
    {
        return false;
    }

    if( rightA <= leftB )
    {
        return false;
    }

    if( leftA >= rightB )
    {
        return false;
    }

    //If none of the sides from A are outside B
    return true;
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
    SDL_Rect backGroundRect = {0, 0};
    SDL_Rect backGroundRect2 = {0, 0};
    SDL_Rect displayRect = {0, 0};
    int howManyFlips = 0;
    int scrollingOffset = 0;
    SDL_RendererFlip flipType = SDL_FLIP_NONE;
    int width = gBackgroundTexture7.getWidth();
    int multiplier = 1;
    

    bool isPlayerOnBlock = false;
    // Mix_PlayMusic( gMusic, -1 );

    int animationFrameDelay = 0; // Frame delay counter

    while (!quit) {
        bool variable = false;
        capTimer.start();
        
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                        movingLeft = true;
                        break;
                    case SDLK_RIGHT:
                        movingRight = true;
                        break;
                    case SDLK_SPACE:
                        if (!player.isJumping) {
                            // Mix_PlayChannel( -1, gJump, 0 );
                            variable = true;
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
            } else if (e.type == SDL_KEYUP) {
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

        Uint32 currentTime = SDL_GetTicks();
        PlayerState currentState = { player.hitBox, player.velocity, player.isJumping, currentTime };
        playerHistory.push_back(currentState);

        // Remove states older than 4 seconds
        while (!playerHistory.empty() && currentTime - playerHistory.front().timestamp > rewindDuration) {
            playerHistory.pop_front();
        }

        if (isRewinding) {
            // if (rewindTimer.getTicks() < rewindDuration) {
            //     // Rewind player state
            //     while (!playerHistory.empty() && currentTime - playerHistory.back().timestamp > (rewindDuration - rewindTimer.getTicks())) {
            //         playerHistory.pop_back();
            //     }
            //     if (!playerHistory.empty()) {
                    SDL_Log("%d %d %d %d", playerHistory.front().hitBox.x, playerHistory.front().hitBox.y, playerHistory.front().timestamp, playerHistory.front().isJumping );
                    PlayerState rewindState = playerHistory.front();
                    player.hitBox = rewindState.hitBox;
                    player.velocity = rewindState.velocity;
                    player.isJumping = rewindState.isJumping;
                // }
            // }
            //  else {
                isRewinding = false;
                rewindTimer.stop();
            // }
        }
        else {

        if (movingLeft && !movingRight) {
            flipType = SDL_FLIP_HORIZONTAL;
            --scrollingOffset;
            player.moveLeft();
        } else if (movingRight && !movingLeft) {
            flipType = SDL_FLIP_NONE;
            ++scrollingOffset;
            player.moveRight();
        } else {
            player.stopMoving();
        }

        if (blockGenerationTimer.getTicks() > blockGenerationInterval) {
            generateBlockIndependent();
            blockGenerationTimer.stop(); // Zresetuj timer
            blockGenerationTimer.start(); // Zresetuj timer
        }

        player.hitBox.x += static_cast<int>(player.velocity.x);


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

                if (tCollision < bCollision && tCollision < lCollision && tCollision < rCollision && !variable) {
                    player.hitBox.y = block.rect.y - player.hitBox.h + 1;
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
            }
        }

        bool dupa = false;
        for (auto& block : blocks) {
            if(checkCollision(player.hitBox, block.rect)){
                dupa = true;
            }
        }
        if(!dupa){
            player.isJumping = true;
        }

        player.applyGravity();

        if (player.hitBox.y >=LEVEL_HEIGHT - PLAYER_HEIGHT - 50) {
            player.velocity.y = 0;
            player.hitBox.y = LEVEL_HEIGHT - PLAYER_HEIGHT - 50;
            player.isJumping = false;
        }

        if(player.hitBox.x < 0){
            player.hitBox.x = 0;
            camera.x = 0;
        }

        float avgFPS = static_cast<float>(countedFrames) / (static_cast<float>(fpsTimer.getTicks()) / 1000.f);
        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(gRenderer);
        
        SDL_Rect* currentClip = &IdleSpriteClips[frame % IDLE_ANIMATION_FRAMES];

        if (movingLeft || movingRight) currentClip = &RunningSpriteClips[frame % WALKING_ANIMATION_FRAMES];

        if (player.isJumping) currentClip = &JumpSpriteClips[frame % JUMP_ANIMATION_FRAMES];

        SDL_Log("%d", frame % JUMP_ANIMATION_FRAMES);

        SDL_Point rotationPoint = {currentClip->w / 2, currentClip->h / 2};

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

        if (camera.x > (gBackgroundTexture7.getWidth() - SCREEN_WIDTH) && camera.x < gBackgroundTexture7.getWidth()) {
            portion = 1.0f - (gBackgroundTexture7.getWidth() - camera.x) / static_cast<float>(SCREEN_WIDTH);
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

        gBackgroundTexture7.render(0, 0, &camera);
        gBackgroundTexture6.render(0, 0, &camera);
        gBackgroundTexture5.render(0, 0, &camera);
        gBackgroundTexture4.render(0, 0, &camera);
        gBackgroundTexture3.render(0, 0, &camera);
        gBackgroundTexture2.render(0, 0, &camera);
        gBackgroundTexture1.render(0, 0, &camera);

        if (camera.x > (gBackgroundTexture7.getWidth() - SCREEN_WIDTH) && camera.x < gBackgroundTexture7.getWidth()) {
            gBackgroundTexture7.render(camera.w, 0, &camera2);
            gBackgroundTexture6.render(camera.w, 0, &camera2);
            gBackgroundTexture5.render(camera.w, 0, &camera2);
            gBackgroundTexture4.render(camera.w, 0, &camera2);
            gBackgroundTexture3.render(camera.w, 0, &camera2);
            gBackgroundTexture2.render(camera.w, 0, &camera2);
            gBackgroundTexture1.render(camera.w, 0, &camera2);
        }

        for (auto& block : blocks) {
            gBlocksSheetTexture.render(block.rect.x - (player.hitBox.x) + SCREEN_WIDTH / 2 - 15, block.rect.y - (player.hitBox.y + PLAYER_HEIGHT - SCREEN_HEIGHT + 50), &BlockSpriteClip);
        }

        gSpriteSheetTexture.render(player.hitBox.x - playerCamera.x, player.hitBox.y - playerCamera.y, currentClip, 0.0f, &rotationPoint, flipType);
        SDL_RenderPresent(gRenderer);

        if(animationFrameDelay > ANIMATION_DELAY){
            ++frame;
            animationFrameDelay = 0;
        }
        else{
            animationFrameDelay++;
        }

        // if(player.isJumping && (frame / JUMP_ANIMATION_FRAMES >= JUMP_ANIMATION_FRAMES)){
        //     frame = 0;
        // }
        // else if ((movingLeft || movingRight) && (frame / WALKING_ANIMATION_FRAMES >= WALKING_ANIMATION_FRAMES)) {
        //     frame = 0;
        // } else if (frame / IDLE_ANIMATION_FRAMES >= IDLE_ANIMATION_FRAMES) {
        //     frame = 0;
        // }
        }
        ++countedFrames;

        int frameTicks = capTimer.getTicks();
        if (frameTicks < SCREEN_TICKS_PER_FRAME) {
            SDL_Delay(SCREEN_TICKS_PER_FRAME - frameTicks);

        }
    }
}

bool Game::init(){
	//Initialization flag
	bool success = true;

    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
    {
        printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        success = false;
    }

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ){
		SDL_Log( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) ){
			SDL_Log( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "Running game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL ){
			SDL_Log( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
			if( gRenderer == NULL ){
				SDL_Log( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					SDL_Log( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}
                 //Initialize SDL_mixer
                if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
                {
                    printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
                    success = false;
                }
			}
		}
	}

	gSpriteSheetTexture = TextureHandler(gRenderer);
	gBlocksSheetTexture = TextureHandler(gRenderer);
	gBackgroundTexture1 = TextureHandler(gRenderer);
	gBackgroundTexture2 = TextureHandler(gRenderer);
	gBackgroundTexture3 = TextureHandler(gRenderer);
	gBackgroundTexture4 = TextureHandler(gRenderer);
	gBackgroundTexture5 = TextureHandler(gRenderer);
	gBackgroundTexture6 = TextureHandler(gRenderer);
	gBackgroundTexture7 = TextureHandler(gRenderer);
	return success;
}

bool Game::loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load sprite sheet texture
	if( !gSpriteSheetTexture.loadFromFile( "sprites/running_man.png" ) )
	{
		SDL_Log( "Failed to load sprite sheet texture!\n" );
		success = false;
	}
	else
	{
        //Set sprite clips
		//------------- RUNNING -------------
        RunningSpriteClips[ 0 ].x = 63;
        RunningSpriteClips[ 0 ].y = 37;
        RunningSpriteClips[ 0 ].w = 24;
        RunningSpriteClips[ 0 ].h = 37;
        
        RunningSpriteClips[ 1 ].x = 113;
        RunningSpriteClips[ 1 ].y = 37;
        RunningSpriteClips[ 1 ].w = 24;
        RunningSpriteClips[ 1 ].h = 37;

        RunningSpriteClips[ 2 ].x = 163;
        RunningSpriteClips[ 2 ].y = 37;
        RunningSpriteClips[ 2 ].w = 24;
        RunningSpriteClips[ 2 ].h = 37;

        RunningSpriteClips[ 3 ].x = 213;
        RunningSpriteClips[ 3 ].y = 37;
        RunningSpriteClips[ 3 ].w = 24;
        RunningSpriteClips[ 3 ].h = 37;

        RunningSpriteClips[ 4 ].x = 263;
        RunningSpriteClips[ 4 ].y = 37;
        RunningSpriteClips[ 4 ].w = 24;
        RunningSpriteClips[ 4 ].h = 37;
        
        RunningSpriteClips[ 5 ].x = 313;
        RunningSpriteClips[ 5 ].y = 37;
        RunningSpriteClips[ 5 ].w = 24;
        RunningSpriteClips[ 5 ].h = 37;

		//------------- IDLE -------------

		IdleSpriteClips[ 0 ].x = 13;
        IdleSpriteClips[ 0 ].y = 0;
        IdleSpriteClips[ 0 ].w = 24;
        IdleSpriteClips[ 0 ].h = 37;
        
        IdleSpriteClips[ 1 ].x = 63;
        IdleSpriteClips[ 1 ].y = 0;
        IdleSpriteClips[ 1 ].w = 24;
        IdleSpriteClips[ 1 ].h = 37;

        IdleSpriteClips[ 2 ].x = 113;
        IdleSpriteClips[ 2 ].y = 0;
        IdleSpriteClips[ 2 ].w = 24;
        IdleSpriteClips[ 2 ].h = 37;

        IdleSpriteClips[ 3 ].x = 163;
        IdleSpriteClips[ 3 ].y = 0;
        IdleSpriteClips[ 3 ].w = 24;
        IdleSpriteClips[ 3 ].h = 37;

		//-----------JUMP-----------------

        JumpSpriteClips[ 0 ].x = 13;
        JumpSpriteClips[ 0 ].y = 74;
        JumpSpriteClips[ 0 ].w = 24;
        JumpSpriteClips[ 0 ].h = 37;

        JumpSpriteClips[ 1 ].x = 63;
        JumpSpriteClips[ 1 ].y = 74;
        JumpSpriteClips[ 1 ].w = 24;
        JumpSpriteClips[ 1 ].h = 37;

        JumpSpriteClips[ 2 ].x = 113;
        JumpSpriteClips[ 2 ].y = 74;
        JumpSpriteClips[ 2 ].w = 24;
        JumpSpriteClips[ 2 ].h = 37;

        JumpSpriteClips[ 3 ].x = 163;
        JumpSpriteClips[ 3 ].y = 74;
        JumpSpriteClips[ 3 ].w = 24;
        JumpSpriteClips[ 3 ].h = 37;

        JumpSpriteClips[ 4 ].x = 213;
        JumpSpriteClips[ 4 ].y = 74;
        JumpSpriteClips[ 4 ].w = 24;
        JumpSpriteClips[ 4 ].h = 37;

        JumpSpriteClips[ 5 ].x = 263;
        JumpSpriteClips[ 5 ].y = 74;
        JumpSpriteClips[ 5 ].w = 24;
        JumpSpriteClips[ 5 ].h = 37;

        JumpSpriteClips[ 6 ].x = 313;
        JumpSpriteClips[ 6 ].y = 74;
        JumpSpriteClips[ 6 ].w = 24;
        JumpSpriteClips[ 6 ].h = 37;

        JumpSpriteClips[ 7 ].x = 13;
        JumpSpriteClips[ 7 ].y = 111;
        JumpSpriteClips[ 7 ].w = 24;
        JumpSpriteClips[ 7 ].h = 37;

        JumpSpriteClips[ 8 ].x = 63;
        JumpSpriteClips[ 8 ].y = 111;
        JumpSpriteClips[ 8 ].w = 24;
        JumpSpriteClips[ 8 ].h = 37;

        JumpSpriteClips[ 9 ].x = 113;
        JumpSpriteClips[ 9 ].y = 111;
        JumpSpriteClips[ 9 ].w = 24;
        JumpSpriteClips[ 9 ].h = 37;

		//----------BLOCKS---------------

		BlockSpriteClip.x = 0;
		BlockSpriteClip.y = 0;
		BlockSpriteClip.w = 47;
		BlockSpriteClip.h = 47;
    }

	if( !gBackgroundTexture1.loadFromFile( "sprites/Background/Layer1.png" ) ){
		SDL_Log( "Failed to load sprite sheet texture!\n" );
		success = false;
	}

	if( !gBackgroundTexture2.loadFromFile( "sprites/Background/Layer2.png" ) ){
		SDL_Log( "Failed to load sprite sheet texture!\n" );
		success = false;
	}

	if( !gBackgroundTexture3.loadFromFile( "sprites/Background/Layer3.png" ) ){
		SDL_Log( "Failed to load sprite sheet texture!\n" );
		success = false;
	}

	if( !gBackgroundTexture4.loadFromFile( "sprites/Background/Layer4.png" ) ){
		SDL_Log( "Failed to load sprite sheet texture!\n" );
		success = false;
	}

	if( !gBackgroundTexture5.loadFromFile( "sprites/Background/Layer5.png" ) ){
		SDL_Log( "Failed to load sprite sheet texture!\n" );
		success = false;
	}

	if( !gBackgroundTexture6.loadFromFile( "sprites/Background/Layer6.png" ) ){
		SDL_Log( "Failed to load sprite sheet texture!\n" );
		success = false;
	}

	if( !gBackgroundTexture7.loadFromFile( "sprites/Background/Layer7.png" ) ){
		SDL_Log( "Failed to load sprite sheet texture!\n" );
		success = false;
	}


	if( !gBlocksSheetTexture.loadFromFile( "sprites/boxes/Terrain.png" ) ){
		SDL_Log( "Failed to load sprite sheet texture!\n" );
		success = false;
	}

    //Load music
    gMusic = Mix_LoadMUS( "sprites/Music/song.wav" );
    if( gMusic == NULL )
    {
        SDL_Log( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
        success = false;
    }
    
    //Load sound effects
    gJump = Mix_LoadWAV( "sprites/boxes/sounds/jump.wav" );
    if( gJump == NULL )
    {
        SDL_Log( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
        success = false;
    }

	return success;
}

void Game::close()
{
	//Free loaded images
    Mix_FreeChunk( gJump );
    gJump = NULL;
    Mix_FreeMusic( gMusic );
    gMusic = NULL;

	gSpriteSheetTexture.free();
	gBlocksSheetTexture.free();
	gBackgroundTexture1.free();
	gBackgroundTexture2.free();
	gBackgroundTexture3.free();
	gBackgroundTexture4.free();
	gBackgroundTexture5.free();
	gBackgroundTexture6.free();
	gBackgroundTexture7.free();

	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
    Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}

