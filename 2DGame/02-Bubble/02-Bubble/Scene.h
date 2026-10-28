#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE

#include "Bomb.h"
#include "Door.h"
#include "Enemy.h"
#include "Explosion.h"
#include "Player.h"
#include "PowerUp.h"
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
  CHEATS,
  CREDITS,
  PLAYING,
  PAUSED,
  GAME_OVER,
  WIN,
  TRY_AGAIN,
  STAGE_INTRO
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

  // Power-ups API (accessible pel sistema de blocs destruïbles)
  PowerUp *spawnPowerUp(PowerUpType type, const glm::vec2 &pos);
  PowerUp *tryDropPowerUp(const glm::vec2 &pos, float probability = 0.40f);
  void clearPowerUps();

private:
  void breakDestructibleTiles(Explosion *explosion);
  void initShaders();
  bool isKeyJustPressed(int key);

  void initMenuScreens();
  void freeMenuScreens();

  void renderMainMenu();
  void renderInstructions();
  void renderCheats();
  void renderCredits();
  void renderPauseOverlay();
  void renderGameOver();
  void renderWin();
  void renderTryAgain();
  void initTryAgainScreen();
  void freeTryAgainScreen();
  void renderStageIntro();
  void initStageIntroScreen();
  void freeStageIntroScreen();
  void renderEnemies();
  void renderBombs();
  void renderExplosions();
  void renderPowerUps();
  void renderHUD();
  void placeBomb();
  void showGodModeMessage(bool enabled);
  void showDoorOpenMessage();
  void initLevelBackground(int level);
  void freeLevelBackground();
  void startLevel(int levelNum = 1, bool keepPowerUps = true);
  void nextLevel();
  void onPlayerDeath();
  void initHUDTextures();
  void freeHUD();
  void updateTimerHUD();
  void updateLevelHUD();
  void updateEnemiesHUD();
  void updateStageHUD();
  void updateRadiusHUD();
  void updateBombsHUD();
  void updateLivesHUD();
  void updateHeartsHUD();
  void initHeartTexture();
  void clearEnemies();
  void spawnEnemies();
  void updateEnemies(int deltaTime);
  int countAliveEnemies() const;
  void updateDoorFromEnemies();
  void updatePowerUps(int deltaTime);

  TileMap *map;
  Player *player;
  Door *door;
  std::vector<Enemy *> enemies;
  int currentLevel;
  int externalLives; // Contador extern (re-runs fins al Game Over)

  // Fons del nivell (Backgrounds tematics per a cada mon)
  Texture bgTex;
  Sprite *bgSprite;

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
  TextLabel menuLabels[5];
  TextLabel menuFooterLabel;

  // Instruccions
  std::vector<TextLabel*> instructionsLabels;

  // Trucs
  std::vector<TextLabel*> cheatsLabels;

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

  // Pantalla intermèdia en perdre una vida (Try Again)
  int tryAgainTimer;
  Texture tryAgainBomberTex;
  Sprite *tryAgainBomberSprite;
  TextLabel tryAgainStageLabel;
  TextLabel tryAgainTitleLabel;
  TextLabel tryAgainLivesLabel;
  TextLabel tryAgainChanceLabel;
  TextLabel tryAgainEncourageLabel;
  TextLabel tryAgainPromptLabel;
  TextLabel tryAgainMenuLabel;

  // Mini pantalla d'inici de nivell (Stage Intro)
  int stageIntroTimer;
  Texture stageIntroArtTex;
  Sprite *stageIntroArtSprite;
  Texture stageIntroBomberTex;
  Sprite *stageIntroBomberSprite;
  TextLabel stageIntroTitleLabel;
  TextLabel stageIntroReadyLabel;
  TextLabel stageIntroLivesLabel;
  TextLabel stageIntroPromptLabel;

  std::vector<Bomb *> bombs;
  std::vector<Explosion *> explosions;
  std::vector<PowerUp *> powerUps;
  int powerUpCycle; // Rota Bomb/Fire/Heart/Skates en sortir dels blocs D

  // HUD: Barra inferior estil Game Boy Color (Pocket Bomberman)
  Texture hudBarTex;
  Sprite *hudBarSprite;

  Texture clockIconTex;
  Sprite *clockIconSprite;

  Texture doorIconTex;
  Sprite *doorIconSprite;

  Texture flameIconTex;
  Sprite *flameIconSprite;

  Texture bombIconTex;
  Sprite *bombIconSprite;

  Texture hudTex;
  Sprite *lifeIcon;

  Texture heartTex;
  Sprite *heartIcon;

  int levelTimeLeft;
  Texture timerTex;
  Sprite *timerSprite;
  int lastDisplayedSeconds;

  Texture enemiesHUDTex;
  Sprite *enemiesHUDSprite;
  int lastDisplayedEnemies;

  Texture stageHUDTex;
  Sprite *stageHUDSprite;
  int lastDisplayedStage;

  Texture flameHUDTex;
  Sprite *flameHUDSprite;
  int lastDisplayedRadius;

  Texture bombsHUDTex;
  Sprite *bombsHUDSprite;
  int lastDisplayedMaxBombs;

  Texture livesHUDTex;
  Sprite *livesHUDSprite;
  int lastDisplayedLives;

  Texture heartsHUDTex;
  Sprite *heartsHUDSprite;
  int lastDisplayedHearts;

  // Missatge temporal de god mode
  Texture godModeOnTex;
  Texture godModeOffTex;
  Sprite *godModeOnSprite;
  Sprite *godModeOffSprite;
  int godModeMsgTime;
  bool godModeMsgActive;

  // Missatge temporal d'avís de porta oberta
  Texture doorOpenTex;
  Sprite *doorOpenSprite;
  int doorOpenMsgTime;

  // Càmera dinàmica centrada al jugador
  float camX, camY;        // posició (cantonada superior-esquerra) de la càmera en world coords
  float viewWidth, viewHeight; // quantes unitats del món es veuen (la «finestra de zoom»)
};

#endif // _SCENE_INCLUDE
