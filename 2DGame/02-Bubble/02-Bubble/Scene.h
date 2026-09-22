#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"


enum GameState {
	MAIN_MENU,
	INSTRUCTIONS,
	CREDITS,
	PLAYING,
	PAUSED,
	GAME_OVER,
	WIN
};

// Scene contains all the entities of our game.
// It is responsible for updating and render them.


class Scene
{

public:
	Scene();
	~Scene();

	void init();
	void update(int deltaTime);
	void render();

	GameState getGameState() const { return gameState; }
	void setGameState(GameState state) { gameState = state; }

private:
	void initShaders();
	bool isKeyJustPressed(int key);

	void renderMainMenu();
	void renderInstructions();
	void renderCredits();
	void renderPauseOverlay();
	void renderGameOver();
	void renderWin();
	void renderEnemies();
	void renderBombs();
	void renderHUD();

private:
	TileMap *map;
	Player *player;
	ShaderProgram texProgram;
	float currentTime;
	glm::mat4 projection;
	GameState gameState;
	bool keyLastState[GLFW_KEY_LAST + 1];

	Texture menuTex;
	Sprite *menuSprite;

	Texture instructionsTex;
	Sprite *instructionsSprite;

	Texture creditsTex;
	Sprite *creditsSprite;

	Texture pauseTex;
	Sprite *pauseSprite;

	Texture gameOverTex;
	Sprite *gameOverSprite;

	Texture winTex;
	Sprite *winSprite;
};


#endif // _SCENE_INCLUDE

