#include <iostream>
#include "Game.h"
#include "InputManager.h"
#include "EndState.h"
#include "PlayState.h"

EndState EndState::m_EndState;

using namespace std;
bool EndState::won;

void EndState::init()
{
     if (!font.loadFromFile("data/fonts/emulogic.ttf")) {
        cout << "Cannot load emulogic.ttf font!" << endl;
        exit(1);
    }
    text = sf::Text();
    text.setFont(font);
    if(won)
        text.setString(L"You Win!");
    else
        text.setString(L"Game Over");
    text.setCharacterSize(50);
    text.setFillColor(sf::Color::White);
    text.setStyle(sf::Text::Bold);

    text2 = sf::Text();
    text2.setFont(font);
    text2.setString(L"Press enter to restart");
    text2.setCharacterSize(25);
    text2.setFillColor(sf::Color::White);
    text2.setStyle(sf::Text::Bold);
    cout << "EndState: Init" << endl;
}

void EndState::cleanup()
{
    cout << "EndState: Clean" << endl;
}

void EndState::pause()
{
    cout << "EndState: Paused" << endl;
}

void EndState::resume()
{
    cout << "EndState: Resumed" << endl;
}

void EndState::handleEvents(cgf::Game* game)
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
            if(event.key.code == sf::Keyboard::Escape)
                game->quit();
            if(event.key.code == sf::Keyboard::Return)
                game->changeState(PlayState::instance());
            break;
        default:
            break;
        }
    }
}

void EndState::update(cgf::Game* game)
{
}

void EndState::draw(cgf::Game *game)
{
    screen = game->getScreen();
    if(won)
        text.setPosition(screen->getView().getCenter().x - 200, screen->getView().getCenter().y - 100);
    else
        text.setPosition(screen->getView().getCenter().x - 225, screen->getView().getCenter().y - 100);
    text2.setPosition(screen->getView().getCenter().x - 280, screen->getView().getCenter().y - 40);
    screen->draw(text);
    screen->draw(text2);
}

