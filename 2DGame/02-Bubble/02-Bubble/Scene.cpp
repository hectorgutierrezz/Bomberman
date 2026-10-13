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

// Mida de la finestra de càmera en world pixels (zoom ~2.5x respecte 640x480)
// 256 unitats horitz. → 640/256 = 2.5x, 224 unitats vert. → 480/224 ≈ 2.14x
#define CAM_VIEW_W 256.f
#define CAM_VIEW_H 224.f

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
  godModeOnSprite = NULL;
  godModeOffSprite = NULL;
  godModeMsgTime = 0;
  godModeMsgActive = false;
  levelTimeLeft = 120000;
  lastDisplayedSeconds = -1;
  lastDisplayedLevel = -1;
  enemiesHUDSprite = NULL;
  lastDisplayedEnemies = -1;
  externalLives = 3;
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
  if (lifeIcon != NULL)
    delete lifeIcon;
  if (heartIcon != NULL)
    delete heartIcon;
  if (timerSprite != NULL)
    delete timerSprite;
  if (levelHUDSprite != NULL)
    delete levelHUDSprite;
  if (godModeOnSprite != NULL)
    delete godModeOnSprite;
  if (godModeOffSprite != NULL)
    delete godModeOffSprite;
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
  if (enemiesHUDSprite != NULL) {
    delete enemiesHUDSprite;
    enemiesHUDSprite = NULL;
  }
}

