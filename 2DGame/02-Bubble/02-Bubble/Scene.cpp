#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"
#include "BitmapText.h"


#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 4
// Y se calcula en startLevel según la altura del mapa (2 tiles por encima del suelo)

// Mida de la finestra de càmera en world pixels (zoom ~2.5x respecte 640x480)
// 256 unitats horitz. → 640/256 = 2.5x, 224 unitats vert. → 480/224 ≈ 2.14x
#define CAM_VIEW_W 256.f
#define CAM_VIEW_H 224.f

#define GOD_MODE_MSG_DURATION 2000

// Límits del món on pot anar la càmera (marge = SCREEN_X + 36 tiles*16px)
// Calculat dinàmicament a init() a partir del mapa

Scene::Scene()
{
	map = NULL;
	player = NULL;
	door = NULL;
	currentLevel = 1;
	menuSprite = NULL;
	instructionsSprite = NULL;
	creditsSprite = NULL;
	pauseSprite = NULL;
	gameOverSprite = NULL;
	winSprite = NULL;
	lifeIcon = NULL;
	timerSprite = NULL;
	levelHUDSprite = NULL;
	godModeOnSprite = NULL;
	godModeOffSprite = NULL;
	godModeMsgTime = 0;
	godModeMsgActive = false;
	levelTimeLeft = 120000;
	lastDisplayedSeconds = -1;
	lastDisplayedLevel = -1;
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
	if(door != NULL)
		delete door;
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
	if(lifeIcon != NULL)
		delete lifeIcon;
	if(timerSprite != NULL)
		delete timerSprite;
	if(levelHUDSprite != NULL)
		delete levelHUDSprite;
	if(godModeOnSprite != NULL)
		delete godModeOnSprite;
	if(godModeOffSprite != NULL)
		delete godModeOffSprite;
	for(size_t i = 0; i < bombs.size(); ++i) {
		if(bombs[i] != NULL)
			delete bombs[i];
	}
	bombs.clear();
	for(size_t i = 0; i < explosions.size(); ++i) {
		if(explosions[i] != NULL)
			delete explosions[i];
	}
	explosions.clear();
}


void Scene::startLevel(int levelNum)
{
	currentLevel = levelNum;
	if(currentLevel < 1) currentLevel = 1;
	if(currentLevel > 5) currentLevel = 5;

	levelTimeLeft = 120000; // 2 minuts (120 segons) per nivell
	lastDisplayedSeconds = -1;
	lastDisplayedLevel = -1;

	if(timerSprite != NULL) {
		delete timerSprite;
		timerSprite = NULL;
	}
	if(levelHUDSprite != NULL) {
		delete levelHUDSprite;
		levelHUDSprite = NULL;
	}

	char levelPath[64];
	snprintf(levelPath, sizeof(levelPath), "levels/level%02d.txt", currentLevel);

	if(map != NULL) {
		delete map;
		map = NULL;
	}
	map = TileMap::createTileMap(levelPath, glm::vec2(SCREEN_X, SCREEN_Y), texProgram);

	// Spawn i porta a 2 tiles per damunt del terra (última fila del mapa)
	int tileSize = map->getTileSize();
	int mapTilesY = map->getMapHeight() / tileSize;
	int spawnTileY = mapTilesY - 3;
	if(spawnTileY < 1) spawnTileY = 1;
	int doorTileX = 30;
	int doorTileY = spawnTileY;

	if(player == NULL) {
		player = new Player();
	}
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * tileSize, spawnTileY * tileSize));
	player->setTileMap(map);
	player->setBombs(&bombs);

	// Inicialitzar / configurar la porta del nivell
	if(door == NULL) {
		door = new Door();
		door->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	}
	door->setPosition(glm::vec2(doorTileX * tileSize, doorTileY * tileSize));
	// De moment no hi ha enemics, així que la porta s'obre directament
	door->setOpen(true);

	for(size_t i = 0; i < bombs.size(); ++i) {
		if(bombs[i] != NULL)
			delete bombs[i];
	}
	bombs.clear();
	for(size_t i = 0; i < explosions.size(); ++i) {
		if(explosions[i] != NULL)
			delete explosions[i];
	}
	explosions.clear();

	// Centrar la càmera al jugador al carregar el nivell
	viewWidth  = CAM_VIEW_W;
	viewHeight = CAM_VIEW_H;
	glm::ivec2 initPos = player->getPosition();
	camX = float(SCREEN_X + initPos.x) + 16.f - viewWidth  * 0.5f;
	camY = float(SCREEN_Y + initPos.y) + 16.f - viewHeight * 0.5f;

	float mapRight  = float(SCREEN_X) + float(map->getMapWidth())  - viewWidth;
	float mapBottom = float(SCREEN_Y) + float(map->getMapHeight()) - viewHeight;
	if(camX < 0.f)        camX = 0.f;
	if(camY < 0.f)        camY = 0.f;
	if(camX > mapRight)   camX = mapRight;
	if(camY > mapBottom)  camY = mapBottom;

	projection = glm::ortho(camX, camX + viewWidth, camY + viewHeight, camY);
}

