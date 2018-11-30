/*
 *  PlayState.cpp
 *  Normal "play" state
 *
 *  Created by Marcelo Cohen on 08/13.
 *  Edited by Leonardo Porto on 10/18.
 *  Copyright 2013 PUCRS. All rights reserved.
 *
 */

#include <iostream>
#include <cmath>
#include "Game.h"
#include "PlayState.h"
#include "InputManager.h"
#include "EndState.h"
#include "PauseState.h"
#include <time.h>
#include <sstream>
#include <iomanip>
#include "VectorUtils.h"

PlayState PlayState::m_PlayState;

using namespace std;

void PlayState::init()
{
    beginningSoundBuffer.loadFromFile("data/audio/pacman/pacman_beginning.wav");
    beginningSound.setBuffer(beginningSoundBuffer);
    beginningSound.setAttenuation(0);
    beginningSound.play();

    if (!font.loadFromFile("data/fonts/emulogic.ttf")) {
        cout << "Cannot load emulogic.ttf font!" << endl;
        exit(1);
    }
    pontos = 0;
    text.setFont(font);
    ostringstream stringStream;
    stringStream << "Pontos: 0000";
    text.setString(stringStream.str());
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::White);
    text.setStyle(sf::Text::Bold);

    textVida.setFont(font);
    ostringstream stringStreamVida;
    stringStreamVida << "Vidas: ";
    textVida.setString(stringStreamVida.str());
    textVida.setCharacterSize(20);
    textVida.setFillColor(sf::Color::White);
    textVida.setStyle(sf::Text::Bold);

    mapa = new tmx::MapLoader("data/maps/pacman");
    mapa->Load("mapa_pacman.tmx");

    walkStates[0] = "andar-direita";
    walkStates[1] = "andar-esquerda";
    walkStates[2] = "andar-cima";
    walkStates[3] = "andar-baixo";
    currentDir = RIGHT;
    player.load("data/img/pacman/pacman_32x32.png",32,32,0,0,0,0,12,1,12);
    player.setPosition(32,64);
    player.loadAnimation("data/img/pacman/pacman_anim.xml");
    player.setAnimation(walkStates[currentDir]);
    player.setAnimRate(30);
    player.setScale(0.7,0.7);
    player.play();

    playerK.pos.x = 32;
    playerK.pos.y = 64;
    playerK.vel.x = playerK.vel.y = 0;
    playerK.sprite = &player;
    speed = 190;

    firstTime = true;

    initPointsFixed();
    initGhosts();

    steerMode = PURSUIT_BEHAVIOR;

    initVidas();

    dirx = 0;
    diry = 0;
    modoPowerup = false;
    tempo = 0;

    im = cgf::InputManager::instance();

    im->addKeyInput("left", sf::Keyboard::Left);
    im->addKeyInput("right", sf::Keyboard::Right);
    im->addKeyInput("up", sf::Keyboard::Up);
    im->addKeyInput("down", sf::Keyboard::Down);
    im->addKeyInput("quit", sf::Keyboard::Escape);
    im->addKeyInput("stats", sf::Keyboard::S);
    im->addKeyInput("pause", sf::Keyboard::P);

    cout << "PlayState: Init" << endl;
}

void PlayState::setup(cgf::Game* game)
{
    eatGhostSoundBuffer.loadFromFile("data/audio/pacman/pacman_eatghost.wav");
    eatGhostSound.setBuffer(eatGhostSoundBuffer);
    eatGhostSound.setAttenuation(0);

    eatPowerupSoundBuffer.loadFromFile("data/audio/pacman/pacman_eatfruit.wav");
    eatPowerupSound.setBuffer(eatPowerupSoundBuffer);
    eatPowerupSound.setAttenuation(0);

    deathSoundBuffer.loadFromFile("data/audio/pacman/pacman_death.wav");
    deathSound.setBuffer(deathSoundBuffer);
    deathSound.setAttenuation(0);

    chompSoundBuffer.loadFromFile("data/audio/pacman/eating.wav");
    chompSound.setBuffer(chompSoundBuffer);
    chompSound.setAttenuation(0);

    musicPowerup.openFromFile("data/audio/pacman/pacman_intermission.wav");
    musicPowerup.setVolume(30);
    musicPowerup.setLoop(true);
}

