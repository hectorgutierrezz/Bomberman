#include "Scene.h"
#include "BitmapText.h"
#include "Game.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 4
// Y se calcula en startLevel según la altura del mapa (2 tiles por encima del
// suelo)

// Mida de la finestra de càmera en world pixels (zoom ~3.1x uniforme respecte 640x480)
#define CAM_VIEW_W 208.f
#define CAM_VIEW_H 156.f

#define GOD_MODE_MSG_DURATION 2000

TextLabel::~TextLabel() { free(); }

void TextLabel::init(const std::string &text, int scale, const glm::vec2 &p,
                     const glm::vec4 &col, ShaderProgram *program) {
  free();
  pos = p;
  color = col;
  if (BitmapText::createTexture(tex, text, scale)) {
    sprite =
        Sprite::createSprite(glm::vec2(float(tex.width()), float(tex.height())),
                             glm::vec2(1.f, 1.f), &tex, program);
    sprite->setPosition(pos);
  }
}

void TextLabel::render(ShaderProgram &program) const {
  if (sprite != NULL) {
    program.setUniform4f("color", color.r, color.g, color.b, color.a);
    sprite->render();
    program.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
  }
}

void TextLabel::renderWithColor(ShaderProgram &program,
                                const glm::vec4 &c) const {
  if (sprite != NULL) {
    program.setUniform4f("color", c.r, c.g, c.b, c.a);
    sprite->render();
    program.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
  }
}

void TextLabel::free() {
  if (sprite != NULL) {
    delete sprite;
    sprite = NULL;
  }
  tex.free();
}

Scene::Scene() {
  map = NULL;
  player = NULL;
  door = NULL;
  currentLevel = 1;
  logoSprite = NULL;
  copyrightSprite = NULL;
  hudsonSprite = NULL;
  cursorSprite = NULL;
  pauseSprite = NULL;
  gameOverSprite = NULL;
  timeOverSprite = NULL;
  worldClearSprite = NULL;
  celebrateSprite = NULL;
  menuOption = 0;
  bTimeOver = false;
  lifeIcon = NULL;
  heartIcon = NULL;
  timerSprite = NULL;
  levelHUDSprite = NULL;
  enemiesHUDSprite = NULL;
  lastDisplayedEnemies = -1;
  godModeOnSprite = NULL;
  godModeOffSprite = NULL;
  godModeMsgTime = 0;
  godModeMsgActive = false;
  doorOpenSprite = NULL;
  doorOpenMsgTime = 0;
  levelTimeLeft = 120000;
  lastDisplayedSeconds = -1;
  lastDisplayedLevel = -1;
  externalLives = 3;
  tryAgainTimer = 0;
  tryAgainBomberSprite = NULL;
  stageIntroTimer = 0;
  stageIntroBomberSprite = NULL;
  gameState = MAIN_MENU;
  for (int i = 0; i <= GLFW_KEY_LAST; ++i)
    keyLastState[i] = false;
}

Scene::~Scene() {
  texProgram.free();
  if (map != NULL)
    delete map;
  if (player != NULL)
    delete player;
  if (door != NULL)
    delete door;
  freeMenuScreens();
  freeTryAgainScreen();
  freeStageIntroScreen();
  if (lifeIcon != NULL)
    delete lifeIcon;
  if (heartIcon != NULL)
    delete heartIcon;
  if (timerSprite != NULL)
    delete timerSprite;
  if (levelHUDSprite != NULL)
    delete levelHUDSprite;
  if (enemiesHUDSprite != NULL) {
    delete enemiesHUDSprite;
    enemiesHUDSprite = NULL;
  }
  if (godModeOnSprite != NULL)
    delete godModeOnSprite;
  if (godModeOffSprite != NULL)
    delete godModeOffSprite;
  if (doorOpenSprite != NULL)
    delete doorOpenSprite;
  for (size_t i = 0; i < bombs.size(); ++i) {
    if (bombs[i] != NULL)
      delete bombs[i];
  }
  bombs.clear();
  for (size_t i = 0; i < explosions.size(); ++i) {
    if (explosions[i] != NULL)
      delete explosions[i];
  }
  explosions.clear();
  clearEnemies();
  clearPowerUps();
}

void Scene::clearEnemies() {
  for (size_t i = 0; i < enemies.size(); ++i) {
    if (enemies[i] != NULL)
      delete enemies[i];
  }
  enemies.clear();
}

void Scene::clearPowerUps() {
  for (size_t i = 0; i < powerUps.size(); ++i) {
    if (powerUps[i] != NULL)
      delete powerUps[i];
  }
  powerUps.clear();
}

PowerUp *Scene::spawnPowerUp(PowerUpType type, const glm::vec2 &pos) {
  PowerUp *p = new PowerUp(type, pos);
  p->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
  powerUps.push_back(p);
  return p;
}

PowerUp *Scene::tryDropPowerUp(const glm::vec2 &pos, float probability) {
  float roll = float(rand()) / float(RAND_MAX);
  if (roll >= probability)
    return NULL;
  int pick = rand() % 4;
  return spawnPowerUp(PowerUpType(pick), pos);
}

void Scene::updatePowerUps(int deltaTime) {
  for (size_t i = 0; i < powerUps.size(); ++i) {
    if (powerUps[i] != NULL && powerUps[i]->isActive()) {
      powerUps[i]->update(deltaTime);
      if (player != NULL && !player->isDying() && !player->isDead()) {
        if (powerUps[i]->checkCollision(player->getPosition(), player->getSize())) {
          powerUps[i]->applyEffect(player);
        }
      }
    }
  }

  for (auto it = powerUps.begin(); it != powerUps.end();) {
    if (*it != NULL && !(*it)->isActive()) {
      delete *it;
      it = powerUps.erase(it);
    } else {
      ++it;
    }
  }
}

void Scene::renderPowerUps() {
  for (size_t i = 0; i < powerUps.size(); ++i) {
    if (powerUps[i] != NULL && powerUps[i]->isActive())
      powerUps[i]->render();
  }
}

