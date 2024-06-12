#include "Game.h"

int main( int argc, char* args[] )
{
	Game game;

	game.gameLoop();

	game.close();

	return 0;
}