void PlayState::cleanup()
{
    delete mapa;
    pointsVector.clear();
    cout << "PlayState: Clean" << endl;
}

void PlayState::pause()
{
    cout << "PlayState: Paused" << endl;
}

void PlayState::resume()
{
    cout << "PlayState: Resumed" << endl;
}

void PlayState::handleEvents(cgf::Game* game)
{
    if(firstTime) {
        setup(game);
        firstTime = false;
    }
    screen = game->getScreen();
    sf::View view = screen->getView();
    sf::Event event;

    while (screen->pollEvent(event))
    {
        if(event.type == sf::Event::Closed)
            game->quit();
    }

    dirx = diry = 0;
    int newDir = currentDir;

    if(im->testEvent("left")) {
        dirx = -1;
        newDir = LEFT;
    }

    if(im->testEvent("right")) {
        dirx = 1;
        newDir = RIGHT;
    }

    if(im->testEvent("up")) {
        diry = -1;
        newDir = UP;
    }

    if(im->testEvent("down")) {
        diry = 1;
        newDir = DOWN;
    }

    if(im->testEvent("pause"))
        game->pushState(PauseState::instance());

    if(im->testEvent("quit"))
        game->quit();

    if(im->testEvent("stats"))
        game->toggleStats();

    if(im->testEvent("zoomin")) {
        view.zoom(1.01);
        screen->setView(view);
    }

    if(im->testEvent("zoomout")) {
        view.zoom(0.99);
        screen->setView(view);
    }

    if(dirx == 0 && diry == 0) {
        player.pause();
    }
    else {
        if(currentDir != newDir) {
            player.setAnimation(walkStates[newDir]);
            currentDir = newDir;
        }
        player.play();
    }

    playerK.vel.x = dirx*speed;
    playerK.vel.y = diry*speed;
}

// Steering Behavior: chase target
sf::Vector3f PlayState::chase(Kinematic& vehicle, sf::Vector3f& target)
{
    sf::Vector3f desiredVel = target - vehicle.pos;
    vecutils::normalize(desiredVel);
    desiredVel *= vehicle.maxSpeed;
    return desiredVel - vehicle.vel;
}

// Steering Behavior: arrive at target
sf::Vector3f PlayState::arrive(Kinematic& vehicle, sf::Vector3f& target, float decel)
{
    sf::Vector3f toTarget = target - vehicle.pos;
    float d = vecutils::length(toTarget);
    if(d > 0)
    {
        // Calculate the speed required to reach the target given the desired
        // deceleration
        float speed = d / decel;

        // Make sure the velocity does not exceed the max
        speed = min(speed, vehicle.maxSpeed);

        // From here proceed just like chase, except we don't need to normalize
        // the toTarget vector because we have already gone to the trouble
        // of calculating its length: d
        sf::Vector3f desiredVel = toTarget * speed / d;
        return desiredVel - vehicle.vel;
    }
    return sf::Vector3f(0,0,0);
}

// Steering Behavior: flee from target
sf::Vector3f PlayState::flee(Kinematic& vehicle, sf::Vector3f& target, float panicDistance)
{
    float panicDistance2 = panicDistance * panicDistance;
    if(vecutils::distanceSquared(vehicle.pos, target) > panicDistance2)
        return sf::Vector3f(0,0,0);
    sf::Vector3f desiredVel = vehicle.pos - target;
    vecutils::normalize(desiredVel);
    desiredVel *= vehicle.maxSpeed;
    return desiredVel - vehicle.vel;
}

