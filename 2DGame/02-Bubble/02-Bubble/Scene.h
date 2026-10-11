#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE

#include "Bomb.h"
#include "Door.h"
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

struct TextLabel {
  Texture tex;
  Sprite *sprite;
  glm::vec2 pos;
  glm::vec4 color;

  TextLabel() : sprite(NULL), pos(0.f), color(1.f) {}
  ~TextLabel();
  void init(const std::string &text, int scale, const glm::vec2 &p, const glm::vec4 &col, ShaderProgram *program);
  void render(ShaderProgram &program) const;
  void renderWithColor(ShaderProgram &program, const glm::vec4 &c) const;
  void free();
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

  void initMenuScreens();
  void freeMenuScreens();

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
  void startLevel(int levelNum = 1);
  void nextLevel();
  void updateTimerHUD();
  void updateLevelHUD();

  TileMap *map;
  Player *player;
  Door *door;
  int currentLevel;

  ShaderProgram texProgram;
  float currentTime;
  glm::mat4 projection;
  GameState gameState;
  bool keyLastState[GLFW_KEY_LAST + 1];

  // Menú Principal
  Texture logoTex;
  Sprite *logoSprite;
  Texture copyrightTex;
  Sprite *copyrightSprite;
  Texture hudsonTex;
  Sprite *hudsonSprite;
  Texture cursorTex;
  Sprite *cursorSprite;
  int menuOption;
  TextLabel menuLabels[4];
  TextLabel menuFooterLabel;

  // Instruccions
  std::vector<TextLabel*> instructionsLabels;

  // Crèdits
  std::vector<TextLabel*> creditsLabels;

  // Pausa
  Texture pauseTex;
  Sprite *pauseSprite;
  TextLabel pauseTitleLabel;
  TextLabel pauseResumeLabel;
  TextLabel pauseMenuLabel;

  // Game Over / Time Over
  Texture gameOverTex;
  Sprite *gameOverSprite;
  Texture timeOverTex;
  Sprite *timeOverSprite;
  bool bTimeOver;
  TextLabel gameOverRetryLabel;
  TextLabel gameOverMenuLabel;
  TextLabel timeOverMsgLabel;

  // Victòria (Win)
  Texture worldClearTex;
  Sprite *worldClearSprite;
  Texture celebrateTex;
  Sprite *celebrateSprite;
  TextLabel winCongratsLabel;
  TextLabel winSubLabel;
  TextLabel winPeaceLabel;
  TextLabel winPromptLabel;

  std::vector<Bomb *> bombs;
  std::vector<Explosion *> explosions;

  // HUD: icones de vides (sprites del personatge)
  Texture hudTex;
  Sprite *lifeIcon;

  // HUD: comptador enrere de temps
  int levelTimeLeft;
  Texture timerTex;
  Sprite *timerSprite;
  int lastDisplayedSeconds;

  // HUD: indicador de nivell
  Texture levelHUDTex;
  Sprite *levelHUDSprite;
  int lastDisplayedLevel;

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

