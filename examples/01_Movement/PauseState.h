#ifndef PAUSESTATE_H
#define PAUSESTATE_H


#include "GameState.h"
#include "Sprite.h"
#include "InputManager.h"

class PauseState : public cgf::GameState
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
    static PauseState* instance()
    {
        return &m_PauseState;
    }

    protected:

    PauseState() {}

    private:

    static PauseState m_PauseState;
    sf::RenderWindow* screen;
    cgf::InputManager* im;
    cgf::Sprite tela_pause;
};

#endif // PAUSESTATE_H_INCLUDED
