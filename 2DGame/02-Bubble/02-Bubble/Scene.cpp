#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"


#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 25

Scene::Scene()
{
	map = NULL;
	player = NULL;
	menuSprite = NULL;
	instructionsSprite = NULL;
	creditsSprite = NULL;
	pauseSprite = NULL;
	gameOverSprite = NULL;
	winSprite = NULL;
	gameState = MAIN_MENU;
	for (int i = 0; i <= GLFW_KEY_LAST; ++i)
		keyLastState[i] = false;
}

Scene::~Scene()
{
	texProgram.free();
	if(map != NULL)
		delete map;
	if(player != NULL)
		delete player;
	if(menuSprite != NULL)
		delete menuSprite;
	if(instructionsSprite != NULL)
		delete instructionsSprite;
	if(creditsSprite != NULL)
		delete creditsSprite;
	if(pauseSprite != NULL)
		delete pauseSprite;
	if(gameOverSprite != NULL)
		delete gameOverSprite;
	if(winSprite != NULL)
		delete winSprite;
}


void Scene::init()
{
	initShaders();
	bombs.clear();
	map = TileMap::createTileMap("levels/level01.txt", glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
	player->setTileMap(map);
	player->setBombs(&bombs);
	projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);
	currentTime = 0.0f;
}

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;

	switch (gameState) {
		case MAIN_MENU:
			if(isKeyJustPressed(GLFW_KEY_1)) {
				gameState = PLAYING;
			}
			else if(isKeyJustPressed(GLFW_KEY_2)) {
				gameState = INSTRUCTIONS;
			}
			else if(isKeyJustPressed(GLFW_KEY_3)) {
				gameState = CREDITS;
			}
		break;

		case INSTRUCTIONS:
			if(isKeyJustPressed(GLFW_KEY_ENTER) || isKeyJustPressed(GLFW_KEY_ESCAPE)) {
				gameState = MAIN_MENU;
			}
		break;

		case CREDITS:
			if(isKeyJustPressed(GLFW_KEY_ENTER) || isKeyJustPressed(GLFW_KEY_ESCAPE)) {
				gameState = MAIN_MENU;
			}
		break;

		case PLAYING:
			if(isKeyJustPressed(GLFW_KEY_ESCAPE)) {
				gameState = PAUSED;
			}
			else {
				if(isKeyJustPressed(GLFW_KEY_SPACE) || isKeyJustPressed(GLFW_KEY_X)) {
					placeBomb();
				}
				for(size_t i = 0; i < bombs.size(); ++i) {
					if(bombs[i] != NULL) {
						bombs[i]->update(deltaTime);
						if(!bombs[i]->isActive()) {
							delete bombs[i];
							bombs[i] = NULL;
						}
					}
				}
				player->update(deltaTime);
			}
		break;

		case PAUSED:
			if(isKeyJustPressed(GLFW_KEY_ESCAPE)) {
				gameState = PLAYING;
			}
		break;

		case GAME_OVER:
			if(isKeyJustPressed(GLFW_KEY_ENTER)) {
				gameState = MAIN_MENU;
			}
		break;

		case WIN:
			if(isKeyJustPressed(GLFW_KEY_ENTER)) {
				gameState = MAIN_MENU;
			}
		break;
	}
}

void Scene::render()
{
	glm::mat4 modelview;

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	modelview = glm::mat4(1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	
	switch (gameState) {
        case MAIN_MENU:
            renderMainMenu(); 
            break;
        case INSTRUCTIONS:
            renderInstructions(); 
            break;
        case CREDITS:
            renderCredits(); 
            break;
        case PLAYING:
            map->render();
            renderBombs();
            player->render();
            //renderEnemies();
            //renderHUD();
            break;
        case PAUSED:
            map->render();
            player->render();
            renderPauseOverlay();
            break;
        case GAME_OVER:
            renderGameOver();
            break;
        case WIN:
            renderWin();
            break;
    }
}


void Scene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();
	if(!texProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << texProgram.log() << endl << endl;
	}
	texProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}

void Scene::renderMainMenu() {
	if(menuSprite != NULL)
		menuSprite->render();
}

void Scene::renderInstructions() {
	if(instructionsSprite != NULL)
		instructionsSprite->render();
}

void Scene::renderCredits() {
	if(creditsSprite != NULL)
		creditsSprite->render();
}

void Scene::renderPauseOverlay() {
	if(pauseSprite != NULL)
		pauseSprite->render();
}

void Scene::renderGameOver() {
	if(gameOverSprite != NULL)
		gameOverSprite->render();
}

void Scene::renderWin() {
	if(winSprite != NULL)
		winSprite->render();
}

bool Scene::isKeyJustPressed(int key)
//aquesta funció la utilitzarem per a que detecti que ha sigut presionada un unic cop fins que la deixes de presionar
{
	bool pressed = Game::instance().getKey(key);
	if(pressed && !keyLastState[key]) {
		keyLastState[key] = true;
		return true;
	}
	if(!pressed) {
		keyLastState[key] = false;
	}
	return false;
}

void Scene::renderEnemies() {}
void Scene::renderBombs() {
	for(size_t i = 0; i < bombs.size(); ++i) {
		if(bombs[i] != NULL)
			bombs[i]->render();
	}
}
void Scene::renderHUD() {}

void Scene::placeBomb()
{
	if(player == NULL || map == NULL)
		return;

	glm::ivec2 playerPos = player->getPosition();
	for(size_t i = 0; i < bombs.size(); ++i) {
		if(bombs[i] != NULL && bombs[i]->isActive()) {
			if(abs(bombs[i]->getPosition().x - float(playerPos.x)) < 20.f && abs(bombs[i]->getPosition().y - float(playerPos.y)) < 20.f)
				return;
		}
	}

	Bomb *bomb = new Bomb();
	bomb->init(texProgram);
	bomb->setPosition(glm::vec2(float(playerPos.x), float(playerPos.y)));
	bombs.push_back(bomb);
}
