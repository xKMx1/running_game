#ifndef TEXTURE_HANDLER_H
#define TEXTURE_HANDLER_H

#include <string>
#include <SDL.h>
#include <SDL_image.h>

class TextureHandler
{
	public:
		SDL_Texture* mTexture;
		
		//Initializes variables
		TextureHandler( SDL_Renderer* renderer );

		//Deallocates memory
		~TextureHandler();

		//Loads image at specified path
		bool loadFromFile( std::string path );

		void free();

		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Renderer* mRenderer;

		//Image dimensions
		int mWidth;
		int mHeight;
};

#endif