// Steering Behavior: pursuit target
sf::Vector3f PlayState::pursuit(Kinematic& vehicle, Kinematic& target)
{
    sf::Vector3f toEvader = target.pos - vehicle.pos;
    double relativeHeading = vecutils::dotProduct(vehicle.heading, target.heading);
    // If target is facing us, go chase it
    if(vecutils::dotProduct(toEvader, vehicle.heading) > 0 && relativeHeading < -0.95) // acos(0.95) = 18 graus
        return arrive(vehicle, target.pos);

    // Not facing, so let's predict where the target will be

    // The look-ahead time is proportional to the distance between the target
    // and the enemy, and is inversely proportional to the sum of the
    // agents' velocities

    float vel = vecutils::length(target.vel);
    float lookAheadTime = vecutils::length(toEvader) / (vehicle.maxSpeed + vel);

    // Now chase to the predicted future position of the target

    sf::Vector3f predicted(target.pos + target.vel * lookAheadTime);
    return arrive(vehicle, predicted, 0.1);
}

// Steering Behavior: evade target
sf::Vector3f PlayState::evade(Kinematic& vehicle, Kinematic& target)
{
    sf::Vector3f toPursuer = target.pos - vehicle.pos;

    // The look-ahead time is proportional to the distance between the pursuer
    // and the vehicle, and is inversely proportional to the sum of the
    // agents' velocities

    float vel = vecutils::length(target.vel);
    float lookAheadTime = vecutils::length(toPursuer) / (vehicle.maxSpeed + vel);

    // Now chase to the predicted future position of the target

    sf::Vector3f predicted(target.pos + target.vel * lookAheadTime);
    return flee(vehicle, predicted);
}

void PlayState::update(cgf::Game* game)
{
    screen = game->getScreen();
    if(modoPowerup){
        tempo += game->getUpdateInterval();
        if(tempo >= 15000){
            musicPowerup.stop();
            //musicChomp.play();
            modoPowerup = false;
            tempo = 0;
            steerMode = PURSUIT_BEHAVIOR;
            ghost1.load("data/img/pacman/inimigos/ghost.png");
            ghost2.load("data/img/pacman/inimigos/ghost.png");
            ghost3.load("data/img/pacman/inimigos/ghost.png");
            ghost1.setVisible(true);
            ghost2.setVisible(true);
            ghost3.setVisible(true);
        }
    }
    //if(beginningSound.getStatus() == sf::Sound::Stopped && musicChomp.getStatus() == sf::Sound::Stopped && !modoPowerup){
   //     musicChomp.play();
    //}

    checkCollision(1, game, playerK);
    centerMapOnPlayer();

    if(ghost1.isVisible()){
        applyBehaviors(ghostK1);
        checkCollision(1, game, ghostK1);
    }
    if(ghost2.isVisible()){
        applyBehaviors(ghostK2);
        checkCollision(1, game, ghostK2);
    }
    if(ghost3.isVisible()){
        applyBehaviors(ghostK3);
        checkCollision(1, game, ghostK3);
    }

    checkCollisionPoint(game);
    checkCollisionGhost(game);
    checkCollisionPowerup(game);
}

void PlayState::applyBehaviors(Kinematic& enemyK)
{
#define STEERING
#ifdef STEERING
    // Apply steering behavior(s)

    //sf::Vector3f steeringForce = flee(enemyK, playerK,100);
    //sf::Vector3f steeringForce = pursuit(enemyK, playerK);
    sf::Vector3f steeringForce;
    switch(steerMode) {
        case CHASE_BEHAVIOR:
            steeringForce = chase(enemyK, playerK.pos);
            break;
        case ARRIVE_BEHAVIOR:
            steeringForce = arrive(enemyK, playerK.pos, 0.3); // 0.3 - rapido ... 1 - lento
            break;
        case PURSUIT_BEHAVIOR:
            steeringForce = pursuit(enemyK, playerK);
            break;
        case FLEE_BEHAVIOR:
            steeringForce = flee(enemyK, playerK.pos, 100);
            break;
        case EVADE_BEHAVIOR:
            steeringForce = evade(enemyK, playerK);
    }
    sf::Vector3f accel = steeringForce/0.7f; // mass;

    enemyK.vel += accel; // * deltaTime

    // Can't exceed max speed
    if(vecutils::lengthSquared(enemyK.vel) > enemyK.maxSpeed*enemyK.maxSpeed) {
        vecutils::normalize(enemyK.vel);
        enemyK.vel *= enemyK.maxSpeed;
    }

    // Only update heading if speed is above minimum threshold
    if(vecutils::lengthSquared(enemyK.vel) > 0.00000001) {
        enemyK.heading = enemyK.vel / vecutils::length(enemyK.vel);
    }

#else
    // Basic chase
    if(enemyK.pos.X < playerK.pos.X)
        enemyK.vel.X = 2;
    else if(enemyK.pos.X > playerK.pos.X)
        enemyK.vel.X = -2;
    if(enemyK.pos.Y < playerK.pos.Y)
        enemyK.vel.Y = 2;
    else if(enemyK.pos.Y > playerK.pos.Y)
        enemyK.vel.Y = -2;
#endif
}

