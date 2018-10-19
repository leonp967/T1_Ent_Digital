#include <iostream>
#include "Game.h"
#include "InputManager.h"
#include "EndState.h"
#include "PlayState.h"

EndState EndState::m_EndState;

using namespace std;

void EndState::init()
{
     if (!font.loadFromFile("data/fonts/emulogic.ttf")) {
        cout << "Cannot load emulogic.ttf font!" << endl;
        exit(1);
    }
    text = sf::Text();
    text.setFont(font);
    text.setString(L"You Win!");
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
        // check the type of the event...
        switch (event.type)
        {
            // window closed
        case sf::Event::Closed:
            game->quit();
            break;

            // key pressed
        case sf::Event::KeyPressed:
            if(event.key.code == sf::Keyboard::Escape)
                game->quit();
            if(event.key.code == sf::Keyboard::Return)
                game->changeState(PlayState::instance());
            //game->changeState(PlayMapTop::instance());
            //game->changeState(PlayMapAI::instance());
            //game->changeState(PlayPhysics::instance());
            //game->changeState(PlayMapPhysics::instance());
            break;

            // we don't process other types of events
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
    text.setPosition(screen->getView().getCenter().x - 200, screen->getView().getCenter().y - 100);
    text2.setPosition(screen->getView().getCenter().x - 280, screen->getView().getCenter().y - 40);
    screen->draw(text);
    screen->draw(text2);
}

