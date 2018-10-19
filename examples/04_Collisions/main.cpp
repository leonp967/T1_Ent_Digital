#include "Game.h"
#include "MenuState.h"

int main(int argc, char **argv)
{
    cgf::Game game(5,30);

	game.init("Pac-Man",900,720,false);

    game.changeState(MenuState::instance());

	while(game.isRunning())
	{
		game.handleEvents();
		game.update();
		game.draw();
	}

	game.clean();

    return 0;
}