void PlayState::draw(cgf::Game* game)
{
    screen = game->getScreen();
    mapa->Draw(*screen);
    screen->draw(player);
    for(cgf::Sprite ponto : pointsVector)
        screen->draw(ponto);

    ghost1.setPosition(ghostK1.pos.x, ghostK1.pos.y);
    screen->draw(ghost1);
    ghost2.setPosition(ghostK2.pos.x, ghostK2.pos.y);
    screen->draw(ghost2);
    ghost3.setPosition(ghostK3.pos.x, ghostK3.pos.y);
    screen->draw(ghost3);

    for(cgf::Sprite vida : vidasVector)
        screen->draw(vida);
    for(cgf::Sprite powerup : powerupsVector)
        screen->draw(powerup);
    screen->draw(textVida);
    screen->draw(text);
}

void PlayState::initPoints(){
    srand(time(NULL));
    int n = rand() % 200 + 50;
    int baseX = 32;
    int baseY = 32;
    int limiteX = (mapa->GetMapSize().x-48)/32;
    int limiteY = (mapa->GetMapSize().y-48)/32;
    for(int i = 0; i < n; i++){
        cgf::Sprite ponto = cgf::Sprite();
        ponto.load("data/img/pacman/sprites/ponto.png");
        int posX = (baseX) + ((rand() % limiteX + 2)*30);
        int posY = (baseY) + ((rand() % limiteY + 2)*30);
        while(getCellFromMap(1, posX, posY) || getCellFromMap(2, posX, posY)){
            posX = (baseX) + ((rand() % limiteX + 2)*30);
            posY = (baseY) + ((rand() % limiteY + 2)*30);
        }
        ponto.setPosition(posX, posY);
        pointsVector.push_back(ponto);
    }
}

void PlayState::centerMapOnPlayer()
{
    sf::View view = screen->getView();
    sf::Vector2u mapsize = mapa->GetMapSize();
    sf::Vector2f viewsize = view.getSize();
    viewsize.x /= 2;
    viewsize.y /= 2;
    sf::Vector2f pos = player.getPosition();

    float panX = viewsize.x; // minimum pan
    if(pos.x >= viewsize.x)
        panX = pos.x;

    if(panX >= mapsize.x - viewsize.x)
        panX = mapsize.x - viewsize.x;

    float panY = viewsize.y; // minimum pan
    if(pos.y >= viewsize.y)
        panY = pos.y;

    if(panY >= mapsize.y - viewsize.y)
        panY = mapsize.y - viewsize.y;

    sf::Vector2f center(panX,panY);
    text.setPosition(panX + 185, panY-(view.getSize().y/2));
    float posX = panX - (view.getSize().x/2);
    posX += 10;
    textVida.setPosition(posX, panY-(view.getSize().y/2));
    posX += 125;
    for(int i = 0; i < vidasVector.size(); i++){
        vidasVector[i].setPosition(posX, panY-(view.getSize().y/2));
        posX += 32;
    }
    view.setCenter(center);
    screen->setView(view);
}

