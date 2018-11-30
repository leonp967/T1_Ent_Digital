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

struct Kinematic
{
    cgf::Sprite* sprite;
    sf::Vector3f pos;
    sf::Vector3f vel;
    sf::Vector3f heading;
    float maxForce;
    float maxSpeed;
    float maxTurnRate;
};

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
    void setup(cgf::Game* game);

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

    float speed;
    cgf::Sprite player, ghost1, ghost2, ghost3;
    Kinematic playerK, ghostK1, ghostK2, ghostK3;
    bool modoPowerup;
    int vidas;
    float tempo;
    bool firstTime;

    sf::SoundBuffer eatGhostSoundBuffer;
    sf::Sound eatGhostSound;
    sf::SoundBuffer chompSoundBuffer;
    sf::Sound chompSound;
    sf::SoundBuffer eatPowerupSoundBuffer;
    sf::Sound eatPowerupSound;
    sf::SoundBuffer deathSoundBuffer;
    sf::Sound deathSound;
    sf::SoundBuffer beginningSoundBuffer;
    sf::Sound beginningSound;
    sf::Music musicPowerup;
    sf::Music musicChomp;

    sf::RenderWindow* screen;
    cgf::InputManager* im;

    tmx::MapLoader* mapa;

    sf::Font font;
    sf::Text text;
    sf::Text textVida;

    std::vector<cgf::Sprite> pointsVector;
    std::vector<cgf::Sprite> powerupsVector;
    std::vector<cgf::Sprite> vidasVector;

    enum {
         CHASE_BEHAVIOR=0, ARRIVE_BEHAVIOR, PURSUIT_BEHAVIOR, FLEE_BEHAVIOR, EVADE_BEHAVIOR
    };

    int steerMode;
    sf::Vector3f chase(Kinematic& vehicle, sf::Vector3f& target); // ir diretamente ao jogador
    sf::Vector3f arrive(Kinematic& vehicle, sf::Vector3f& target, float decel=0.2); // ir diretamente ao jogador
    sf::Vector3f pursuit(Kinematic& vehicle, Kinematic& target); // perseguir o jogador, prevendo a posição futura
	sf::Vector3f flee(Kinematic& vehicle, sf::Vector3f& target, float panicDistance=100);  // fugir do jogador
    sf::Vector3f evade(Kinematic& vehicle, Kinematic& target);

    void centerMapOnPlayer();
    bool checkCollision(uint8_t layer, cgf::Game* game, Kinematic& obj);
    sf::Uint16 getCellFromMap(uint8_t layernum, float x, float y);

    void addPoint(int qtd);
    void checkCollisionPoint(cgf::Game* game);
    void checkCollisionGhost(cgf::Game* game);
    void checkCollisionPowerup(cgf::Game* game);
    void initPoints();
    void initPointsFixed();
    void initGhosts();
    void initVidas();
    void applyBehaviors(Kinematic& enemyK);
};

#endif
