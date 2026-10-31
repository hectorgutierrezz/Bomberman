#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Game.h"
#include "SoundManager.h"


Game::~Game()
{
	SoundManager::instance().free();
}

void Game::init()
{
	bPlay = true;
	bMouseClick = false;
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	SoundManager::instance().init();
	scene.init();
}

bool Game::update(int deltaTime)
{
	scene.update(deltaTime);

	return bPlay;
}

void Game::render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	scene.render();
}

void Game::keyPressed(int key)
{
	keys[key] = true;
}

void Game::keyReleased(int key)
{
	keys[key] = false;
}

void Game::mouseMove(int x, int y)
{
	(void)x;
	(void)y;
}

void Game::mousePress(int button)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		bMouseClick = true;
}

void Game::mouseRelease(int button)
{
	(void)button;
}

bool Game::getKey(int key) const
{
	return keys[key];
}