void PlayState::checkCollisionPowerup(cgf::Game* game){
    int i;
    bool colisao = false;
    for(i = 0; i < powerupsVector.size(); i++){
        if(powerupsVector[i].bboxCollision(player)){
            eatPowerupSound.play();
            //musicChomp.stop();
            musicPowerup.play();
            tempo = 0;
            colisao = true;
            modoPowerup = true;
            addPoint(10);
            steerMode = FLEE_BEHAVIOR;
            ghost1.load("data/img/pacman/inimigos/ghost_fraco.png");
            ghost2.load("data/img/pacman/inimigos/ghost_fraco.png");
            ghost3.load("data/img/pacman/inimigos/ghost_fraco.png");
            break;
        }
    }
    if(powerupsVector.size() > 0 && colisao)
        powerupsVector.erase(powerupsVector.begin() + i);
}

void PlayState::checkCollisionPoint(cgf::Game* game){
    if(pointsVector.size() == 0)
        game->changeState(EndState::instance(true));

    int i;
    bool colisao = false;
    for(i = 0; i < pointsVector.size(); i++){
        if(pointsVector[i].bboxCollision(player)){
            if(chompSound.getStatus() == sf::Sound::Stopped)
                chompSound.play();
            colisao = true;
            addPoint(1);
            break;
        }
    }
    if(pointsVector.size() > 0 && colisao)
        pointsVector.erase(pointsVector.begin() + i);
}

void PlayState::checkCollisionGhost(cgf::Game* game){
    bool colisao = false;

    if(ghost1.isVisible() && ghost1.bboxCollision(player)){
        colisao = true;
        if(modoPowerup){
            addPoint(25);
            eatGhostSound.play();
            ghost1.setVisible(false);
        }
    } else if(ghost2.isVisible() && ghost2.bboxCollision(player)){
        colisao = true;
        if(modoPowerup){
            addPoint(25);
            eatGhostSound.play();
            ghost2.setVisible(false);
        }
    } else if(ghost3.isVisible() && ghost3.bboxCollision(player)){
        colisao = true;
        if(modoPowerup){
            addPoint(25);
            eatGhostSound.play();
            ghost3.setVisible(false);
        }
    }

    if(colisao && !modoPowerup){
        vidasVector.erase(vidasVector.end());
        if(vidasVector.size() == 0)
            game->changeState(EndState::instance(false));
        else {
            deathSound.play();
            if((abs(player.getPosition().x - 32) < 160) &&
            (abs(player.getPosition().y - 64) < 160)){
                player.setPosition(1000, 256);
                playerK.pos.x = 1000;
                playerK.pos.y = 256;
            } else{
                player.setPosition(32, 64);
                playerK.pos.x = 32;
                playerK.pos.y = 64;
            }
        }
    }

//    if(ghostsVector.size() > 0 && colisao && modoPowerup)
//        ghostsVector.erase(ghostsVector.begin() + i);
}

