#include "TextureHandler.h"

TextureHandler::TextureHandler( SDL_Renderer* renderer )
{
	//Initialize
	mRenderer = renderer;
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

TextureHandler::~TextureHandler()
{
	//Deallocate
	free();
}

bool TextureHandler::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		SDL_Log( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( mRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			SDL_Log( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

void TextureHandler::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void TextureHandler::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };


	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
		// if(renderQuad.w > 100){
			// SDL_Log("%d %d", renderQuad.x, clip->x);
		// }
		SDL_RenderCopyEx( mRenderer, mTexture, clip, &renderQuad, angle, center, flip );
	}
	else{
		SDL_RenderCopyEx( mRenderer, mTexture, NULL, NULL, angle, center, flip );
	}
}

int TextureHandler::getWidth()
{
	return mWidth;
}

int TextureHandler::getHeight()
{
	return mHeight;
}