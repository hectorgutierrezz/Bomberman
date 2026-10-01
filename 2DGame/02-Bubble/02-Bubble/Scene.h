#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE

#include "Bomb.h"
#include "Explosion.h"
#include "Player.h"
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Sprite.h"
#include "Texture.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

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

class Scene {

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
  void renderExplosions();
  void renderHUD();
  void placeBomb();
  void showGodModeMessage(bool enabled);

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

  std::vector<Bomb *> bombs;
  std::vector<Explosion *> explosions;

  // HUD: icones de vides (sprites del personatge)
  Texture hudTex;
  Sprite *lifeIcon;

  // Missatge temporal de god mode
  Texture godModeOnTex;
  Texture godModeOffTex;
  Sprite *godModeOnSprite;
  Sprite *godModeOffSprite;
  int godModeMsgTime;
  bool godModeMsgActive;

  // Càmera dinàmica centrada al jugador
  float camX, camY;        // posició (cantonada superior-esquerra) de la càmera en world coords
  float viewWidth, viewHeight; // quantes unitats del món es veuen (la «finestra de zoom»)
};

#endif // _SCENE_INCLUDE
