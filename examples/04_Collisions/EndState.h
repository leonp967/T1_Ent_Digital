#ifndef _END_STATE_H_
#define _END_STATE_H_

#include "GameState.h"
#include "Sprite.h"

class EndState : public cgf::GameState
{
    public:

    void init();
    void cleanup();

    void pause();
    void resume();

    void handleEvents(cgf::Game* game);
    void update(cgf::Game* game);
    void draw(cgf::Game* game);

    // Implement Singleton Pattern
    static EndState* instance()
    {
        return &m_EndState;
    }

    protected:

    EndState() {}

    private:

    static EndState m_EndState;

    sf::RenderWindow* screen;
    cgf::Sprite endSprite;
    sf::Font font;
    sf::Text text;
    sf::Text text2;
};

#endif

