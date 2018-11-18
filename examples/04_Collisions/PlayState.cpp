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

PlayState PlayState::m_PlayState;

using namespace std;

void PlayState::init()
{
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
    player.setAnimRate(15);
    player.setScale(0.7,0.7);
    player.play();

    initPointsFixed();
    initGhosts();
    initVidas();

    dirx = 0;
    diry = 0;
    modoPowerup = false;

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

    player.setXspeed(150*dirx);
    player.setYspeed(150*diry);
}

void PlayState::update(cgf::Game* game)
{
    screen = game->getScreen();
    checkCollision(1, game, &player);
    checkCollisionPoint(game);
    checkCollisionGhost(game);
    centerMapOnPlayer();
}

void PlayState::draw(cgf::Game* game)
{
    screen = game->getScreen();
    mapa->Draw(*screen);
    screen->draw(player);
    for(cgf::Sprite ponto : pointsVector)
        screen->draw(ponto);
    for(cgf::Sprite ghost : ghostsVector)
        screen->draw(ghost);
    for(cgf::Sprite vida : vidasVector)
        screen->draw(vida);
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
        printf("antesWhile\n");
        while(getCellFromMap(1, posX, posY) || getCellFromMap(2, posX, posY)){
            posX = (baseX) + ((rand() % limiteX + 2)*30);
            posY = (baseY) + ((rand() % limiteY + 2)*30);
        }
        printf("depoisWhile\n");
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

void PlayState::checkCollisionPoint(cgf::Game* game){
    if(pointsVector.size() == 0)
        game->changeState(EndState::instance(true));

    int i;
    bool colisao = false;
    for(i = 0; i < pointsVector.size(); i++){
        if(pointsVector[i].bboxCollision(player)){
            colisao = true;
            addPoint(1);
            break;
        }
    }
    if(pointsVector.size() > 0 && colisao)
        pointsVector.erase(pointsVector.begin() + i);
}

void PlayState::checkCollisionGhost(cgf::Game* game){
    int i;
    bool colisao = false;
    for(i = 0; i < ghostsVector.size(); i++){
        if(ghostsVector[i].bboxCollision(player)){
            colisao = true;
            if(modoPowerup){
                addPoint(25);
            } else{
                vidasVector.erase(vidasVector.end());
                if(vidasVector.size() == 0)
                   game->changeState(EndState::instance(false));
            }
            break;
        }
    }
    if(ghostsVector.size() > 0 && colisao && modoPowerup)
        ghostsVector.erase(ghostsVector.begin() + i);
}

bool PlayState::checkCollision(uint8_t layer, cgf::Game* game, cgf::Sprite* obj)
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
    sf::Vector2u objsize = obj->getSize();
    objsize.x *= obj->getScale().x;
    objsize.y *= obj->getScale().y;

    float px = obj->getPosition().x;
    float py = obj->getPosition().y;

    double deltaTime = game->getUpdateInterval();

    sf::Vector2f offset(obj->getXspeed()/1000 * deltaTime, obj->getYspeed()/1000 * deltaTime);

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

    obj->setPosition(px+vx,py+vy);
    px = obj->getPosition().x;
    py = obj->getPosition().y;

    obj->update(deltaTime, false); // only update animation

    // Check collision with edges of map
    if (px < 0)
        obj->setPosition(px,py);
    else if (px + objsize.x >= mapsize.x * tilesize.x)
        obj->setPosition(mapsize.x*tilesize.x - objsize.x - 1,py);

    if(py < 0)
       obj->setPosition(px,0);
    else if(py + objsize.y >= mapsize.y * tilesize.y)
        obj->setPosition(px, mapsize.y*tilesize.y - objsize.y - 1);

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
    for(int x = 0; x < mapa->GetMapSize().x; x+=mapa->GetMapTileSize().x){
        for(int y = 0; y < mapa->GetMapSize().y; y+=mapa->GetMapTileSize().y){
            float posX = x + offsetX;
            float posY = y + offsetY;
            if(!getCellFromMap(1, posX, posY) && !getCellFromMap(2, posX, posY)){
                cgf::Sprite ponto = cgf::Sprite();
                ponto.load("data/img/pacman/sprites/ponto.png");
                ponto.setPosition(x, y);
                pointsVector.push_back(ponto);
            }
        }
    }
}

void PlayState::initGhosts(){
    int x = 448;
    for(int i = 0; i < 5; i++){
        cgf::Sprite ghost = cgf::Sprite();
        ghost.load("data/img/pacman/inimigos/ghost.png");
        ghost.setPosition(x, 512);
        x += 32;
        ghostsVector.push_back(ghost);
    }
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
