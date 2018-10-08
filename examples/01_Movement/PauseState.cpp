#include <iostream>
#include <cmath>
#include "Game.h"
#include "PauseState.h"
#include "InputManager.h"

PauseState PauseState::m_PauseState;

using namespace std;

void PauseState::init()
{
    tela_pause.load("data/img/paused.png");
	tela_pause.setPosition(500,500);

    im = cgf::InputManager::instance();

    im->addKeyInput("pauseP", sf::Keyboard::L);

	cout << "PauseState: Init" << endl;
}

void PauseState::cleanup()
{
    free(im);
	cout << "PauseState: Clean" << endl;
}

void PauseState::pause()
{
	cout << "PauseState: Paused" << endl;
}

void PauseState::resume()
{
	cout << "PauseState: Resumed" << endl;
}

void PauseState::handleEvents(cgf::Game* game)
{
    sf::Event event;

    while (screen->pollEvent(event))
    {
        if(event.type == sf::Event::Closed)
            game->quit();
    }

    if(im->testEvent("pauseP")){
        game->popState();
    }
}

void PauseState::update(cgf::Game* game)
{

}

void PauseState::draw(cgf::Game* game)
{
    screen = game->getScreen();
    screen->draw(tela_pause);
}