bool PlayState::checkCollision(uint8_t layer, cgf::Game* game, Kinematic& obj)
{
    int i, x1, x2, y1, y2;
    bool bump = false;

    // Get the limits of the map
    sf::Vector2u mapsize = mapa->GetMapSize();
    // Get the width and height of a single tile
    sf::Vector2u tilesize = mapa->GetMapTileSize();

    mapsize.x /= tilesize.x;
    mapsize.y /= tilesize.y;
    mapsize.x--;
    mapsize.y--;

    // Get the height and width of the object (in this case, 100% of a tile)
    sf::Vector2u objsize = obj.sprite->getSize();
    objsize.x *= obj.sprite->getScale().x;
    objsize.y *= obj.sprite->getScale().y;

    float px = obj.pos.x; //->getPosition().x;
    float py = obj.pos.y; //->getPosition().y;

    double deltaTime = game->getUpdateInterval();

    sf::Vector2f offset(obj.vel.x/1000 * deltaTime, obj.vel.y/1000 * deltaTime);

    float vx = offset.x;
    float vy = offset.y;

    //cout << "px,py: " << px << " " << py << endl;

    //cout << "tilesize " << tilesize.x << " " << tilesize.y << endl;
    //cout << "mapsize " << mapsize.x << " " << mapsize.y << endl;

    // Test the horizontal movement first
    i = objsize.y > tilesize.y ? tilesize.y : objsize.y;

    for (;;)
    {
        x1 = (px + vx) / tilesize.x;
        x2 = (px + vx + objsize.x - 1) / tilesize.x;

        y1 = (py) / tilesize.y;
        y2 = (py + i - 1) / tilesize.y;

        if (x1 >= 0 && x2 < mapsize.x && y1 >= 0 && y2 < mapsize.y)
        {
            if (vx > 0)
            {
                // Trying to move right

                int upRight   = getCellFromMap(layer, x2*tilesize.x, y1*tilesize.y);
                int downRight = getCellFromMap(layer, x2*tilesize.x, y2*tilesize.y);
                if (upRight || downRight)
                {
                    // Place the player as close to the solid tile as possible
                    px = x2 * tilesize.x;
                    px -= objsize.x;// + 1;
                    vx = 0;
                    bump = true;
                }
            }

            else if (vx < 0)
            {
                // Trying to move left

                int upLeft   = getCellFromMap(layer, x1*tilesize.x, y1*tilesize.y);
                int downLeft = getCellFromMap(layer, x1*tilesize.x, y2*tilesize.y);
                if (upLeft || downLeft)
                {
                    // Place the player as close to the solid tile as possible
                    px = (x1+1) * tilesize.x;
                    vx = 0;
                    bump = true;
                }
            }
        }

        if (i == objsize.y) // Checked player height with all tiles ?
        {
            break;
        }

        i += tilesize.y; // done, check next tile upwards

        if (i > objsize.y)
        {
            i = objsize.y;
        }
    }

    // Now test the vertical movement

    i = objsize.x > tilesize.x ? tilesize.x : objsize.x;

    for (;;)
    {
        x1 = (px / tilesize.x);
        x2 = ((px + i-1) / tilesize.x);

        y1 = ((py + vy) / tilesize.y);
        y2 = ((py + vy + objsize.y-1) / tilesize.y);

        if (x1 >= 0 && x2 < mapsize.x && y1 >= 0 && y2 < mapsize.y)
        {
            if (vy > 0)
            {
                // Trying to move down
                int downLeft  = getCellFromMap(layer, x1*tilesize.x, y2*tilesize.y);
                int downRight = getCellFromMap(layer, x2*tilesize.x, y2*tilesize.y);
                if (downLeft || downRight)
                {
                    // Place the player as close to the solid tile as possible
                    py = y2 * tilesize.y;
                    py -= objsize.y;
                    vy = 0;
                    bump = true;
                }
            }

            else if (vy < 0)
            {
                // Trying to move up

                int upLeft  = getCellFromMap(layer, x1*tilesize.x, y1*tilesize.y);
                int upRight = getCellFromMap(layer, x2*tilesize.x, y1*tilesize.y);
                if (upLeft || upRight)
                {
                    // Place the player as close to the solid tile as possible
                    py = (y1 + 1) * tilesize.y;
                    vy = 0;
                    bump = true;
                }
            }
        }

        if (i == objsize.x)
        {
            break;
        }

        i += tilesize.x; // done, check next tile to the right

        if (i > objsize.x)
        {
            i = objsize.x;
        }
    }

    // Now apply the movement and animation

    obj.pos.x = px+vx;
    obj.pos.y = py+vy;

    // Check collision with edges of map
    if (obj.pos.x < 0)
        obj.pos.x = 0;
    else if (obj.pos.x + objsize.x >= mapsize.x * tilesize.x)
        obj.pos.x = mapsize.x*tilesize.x - objsize.x - 1;

    if(obj.pos.y < 0)
        obj.pos.y = 0;
    else if(obj.pos.y + objsize.y >= mapsize.y * tilesize.y)
        obj.pos.y = mapsize.y*tilesize.y - objsize.y - 1;

    obj.sprite->setPosition(obj.pos.x, obj.pos.y);
    obj.sprite->update(deltaTime, false); // only update animation

    return bump;
}

