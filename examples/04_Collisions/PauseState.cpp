#include <iostream>
#include <cmath>
#include "Game.h"
#include "PauseState.h"

PauseState PauseState::m_PauseState;

using namespace std;

void PauseState::init()
{
    if (!font.loadFromFile("data/fonts/emulogic.ttf")) {
        cout << "Cannot load emulogic.ttf font!" << endl;
        exit(1);
    }
    text = sf::Text();
    text.setFont(font);
    text.setString(L"PAUSE");
    text.setCharacterSize(50);
    text.setFillColor(sf::Color::White);
    text.setStyle(sf::Text::Bold);

    text2 = sf::Text();
    text2.setFont(font);
    text2.setString(L"Press enter to resume");
    text2.setCharacterSize(22);
    text2.setFillColor(sf::Color::White);
    text2.setStyle(sf::Text::Bold);

	cout << "PauseState: Init" << endl;
}

void PauseState::cleanup()
{
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
    sf::RenderWindow* screen = game->getScreen();

    while (screen->pollEvent(event))
    {
        switch (event.type)
        {
        case sf::Event::Closed:
            game->quit();
            break;

        case sf::Event::KeyPressed:
            if(event.key.code == sf::Keyboard::Return)
                game->popState();
            if(event.key.code == sf::Keyboard::Escape)
                game->quit();
            break;
        default:
            break;
        }
    }
}

void PauseState::update(cgf::Game* game)
{
}

void PauseState::draw(cgf::Game* game)
{
    sf::RenderWindow* screen = game->getScreen();
    text.setPosition(screen->getView().getCenter().x - 150, screen->getView().getCenter().y - 100);
    text2.setPosition(screen->getView().getCenter().x - 240, screen->getView().getCenter().y - 40);
    screen->draw(text);
    screen->draw(text2);
}