void Scene::spawnEnemies() {
  clearEnemies();
  if (map == NULL)
    return;

  int tileSize = map->getTileSize();
  int mapTilesY = map->getMapHeight() / tileSize;
  int groundY = mapTilesY - 3;
  if (groundY < 1)
    groundY = 1;

  // Posicions base segons el disseny de plataformes de cada nivell
  struct SpawnSpec {
    EnemyType type;
    int tileX;
    int tileY;
  };

  std::vector<SpawnSpec> specs;
  switch (currentLevel) {
  case 1:
    // Nivell 1: 2 enemics terrestres (nivell introductori, sense voladors ni explosius)
    specs.push_back({ENEMY_HORIZONTAL, 11, 25});  // Patrulla sobre el terra
    specs.push_back({ENEMY_HORIZONTAL, 22, 17});  // Patrulla plataforma fila 18
    break;
  case 2:
    // Nivell 2: 3 enemics (1 caminant i 2 voladors en corredors verticals)
    specs.push_back({ENEMY_HORIZONTAL, 15, 21});  // Patrulla plataforma fila 22
    specs.push_back({ENEMY_VERTICAL, 5, 14});      // Volador columna esquerra
    specs.push_back({ENEMY_VERTICAL, 29, 14});     // Volador columna dreta
    break;
  case 3:
    // Nivell 3: 4 enemics (2 caminants, 1 volador i 1 globus estàtic)
    specs.push_back({ENEMY_HORIZONTAL, 8, 25});   // Patrulla sobre el terra
    specs.push_back({ENEMY_HORIZONTAL, 23, 21});  // Patrulla plataforma fila 22
    specs.push_back({ENEMY_VERTICAL, 28, 14});    // Volador eix dret
    specs.push_back({ENEMY_STATIC, 17, 11});      // Guardià estàtic al santuari superior
    break;
  case 4:
    // Nivell 4: 4 enemics (voladors agressius i enemics terrestres)
    specs.push_back({ENEMY_HORIZONTAL, 16, 21});  // Patrulla plataforma central
    specs.push_back({ENEMY_VERTICAL, 6, 14});     // Volador eix esquerre
    specs.push_back({ENEMY_VERTICAL, 29, 14});    // Volador eix dret
    specs.push_back({ENEMY_STATIC, 24, 25});      // Guardià sobre el terra
    break;
  default: // nivell 5
    // Nivell 5: 5 enemics (màxim desafiament a la fortalesa final)
    specs.push_back({ENEMY_HORIZONTAL, 6, 25});   // Caminant al terra
    specs.push_back({ENEMY_HORIZONTAL, 22, 21});  // Caminant plataforma fila 22
    specs.push_back({ENEMY_VERTICAL, 8, 12});     // Volador torre esquerra
    specs.push_back({ENEMY_VERTICAL, 28, 12});    // Volador torre dreta
    specs.push_back({ENEMY_STATIC, 17, 6});       // Guardià estàtic davant la porta final a dalt
    break;
  }

  for (size_t i = 0; i < specs.size(); ++i) {
    Enemy *e = new Enemy();
    e->init(specs[i].type, glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
    e->setTileMap(map);
    e->setBombs(&bombs);
    e->setPosition(glm::vec2(specs[i].tileX * tileSize, specs[i].tileY * tileSize));
    enemies.push_back(e);
  }
}

int Scene::countAliveEnemies() const {
  int n = 0;
  for (size_t i = 0; i < enemies.size(); ++i) {
    if (enemies[i] != NULL && enemies[i]->isAlive())
      ++n;
  }
  return n;
}

void Scene::updateDoorFromEnemies() {
  if (door == NULL)
    return;
  bool wasOpen = door->isOpen();
  bool nowOpen = (countAliveEnemies() == 0);
  door->setOpen(nowOpen);
  if (!wasOpen && nowOpen) {
    showDoorOpenMessage();
  }
}

void Scene::updateEnemies(int deltaTime) {
  glm::ivec2 pPos = (player != NULL) ? player->getPosition() : glm::ivec2(0);
  for (size_t i = 0; i < enemies.size(); ++i) {
    if (enemies[i] != NULL)
      enemies[i]->update(deltaTime, pPos);
  }

  // L'enemic estàtic detona en morir, amb el mateix abast que una bomba.
  for (size_t i = 0; i < enemies.size(); ++i) {
    if (enemies[i] != NULL && enemies[i]->shouldSpawnExplosion()) {
      Explosion *exp = new Explosion();
      exp->init(glm::vec2(enemies[i]->getPosition()),
                glm::ivec2(SCREEN_X, SCREEN_Y), 1, map, texProgram);
      explosions.push_back(exp);
      enemies[i]->clearExplosionTrigger();
    }
  }

  // Eliminar enemics que ja han acabat l'animació de mort
  for (auto it = enemies.begin(); it != enemies.end();) {
    if (*it != NULL && (*it)->isFinished()) {
      delete *it;
      it = enemies.erase(it);
    } else {
      ++it;
    }
  }

  updateDoorFromEnemies();
}

void Scene::startLevel(int levelNum, bool keepPowerUps) {
  clearEnemies();
  currentLevel = levelNum;
  if (currentLevel < 1)
    currentLevel = 1;
  if (currentLevel > 5)
    currentLevel = 5;

  levelTimeLeft = 120000; // 2 minuts (120 segons) per nivell
  lastDisplayedSeconds = -1;
  lastDisplayedLevel = -1;
  lastDisplayedEnemies = -1;

  if (timerSprite != NULL) {
    delete timerSprite;
    timerSprite = NULL;
  }
  if (levelHUDSprite != NULL) {
    delete levelHUDSprite;
    levelHUDSprite = NULL;
  }
  if (enemiesHUDSprite != NULL) {
    delete enemiesHUDSprite;
    enemiesHUDSprite = NULL;
  }
  lastDisplayedEnemies = -1;

  char levelPath[64];
  snprintf(levelPath, sizeof(levelPath), "levels/level%02d.txt", currentLevel);

  if (map != NULL) {
    delete map;
    map = NULL;
  }
  map = TileMap::createTileMap(levelPath, glm::vec2(SCREEN_X, SCREEN_Y),
                               texProgram);

  // Spawn i porta alineats exactament sobre el terra (fila 26 de tiles)
  int tileSize = map->getTileSize();
  int mapTilesY = map->getMapHeight() / tileSize;
  int spawnTileY = mapTilesY - 3; // 28 - 3 = 25: entitat de 16px descansa sobre la fila 26 (25*16 + 16 = 416)
  if (spawnTileY < 1)
    spawnTileY = 1;

  // Configuració de la porta segons el nivell (Nivells 1, 3 i 5 a dalt; 2 i 4 a baix)
  int doorTileX = 30;
  int doorTileY = 24; // Per defecte al terra (fila 24 per a porta de 32px: 24*16 + 32 = 416)
  if (currentLevel == 1 || currentLevel == 3 || currentLevel == 5) {
    doorTileX = 17;
    doorTileY = 5; // Sobre la plataforma superior de la fila 7 (5*16 + 32 = 112 = 7*16)
  }

  glm::vec2 spawnPos(INIT_PLAYER_X_TILES * tileSize, spawnTileY * tileSize);

  if (player == NULL) {
    player = new Player();
    player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
    player->setPosition(spawnPos);
  } else {
    if (!keepPowerUps)
      player->resetPowerUps();
    player->respawn(spawnPos);
  }
  player->setTileMap(map);
  player->setBombs(&bombs);

  // Inicialitzar / configurar la porta del nivell
  if (door == NULL) {
    door = new Door();
    door->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
  }
  door->setPosition(glm::vec2(doorTileX * tileSize, doorTileY * tileSize));
  door->setOpen(false);
  doorOpenMsgTime = 0;

  for (size_t i = 0; i < bombs.size(); ++i) {
    if (bombs[i] != NULL)
      delete bombs[i];
  }
  bombs.clear();
  for (size_t i = 0; i < explosions.size(); ++i) {
    if (explosions[i] != NULL)
      delete explosions[i];
  }
  explosions.clear();
  clearPowerUps();

  // Deixar els 4 power-ups al terra del mapa per provar-los de forma immediata
  int pY = spawnTileY * tileSize;
  spawnPowerUp(POWERUP_BOMB_UP, glm::vec2(6 * tileSize, pY));
  spawnPowerUp(POWERUP_FIRE_UP, glm::vec2(9 * tileSize, pY));
  spawnPowerUp(POWERUP_HEART,   glm::vec2(12 * tileSize, pY));
  spawnPowerUp(POWERUP_SKATES,  glm::vec2(15 * tileSize, pY));

  spawnEnemies();
  updateDoorFromEnemies();

  // Centrar la càmera al jugador al carregar el nivell
  viewWidth = CAM_VIEW_W;
  viewHeight = CAM_VIEW_H;
  glm::ivec2 initPos = player->getPosition();
  camX = float(SCREEN_X + initPos.x) + 8.f - viewWidth * 0.5f;
  camY = float(SCREEN_Y + initPos.y) + 8.f - viewHeight * 0.5f;

  float mapRight = float(SCREEN_X) + float(map->getMapWidth()) - viewWidth;
  float mapBottom = float(SCREEN_Y) + float(map->getMapHeight()) - viewHeight;
  if (camX < 0.f)
    camX = 0.f;
  if (camY < 0.f)
    camY = 0.f;
  if (camX > mapRight)
    camX = mapRight;
  if (camY > mapBottom)
    camY = mapBottom;

  projection = glm::ortho(camX, camX + viewWidth, camY + viewHeight, camY);

  initStageIntroScreen();
  stageIntroTimer = 0;
  gameState = STAGE_INTRO;
}

void Scene::nextLevel() {
  if (currentLevel < 5) {
    startLevel(currentLevel + 1, true);
  } else {
    gameState = WIN;
  }
}

void Scene::onPlayerDeath() {
  externalLives--;
  if (externalLives <= 0) {
    externalLives = 0;
    if (levelTimeLeft > 0)
      bTimeOver = false;
    // Netejar bombes, explosions i enemics actius
    for (size_t i = 0; i < bombs.size(); ++i) {
      if (bombs[i] != NULL)
        delete bombs[i];
    }
    bombs.clear();
    for (size_t i = 0; i < explosions.size(); ++i) {
      if (explosions[i] != NULL)
        delete explosions[i];
    }
    explosions.clear();
    clearEnemies();
    gameState = GAME_OVER;
  } else {
    // Netejar bombes, explosions i enemics
    for (size_t i = 0; i < bombs.size(); ++i) {
      if (bombs[i] != NULL)
        delete bombs[i];
    }
    bombs.clear();
    for (size_t i = 0; i < explosions.size(); ++i) {
      if (explosions[i] != NULL)
        delete explosions[i];
    }
    explosions.clear();
    clearEnemies();

    // Mostrar la pantalla intermèdia "TRY AGAIN" abans de reaparèixer
    initTryAgainScreen();
    tryAgainTimer = 0;
    gameState = TRY_AGAIN;
  }
}

void Scene::freeMenuScreens() {
  if (logoSprite != NULL) {
    delete logoSprite;
    logoSprite = NULL;
  }
  if (copyrightSprite != NULL) {
    delete copyrightSprite;
    copyrightSprite = NULL;
  }
  if (hudsonSprite != NULL) {
    delete hudsonSprite;
    hudsonSprite = NULL;
  }
  if (cursorSprite != NULL) {
    delete cursorSprite;
    cursorSprite = NULL;
  }
  if (pauseSprite != NULL) {
    delete pauseSprite;
    pauseSprite = NULL;
  }
  if (gameOverSprite != NULL) {
    delete gameOverSprite;
    gameOverSprite = NULL;
  }
  if (timeOverSprite != NULL) {
    delete timeOverSprite;
    timeOverSprite = NULL;
  }
  if (worldClearSprite != NULL) {
    delete worldClearSprite;
    worldClearSprite = NULL;
  }
  if (celebrateSprite != NULL) {
    delete celebrateSprite;
    celebrateSprite = NULL;
  }

  for (int i = 0; i < 5; ++i)
    menuLabels[i].free();
  menuFooterLabel.free();

  for (size_t i = 0; i < instructionsLabels.size(); ++i) {
    if (instructionsLabels[i] != NULL)
      delete instructionsLabels[i];
  }
  instructionsLabels.clear();

  for (size_t i = 0; i < cheatsLabels.size(); ++i) {
    if (cheatsLabels[i] != NULL)
      delete cheatsLabels[i];
  }
  cheatsLabels.clear();

  for (size_t i = 0; i < creditsLabels.size(); ++i) {
    if (creditsLabels[i] != NULL)
      delete creditsLabels[i];
  }
  creditsLabels.clear();

  pauseTitleLabel.free();
  pauseResumeLabel.free();
  pauseMenuLabel.free();

  gameOverMenuLabel.free();
  timeOverMsgLabel.free();

  winCongratsLabel.free();
  winSubLabel.free();
  winPeaceLabel.free();
  winPromptLabel.free();

  freeTryAgainScreen();
}

void Scene::freeTryAgainScreen() {
  if (tryAgainBomberSprite != NULL) {
    delete tryAgainBomberSprite;
    tryAgainBomberSprite = NULL;
  }
  tryAgainStageLabel.free();
  tryAgainTitleLabel.free();
  tryAgainLivesLabel.free();
  tryAgainChanceLabel.free();
  tryAgainEncourageLabel.free();
  tryAgainPromptLabel.free();
  tryAgainMenuLabel.free();
}

void Scene::initTryAgainScreen() {
  freeTryAgainScreen();

  char stageBuf[32];
  snprintf(stageBuf, sizeof(stageBuf), "NIVELL 1-%d", currentLevel);

  auto centerLabel = [&](TextLabel &lbl, const std::string &txt, int scale, float y, const glm::vec4 &col) {
    Texture t;
    if (BitmapText::createTexture(t, txt, scale)) {
      float x = (float(SCREEN_WIDTH) - float(t.width())) * 0.5f;
      lbl.init(txt, scale, glm::vec2(x, y), col, &texProgram);
    }
  };

  centerLabel(tryAgainStageLabel, stageBuf, 3, 75.f, glm::vec4(1.f, 0.9f, 0.1f, 1.f));
  centerLabel(tryAgainTitleLabel, "TORNA-HO A PROVAR!", 3, 125.f, glm::vec4(1.f, 0.35f, 0.2f, 1.f));

  // Sprite autèntic de la cara de Bomberman per representar les vides restants
  if (tryAgainBomberTex.loadFromFile("images/Sprites/Original/Color/Characters/bomber.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
    glm::vec2 sizeInUV = glm::vec2(17.f / 253.f, 17.f / 632.f);
    tryAgainBomberSprite = Sprite::createSprite(
        glm::vec2(34.f, 34.f), sizeInUV, &tryAgainBomberTex, &texProgram);
    tryAgainBomberSprite->setNumberAnimations(1);
    tryAgainBomberSprite->setAnimationSpeed(0, 8);
    tryAgainBomberSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
    tryAgainBomberSprite->changeAnimation(0);
    tryAgainBomberSprite->setPosition(glm::vec2(270.f, 195.f));
  }

  char livesBuf[32];
  snprintf(livesBuf, sizeof(livesBuf), "x %d", externalLives);
  tryAgainLivesLabel.init(livesBuf, 3, glm::vec2(318.f, 202.f), glm::vec4(1.f, 1.f, 1.f, 1.f), &texProgram);

  if (levelTimeLeft <= 0)
    centerLabel(tryAgainChanceLabel, "S'HA ACABAT EL TEMPS! AFANYA'T AQUEST COP!", 2, 285.f, glm::vec4(1.f, 0.6f, 0.2f, 1.f));
  else
    centerLabel(tryAgainChanceLabel, "TENS UNA ALTRA OPORTUNITAT!", 2, 285.f, glm::vec4(0.3f, 0.9f, 1.f, 1.f));

  centerLabel(tryAgainEncourageLabel, "NO ET RENDEIXIS!", 2, 320.f, glm::vec4(0.9f, 0.9f, 1.f, 1.f));
  centerLabel(tryAgainPromptLabel, "PREM ENTER O ESPAI PER CONTINUAR", 2, 380.f, glm::vec4(1.f, 0.9f, 0.2f, 1.f));
  centerLabel(tryAgainMenuLabel, "PREM ESC O M PER AL MENU PRINCIPAL", 2, 420.f, glm::vec4(0.6f, 0.6f, 0.7f, 1.f));
}

void Scene::initStageIntroScreen() {
  freeStageIntroScreen();

  auto centerLabel = [&](TextLabel &lbl, const std::string &txt, int scale, float y, const glm::vec4 &col) {
    Texture t;
    if (BitmapText::createTexture(t, txt, scale)) {
      float x = (float(SCREEN_WIDTH) - float(t.width())) * 0.5f;
      lbl.init(txt, scale, glm::vec2(x, y), col, &texProgram);
    }
  };

  char stageBuf[32];
  snprintf(stageBuf, sizeof(stageBuf), "NIVELL 1-%d", currentLevel);
  centerLabel(stageIntroTitleLabel, stageBuf, 4, 150.f, glm::vec4(1.f, 0.9f, 0.15f, 1.f));

  centerLabel(stageIntroReadyLabel, "PREPARAT!", 3, 230.f, glm::vec4(0.3f, 0.9f, 1.f, 1.f));

  if (stageIntroBomberTex.loadFromFile("images/Sprites/Original/Color/Characters/bomber.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
    glm::vec2 sizeInUV = glm::vec2(17.f / 253.f, 17.f / 632.f);
    stageIntroBomberSprite = Sprite::createSprite(
        glm::vec2(34.f, 34.f), sizeInUV, &stageIntroBomberTex, &texProgram);
    stageIntroBomberSprite->setNumberAnimations(1);
    stageIntroBomberSprite->setAnimationSpeed(0, 8);
    stageIntroBomberSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
    stageIntroBomberSprite->changeAnimation(0);
    stageIntroBomberSprite->setPosition(glm::vec2(275.f, 305.f));
  }

  char livesBuf[32];
  snprintf(livesBuf, sizeof(livesBuf), "x %d", externalLives);
  stageIntroLivesLabel.init(livesBuf, 3, glm::vec2(322.f, 312.f), glm::vec4(1.f, 1.f, 1.f, 1.f), &texProgram);

  centerLabel(stageIntroPromptLabel, "PREM ENTER O ESPAI", 2, 400.f, glm::vec4(0.75f, 0.75f, 0.75f, 1.f));
}

void Scene::freeStageIntroScreen() {
  if (stageIntroBomberSprite != NULL) {
    delete stageIntroBomberSprite;
    stageIntroBomberSprite = NULL;
  }
  stageIntroBomberTex.free();
  stageIntroTitleLabel.free();
  stageIntroReadyLabel.free();
  stageIntroLivesLabel.free();
  stageIntroPromptLabel.free();
}

void Scene::renderStageIntro() {
  glm::mat4 modelview = glm::mat4(1.0f);
  texProgram.setUniformMatrix4f("modelview", modelview);
  texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

  stageIntroTitleLabel.render(texProgram);
  stageIntroReadyLabel.render(texProgram);
  if (stageIntroBomberSprite != NULL)
    stageIntroBomberSprite->render();
  stageIntroLivesLabel.render(texProgram);

  // Parpelleig suau de la indicació
  if ((stageIntroTimer / 300) % 2 == 0) {
    stageIntroPromptLabel.render(texProgram);
  }
}

void Scene::initMenuScreens() {
  freeMenuScreens();

  // 1. Menú Principal (Logo, Copyright, Hudson Bee, Cursor)
  if (logoTex.loadFromFile("images/Menu/logo.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
    logoSprite = Sprite::createSprite(
        glm::vec2(278.f, 160.f), glm::vec2(1.f, 1.f), &logoTex, &texProgram);
    logoSprite->setPosition(
        glm::vec2((float(SCREEN_WIDTH) - 278.f) * 0.5f, 35.f));
  }

  if (copyrightTex.loadFromFile("images/Menu/copyright.png",
                                TEXTURE_PIXEL_FORMAT_RGBA)) {
    copyrightSprite =
        Sprite::createSprite(glm::vec2(286.f, 27.f), glm::vec2(1.f, 1.f),
                             &copyrightTex, &texProgram);
    copyrightSprite->setPosition(
        glm::vec2((float(SCREEN_WIDTH) - 286.f) * 0.5f, 412.f));
  }

  if (hudsonTex.loadFromFile("images/Menu/hudson.png",
                             TEXTURE_PIXEL_FORMAT_RGBA)) {
    hudsonSprite = Sprite::createSprite(
        glm::vec2(46.f, 44.f), glm::vec2(1.f, 1.f), &hudsonTex, &texProgram);
    hudsonSprite->setPosition(glm::vec2(40.f, 404.f));
  }

  if (cursorTex.loadFromFile("images/Menu/cursor.png",
                             TEXTURE_PIXEL_FORMAT_RGBA)) {
    cursorSprite = Sprite::createSprite(
        glm::vec2(24.f, 30.f), glm::vec2(1.f, 1.f), &cursorTex, &texProgram);
  }

  menuLabels[0].init("1. COMENCA EL JOC", 3, glm::vec2(168.f, 214.f),
                     glm::vec4(1.f), &texProgram);
  menuLabels[1].init("2. INSTRUCCIONS", 3, glm::vec2(186.f, 250.f),
                     glm::vec4(1.f), &texProgram);
  menuLabels[2].init("3. TRUCS", 3, glm::vec2(249.f, 286.f), glm::vec4(1.f),
                     &texProgram);
  menuLabels[3].init("4. CREDITS", 3, glm::vec2(231.f, 322.f), glm::vec4(1.f),
                     &texProgram);
  menuLabels[4].init("5. SURT", 3, glm::vec2(258.f, 358.f), glm::vec4(1.f),
                     &texProgram);
  menuFooterLabel.init("AMUNT/AVALL: TRIA  ENTER/ESPAI: OK  ESC: SURT", 2,
                       glm::vec2(32.f, 452.f), glm::vec4(0.8f, 0.8f, 0.9f, 1.f),
                       &texProgram);

  // 2. Pantalla d'Instruccions
  auto addInst = [&](const std::string &txt, int scale, float x, float y,
                     const glm::vec4 &col) {
    TextLabel *lbl = new TextLabel();
    lbl->init(txt, scale, glm::vec2(x, y), col, &texProgram);
    instructionsLabels.push_back(lbl);
  };
  addInst("INSTRUCCIONS I CONTROLS", 3, 100.f, 22.f,
          glm::vec4(1.f, 0.9f, 0.1f, 1.f));
  addInst("CONTROLS:", 2, 40.f, 58.f, glm::vec4(0.3f, 0.9f, 1.f, 1.f));
  addInst("  MOU-TE ESQUERRA/DRETA : FLETXES O A/D", 2, 40.f, 78.f,
          glm::vec4(1.f));
  addInst("  SALTA                  : FLETXA AMUNT O W", 2, 40.f, 96.f,
          glm::vec4(1.f));
  addInst("  DEIXA UNA BOMBA         : ESPAI O X", 2, 40.f, 114.f,
          glm::vec4(1.f));
  addInst("  PAUSA                  : P O ESC", 2, 40.f, 132.f,
          glm::vec4(1.f));

  addInst("OBJECTIU DEL JOC:", 2, 40.f, 160.f,
          glm::vec4(0.3f, 0.9f, 1.f, 1.f));
  addInst("  - POSA BOMBES PER ELIMINAR ELS ENEMICS", 2, 40.f, 180.f,
          glm::vec4(1.f));
  addInst("  - SALTA SOBRE LES BOMBES PER PUJAR MES AMUNT", 2, 40.f, 198.f,
          glm::vec4(1.f));
  addInst("  - ARRIBA A LA PORTA DE SORTIDA PER ACABAR", 2, 40.f, 216.f,
          glm::vec4(1.f));
  addInst("  - SUPERA EL NIVELL EN 2 MINUTS (120 S)", 2, 40.f, 234.f,
          glm::vec4(1.f));

  addInst("TOTS ELS TRUCS SON AL MENU PRINCIPAL", 2,
          40.f, 274.f, glm::vec4(0.3f, 0.9f, 1.f, 1.f));

  addInst("[ PREM ENTER O ESC PER TORNAR AL MENU ]", 2, 85.f, 452.f,
          glm::vec4(1.f, 0.9f, 0.2f, 1.f));

  // 3. Pantalla de Trucs
  auto addCheat = [&](const std::string &txt, int scale, float x, float y,
                      const glm::vec4 &col) {
    TextLabel *lbl = new TextLabel();
    lbl->init(txt, scale, glm::vec2(x, y), col, &texProgram);
    cheatsLabels.push_back(lbl);
  };
  addCheat("TRUCS DE PROVA", 3, 195.f, 35.f,
           glm::vec4(1.f, 0.9f, 0.1f, 1.f));
  addCheat("G: ACTIVA/DESACTIVA LA INVULNERABILITAT", 2, 50.f, 100.f,
           glm::vec4(1.f));
  addCheat("K: ELIMINA ELS ENEMICS I OBRE LA PORTA", 2, 50.f, 136.f,
           glm::vec4(1.f));
  addCheat("H: AFEGEIX UN COR", 2, 50.f, 172.f, glm::vec4(1.f));
  addCheat("B: AUGMENTA EL NOMBRE DE BOMBES", 2, 50.f, 208.f,
           glm::vec4(1.f));
  addCheat("F: AUGMENTA EL RADI DE L'EXPLOSIO", 2, 50.f, 244.f,
           glm::vec4(1.f));
  addCheat("S: AUGMENTA LA VELOCITAT", 2, 50.f, 280.f,
           glm::vec4(1.f));
  addCheat("N: PASSA AL NIVELL SEGUENT", 2, 50.f, 316.f,
           glm::vec4(1.f));
  addCheat("1-5: CANVIA DIRECTAMENT AL NIVELL TRIAT", 2, 50.f, 352.f,
           glm::vec4(1.f));
  addCheat("ELS TRUCS FUNCIONEN DURANT LA PARTIDA", 2, 86.f, 394.f,
           glm::vec4(0.3f, 0.9f, 1.f, 1.f));
  addCheat("[ PREM ENTER O ESC PER TORNAR AL MENU ]", 2, 85.f, 442.f,
           glm::vec4(1.f, 0.9f, 0.2f, 1.f));

  // 4. Pantalla de Crèdits
  auto addCred = [&](const std::string &txt, int scale, float x, float y,
                     const glm::vec4 &col) {
    TextLabel *lbl = new TextLabel();
    lbl->init(txt, scale, glm::vec2(x, y), col, &texProgram);
    creditsLabels.push_back(lbl);
  };
  addCred("CREDITS", 3, 245.f, 28.f, glm::vec4(1.f, 0.9f, 0.1f, 1.f));
  addCred("POCKET BOMBERMAN 2D", 2, 180.f, 85.f,
          glm::vec4(0.3f, 0.9f, 1.f, 1.f));
  addCred("PROJECTE FIB - VJ (2026/27)", 2, 180.f, 110.f, glm::vec4(1.f));
  addCred("FACULTAT D'INFORMATICA DE BARCELONA", 2, 180.f, 135.f,
          glm::vec4(1.f));
  addCred("AUTORS DEL PROJECTE:", 2, 180.f, 175.f,
          glm::vec4(0.3f, 0.9f, 1.f, 1.f));
  addCred("  - HECTOR GUTIERREZ JOSE", 2, 180.f, 200.f, glm::vec4(1.f));
  addCred("  - JAN DIEZ DE LOS RIOS PENAFIEL", 2, 180.f, 225.f, glm::vec4(1.f));
  addCred("RECURSOS ORIGINALS I SPRITES:", 2, 180.f, 265.f,
          glm::vec4(0.3f, 0.9f, 1.f, 1.f));
  addCred("  (C) 1998 HUDSON SOFT / NINTENDO", 2, 180.f, 290.f, glm::vec4(1.f));
  addCred("  SPRITE RIPS: FRET AND WOPPET", 2, 180.f, 315.f,
          glm::vec4(0.8f, 0.8f, 0.8f, 1.f));
  addCred("AGRAIMENTS ALS PROFESSORS DE VIDEOJOCS", 2, 95.f, 375.f,
          glm::vec4(0.7f, 0.8f, 1.f, 1.f));
  addCred("[ PREM ENTER O ESC PER TORNAR AL MENU ]", 2, 85.f, 442.f,
          glm::vec4(1.f, 0.9f, 0.2f, 1.f));

  // 4. Pausa
  if (pauseTex.loadFromFile("images/Menu/pause.png",
                            TEXTURE_PIXEL_FORMAT_RGBA)) {
    pauseSprite = Sprite::createSprite(
        glm::vec2(208.f, 217.f), glm::vec2(1.f, 1.f), &pauseTex, &texProgram);
    pauseSprite->setPosition(
        glm::vec2((float(SCREEN_WIDTH) - 208.f) * 0.5f, 45.f));
  }
  {
    Texture tPause;
    if (BitmapText::createTexture(tPause, "JOC EN PAUSA", 3)) {
      float x = (float(SCREEN_WIDTH) - float(tPause.width())) * 0.5f;
      pauseTitleLabel.init("JOC EN PAUSA", 3, glm::vec2(x, 280.f),
                           glm::vec4(1.f, 0.9f, 0.1f, 1.f), &texProgram);
    }
    if (BitmapText::createTexture(tPause, "PREM P O ESC PER REPRENDRE", 2)) {
      float x = (float(SCREEN_WIDTH) - float(tPause.width())) * 0.5f;
      pauseResumeLabel.init("PREM P O ESC PER REPRENDRE", 2, glm::vec2(x, 335.f),
                            glm::vec4(1.f), &texProgram);
    }
    if (BitmapText::createTexture(tPause, "PREM M PER TORNAR AL MENU PRINCIPAL", 2)) {
      float x = (float(SCREEN_WIDTH) - float(tPause.width())) * 0.5f;
      pauseMenuLabel.init("PREM M PER TORNAR AL MENU PRINCIPAL", 2,
                          glm::vec2(x, 370.f), glm::vec4(0.4f, 0.9f, 1.f, 1.f),
                          &texProgram);
    }
  }

  // 5. Game Over / Time Over
  if (gameOverTex.loadFromFile("images/Menu/game_over.png",
                               TEXTURE_PIXEL_FORMAT_RGBA)) {
    gameOverSprite =
        Sprite::createSprite(glm::vec2(310.f, 121.f), glm::vec2(1.f, 1.f),
                             &gameOverTex, &texProgram);
    gameOverSprite->setPosition(
        glm::vec2((float(SCREEN_WIDTH) - 310.f) * 0.5f, 100.f));
  }
  if (timeOverTex.loadFromFile("images/Menu/time_over.png",
                               TEXTURE_PIXEL_FORMAT_RGBA)) {
    timeOverSprite =
        Sprite::createSprite(glm::vec2(205.f, 202.f), glm::vec2(1.f, 1.f),
                             &timeOverTex, &texProgram);
    timeOverSprite->setPosition(
        glm::vec2((float(SCREEN_WIDTH) - 205.f) * 0.5f, 45.f));
  }
  Texture tTimeOver;
  if (BitmapText::createTexture(tTimeOver, "S'HA ACABAT EL TEMPS!", 3)) {
    float x = (float(SCREEN_WIDTH) - float(tTimeOver.width())) * 0.5f;
    timeOverMsgLabel.init("S'HA ACABAT EL TEMPS!", 3, glm::vec2(x, 275.f),
                          glm::vec4(1.f, 0.3f, 0.2f, 1.f), &texProgram);
  }
  Texture tGameOverMenu;
  if (BitmapText::createTexture(tGameOverMenu, "PREM ENTER O ESC PER AL MENU PRINCIPAL", 2)) {
    float x = (float(SCREEN_WIDTH) - float(tGameOverMenu.width())) * 0.5f;
    gameOverMenuLabel.init("PREM ENTER O ESC PER AL MENU PRINCIPAL", 2,
                           glm::vec2(x, 350.f),
                           glm::vec4(1.f, 0.9f, 0.2f, 1.f), &texProgram);
  }

  // 6. Victòria (Win)
  if (worldClearTex.loadFromFile("images/Menu/world_clear.png",
                                 TEXTURE_PIXEL_FORMAT_RGBA)) {
    worldClearSprite =
        Sprite::createSprite(glm::vec2(316.f, 52.f), glm::vec2(1.f, 1.f),
                             &worldClearTex, &texProgram);
    worldClearSprite->setPosition(
        glm::vec2((float(SCREEN_WIDTH) - 316.f) * 0.5f, 60.f));
  }
  if (celebrateTex.loadFromFile("images/Menu/knights_celebrate.png",
                                TEXTURE_PIXEL_FORMAT_RGBA)) {
    celebrateSprite =
        Sprite::createSprite(glm::vec2(316.f, 79.f), glm::vec2(1.f, 1.f),
                             &celebrateTex, &texProgram);
    celebrateSprite->setPosition(
        glm::vec2((float(SCREEN_WIDTH) - 316.f) * 0.5f, 135.f));
  }
  winCongratsLabel.init("FELICITATS!", 3, glm::vec2(219.f, 240.f),
                        glm::vec4(1.f, 0.9f, 0.1f, 1.f), &texProgram);
  winSubLabel.init("HAS SUPERAT ELS 5 MONS!", 2, glm::vec2(176.f, 295.f),
                   glm::vec4(1.f), &texProgram);
  winPeaceLabel.init("LA PAU HA TORNAT A BOMBER ISLAND!", 2,
                     glm::vec2(110.f, 330.f), glm::vec4(0.4f, 0.9f, 1.f, 1.f),
                     &texProgram);
  winPromptLabel.init("PREM ENTER O ESC PER TORNAR AL MENU", 2,
                      glm::vec2(104.f, 390.f), glm::vec4(1.f, 0.9f, 0.2f, 1.f),
                      &texProgram);
}

void Scene::init() {
  initShaders();

  if (timerSprite != NULL) {
    delete timerSprite;
    timerSprite = NULL;
  }
  if (levelHUDSprite != NULL) {
    delete levelHUDSprite;
    levelHUDSprite = NULL;
  }
  if (heartIcon != NULL) {
    delete heartIcon;
    heartIcon = NULL;
  }
  if (godModeOnSprite != NULL) {
    delete godModeOnSprite;
    godModeOnSprite = NULL;
  }
  if (godModeOffSprite != NULL) {
    delete godModeOffSprite;
    godModeOffSprite = NULL;
  }
  if (doorOpenSprite != NULL) {
    delete doorOpenSprite;
    doorOpenSprite = NULL;
  }
  if (enemiesHUDSprite != NULL) {
    delete enemiesHUDSprite;
    enemiesHUDSprite = NULL;
  }
  lastDisplayedEnemies = -1;

  initMenuScreens();

  externalLives = 3;
  startLevel(1, false);
  gameState = MAIN_MENU;
  currentTime = 0.0f;

  // Icona de vida per al HUD (mateix spritesheet del jugador)
  hudTex.loadFromFile("images/Sprites/Original/Color/Characters/bomber.png",
                      TEXTURE_PIXEL_FORMAT_RGBA);
  glm::vec2 sizeInUV = glm::vec2(17.f / 253.f, 17.f / 632.f);
  lifeIcon =
      Sprite::createSprite(glm::ivec2(16, 16), sizeInUV, &hudTex, &texProgram);
  lifeIcon->setNumberAnimations(1);
  lifeIcon->setAnimationSpeed(0, 8);
  lifeIcon->addKeyframe(0, glm::vec2(0.f, 0.f));
  lifeIcon->changeAnimation(0);

  // Inicialitzar textura i sprite de cor pixel-art per a les vides de nivell
  initHeartTexture();

  BitmapText::createTexture(godModeOnTex, "MODE DEU ACTIVAT");
  godModeOnSprite = Sprite::createSprite(
      glm::vec2(float(godModeOnTex.width()), float(godModeOnTex.height())),
      glm::vec2(1.f, 1.f), &godModeOnTex, &texProgram);
  BitmapText::createTexture(godModeOffTex, "MODE DEU DESACTIVAT");
  godModeOffSprite = Sprite::createSprite(
      glm::vec2(float(godModeOffTex.width()), float(godModeOffTex.height())),
      glm::vec2(1.f, 1.f), &godModeOffTex, &texProgram);
  BitmapText::createTexture(doorOpenTex, "PORTA OBERTA!", 2);
  doorOpenSprite = Sprite::createSprite(
      glm::vec2(float(doorOpenTex.width()), float(doorOpenTex.height())),
      glm::vec2(1.f, 1.f), &doorOpenTex, &texProgram);
}

void Scene::update(int deltaTime) {
  currentTime += deltaTime;

  if (godModeMsgTime > 0) {
    godModeMsgTime -= deltaTime;
    if (godModeMsgTime < 0)
      godModeMsgTime = 0;
  }
  if (doorOpenMsgTime > 0) {
    doorOpenMsgTime -= deltaTime;
    if (doorOpenMsgTime < 0)
      doorOpenMsgTime = 0;
  }

  switch (gameState) {
  case MAIN_MENU:
    if (isKeyJustPressed(GLFW_KEY_UP) || isKeyJustPressed(GLFW_KEY_W)) {
      menuOption = (menuOption + 4) % 5;
    } else if (isKeyJustPressed(GLFW_KEY_DOWN) ||
               isKeyJustPressed(GLFW_KEY_S)) {
      menuOption = (menuOption + 1) % 5;
    } else if (isKeyJustPressed(GLFW_KEY_ENTER) ||
               isKeyJustPressed(GLFW_KEY_SPACE)) {
      if (menuOption == 0) {
        externalLives = 3;
        startLevel(1, false);
      } else if (menuOption == 1) {
        gameState = INSTRUCTIONS;
      } else if (menuOption == 2) {
        gameState = CHEATS;
      } else if (menuOption == 3) {
        gameState = CREDITS;
      } else if (menuOption == 4) {
        Game::instance().exit();
      }
    } else if (isKeyJustPressed(GLFW_KEY_1)) {
      externalLives = 3;
      startLevel(1, false);
    } else if (isKeyJustPressed(GLFW_KEY_2)) {
      gameState = INSTRUCTIONS;
    } else if (isKeyJustPressed(GLFW_KEY_3)) {
      gameState = CHEATS;
    } else if (isKeyJustPressed(GLFW_KEY_4)) {
      gameState = CREDITS;
    } else if (isKeyJustPressed(GLFW_KEY_5) ||
               isKeyJustPressed(GLFW_KEY_ESCAPE)) {
      Game::instance().exit();
    }
    break;

  case INSTRUCTIONS:
    if (isKeyJustPressed(GLFW_KEY_ENTER) || isKeyJustPressed(GLFW_KEY_ESCAPE) ||
        isKeyJustPressed(GLFW_KEY_SPACE)) {
      gameState = MAIN_MENU;
    }
    break;

  case CHEATS:
    if (isKeyJustPressed(GLFW_KEY_ENTER) || isKeyJustPressed(GLFW_KEY_ESCAPE) ||
        isKeyJustPressed(GLFW_KEY_SPACE)) {
      gameState = MAIN_MENU;
    }
    break;

  case CREDITS:
    if (isKeyJustPressed(GLFW_KEY_ENTER) || isKeyJustPressed(GLFW_KEY_ESCAPE) ||
        isKeyJustPressed(GLFW_KEY_SPACE)) {
      gameState = MAIN_MENU;
    }
    break;

  case PLAYING:
    if (isKeyJustPressed(GLFW_KEY_ESCAPE) || isKeyJustPressed(GLFW_KEY_P)) {
      gameState = PAUSED;
    } else {
      // Descomptar el temps del nivell només si el jugador no està morint
      if (!player->isDying() && !player->isDead()) {
        levelTimeLeft -= deltaTime;
        if (levelTimeLeft <= 0) {
          levelTimeLeft = 0;
          player->startDying();
        }
      }

      // Tecles de drecera / debug per provar canvi de nivells
      if (isKeyJustPressed(GLFW_KEY_N)) {
        nextLevel();
        break;
      }
      if (isKeyJustPressed(GLFW_KEY_P)) {
        int prev = (currentLevel > 1) ? currentLevel - 1 : 1;
        startLevel(prev, true);
        break;
      }
      if (isKeyJustPressed(GLFW_KEY_1)) {
        startLevel(1, true);
        break;
      }
      if (isKeyJustPressed(GLFW_KEY_2)) {
        startLevel(2, true);
        break;
      }
      if (isKeyJustPressed(GLFW_KEY_3)) {
        startLevel(3, true);
        break;
      }
      if (isKeyJustPressed(GLFW_KEY_4)) {
        startLevel(4, true);
        break;
      }
      if (isKeyJustPressed(GLFW_KEY_5)) {
        startLevel(5, true);
        break;
      }

      if (isKeyJustPressed(GLFW_KEY_G)) {
        player->toggleGodMode();
        showGodModeMessage(player->isGodMode());
      }
      if (isKeyJustPressed(GLFW_KEY_K)) {
        // Cheat: elimina tots els enemics i obre la porta
        for (size_t i = 0; i < enemies.size(); ++i) {
          if (enemies[i] != NULL && enemies[i]->isAlive())
            enemies[i]->kill();
        }
        if (door != NULL) {
          if (!door->isOpen())
            showDoorOpenMessage();
          door->setOpen(true);
        }
      }
      // Cheats de testeig per provar el sistema de power-ups i vides
      if (isKeyJustPressed(GLFW_KEY_H)) {
        player->addInternalLife(); // Afegeix vida interna (cor/escut)
      }
      if (isKeyJustPressed(GLFW_KEY_B)) {
        player->addMaxBomb(); // Incrementa bombes simultànies
      }
      if (isKeyJustPressed(GLFW_KEY_F)) {
        player->addBombRadius(); // Incrementa radi d'explosió (Fire Up)
      }
      if (isKeyJustPressed(GLFW_KEY_S)) {
        player->addSpeed(); // Incrementa velocitat de moviment (Patins)
      }
      if (isKeyJustPressed(GLFW_KEY_SPACE) || isKeyJustPressed(GLFW_KEY_X)) {
        placeBomb();
      }

      // Actualitzar porta i comprovar col·lisió per passar de nivell
      if (door != NULL && !player->isDying() && !player->isDead()) {
        door->update(deltaTime);
        if (door->isOpen() &&
            door->checkCollision(player->getPosition(), player->getSize())) {
          nextLevel();
          break;
        }
      }

      // 1. Actualitzar bombes
      for (size_t i = 0; i < bombs.size(); ++i) {
        if (bombs[i] != NULL) {
          bombs[i]->update(deltaTime);
          if (bombs[i]->shouldExplode()) {
            Explosion *exp = new Explosion();
            exp->init(bombs[i]->getPosition(), glm::ivec2(SCREEN_X, SCREEN_Y),
                      bombs[i]->getRadius(), map, texProgram);
            explosions.push_back(exp);

            delete bombs[i];
            bombs[i] = NULL;
          }
        }
      }
      for (auto it = bombs.begin(); it != bombs.end();) {
        if (*it == NULL)
          it = bombs.erase(it);
        else
          ++it;
      }

      // 2. Actualitzar explosions
      for (size_t i = 0; i < explosions.size(); ++i) {
        if (explosions[i] != NULL) {
          explosions[i]->update(deltaTime);

          // Reacció en cadena amb altres bombes
          for (size_t b = 0; b < bombs.size(); ++b) {
            if (bombs[b] != NULL && bombs[b]->isActive()) {
              if (explosions[i]->checkCollision(
                      glm::ivec2(bombs[b]->getPosition()),
                      glm::ivec2(16, 16))) {
                bombs[b]->explode();
              }
            }
          }

          // Col·lisió amb el jugador: l'explosió fa dany
          if (explosions[i]->checkCollision(player->getPosition(),
                                            player->getSize())) {
            if (player->hit()) {
              // Ha iniciat l'animació de mort (startDying)
              break;
            }
          }

          // Col·lisió amb enemics: l'explosió els mata
          for (size_t e = 0; e < enemies.size(); ++e) {
            if (enemies[e] != NULL && enemies[e]->isAlive()) {
              if (explosions[i]->checkCollision(enemies[e]->getPosition(),
                                                enemies[e]->getSize())) {
                enemies[e]->kill();
              }
            }
          }

          if (!explosions[i]->isActive()) {
            delete explosions[i];
            explosions[i] = NULL;
          }
        }
      }
      if (gameState != PLAYING)
        break;

      for (auto it = explosions.begin(); it != explosions.end();) {
        if (*it == NULL)
          it = explosions.erase(it);
        else
          ++it;
      }

      // 3. Actualitzar enemics i estat de la porta
      updateEnemies(deltaTime);

      // 4. Contacte enemic ↔ jugador (dany al sistema de vides)
      if (!player->isDying() && !player->isDead() && !player->isInvulnerable()) {
        for (size_t e = 0; e < enemies.size(); ++e) {
          if (enemies[e] != NULL && enemies[e]->isAlive() &&
              enemies[e]->checkCollision(player->getPosition(),
                                         player->getSize())) {
            player->hit();
            break;
          }
        }
      }

      // 5. Actualitzar power-ups (animació de flotació i recol·lecció pel jugador)
      updatePowerUps(deltaTime);

      player->update(deltaTime);

      // Comprovar si l'animació de mort ha finalitzat per descomptar la vida general i reiniciar / Game Over
      if (player->isDead()) {
        if (levelTimeLeft <= 0)
          bTimeOver = (externalLives <= 1);
        onPlayerDeath();
        break;
      }

      // --- Actualitzar la càmera per seguir el jugador ---
      {
        glm::ivec2 pPos = player->getPosition();
        float targetX = float(SCREEN_X + pPos.x) + 8.f - viewWidth * 0.5f;
        float targetY = float(SCREEN_Y + pPos.y) + 8.f - viewHeight * 0.5f;

        float lerpSpeed = 0.08f;
        camX += (targetX - camX) * lerpSpeed;
        camY += (targetY - camY) * lerpSpeed;

        float mapRight =
            float(SCREEN_X) + float(map->getMapWidth()) - viewWidth;
        float mapBottom =
            float(SCREEN_Y) + float(map->getMapHeight()) - viewHeight;
        if (camX < 0.f)
          camX = 0.f;
        if (camY < 0.f)
          camY = 0.f;
        if (camX > mapRight)
          camX = mapRight;
        if (camY > mapBottom)
          camY = mapBottom;

        projection =
            glm::ortho(camX, camX + viewWidth, camY + viewHeight, camY);
      }
    }
    break;

  case PAUSED:
    if (isKeyJustPressed(GLFW_KEY_ESCAPE) || isKeyJustPressed(GLFW_KEY_P)) {
      gameState = PLAYING;
    } else if (isKeyJustPressed(GLFW_KEY_M)) {
      gameState = MAIN_MENU;
    }
    break;

  case GAME_OVER:
    if (isKeyJustPressed(GLFW_KEY_ENTER) ||
        isKeyJustPressed(GLFW_KEY_SPACE) ||
        isKeyJustPressed(GLFW_KEY_ESCAPE) ||
        isKeyJustPressed(GLFW_KEY_M)) {
      gameState = MAIN_MENU;
    }
    break;

  case WIN:
    if (isKeyJustPressed(GLFW_KEY_ENTER) || isKeyJustPressed(GLFW_KEY_ESCAPE) ||
        isKeyJustPressed(GLFW_KEY_SPACE)) {
      gameState = MAIN_MENU;
    }
    break;

  case TRY_AGAIN:
    tryAgainTimer += deltaTime;
    if (isKeyJustPressed(GLFW_KEY_ENTER) ||
        isKeyJustPressed(GLFW_KEY_SPACE) ||
        isKeyJustPressed(GLFW_KEY_X) ||
        isKeyJustPressed(GLFW_KEY_Z) ||
        Game::instance().getAndResetMouseClick()) {
      startLevel(currentLevel, true);
    } else if (isKeyJustPressed(GLFW_KEY_ESCAPE) ||
               isKeyJustPressed(GLFW_KEY_M)) {
      gameState = MAIN_MENU;
    }
    break;

  case STAGE_INTRO:
    stageIntroTimer += deltaTime;
    if (stageIntroTimer >= 1800 ||
        isKeyJustPressed(GLFW_KEY_ENTER) ||
        isKeyJustPressed(GLFW_KEY_SPACE) ||
        isKeyJustPressed(GLFW_KEY_X) ||
        isKeyJustPressed(GLFW_KEY_Z) ||
        Game::instance().getAndResetMouseClick()) {
      gameState = PLAYING;
    }
    break;
  }
}

void Scene::render() {
  glm::mat4 modelview;

  texProgram.use();
  texProgram.setUniformMatrix4f("projection", projection);
  texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
  modelview = glm::mat4(1.0f);
  texProgram.setUniformMatrix4f("modelview", modelview);
  texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);

  glm::mat4 screenProjection =
      glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);

  switch (gameState) {
  case MAIN_MENU:
    texProgram.setUniformMatrix4f("projection", screenProjection);
    renderMainMenu();
    break;
  case INSTRUCTIONS:
    texProgram.setUniformMatrix4f("projection", screenProjection);
    renderInstructions();
    break;
  case CHEATS:
    texProgram.setUniformMatrix4f("projection", screenProjection);
    renderCheats();
    break;
  case CREDITS:
    texProgram.setUniformMatrix4f("projection", screenProjection);
    renderCredits();
    break;
  case PLAYING:
    map->render();
    if (door != NULL)
      door->render();
    renderBombs();
    renderExplosions();
    renderPowerUps();
    renderEnemies();
    player->render();
    renderHUD();
    break;
  case PAUSED:
    map->render();
    if (door != NULL)
      door->render();
    renderBombs();
    renderExplosions();
    renderPowerUps();
    renderEnemies();
    player->render();
    texProgram.setUniformMatrix4f("projection", screenProjection);
    renderPauseOverlay();
    break;
  case GAME_OVER:
    texProgram.setUniformMatrix4f("projection", screenProjection);
    renderGameOver();
    break;
  case WIN:
    texProgram.setUniformMatrix4f("projection", screenProjection);
    renderWin();
    break;
  case TRY_AGAIN:
    texProgram.setUniformMatrix4f("projection", screenProjection);
    renderTryAgain();
    break;
  case STAGE_INTRO:
    texProgram.setUniformMatrix4f("projection", screenProjection);
    renderStageIntro();
    break;
  }
}

void Scene::initShaders() {
  Shader vShader, fShader;

  vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
  if (!vShader.isCompiled()) {
    cout << "Vertex Shader Error" << endl;
    cout << "" << vShader.log() << endl << endl;
  }
  fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
  if (!fShader.isCompiled()) {
    cout << "Fragment Shader Error" << endl;
    cout << "" << fShader.log() << endl << endl;
  }
  texProgram.init();
  texProgram.addShader(vShader);
  texProgram.addShader(fShader);
  texProgram.link();
  if (!texProgram.isLinked()) {
    cout << "Shader Linking Error" << endl;
    cout << "" << texProgram.log() << endl << endl;
  }
  texProgram.bindFragmentOutput("outColor");
  vShader.free();
  fShader.free();
}

void Scene::renderMainMenu() {
  glm::mat4 modelview = glm::mat4(1.0f);
  texProgram.setUniformMatrix4f("modelview", modelview);
  texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

  if (logoSprite != NULL)
    logoSprite->render();

  for (int i = 0; i < 5; ++i) {
    if (i == menuOption) {
      menuLabels[i].renderWithColor(texProgram,
                                    glm::vec4(1.0f, 0.95f, 0.15f, 1.0f));
    } else {
      menuLabels[i].renderWithColor(texProgram,
                                    glm::vec4(0.72f, 0.72f, 0.78f, 1.0f));
    }
  }

  if (cursorSprite != NULL) {
    float bob = sin(currentTime * 0.008f) * 4.0f;
    float cursorX = 142.f + bob;
    float cursorY = 211.f + float(menuOption) * 36.f;
    cursorSprite->setPosition(glm::vec2(cursorX, cursorY));
    cursorSprite->render();
  }

  if (hudsonSprite != NULL)
    hudsonSprite->render();

  if (copyrightSprite != NULL)
    copyrightSprite->render();

  menuFooterLabel.render(texProgram);
}

void Scene::renderInstructions() {
  glm::mat4 modelview = glm::mat4(1.0f);
  texProgram.setUniformMatrix4f("modelview", modelview);
  texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

  for (size_t i = 0; i < instructionsLabels.size(); ++i) {
    if (instructionsLabels[i] != NULL)
      instructionsLabels[i]->render(texProgram);
  }
}

void Scene::renderCheats() {
  glm::mat4 modelview = glm::mat4(1.0f);
  texProgram.setUniformMatrix4f("modelview", modelview);
  texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

  for (size_t i = 0; i < cheatsLabels.size(); ++i) {
    if (cheatsLabels[i] != NULL)
      cheatsLabels[i]->render(texProgram);
  }
}

void Scene::renderCredits() {
  glm::mat4 modelview = glm::mat4(1.0f);
  texProgram.setUniformMatrix4f("modelview", modelview);
  texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

  if (hudsonSprite != NULL) {
    hudsonSprite->setPosition(glm::vec2(70.f, 95.f));
    hudsonSprite->render();
    // Restore pos for main menu
    hudsonSprite->setPosition(glm::vec2(40.f, 404.f));
  }

  for (size_t i = 0; i < creditsLabels.size(); ++i) {
    if (creditsLabels[i] != NULL)
      creditsLabels[i]->render(texProgram);
  }
}

void Scene::renderPauseOverlay() {
  glm::mat4 modelview = glm::mat4(1.0f);
  texProgram.setUniformMatrix4f("modelview", modelview);
  texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

  if (pauseSprite != NULL)
    pauseSprite->render();

  pauseTitleLabel.render(texProgram);
  pauseResumeLabel.render(texProgram);
  pauseMenuLabel.render(texProgram);
}

void Scene::renderGameOver() {
  glm::mat4 modelview = glm::mat4(1.0f);
  texProgram.setUniformMatrix4f("modelview", modelview);
  texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

  if (bTimeOver) {
    if (timeOverSprite != NULL)
      timeOverSprite->render();
    timeOverMsgLabel.render(texProgram);
  } else {
    if (gameOverSprite != NULL)
      gameOverSprite->render();
  }

  gameOverMenuLabel.render(texProgram);
}

void Scene::renderWin() {
  glm::mat4 modelview = glm::mat4(1.0f);
  texProgram.setUniformMatrix4f("modelview", modelview);
  texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

  if (worldClearSprite != NULL)
    worldClearSprite->render();

  if (celebrateSprite != NULL)
    celebrateSprite->render();

  winCongratsLabel.render(texProgram);
  winSubLabel.render(texProgram);
  winPeaceLabel.render(texProgram);
  winPromptLabel.render(texProgram);
}

void Scene::renderTryAgain() {
  glm::mat4 modelview = glm::mat4(1.0f);
  texProgram.setUniformMatrix4f("modelview", modelview);
  texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

  if (tryAgainBomberSprite != NULL)
    tryAgainBomberSprite->render();

  tryAgainStageLabel.render(texProgram);
  tryAgainTitleLabel.render(texProgram);
  tryAgainLivesLabel.render(texProgram);
  tryAgainChanceLabel.render(texProgram);
  tryAgainEncourageLabel.render(texProgram);

  // Efecte de parpelleig suau del text de reprendre per destacar la interacció
  if ((tryAgainTimer / 300) % 2 == 0)
    tryAgainPromptLabel.render(texProgram);
  else
    tryAgainPromptLabel.renderWithColor(texProgram, glm::vec4(1.f, 0.9f, 0.2f, 0.4f));

  tryAgainMenuLabel.render(texProgram);
}

bool Scene::isKeyJustPressed(int key)
// aquesta funció la utilitzarem per a que detecti que ha sigut presionada un
// unic cop fins que la deixes de presionar
{
  if (key < 0 || key > GLFW_KEY_LAST)
    return false;
  bool pressed = Game::instance().getKey(key);
  if (pressed && !keyLastState[key]) {
    keyLastState[key] = true;
    return true;
  }
  if (!pressed) {
    keyLastState[key] = false;
  }
  return false;
}

void Scene::renderEnemies() {
  for (size_t i = 0; i < enemies.size(); ++i) {
    if (enemies[i] != NULL)
      enemies[i]->render();
  }
}
void Scene::renderBombs() {
  for (size_t i = 0; i < bombs.size(); ++i) {
    if (bombs[i] != NULL)
      bombs[i]->render();
  }
}
void Scene::renderExplosions() {
  for (size_t i = 0; i < explosions.size(); ++i) {
    if (explosions[i] != NULL)
      explosions[i]->render();
  }
}
void Scene::updateTimerHUD() {
  int totalSecs = (levelTimeLeft + 999) / 1000;
  if (totalSecs < 0)
    totalSecs = 0;

  if (totalSecs != lastDisplayedSeconds) {
    lastDisplayedSeconds = totalSecs;
    int mins = totalSecs / 60;
    int secs = totalSecs % 60;

    char buf[32];
    snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);

    if (timerSprite != NULL) {
      delete timerSprite;
      timerSprite = NULL;
    }

    if (BitmapText::createTexture(timerTex, buf, 1)) {
      timerSprite = Sprite::createSprite(
          glm::vec2(float(timerTex.width()), float(timerTex.height())),
          glm::vec2(1.f, 1.f), &timerTex, &texProgram);
    }
  }
}

void Scene::updateLevelHUD() {
  if (currentLevel != lastDisplayedLevel) {
    lastDisplayedLevel = currentLevel;
    char buf[32];
    snprintf(buf, sizeof(buf), "1-%d", currentLevel);

    if (levelHUDSprite != NULL) {
      delete levelHUDSprite;
      levelHUDSprite = NULL;
    }

    if (BitmapText::createTexture(levelHUDTex, buf, 1)) {
      levelHUDSprite = Sprite::createSprite(
          glm::vec2(float(levelHUDTex.width()), float(levelHUDTex.height())),
          glm::vec2(1.f, 1.f), &levelHUDTex, &texProgram);
    }
  }
}

void Scene::updateEnemiesHUD() {
  int count = countAliveEnemies();
  if (count == lastDisplayedEnemies && (count == 0 || enemiesHUDSprite != NULL))
    return;

  lastDisplayedEnemies = count;
  if (enemiesHUDSprite != NULL) {
    delete enemiesHUDSprite;
    enemiesHUDSprite = NULL;
  }
  enemiesHUDTex.free();

  if (count > 0) {
    char buf[32];
    snprintf(buf, sizeof(buf), "E:%d", count);
    if (BitmapText::createTexture(enemiesHUDTex, buf, 1)) {
      enemiesHUDSprite = Sprite::createSprite(
          glm::vec2(float(enemiesHUDTex.width()), float(enemiesHUDTex.height())),
          glm::vec2(1.f, 1.f), &enemiesHUDTex, &texProgram);
    }
  }
}

void Scene::renderHUD() {
  if (player == NULL || lifeIcon == NULL)
    return;

  const float hudY = camY + 4.f;
  const float hudPad = 6.f;

  // 1. Contador extern: nombre de vides generals (cares de Bomberman de 16x16)
  for (int i = 0; i < externalLives; ++i) {
    float x = camX + hudPad + float(i) * 16.f;
    lifeIcon->setPosition(glm::vec2(x, hudY));
    lifeIcon->render();
  }

  const float livesRight = hudPad + float(externalLives) * 16.f;

  // 2. Vida de nivell: cors pixel-art (1 de base; augmenta amb power-ups com cors)
  float heartsRight = livesRight;
  if (heartIcon != NULL) {
    int hp = player->getInternalLives();
    for (int j = 0; j < hp; ++j) {
      float hx = camX + livesRight + 4.f + float(j) * 13.f;
      heartIcon->setPosition(glm::vec2(hx, hudY + 2.f));
      heartIcon->render();
    }
    if (hp > 0)
      heartsRight = livesRight + 4.f + float(hp) * 13.f;
  }

  // 3. Temporitzador a la cantonada dreta
  updateTimerHUD();
  float timerLeft = camX + viewWidth - hudPad;
  if (timerSprite != NULL) {
    float timerW = float(timerTex.width());
    timerLeft = camX + viewWidth - timerW - hudPad;
    timerSprite->setPosition(glm::vec2(timerLeft, hudY + 4.f));

    int totalSecs = (levelTimeLeft + 999) / 1000;
    if (totalSecs <= 30)
      texProgram.setUniform4f("color", 1.0f, 0.2f, 0.2f,
                              1.0f); // Vermell <= 30s
    else
      texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

    timerSprite->render();
    texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
  }

  // 4. Comptador d'enemics restants al costat del temporitzador
  updateEnemiesHUD();
  float enemyHudLeft = timerLeft;
  if (enemiesHUDSprite != NULL) {
    const float enemyHudWidth = float(enemiesHUDTex.width());
    enemyHudLeft = timerLeft - enemyHudWidth - 8.f;
    enemiesHUDSprite->setPosition(glm::vec2(enemyHudLeft, hudY + 4.f));
    texProgram.setUniform4f("color", 1.0f, 0.85f, 0.2f, 1.0f); // Groc daurat
    enemiesHUDSprite->render();
    texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
  }

  // 5. Indicador de nivell centrat entre les vides i els comptadors drets
  updateLevelHUD();
  if (levelHUDSprite != NULL) {
    float hudW = float(levelHUDTex.width());
    float gapLeft = camX + heartsRight + 6.f;
    float gapRight = enemyHudLeft - 6.f;
    float x = gapLeft + (gapRight - gapLeft - hudW) * 0.5f;
    if (x < gapLeft)
      x = gapLeft;
    levelHUDSprite->setPosition(glm::vec2(x, hudY + 4.f));
    texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
    levelHUDSprite->render();
  }

  // 6. Missatge temporal de god mode si s'ha activat / desactivat
  if (godModeMsgTime > 0) {
    Sprite *msgSprite = godModeMsgActive ? godModeOnSprite : godModeOffSprite;
    if (msgSprite != NULL) {
      float msgW = godModeMsgActive ? float(godModeOnTex.width())
                                    : float(godModeOffTex.width());
      float x = camX + (viewWidth - msgW) * 0.5f;
      float y = camY + 28.f;
      msgSprite->setPosition(glm::vec2(x, y));

      if (godModeMsgActive)
        texProgram.setUniform4f("color", 1.0f, 1.0f, 0.0f, 1.0f); // groc
      else
        texProgram.setUniform4f("color", 1.0f, 0.15f, 0.15f, 1.0f); // vermell

      msgSprite->render();
      texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
    }
  }

  // 7. Missatge temporal d'avís de porta oberta quan moren tots els enemics
  if (doorOpenMsgTime > 0 && doorOpenSprite != NULL) {
    float msgW = float(doorOpenTex.width());
    float x = camX + (viewWidth - msgW) * 0.5f;
    float y = camY + (godModeMsgTime > 0 ? 46.f : 36.f);
    doorOpenSprite->setPosition(glm::vec2(x, y));

    // Parpelleig suau als últims 400ms abans de desaparèixer
    if (doorOpenMsgTime < 400 && ((doorOpenMsgTime / 100) % 2 == 0)) {
      // parpelleig transparent
    } else {
      texProgram.setUniform4f("color", 0.2f, 1.0f, 0.3f, 1.0f); // Verd brillant d'èxit
      doorOpenSprite->render();
      texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
    }
  }
}

void Scene::showDoorOpenMessage() {
  doorOpenMsgTime = 2500; // Visible durant 2.5 segons (entre 2 i 3 segons)
}

void Scene::showGodModeMessage(bool enabled) {
  godModeMsgActive = enabled;
  godModeMsgTime = GOD_MODE_MSG_DURATION;
}

void Scene::placeBomb() {
  if (player == NULL || map == NULL || player->isDying() || player->isDead())
    return;

  // Limitar bombes actives simultànies (1 de base; power-up Bomb Up
  // incrementarà maxBombs)
  int activeBombs = 0;
  for (size_t i = 0; i < bombs.size(); ++i) {
    if (bombs[i] != NULL && bombs[i]->isActive())
      activeBombs++;
  }
  if (activeBombs >= player->getMaxBombs())
    return;

  glm::ivec2 playerPos = player->getPosition();

  // Col·locar la bomba a la mateixa casella del jugador (directament a sota)
  int bombX = int(round(float(playerPos.x) / 16.f)) * 16;
  int bombY = int(round(float(playerPos.y) / 16.f)) * 16;

  // Evitar posar dues bombes a la mateixa casella
  for (size_t i = 0; i < bombs.size(); ++i) {
    if (bombs[i] != NULL && bombs[i]->isActive()) {
      glm::vec2 bPos = bombs[i]->getPosition();
      if (abs(bPos.x - float(bombX)) < 16.f &&
          abs(bPos.y - float(bombY)) < 16.f)
        return;
    }
  }

  Bomb *bomb = new Bomb();
  bomb->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
  bomb->setPosition(glm::vec2(float(bombX), float(bombY)));
  bomb->setRadius(player->getBombRadius());
  bomb->updatePlayerOverlap(playerPos, player->getSize());
  bombs.push_back(bomb);
}

void Scene::initHeartTexture() {
  const int HW = 12;
  const int HH = 12;
  static const char *pattern[HH] = {
    "..XX....XX..",
    ".XRRX..XRRX.",
    "XWRRRXXRRRRX",
    "XWRRRRRRRRRX",
    "XRRRRRRRRRRX",
    ".XRRRRRRRRX.",
    "..XRRRRRRX..",
    "...XRRRRX...",
    "....XRRX....",
    ".....XX.....",
    "............",
    "............"
  };

  std::vector<unsigned char> pixels(HW * HH * 4, 0);
  for (int y = 0; y < HH; ++y) {
    for (int x = 0; x < HW; ++x) {
      char c = pattern[y][x];
      int idx = (y * HW + x) * 4;
      if (c == 'X') {
        pixels[idx + 0] = 50;
        pixels[idx + 1] = 10;
        pixels[idx + 2] = 20;
        pixels[idx + 3] = 255;
      } else if (c == 'R') {
        pixels[idx + 0] = 235;
        pixels[idx + 1] = 35;
        pixels[idx + 2] = 50;
        pixels[idx + 3] = 255;
      } else if (c == 'W') {
        pixels[idx + 0] = 255;
        pixels[idx + 1] = 180;
        pixels[idx + 2] = 190;
        pixels[idx + 3] = 255;
      }
    }
  }

  heartTex.loadFromRGBABuffer(pixels.data(), HW, HH);
  heartTex.setMinFilter(GL_NEAREST);
  heartTex.setMagFilter(GL_NEAREST);
  heartTex.setWrapS(GL_CLAMP_TO_EDGE);
  heartTex.setWrapT(GL_CLAMP_TO_EDGE);

  if (heartIcon != NULL)
    delete heartIcon;
  heartIcon = Sprite::createSprite(glm::ivec2(12, 12), glm::vec2(1.f, 1.f),
                                   &heartTex, &texProgram);
  heartIcon->setNumberAnimations(1);
  heartIcon->setAnimationSpeed(0, 8);
  heartIcon->addKeyframe(0, glm::vec2(0.f, 0.f));
  heartIcon->changeAnimation(0);
}