void Scene::clearEnemies() {
  for (size_t i = 0; i < enemies.size(); ++i) {
    if (enemies[i] != NULL)
      delete enemies[i];
  }
  enemies.clear();
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

  SpawnSpec specs[3];
  switch (currentLevel) {
  case 1:
    specs[0] = {ENEMY_HORIZONTAL, 14, 20}; // Patrulla plataforma fila 22
    specs[1] = {ENEMY_VERTICAL, 29, 18};   // Volador eix vertical dret
    specs[2] = {ENEMY_STATIC, 24, 25};     // Guardià sobre el terra
    break;
  case 2:
    specs[0] = {ENEMY_HORIZONTAL, 8, 16};  // Patrulla plataforma fila 18
    specs[1] = {ENEMY_VERTICAL, 20, 12};   // Volador obertura central
    specs[2] = {ENEMY_STATIC, 26, 25};     // Guardià sobre el terra
    break;
  case 3:
    specs[0] = {ENEMY_HORIZONTAL, 14, 25}; // Patrulla plataforma fila 27
    specs[1] = {ENEMY_VERTICAL, 28, 20};   // Volador corredor vertical
    specs[2] = {ENEMY_STATIC, 18, 20};     // Guardià plataforma central fila 22
    break;
  case 4:
    specs[0] = {ENEMY_HORIZONTAL, 16, 25}; // Patrulla plataforma fila 27
    specs[1] = {ENEMY_VERTICAL, 5, 20};    // Volador eix esquerre
    specs[2] = {ENEMY_STATIC, 18, 20};     // Guardià plataforma central fila 22
    break;
  default: // nivell 5
    specs[0] = {ENEMY_HORIZONTAL, 16, 25}; // Patrulla plataforma fila 27
    specs[1] = {ENEMY_VERTICAL, 29, 18};   // Volador eix dret
    specs[2] = {ENEMY_STATIC, 18, 15};     // Guardià plataforma superior fila 17
    break;
  }

  for (int i = 0; i < 3; ++i) {
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
  // La porta només s'obre quan no queda cap enemic viu
  door->setOpen(countAliveEnemies() == 0);
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
  currentLevel = levelNum;
  if (currentLevel < 1)
    currentLevel = 1;
  if (currentLevel > 5)
    currentLevel = 5;

  levelTimeLeft = 120000; // 2 minuts (120 segons) per nivell
  lastDisplayedSeconds = -1;
  lastDisplayedLevel = -1;

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

  // Spawn i porta a 2 tiles per damunt del terra (última fila del mapa)
  int tileSize = map->getTileSize();
  int mapTilesY = map->getMapHeight() / tileSize;
  int spawnTileY = mapTilesY - 3;
  if (spawnTileY < 1)
    spawnTileY = 1;
  int doorTileX = 30;
  int doorTileY = spawnTileY;

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

  // Spawnear els 3 enemics del nivell; la porta resta tancada fins a eliminar-los
  spawnEnemies();
  updateDoorFromEnemies();

  // Centrar la càmera al jugador al carregar el nivell
  viewWidth = CAM_VIEW_W;
  viewHeight = CAM_VIEW_H;
  glm::ivec2 initPos = player->getPosition();
  camX = float(SCREEN_X + initPos.x) + 16.f - viewWidth * 0.5f;
  camY = float(SCREEN_Y + initPos.y) + 16.f - viewHeight * 0.5f;

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
    // Encara queden vides generals: es reinicia la pantalla mantenint els power-ups
    startLevel(currentLevel, true);
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

  for (int i = 0; i < 4; ++i)
    menuLabels[i].free();
  menuFooterLabel.free();

  for (size_t i = 0; i < instructionsLabels.size(); ++i) {
    if (instructionsLabels[i] != NULL)
      delete instructionsLabels[i];
  }
  instructionsLabels.clear();

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

  menuLabels[0].init("1. START GAME", 3, glm::vec2(204.f, 220.f),
                     glm::vec4(1.f), &texProgram);
  menuLabels[1].init("2. INSTRUCTIONS", 3, glm::vec2(186.f, 265.f),
                     glm::vec4(1.f), &texProgram);
  menuLabels[2].init("3. CREDITS", 3, glm::vec2(231.f, 310.f), glm::vec4(1.f),
                     &texProgram);
  menuLabels[3].init("4. EXIT", 3, glm::vec2(258.f, 355.f), glm::vec4(1.f),
                     &texProgram);
  menuFooterLabel.init("UP/DOWN: SELECT   ENTER: OK   ESC: EXIT", 2,
                       glm::vec2(90.f, 452.f), glm::vec4(0.8f, 0.8f, 0.9f, 1.f),
                       &texProgram);

  // 2. Pantalla d'Instruccions
  auto addInst = [&](const std::string &txt, int scale, float x, float y,
                     const glm::vec4 &col) {
    TextLabel *lbl = new TextLabel();
    lbl->init(txt, scale, glm::vec2(x, y), col, &texProgram);
    instructionsLabels.push_back(lbl);
  };
  addInst("INSTRUCTIONS & CONTROLS", 3, 100.f, 22.f,
          glm::vec4(1.f, 0.9f, 0.1f, 1.f));
  addInst("CONTROLS:", 2, 40.f, 68.f, glm::vec4(0.3f, 0.9f, 1.f, 1.f));
  addInst("  MOVE LEFT / RIGHT : ARROWS (OR A / D)", 2, 40.f, 90.f,
          glm::vec4(1.f));
  addInst("  JUMP              : UP ARROW (OR W)", 2, 40.f, 110.f,
          glm::vec4(1.f));
  addInst("  DROP BOMB         : SPACE / X KEY", 2, 40.f, 130.f,
          glm::vec4(1.f));
  addInst("  PAUSE GAME        : P / ESC", 2, 40.f, 150.f, glm::vec4(1.f));

  addInst("GAMEPLAY OBJECTIVE:", 2, 40.f, 185.f,
          glm::vec4(0.3f, 0.9f, 1.f, 1.f));
  addInst("  - DROP BOMBS TO CLEAR ENEMIES AND OBSTACLES", 2, 40.f, 207.f,
          glm::vec4(1.f));
  addInst("  - JUMP ON TOP OF BOMBS AS SOLID PLATFORMS!", 2, 40.f, 227.f,
          glm::vec4(1.f));
  addInst("  - REACH THE EXIT DOOR TO COMPLETE THE LEVEL", 2, 40.f, 247.f,
          glm::vec4(1.f));
  addInst("  - BEAT THE CLOCK: YOU HAVE 2 MINUTES (120s)!", 2, 40.f, 267.f,
          glm::vec4(1.f));

  addInst("CHEATS (TRUCS DE PROVA):", 2, 40.f, 302.f,
          glm::vec4(0.3f, 0.9f, 1.f, 1.f));
  addInst("  - G  : TOGGLE GOD MODE (INVINCIBILITY)", 2, 40.f, 324.f,
          glm::vec4(1.f, 0.9f, 0.2f, 1.f));
  addInst("  - K  : OPEN EXIT DOOR IMMEDIATELY", 2, 40.f, 344.f,
          glm::vec4(1.f, 0.9f, 0.2f, 1.f));
  addInst("  - H: ADD LEVEL LIFE / HEART (+1 HP)", 2, 40.f, 364.f,
          glm::vec4(1.f, 0.9f, 0.2f, 1.f));
  addInst("  - B: ADD ONE BOMB UP", 2, 40.f, 384.f,
          glm::vec4(1.f, 0.9f, 0.2f, 1.f));
  addInst("  - N: GO TO THE NEXT LEVEL", 2, 40.f, 404.f,
          glm::vec4(1.f, 0.9f, 0.2f, 1.f));
  addInst("  - 1-5: DIRECT LEVEL WARP (STAGES 1 TO 5)", 2, 40.f, 424.f,
          glm::vec4(1.f, 0.9f, 0.2f, 1.f));

  addInst("[ PRESS ENTER OR ESC TO RETURN TO MENU ]", 2, 85.f, 452.f,
          glm::vec4(1.f, 0.9f, 0.2f, 1.f));

  // 3. Pantalla de Crèdits
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
  addCred("[ PRESS ENTER OR ESC TO RETURN TO MENU ]", 2, 85.f, 442.f,
          glm::vec4(1.f, 0.9f, 0.2f, 1.f));

  // 4. Pausa
  if (pauseTex.loadFromFile("images/Menu/pause.png",
                            TEXTURE_PIXEL_FORMAT_RGBA)) {
    pauseSprite = Sprite::createSprite(
        glm::vec2(208.f, 217.f), glm::vec2(1.f, 1.f), &pauseTex, &texProgram);
    pauseSprite->setPosition(
        glm::vec2((float(SCREEN_WIDTH) - 208.f) * 0.5f, 45.f));
  }
  pauseTitleLabel.init("GAME PAUSED", 3, glm::vec2(210.f, 280.f),
                       glm::vec4(1.f, 0.9f, 0.1f, 1.f), &texProgram);
  pauseResumeLabel.init("PRESS P OR ESC TO RESUME", 2, glm::vec2(165.f, 335.f),
                        glm::vec4(1.f), &texProgram);
  pauseMenuLabel.init("PRESS M TO RETURN TO MAIN MENU", 2,
                      glm::vec2(130.f, 370.f), glm::vec4(0.4f, 0.9f, 1.f, 1.f),
                      &texProgram);

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
  timeOverMsgLabel.init("TIME OVER!", 3, glm::vec2(225.f, 275.f),
                        glm::vec4(1.f, 0.3f, 0.2f, 1.f), &texProgram);
  gameOverMenuLabel.init("PRESS ENTER OR ESC FOR MAIN MENU", 2,
                         glm::vec2(120.f, 350.f),
                         glm::vec4(1.f, 0.9f, 0.2f, 1.f), &texProgram);

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
  winCongratsLabel.init("CONGRATULATIONS!", 3, glm::vec2(165.f, 240.f),
                        glm::vec4(1.f, 0.9f, 0.1f, 1.f), &texProgram);
  winSubLabel.init("YOU HAVE CLEARED ALL 5 WORLDS!", 2, glm::vec2(135.f, 295.f),
                   glm::vec4(1.f), &texProgram);
  winPeaceLabel.init("PEACE HAS RETURNED TO BOMBER ISLAND!", 2,
                     glm::vec2(95.f, 330.f), glm::vec4(0.4f, 0.9f, 1.f, 1.f),
                     &texProgram);
  winPromptLabel.init("PRESS ENTER OR ESC FOR MAIN MENU", 2,
                      glm::vec2(120.f, 390.f), glm::vec4(1.f, 0.9f, 0.2f, 1.f),
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
  if (enemiesHUDSprite != NULL) {
    delete enemiesHUDSprite;
    enemiesHUDSprite = NULL;
  }
  lastDisplayedEnemies = -1;

  initMenuScreens();

  externalLives = 3;
  startLevel(1, false);
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

  BitmapText::createTexture(godModeOnTex, "GOD MODE ON");
  godModeOnSprite = Sprite::createSprite(
      glm::vec2(float(godModeOnTex.width()), float(godModeOnTex.height())),
      glm::vec2(1.f, 1.f), &godModeOnTex, &texProgram);
  BitmapText::createTexture(godModeOffTex, "GOD MODE OFF");
  godModeOffSprite = Sprite::createSprite(
      glm::vec2(float(godModeOffTex.width()), float(godModeOffTex.height())),
      glm::vec2(1.f, 1.f), &godModeOffTex, &texProgram);
}

void Scene::update(int deltaTime) {
  currentTime += deltaTime;

  if (godModeMsgTime > 0) {
    godModeMsgTime -= deltaTime;
    if (godModeMsgTime < 0)
      godModeMsgTime = 0;
  }

  switch (gameState) {
  case MAIN_MENU:
    if (isKeyJustPressed(GLFW_KEY_UP) || isKeyJustPressed(GLFW_KEY_W)) {
      menuOption = (menuOption + 3) % 4;
    } else if (isKeyJustPressed(GLFW_KEY_DOWN) ||
               isKeyJustPressed(GLFW_KEY_S)) {
      menuOption = (menuOption + 1) % 4;
    } else if (isKeyJustPressed(GLFW_KEY_ENTER) ||
               isKeyJustPressed(GLFW_KEY_SPACE)) {
      if (menuOption == 0) {
        externalLives = 3;
        startLevel(1, false);
        gameState = PLAYING;
      } else if (menuOption == 1) {
        gameState = INSTRUCTIONS;
      } else if (menuOption == 2) {
        gameState = CREDITS;
      } else if (menuOption == 3) {
        Game::instance().exit();
      }
    } else if (isKeyJustPressed(GLFW_KEY_1)) {
      externalLives = 3;
      startLevel(1, false);
      gameState = PLAYING;
    } else if (isKeyJustPressed(GLFW_KEY_2)) {
      gameState = INSTRUCTIONS;
    } else if (isKeyJustPressed(GLFW_KEY_3)) {
      gameState = CREDITS;
    } else if (isKeyJustPressed(GLFW_KEY_4) ||
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
        if (door != NULL)
          door->setOpen(true);
      }
      // Cheats de testeig per provar el sistema de power-ups i vides
      if (isKeyJustPressed(GLFW_KEY_H)) {
        player->addInternalLife(); // Afegeix vida interna (cor/escut)
      }
      if (isKeyJustPressed(GLFW_KEY_B)) {
        player->addMaxBomb(); // Incrementa bombes simultànies
      }
      if (isKeyJustPressed(GLFW_KEY_SPACE) || isKeyJustPressed(GLFW_KEY_X)) {
        placeBomb();
      }

      // Actualitzar porta i comprovar col·lisió per passar de nivell
      if (door != NULL && !player->isDying() && !player->isDead()) {
        door->update(deltaTime);
        if (door->isOpen() &&
            door->checkCollision(player->getPosition(), glm::ivec2(32, 32))) {
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
                      1, map, texProgram);
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
                      glm::ivec2(32, 32))) {
                bombs[b]->explode();
              }
            }
          }

          // Col·lisió amb el jugador: l'explosió fa dany
          if (explosions[i]->checkCollision(player->getPosition(),
                                            glm::ivec2(32, 32))) {
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
                                         glm::ivec2(32, 32))) {
            player->hit();
            break;
          }
        }
      }

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
        float targetX = float(SCREEN_X + pPos.x) + 16.f - viewWidth * 0.5f;
        float targetY = float(SCREEN_Y + pPos.y) + 16.f - viewHeight * 0.5f;

        float lerpSpeed = 0.12f;
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

  for (int i = 0; i < 4; ++i) {
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
    float cursorY = 217.f + float(menuOption) * 45.f;
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

bool Scene::isKeyJustPressed(int key)
// aquesta funció la utilitzarem per a que detecti que ha sigut presionada un
// unic cop fins que la deixes de presionar
{
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

    if (BitmapText::createTexture(timerTex, buf, 2)) {
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

    if (BitmapText::createTexture(levelHUDTex, buf, 2)) {
      levelHUDSprite = Sprite::createSprite(
          glm::vec2(float(levelHUDTex.width()), float(levelHUDTex.height())),
          glm::vec2(1.f, 1.f), &levelHUDTex, &texProgram);
    }
  }
}

void Scene::updateEnemiesHUD() {
  int count = countAliveEnemies();
  if (count != lastDisplayedEnemies) {
    lastDisplayedEnemies = count;
    char buf[32];
    if (count > 0)
      snprintf(buf, sizeof(buf), "E:%d", count);
    else
      snprintf(buf, sizeof(buf), "OPEN!");

    if (enemiesHUDSprite != NULL) {
      delete enemiesHUDSprite;
      enemiesHUDSprite = NULL;
    }

    if (BitmapText::createTexture(enemiesHUDTex, buf, 2)) {
      enemiesHUDSprite = Sprite::createSprite(
          glm::vec2(float(enemiesHUDTex.width()), float(enemiesHUDTex.height())),
          glm::vec2(1.f, 1.f), &enemiesHUDTex, &texProgram);
    }
  }
}

void Scene::renderHUD() {
  if (player == NULL || lifeIcon == NULL)
    return;

  const float hudY = camY + 8.f;
  const float hudPad = 8.f;

  // 1. Contador extern: nombre de vides generals (cares de Bomberman)
  for (int i = 0; i < externalLives; ++i) {
    float x = camX + hudPad + float(i) * 18.f;
    lifeIcon->setPosition(glm::vec2(x, hudY));
    lifeIcon->render();
  }

  const float livesRight = hudPad + float(externalLives) * 18.f;

  // 2. Vida de nivell: cors pixel-art (1 de base; augmenta amb power-ups com cors)
  float heartsRight = livesRight;
  if (heartIcon != NULL) {
    int hp = player->getInternalLives();
    for (int j = 0; j < hp; ++j) {
      float hx = camX + livesRight + 6.f + float(j) * 14.f;
      heartIcon->setPosition(glm::vec2(hx, hudY + 2.f));
      heartIcon->render();
    }
    if (hp > 0)
      heartsRight = livesRight + 6.f + float(hp) * 14.f;
  }

  // 3. Temporitzador a la cantonada dreta
  updateTimerHUD();
  float timerLeft = camX + viewWidth - hudPad;
  if (timerSprite != NULL) {
    float timerW = float(timerTex.width());
    timerLeft = camX + viewWidth - timerW - hudPad;
    timerSprite->setPosition(glm::vec2(timerLeft, hudY));

    int totalSecs = (levelTimeLeft + 999) / 1000;
    if (totalSecs <= 30)
      texProgram.setUniform4f("color", 1.0f, 0.2f, 0.2f,
                              1.0f); // Vermell <= 30s
    else
      texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

    timerSprite->render();
    texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
  }

  // 4. Indicador de nivell centrat entre els cors i el temporitzador
  updateLevelHUD();
  float levelRight = heartsRight;
  if (levelHUDSprite != NULL) {
    float hudW = float(levelHUDTex.width());
    float gapLeft = camX + heartsRight + 4.f;
    float gapRight = timerLeft - 4.f;
    float x = gapLeft + (gapRight - gapLeft - hudW) * 0.35f;
    if (x < gapLeft)
      x = gapLeft;
    levelHUDSprite->setPosition(glm::vec2(x, hudY));
    texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
    levelHUDSprite->render();
    levelRight = x - camX + hudW;
  }

  // 5. Indicador d'enemics restants / porta oberta
  updateEnemiesHUD();
  if (enemiesHUDSprite != NULL) {
    float eW = float(enemiesHUDTex.width());
    float gapLeft = camX + levelRight + 8.f;
    float gapRight = timerLeft - 6.f;
    float x = gapLeft + (gapRight - gapLeft - eW) * 0.5f;
    if (x < gapLeft)
      x = gapLeft;
    enemiesHUDSprite->setPosition(glm::vec2(x, hudY));
    if (lastDisplayedEnemies == 0)
      texProgram.setUniform4f("color", 0.2f, 1.0f, 0.3f, 1.0f); // Verd quan la porta s'obre
    else
      texProgram.setUniform4f("color", 1.0f, 0.85f, 0.2f, 1.0f); // Groc daurat
    enemiesHUDSprite->render();
    texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
  }

  // 5. Missatge temporal de god mode si s'ha activat / desactivat
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
  bomb->updatePlayerOverlap(playerPos, glm::ivec2(32, 32));
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