void Scene::nextLevel()
{
	if(currentLevel < 5) {
		startLevel(currentLevel + 1);
	}
	else {
		gameState = WIN;
	}
}

void Scene::init()
{
	initShaders();

	if(menuSprite != NULL) { delete menuSprite; menuSprite = NULL; }
	if(instructionsSprite != NULL) { delete instructionsSprite; instructionsSprite = NULL; }
	if(creditsSprite != NULL) { delete creditsSprite; creditsSprite = NULL; }
	if(pauseSprite != NULL) { delete pauseSprite; pauseSprite = NULL; }
	if(gameOverSprite != NULL) { delete gameOverSprite; gameOverSprite = NULL; }
	if(winSprite != NULL) { delete winSprite; winSprite = NULL; }
	if(timerSprite != NULL) { delete timerSprite; timerSprite = NULL; }
	if(levelHUDSprite != NULL) { delete levelHUDSprite; levelHUDSprite = NULL; }

	// Carregar les imatges de menús i pantalles
	if (menuTex.loadFromFile("images/Menu/main_menu.png", TEXTURE_PIXEL_FORMAT_RGBA) ||
	    menuTex.loadFromFile("images/Sprites/Original/Color/Intro_Ending_Menus/Game Boy _ GBC - Pocket Bomberman - Miscellaneous - Menus (Color).png", TEXTURE_PIXEL_FORMAT_RGBA))
	{
		menuSprite = Sprite::createSprite(glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT), glm::vec2(1.f, 1.f), &menuTex, &texProgram);
		menuSprite->setNumberAnimations(1);
		menuSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
		menuSprite->changeAnimation(0);
	}

	if (instructionsTex.loadFromFile("images/Menu/instructions.png", TEXTURE_PIXEL_FORMAT_RGBA) ||
	    instructionsTex.loadFromFile("images/Sprites/Original/Color/Intro_Ending_Menus/Game Boy _ GBC - Pocket Bomberman - Miscellaneous - Menus (Color).png", TEXTURE_PIXEL_FORMAT_RGBA))
	{
		instructionsSprite = Sprite::createSprite(glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT), glm::vec2(1.f, 1.f), &instructionsTex, &texProgram);
		instructionsSprite->setNumberAnimations(1);
		instructionsSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
		instructionsSprite->changeAnimation(0);
	}

	if (creditsTex.loadFromFile("images/Menu/credits.png", TEXTURE_PIXEL_FORMAT_RGBA) ||
	    creditsTex.loadFromFile("images/Sprites/Original/Color/Intro_Ending_Menus/Game Boy _ GBC - Pocket Bomberman - Miscellaneous - Menus (Color).png", TEXTURE_PIXEL_FORMAT_RGBA))
	{
		creditsSprite = Sprite::createSprite(glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT), glm::vec2(1.f, 1.f), &creditsTex, &texProgram);
		creditsSprite->setNumberAnimations(1);
		creditsSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
		creditsSprite->changeAnimation(0);
	}

	if (pauseTex.loadFromFile("images/Menu/pause.png", TEXTURE_PIXEL_FORMAT_RGBA) ||
	    pauseTex.loadFromFile("images/Sprites/Original/Color/Intro_Ending_Menus/Game Boy _ GBC - Pocket Bomberman - Miscellaneous - Menus (Color).png", TEXTURE_PIXEL_FORMAT_RGBA))
	{
		pauseSprite = Sprite::createSprite(glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT), glm::vec2(1.f, 1.f), &pauseTex, &texProgram);
		pauseSprite->setNumberAnimations(1);
		pauseSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
		pauseSprite->changeAnimation(0);
	}

	if (gameOverTex.loadFromFile("images/Menu/game_over.png", TEXTURE_PIXEL_FORMAT_RGBA) ||
	    gameOverTex.loadFromFile("images/Sprites/Original/Color/Intro_Ending_Menus/Game Boy _ GBC - Pocket Bomberman - Miscellaneous - Ending (Color).png", TEXTURE_PIXEL_FORMAT_RGBA))
	{
		gameOverSprite = Sprite::createSprite(glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT), glm::vec2(1.f, 1.f), &gameOverTex, &texProgram);
		gameOverSprite->setNumberAnimations(1);
		gameOverSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
		gameOverSprite->changeAnimation(0);
	}

	if (winTex.loadFromFile("images/Menu/win.png", TEXTURE_PIXEL_FORMAT_RGBA) ||
	    winTex.loadFromFile("images/Sprites/Original/Color/Intro_Ending_Menus/Game Boy _ GBC - Pocket Bomberman - Miscellaneous - Ending (Color).png", TEXTURE_PIXEL_FORMAT_RGBA))
	{
		winSprite = Sprite::createSprite(glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT), glm::vec2(1.f, 1.f), &winTex, &texProgram);
		winSprite->setNumberAnimations(1);
		winSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
		winSprite->changeAnimation(0);
	}

	startLevel(1);
	currentTime = 0.0f;

	// Icona de vida per al HUD (mateix spritesheet del jugador)
	hudTex.loadFromFile("images/Sprites/Original/Color/Characters/bomber.png", TEXTURE_PIXEL_FORMAT_RGBA);
	glm::vec2 sizeInUV = glm::vec2(17.f / 253.f, 17.f / 632.f);
	lifeIcon = Sprite::createSprite(glm::ivec2(16, 16), sizeInUV, &hudTex, &texProgram);
	lifeIcon->setNumberAnimations(1);
	lifeIcon->setAnimationSpeed(0, 8);
	lifeIcon->addKeyframe(0, glm::vec2(0.f, 0.f));
	lifeIcon->changeAnimation(0);

	BitmapText::createTexture(godModeOnTex, "GOD MODE ON");
	godModeOnSprite = Sprite::createSprite(
		glm::vec2(float(godModeOnTex.width()), float(godModeOnTex.height())),
		glm::vec2(1.f, 1.f), &godModeOnTex, &texProgram);
	BitmapText::createTexture(godModeOffTex, "GOD MODE OFF");
	godModeOffSprite = Sprite::createSprite(
		glm::vec2(float(godModeOffTex.width()), float(godModeOffTex.height())),
		glm::vec2(1.f, 1.f), &godModeOffTex, &texProgram);
}

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;

	if(godModeMsgTime > 0)
	{
		godModeMsgTime -= deltaTime;
		if(godModeMsgTime < 0)
			godModeMsgTime = 0;
	}

	switch (gameState) {
		case MAIN_MENU:
			if(isKeyJustPressed(GLFW_KEY_1)) {
				startLevel(1);
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
				// Descomptar el temps del nivell
				levelTimeLeft -= deltaTime;
				if(levelTimeLeft <= 0) {
					levelTimeLeft = 0;
					gameState = MAIN_MENU; // Si el temps s'acaba, perds i et torna al menú principal
					break;
				}

				// Tecles de drecera / debug per provar canvi de nivells
				if(isKeyJustPressed(GLFW_KEY_N)) {
					nextLevel();
					break;
				}
				if(isKeyJustPressed(GLFW_KEY_P)) {
					int prev = (currentLevel > 1) ? currentLevel - 1 : 1;
					startLevel(prev);
					break;
				}
				if(isKeyJustPressed(GLFW_KEY_1)) { startLevel(1); break; }
				if(isKeyJustPressed(GLFW_KEY_2)) { startLevel(2); break; }
				if(isKeyJustPressed(GLFW_KEY_3)) { startLevel(3); break; }
				if(isKeyJustPressed(GLFW_KEY_4)) { startLevel(4); break; }
				if(isKeyJustPressed(GLFW_KEY_5)) { startLevel(5); break; }

				if(isKeyJustPressed(GLFW_KEY_G)) {
					player->toggleGodMode();
					showGodModeMessage(player->isGodMode());
				}
				if(isKeyJustPressed(GLFW_KEY_K)) {
					if(door != NULL) door->setOpen(true);
				}
				if(isKeyJustPressed(GLFW_KEY_SPACE) || isKeyJustPressed(GLFW_KEY_X)) {
					placeBomb();
				}

				// Actualitzar porta i comprovar col·lisió per passar de nivell
				if(door != NULL) {
					door->update(deltaTime);
					if(door->isOpen() && door->checkCollision(player->getPosition(), glm::ivec2(32, 32))) {
						nextLevel();
						break;
					}
				}

				// 1. Actualitzar bombes
				for(size_t i = 0; i < bombs.size(); ++i) {
					if(bombs[i] != NULL) {
						bombs[i]->update(deltaTime);
						if(bombs[i]->shouldExplode()) {
							Explosion *exp = new Explosion();
							exp->init(bombs[i]->getPosition(), glm::ivec2(SCREEN_X, SCREEN_Y), 1, map, texProgram);
							explosions.push_back(exp);

							delete bombs[i];
							bombs[i] = NULL;
						}
					}
				}
				for(auto it = bombs.begin(); it != bombs.end(); ) {
					if(*it == NULL)
						it = bombs.erase(it);
					else
						++it;
				}

				// 2. Actualitzar explosions
				for(size_t i = 0; i < explosions.size(); ++i) {
					if(explosions[i] != NULL) {
						explosions[i]->update(deltaTime);

						// Reacció en cadena amb altres bombes
						for(size_t b = 0; b < bombs.size(); ++b) {
							if(bombs[b] != NULL && bombs[b]->isActive()) {
								if(explosions[i]->checkCollision(glm::ivec2(bombs[b]->getPosition()), glm::ivec2(32, 32))) {
									bombs[b]->explode();
								}
							}
						}

						// Col·lisió amb el jugador: la explosió fa dany
						if(explosions[i]->checkCollision(player->getPosition(), glm::ivec2(32, 32))) {
							player->hit();
							if(player->getLives() <= 0)
								gameState = GAME_OVER;
						}

						if(!explosions[i]->isActive()) {
							delete explosions[i];
							explosions[i] = NULL;
						}
					}
				}
				for(auto it = explosions.begin(); it != explosions.end(); ) {
					if(*it == NULL)
						it = explosions.erase(it);
					else
						++it;
				}

				player->update(deltaTime);

				// --- Actualitzar la càmera per seguir el jugador ---
				{
					glm::ivec2 pPos = player->getPosition();
					// Centre del jugador en coordenades de món
					float targetX = float(SCREEN_X + pPos.x) + 16.f - viewWidth  * 0.5f;
					float targetY = float(SCREEN_Y + pPos.y) + 16.f - viewHeight * 0.5f;

					// Interpolació suau (lerp) per evitar moviment brusc
					float lerpSpeed = 0.12f;
					camX += (targetX - camX) * lerpSpeed;
					camY += (targetY - camY) * lerpSpeed;

					// Clamping als límits del mapa
					float mapRight  = float(SCREEN_X) + float(map->getMapWidth())  - viewWidth;
					float mapBottom = float(SCREEN_Y) + float(map->getMapHeight()) - viewHeight;
					if(camX < 0.f)        camX = 0.f;
					if(camY < 0.f)        camY = 0.f;
					if(camX > mapRight)   camX = mapRight;
					if(camY > mapBottom)  camY = mapBottom;

					// Reconstruir la matriu de projecció centrada a la càmera
					projection = glm::ortho(camX, camX + viewWidth, camY + viewHeight, camY);
				}
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
	
	glm::mat4 screenProjection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);

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
            if(door != NULL) door->render();
            renderBombs();
            renderExplosions();
            player->render();
            //renderEnemies();
            renderHUD();
            break;
        case PAUSED:
            map->render();
            if(door != NULL) door->render();
            renderBombs();
            renderExplosions();
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
void Scene::renderExplosions() {
	for(size_t i = 0; i < explosions.size(); ++i) {
		if(explosions[i] != NULL)
			explosions[i]->render();
	}
}
void Scene::updateTimerHUD()
{
	int totalSecs = (levelTimeLeft + 999) / 1000;
	if(totalSecs < 0) totalSecs = 0;

	if(totalSecs != lastDisplayedSeconds)
	{
		lastDisplayedSeconds = totalSecs;
		int mins = totalSecs / 60;
		int secs = totalSecs % 60;

		char buf[32];
		snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);

		if(timerSprite != NULL)
		{
			delete timerSprite;
			timerSprite = NULL;
		}

		if(BitmapText::createTexture(timerTex, buf, 2))
		{
			timerSprite = Sprite::createSprite(
				glm::vec2(float(timerTex.width()), float(timerTex.height())),
				glm::vec2(1.f, 1.f), &timerTex, &texProgram);
		}
	}
}

