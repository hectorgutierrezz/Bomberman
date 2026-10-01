#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"
#include "BitmapText.h"


#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 25

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
	menuSprite = NULL;
	instructionsSprite = NULL;
	creditsSprite = NULL;
	pauseSprite = NULL;
	gameOverSprite = NULL;
	winSprite = NULL;
	lifeIcon = NULL;
	godModeOnSprite = NULL;
	godModeOffSprite = NULL;
	godModeMsgTime = 0;
	godModeMsgActive = false;
	gameState = PLAYING;
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


void Scene::init()
{
	initShaders();
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
	map = TileMap::createTileMap("levels/level01.txt", glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
	player->setTileMap(map);
	player->setBombs(&bombs);
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

	// Inicialment la càmera es centra en la posició inicial del jugador
	viewWidth  = CAM_VIEW_W;
	viewHeight = CAM_VIEW_H;
	glm::ivec2 initPos = player->getPosition();
	camX = float(SCREEN_X + initPos.x) + 16.f - viewWidth  * 0.5f;
	camY = float(SCREEN_Y + initPos.y) + 16.f - viewHeight * 0.5f;

	// Clamping inicial als límits del mapa
	float mapRight  = float(SCREEN_X) + float(map->getMapWidth())  - viewWidth;
	float mapBottom = float(SCREEN_Y) + float(map->getMapHeight()) - viewHeight;
	if(camX < 0.f)        camX = 0.f;
	if(camY < 0.f)        camY = 0.f;
	if(camX > mapRight)   camX = mapRight;
	if(camY > mapBottom)  camY = mapBottom;

	projection = glm::ortho(camX, camX + viewWidth, camY + viewHeight, camY);
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
				if(isKeyJustPressed(GLFW_KEY_G)) {
					player->toggleGodMode();
					showGodModeMessage(player->isGodMode());
				}
				if(isKeyJustPressed(GLFW_KEY_SPACE) || isKeyJustPressed(GLFW_KEY_X)) {
					placeBomb();
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
	
	switch (gameState) {
        case MAIN_MENU:
            renderMainMenu(); 
            break;
        case INSTRUCTIONS:
            renderInstructions(); 
            break;
        case CREDITS:
            renderCredits(); 
            break;
        case PLAYING:
            map->render();
            renderBombs();
            renderExplosions();
            player->render();
            //renderEnemies();
            renderHUD();
            break;
        case PAUSED:
            map->render();
            player->render();
            renderPauseOverlay();
            break;
        case GAME_OVER:
            renderGameOver();
            break;
        case WIN:
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
void Scene::renderHUD()
{
	if(player == NULL || lifeIcon == NULL)
		return;

	int lives = player->getLives();
	for(int i = 0; i < lives; ++i)
	{
		// Fixat a la cantonada superior-esquerra de la càmera
		float x = camX + 8.f + float(i) * 18.f;
		float y = camY + 8.f;
		lifeIcon->setPosition(glm::vec2(x, y));
		lifeIcon->render();
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
	int dir = player->getFacingDirection(); // -1 per esquerra, +1 per dreta

	// Col·locar la bomba davant del jugador (un bloc de 32 px endavant)
	int targetX = playerPos.x + dir * 32;
	int targetY = playerPos.y;

	int bombX = int(round(float(targetX) / 16.f)) * 16;
	int bombY = int(round(float(targetY) / 16.f)) * 16;

	// Comprovar si a la casella de davant hi ha un bloc sòlid del mapa (paret)
	bool frontBlocked = map->isSolidTile(glm::ivec2(bombX + 8, bombY + 16)) ||
	                    map->isSolidTile(glm::ivec2(bombX + 24, bombY + 16));

	if(frontBlocked)
	{
		// Si al davant hi ha paret, es col·loca a la casella actual del jugador
		bombX = int(round(float(playerPos.x) / 16.f)) * 16;
		bombY = int(round(float(playerPos.y) / 16.f)) * 16;
	}

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