void PlayState::addPoint(int qtd){
    std::stringstream stringStream;
    pontos += qtd;
    string texto = "Pontos: ";
    stringStream << std::setw(4) << std::setfill('0') << pontos;
    texto = texto.append(stringStream.str());
    text.setString(texto);
}

void PlayState::initPointsFixed(){
    tmx::MapLayer layer = mapa->GetLayers()[0];
    int offsetX = mapa->GetMapTileSize().x / 2;
    int offsetY = mapa->GetMapTileSize().y / 2;
    srand(time(NULL));
    int qtdPowerup = rand() % 15 + 1;
    int linha = rand() % 33;
    linha = linha * mapa->GetMapTileSize().y;
    for(int x = 0; x < mapa->GetMapSize().x; x+=mapa->GetMapTileSize().x){
        for(int y = 0; y < mapa->GetMapSize().y; y+=mapa->GetMapTileSize().y){
            float posX = x + offsetX;
            float posY = y + offsetY;
            if(!getCellFromMap(1, posX, posY) && !getCellFromMap(2, posX, posY)){
                if(y == linha && qtdPowerup > 0){
                    cgf::Sprite poder = cgf::Sprite();
                    poder.load("data/img/pacman/sprites/poder.png");
                    poder.setPosition(x, y);
                    powerupsVector.push_back(poder);
                    qtdPowerup--;
                    cout << "poder" << endl;
                } else {
                    cgf::Sprite ponto = cgf::Sprite();
                    ponto.load("data/img/pacman/sprites/ponto.png");
                    ponto.setPosition(x, y);
                    pointsVector.push_back(ponto);
                }
            }
        }
        linha = rand() % 33;
        linha = linha * mapa->GetMapTileSize().y;
    }
}

void PlayState::initGhosts(){
    ghost1 = cgf::Sprite();
    ghost1.load("data/img/pacman/inimigos/ghost.png");
    ghost1.setPosition(448, 512);
    ghost1.setScale(sf::Vector2f(2,2));

    ghostK1.sprite = &ghost1;
    ghostK1.pos.x = 448;
    ghostK1.pos.y = 512;
    ghostK1.maxSpeed = 150;

    ghost2 = cgf::Sprite();
    ghost2.load("data/img/pacman/inimigos/ghost.png");
    ghost2.setPosition(480, 512);
    ghost2.setScale(sf::Vector2f(2,2));

    ghostK2.sprite = &ghost2;
    ghostK2.pos.x = 480;
    ghostK2.pos.y = 512;
    ghostK2.maxSpeed = 180;

    ghost3 = cgf::Sprite();
    ghost3.load("data/img/pacman/inimigos/ghost.png");
    ghost3.setPosition(492, 512);
    ghost3.setScale(sf::Vector2f(2,2));

    ghostK3.sprite = &ghost3;
    ghostK3.pos.x = 492;
    ghostK3.pos.y = 512;
    ghostK3.maxSpeed = 130;
}

void PlayState::initVidas(){
    vidas = 3;
    for(int i = 0 ; i < vidas; i++){
        cgf::Sprite vida = cgf::Sprite();
        vida.load("data/img/pacman/sprites/32x32/pac2.png");
        vida.setScale(0.7, 0.7);
        vidasVector.push_back(vida);
    }
}

sf::Uint16 PlayState::getCellFromMap(uint8_t layernum, float x, float y)
{
    auto& layers = mapa->GetLayers();
    tmx::MapLayer& layer = layers[layernum];
    sf::Vector2u mapsize = mapa->GetMapSize();
    sf::Vector2u tilesize = mapa->GetMapTileSize();
    mapsize.x /= tilesize.x;
    mapsize.y /= tilesize.y;
    int col = floor(x / tilesize.x);
    int row = floor(y / tilesize.y);
    return layer.tiles[row*mapsize.x + col].gid;
}