void Scene::updateLevelHUD()
{
	if(currentLevel != lastDisplayedLevel)
	{
		lastDisplayedLevel = currentLevel;
		char buf[32];
		snprintf(buf, sizeof(buf), "1-%d", currentLevel);

		if(levelHUDSprite != NULL)
		{
			delete levelHUDSprite;
			levelHUDSprite = NULL;
		}

		if(BitmapText::createTexture(levelHUDTex, buf, 2))
		{
			levelHUDSprite = Sprite::createSprite(
				glm::vec2(float(levelHUDTex.width()), float(levelHUDTex.height())),
				glm::vec2(1.f, 1.f), &levelHUDTex, &texProgram);
		}
	}
}

void Scene::renderHUD()
{
	if(player == NULL || lifeIcon == NULL)
		return;

	const float hudY = camY + 8.f;
	const float hudPad = 8.f;
	const float livesRight = hudPad + float(player->getLives()) * 18.f;

	int lives = player->getLives();
	for(int i = 0; i < lives; ++i)
	{
		float x = camX + hudPad + float(i) * 18.f;
		lifeIcon->setPosition(glm::vec2(x, hudY));
		lifeIcon->render();
	}

	// Timer a la dreta; el nivell es centra al forat lliure entre vides i timer
	updateTimerHUD();
	float timerLeft = camX + viewWidth - hudPad;
	if(timerSprite != NULL)
	{
		float timerW = float(timerTex.width());
		timerLeft = camX + viewWidth - timerW - hudPad;
		timerSprite->setPosition(glm::vec2(timerLeft, hudY));

		int totalSecs = (levelTimeLeft + 999) / 1000;
		if(totalSecs <= 30)
			texProgram.setUniform4f("color", 1.0f, 0.2f, 0.2f, 1.0f); // Vermell quan queden 30 segons o menys
		else
			texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

		timerSprite->render();
		texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	}

	updateLevelHUD();
	if(levelHUDSprite != NULL)
	{
		float hudW = float(levelHUDTex.width());
		float gapLeft = camX + livesRight + 4.f;
		float gapRight = timerLeft - 4.f;
		float x = gapLeft + (gapRight - gapLeft - hudW) * 0.5f;
		// Si no hi ha espai al forat, queda just a la dreta de les vides
		if(x < gapLeft)
			x = gapLeft;
		levelHUDSprite->setPosition(glm::vec2(x, hudY));
		texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
		levelHUDSprite->render();
	}


	if(godModeMsgTime > 0)
	{
		Sprite *msgSprite = godModeMsgActive ? godModeOnSprite : godModeOffSprite;
		if(msgSprite != NULL)
		{
			float msgW = godModeMsgActive ? float(godModeOnTex.width()) : float(godModeOffTex.width());
			float x = camX + (viewWidth - msgW) * 0.5f;
			float y = camY + 28.f;
			msgSprite->setPosition(glm::vec2(x, y));

			if(godModeMsgActive)
				texProgram.setUniform4f("color", 1.0f, 1.0f, 0.0f, 1.0f); // groc
			else
				texProgram.setUniform4f("color", 1.0f, 0.15f, 0.15f, 1.0f); // vermell

			msgSprite->render();
			texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
}

void Scene::showGodModeMessage(bool enabled)
{
	godModeMsgActive = enabled;
	godModeMsgTime = GOD_MODE_MSG_DURATION;
}

void Scene::placeBomb()
{
	if(player == NULL || map == NULL)
		return;

	// Limitar bombes actives simultànies (1 de base; power-up Bomb Up incrementarà maxBombs)
	int activeBombs = 0;
	for(size_t i = 0; i < bombs.size(); ++i) {
		if(bombs[i] != NULL && bombs[i]->isActive())
			activeBombs++;
	}
	if(activeBombs >= player->getMaxBombs())
		return;

	glm::ivec2 playerPos = player->getPosition();

	// Col·locar la bomba a la mateixa casella del jugador (directament a sota)
	int bombX = int(round(float(playerPos.x) / 16.f)) * 16;
	int bombY = int(round(float(playerPos.y) / 16.f)) * 16;

	// Evitar posar dues bombes a la mateixa casella
	for(size_t i = 0; i < bombs.size(); ++i) {
		if(bombs[i] != NULL && bombs[i]->isActive()) {
			glm::vec2 bPos = bombs[i]->getPosition();
			if(abs(bPos.x - float(bombX)) < 16.f && abs(bPos.y - float(bombY)) < 16.f)
				return;
		}
	}

	Bomb *bomb = new Bomb();
	bomb->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	bomb->setPosition(glm::vec2(float(bombX), float(bombY)));
	bomb->updatePlayerOverlap(playerPos, glm::ivec2(32, 32));
	bombs.push_back(bomb);
}
