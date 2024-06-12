#include "Game.h"

const int LEVEL_WIDTH = 11280;
const int LEVEL_HEIGHT = 793;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Current displayed texture
SDL_Texture* gTexture = NULL;

const int WALKING_ANIMATION_FRAMES = 6;
const int IDLE_ANIMATION_FRAMES = 4;

SDL_Rect RunningSpriteClips[ WALKING_ANIMATION_FRAMES ];
SDL_Rect IdleSpriteClips[ IDLE_ANIMATION_FRAMES ];
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

Game::Game(){
    if( !init() ){
		SDL_Log( "Failed to initialize!\n" );
	}
	else{
		//Load media
		if( !loadMedia() ){
			SDL_Log( "Failed to load media!\n" );
		}
    }

    Player();
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

void Game::gameLoop(){
    // Main loop flag
    bool quit = false;

    // Event handler
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

    SDL_Rect box = {rand() % (800 - 100 + 1) + 100, rand() % (650 - 200 + 1) + 200, 47, 47};

    // While application is running
    while (!quit) {
        // Start cap timer
        capTimer.start();
        // Handle events on queue
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
                        player.jump();
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

        player.hitBox.x += static_cast<int>(player.velocity.x);
        player.applyGravity();

        if (player.hitBox.y > LEVEL_HEIGHT - PLAYER_HEIGHT - 50) {
            player.velocity.y = 0;
            player.hitBox.y = LEVEL_HEIGHT - PLAYER_HEIGHT - 50;
            player.isJumping = false;
        }

        bool wasOnBox = checkCollision(player.hitBox, box);

        // Collision handling
        if (checkCollision(player.hitBox, box)) {

            // Determine from which side the collision occurs
            int playerBottom = player.hitBox.y + player.hitBox.h;
            int boxBottom = box.y + box.h;
            int playerRight = player.hitBox.x + player.hitBox.w;
            int boxRight = box.x + box.w;

            int bCollision = boxBottom - player.hitBox.y;
            int tCollision = playerBottom - box.y;
            int lCollision = playerRight - box.x;
            int rCollision = boxRight - player.hitBox.x;

            // If the collision is from the bottom or top
            if (tCollision < bCollision && tCollision < lCollision && tCollision < rCollision) {
                player.hitBox.y = box.y - player.hitBox.h;  // Move player above the box
                player.velocity.y = 0;
                player.isJumping = false;  // Allow jumping again
            } else if (bCollision < tCollision && bCollision < lCollision && bCollision < rCollision) {
                player.hitBox.y = box.y + box.h;  // Move player below the box
                player.velocity.y = 0;
            }
            // If the collision is from the left or right
            else if (lCollision < rCollision && lCollision < tCollision && lCollision < bCollision) {
                player.hitBox.x = box.x - player.hitBox.w;  // Move player to the left of the box
                player.velocity.x = 0;
            } else if (rCollision < lCollision && rCollision < tCollision && rCollision < bCollision) {
                player.hitBox.x = box.x + box.w;  // Move player to the right of the box
                player.velocity.x = 0;
            }
        }

        // If the player walks off the edge of the box
        if (wasOnBox && !checkCollision(player.hitBox, box)) {
            player.isJumping = true;
        }

        // Calculate and correct fps
        float avgFPS = static_cast<float>(countedFrames) / (static_cast<float>(fpsTimer.getTicks()) / 1000.f);

        // Clear screen
        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(gRenderer);
        
        SDL_Rect* currentClip = &IdleSpriteClips[frame / IDLE_ANIMATION_FRAMES];

        if (movingLeft || movingRight) currentClip = &RunningSpriteClips[frame / WALKING_ANIMATION_FRAMES];

        SDL_Point rotationPoint = {currentClip->w / 2, currentClip->h / 2};

        playerCamera.x = (player.hitBox.x + PLAYER_WIDTH / 2) - SCREEN_WIDTH / 2;
        playerCamera.y = (player.hitBox.y + PLAYER_HEIGHT - SCREEN_HEIGHT + 50);

        camera.x = (player.hitBox.x + PLAYER_WIDTH / 2) - SCREEN_WIDTH / 2;
        camera.y = (player.hitBox.y + PLAYER_HEIGHT - SCREEN_HEIGHT + 50);

        while (camera.x > (multiplier + 1) * width) {
            multiplier++;
        }

        // If camera goes out of left boundary
        while (camera.x < multiplier * width) {
            multiplier--;
        }

        // Update camera position only if the multiplier has changed
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
        
        gSpriteSheetTexture.render(player.hitBox.x - playerCamera.x, player.hitBox.y - playerCamera.y, currentClip, 0.0f, &rotationPoint, flipType);

        gBlocksSheetTexture.render(box.x - camera.x, box.y - camera.y, &BlockSpriteClip);

        // Update screen
        SDL_RenderPresent(gRenderer);

        // Go to next frame
        ++frame;

        // Cycle animation
        if ((movingLeft || movingRight) && (frame / WALKING_ANIMATION_FRAMES >= WALKING_ANIMATION_FRAMES)) {
            frame = 0;
        } else if (frame / IDLE_ANIMATION_FRAMES >= IDLE_ANIMATION_FRAMES) {
            frame = 0;
        }

        ++countedFrames;

        int frameTicks = capTimer.getTicks();
        if (frameTicks < SCREEN_TICKS_PER_FRAME) {
            // Wait remaining time
            SDL_Delay(SCREEN_TICKS_PER_FRAME - frameTicks);
        }
    }
}


bool Game::init(){
	//Initialization flag
	bool success = true;

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
        RunningSpriteClips[ 3 ].w = 50;
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

	return success;
}

void Game::close()
{
	//Free loaded images
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
	IMG_Quit();
	SDL_Quit();
}

