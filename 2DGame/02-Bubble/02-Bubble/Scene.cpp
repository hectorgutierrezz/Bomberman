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
}


void Scene::init()
{
	initShaders();
	gameState = MAIN_MENU;
	for (int i = 0; i <= GLFW_KEY_LAST; ++i)
		keyLastState[i] = false;
	map = TileMap::createTileMap("levels/level01.txt", glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
	player->setTileMap(map);
	projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);
	currentTime = 0.0f;
}

bool Scene::isKeyJustPressed(int key)
{
	if (key < 0 || key > GLFW_KEY_LAST)
		return false;
	bool isDown = Game::instance().getKey(key);
	bool justPressed = isDown && !keyLastState[key];
	keyLastState[key] = isDown;
	return justPressed;
}

void Scene::update(int deltaTime)
{
	switch (gameState) {
		case MAIN_MENU:
			if (isKeyJustPressed(GLFW_KEY_ENTER) || isKeyJustPressed(GLFW_KEY_SPACE)) {
				gameState = PLAYING;
			}
			else if (isKeyJustPressed(GLFW_KEY_I)) {
				gameState = INSTRUCTIONS;
			}
			else if (isKeyJustPressed(GLFW_KEY_C)) {
				gameState = CREDITS;
			}
			break;

		case INSTRUCTIONS:
			if (isKeyJustPressed(GLFW_KEY_ENTER) || isKeyJustPressed(GLFW_KEY_ESCAPE)) {
				gameState = MAIN_MENU;
			}
			break;

		case CREDITS:
			if (isKeyJustPressed(GLFW_KEY_ENTER) || isKeyJustPressed(GLFW_KEY_ESCAPE)) {
				gameState = MAIN_MENU;
			}
			break;

		case PLAYING:
			if (isKeyJustPressed(GLFW_KEY_P)) {
				gameState = PAUSED;
			}
			else {
				currentTime += deltaTime;
				player->update(deltaTime);
			}
			break;

		case PAUSED:
			if (isKeyJustPressed(GLFW_KEY_P) || isKeyJustPressed(GLFW_KEY_ENTER)) {
				gameState = PLAYING;
			}
			else if (isKeyJustPressed(GLFW_KEY_M) || isKeyJustPressed(GLFW_KEY_ESCAPE)) {
				gameState = MAIN_MENU;
			}
			break;

		case GAME_OVER:
			if (isKeyJustPressed(GLFW_KEY_ENTER) || isKeyJustPressed(GLFW_KEY_ESCAPE)) {
				gameState = MAIN_MENU;
			}
			break;

		case WIN:
			if (isKeyJustPressed(GLFW_KEY_ENTER) || isKeyJustPressed(GLFW_KEY_ESCAPE)) {
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

	if (gameState == PLAYING || gameState == PAUSED) {
		map->render();
		player->render();
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



