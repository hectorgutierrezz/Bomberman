#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Game.h"
#include "SoundManager.h"

Game::Game()
	: bPlay(false), bMouseClick(false)
{
	for (int i = 0; i <= GLFW_KEY_LAST; ++i)
		keys[i] = false;

	controlKeys[BIND_LEFT_ARROW] = GLFW_KEY_LEFT;
	controlKeys[BIND_LEFT_A] = GLFW_KEY_A;
	controlKeys[BIND_RIGHT_ARROW] = GLFW_KEY_RIGHT;
	controlKeys[BIND_RIGHT_D] = GLFW_KEY_D;
	controlKeys[BIND_JUMP_ARROW] = GLFW_KEY_UP;
	controlKeys[BIND_JUMP_W] = GLFW_KEY_W;
	controlKeys[BIND_BOMB_SPACE] = GLFW_KEY_SPACE;
	controlKeys[BIND_BOMB_X] = GLFW_KEY_X;
	controlKeys[BIND_PAUSE] = GLFW_KEY_P;
	controlKeys[BIND_DOWN_ARROW] = GLFW_KEY_DOWN;
	controlKeys[BIND_DOWN_S] = GLFW_KEY_S;

	cheatKeys[CHEAT_GOD_MODE] = GLFW_KEY_G;
	cheatKeys[CHEAT_KILL_ENEMIES] = GLFW_KEY_K;
	cheatKeys[CHEAT_ADD_HEART] = GLFW_KEY_H;
	cheatKeys[CHEAT_ADD_BOMB] = GLFW_KEY_B;
	cheatKeys[CHEAT_ADD_RADIUS] = GLFW_KEY_F;
	cheatKeys[CHEAT_ADD_SPEED] = GLFW_KEY_V;
	cheatKeys[CHEAT_NEXT_LEVEL] = GLFW_KEY_N;
	cheatKeys[CHEAT_LEVEL_1] = GLFW_KEY_1;
	cheatKeys[CHEAT_LEVEL_2] = GLFW_KEY_2;
	cheatKeys[CHEAT_LEVEL_3] = GLFW_KEY_3;
	cheatKeys[CHEAT_LEVEL_4] = GLFW_KEY_4;
	cheatKeys[CHEAT_LEVEL_5] = GLFW_KEY_5;
}

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
	if (key < 0 || key > GLFW_KEY_LAST)
		return;
	keys[key] = true;
}

void Game::keyReleased(int key)
{
	if (key < 0 || key > GLFW_KEY_LAST)
		return;
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
	if (key < 0 || key > GLFW_KEY_LAST)
		return false;
	return keys[key];
}

bool Game::getActionPressed(GameAction action) const
{
	switch (action) {
	case ACTION_MOVE_LEFT:
		return getKey(controlKeys[BIND_LEFT_ARROW]) || getKey(controlKeys[BIND_LEFT_A]);
	case ACTION_MOVE_RIGHT:
		return getKey(controlKeys[BIND_RIGHT_ARROW]) || getKey(controlKeys[BIND_RIGHT_D]);
	case ACTION_JUMP:
		return getKey(controlKeys[BIND_JUMP_ARROW]) || getKey(controlKeys[BIND_JUMP_W]);
	case ACTION_BOMB:
		return getKey(controlKeys[BIND_BOMB_SPACE]) || getKey(controlKeys[BIND_BOMB_X]);
	case ACTION_PAUSE:
		return getKey(controlKeys[BIND_PAUSE]);
	case ACTION_DROP_THROUGH:
		return getKey(controlKeys[BIND_DOWN_ARROW]) || getKey(controlKeys[BIND_DOWN_S]);
	}
	return false;
}

int Game::getControlKey(ControlBinding binding) const
{
	if (binding < 0 || binding >= CONTROL_BINDING_COUNT)
		return GLFW_KEY_UNKNOWN;
	return controlKeys[binding];
}

bool Game::setControlKey(ControlBinding binding, int key)
{
	if (binding < 0 || binding >= CONTROL_BINDING_COUNT)
		return false;
	if (key == GLFW_KEY_UNKNOWN) {
		controlKeys[binding] = key;
		return true;
	}
	if (key < 0 || key > GLFW_KEY_LAST ||
	    key == GLFW_KEY_ESCAPE || key == GLFW_KEY_ENTER ||
	    (key >= GLFW_KEY_1 && key <= GLFW_KEY_6))
		return false;

	for (int i = 0; i < CONTROL_BINDING_COUNT; ++i) {
		if (i != binding && controlKeys[i] == key)
			return false;
	}
	for (int i = 0; i < GAME_CHEAT_COUNT; ++i) {
		if (cheatKeys[i] == key)
			return false;
	}

	controlKeys[binding] = key;
	return true;
}

int Game::getCheatKey(GameCheat cheat) const
{
	if (cheat < 0 || cheat >= GAME_CHEAT_COUNT)
		return GLFW_KEY_UNKNOWN;
	return cheatKeys[cheat];
}

bool Game::setCheatKey(GameCheat cheat, int key)
{
	if (cheat < 0 || cheat >= GAME_CHEAT_COUNT)
		return false;
	if (key == GLFW_KEY_UNKNOWN) {
		cheatKeys[cheat] = key;
		return true;
	}
	if (key < 0 || key > GLFW_KEY_LAST ||
	    key == GLFW_KEY_ESCAPE || key == GLFW_KEY_ENTER ||
	    (key >= GLFW_KEY_1 && key <= GLFW_KEY_6))
		return false;

	for (int i = 0; i < CONTROL_BINDING_COUNT; ++i) {
		if (controlKeys[i] == key)
			return false;
	}
	for (int i = 0; i < GAME_CHEAT_COUNT; ++i) {
		if (i != cheat && cheatKeys[i] == key)
			return false;
	}

	cheatKeys[cheat] = key;
	return true;
}
