/*
 *  PlayState.h
 *  Normal "play" state
 *
 *  Created by Marcelo Cohen on 08/13.
 *  Edited by Leonardo Porto on 10/18.
 *  Copyright 2013 PUCRS. All rights reserved.
 *
 */

#ifndef PLAY_STATE_H_
#define PLAY_STATE_H_

#include "GameState.h"
#include "Sprite.h"
#include "InputManager.h"
#include <tmx/MapLoader.h>
#include <string>
#include "Physics.h"

class PlayState : public cgf::GameState
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
    static PlayState* instance()
    {
        return &m_PlayState;
    }

    protected:

    PlayState() {}

    private:

    static PlayState m_PlayState;

    enum { RIGHT=0, LEFT, UP, DOWN };
    std::string walkStates[4];
    int currentDir;

    int x, y;
    int dirx, diry;
    int pontos;

    cgf::Sprite player;
    bool modoPowerup;
    int vidas;

    sf::RenderWindow* screen;
    cgf::InputManager* im;

    tmx::MapLoader* mapa;

    sf::Font font;
    sf::Text text;
    sf::Text textVida;

    std::vector<cgf::Sprite> pointsVector;
    std::vector<cgf::Sprite> ghostsVector;
    std::vector<cgf::Sprite> vidasVector;

    void centerMapOnPlayer();
    bool checkCollision(uint8_t layer, cgf::Game* game, cgf::Sprite* obj);
    sf::Uint16 getCellFromMap(uint8_t layernum, float x, float y);

    void addPoint(int qtd);
    void checkCollisionPoint(cgf::Game* game);
    void checkCollisionGhost(cgf::Game* game);
    void initPoints();
    void initPointsFixed();
    void initGhosts();
    void initVidas();
};

#endif
