#include "Enemy.h"
#include <cmath>
#include <iostream>

Texture Enemy::spritesheet;
bool Enemy::textureLoaded = false;

enum EnemyAnims {
	MOVE, DIE
};

Enemy::Enemy()
{
	sprite = NULL;
	map = NULL;
	bombs = NULL;
	alive = true;
	bDying = false;
	dyingTime = 0;
	moveDir = 1;
	bobTimer = 0;
	spawnOriginY = 0;
	moveAccum = 0.f;
	triggerExplosion = false;
	exploded = false;
	type = ENEMY_HORIZONTAL;
	posEnemy = glm::ivec2(0);
	tileMapDispl = glm::ivec2(0);
}

Enemy::~Enemy()
{
	if (sprite != NULL)
		delete sprite;
}

void Enemy::init(EnemyType enemyType, const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	type = enemyType;
	tileMapDispl = tileMapPos;
	alive = true;
	bDying = false;
	dyingTime = 0;
	moveDir = 1;
	bobTimer = 0;
	moveAccum = 0.f;
	triggerExplosion = false;
	exploded = false;

	if (sprite != NULL) {
		delete sprite;
		sprite = NULL;
	}

	if (!textureLoaded) {
		if (!spritesheet.loadFromFile("images/Sprites/Original/Color/Characters/bomber.png",
		                              TEXTURE_PIXEL_FORMAT_RGBA)) {
			std::cout << "Error loading enemy spritesheet" << std::endl;
		}
		textureLoaded = true;
	}

	glm::vec2 sizeInUV = glm::vec2(17.f / 253.f, 17.f / 632.f);
	sprite = Sprite::createSprite(glm::ivec2(32, 32), sizeInUV, &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(2);

	auto uvPx = [](int pxX, int pxY) {
		return glm::vec2(float(pxX) / 253.f, float(pxY) / 632.f);
	};

	switch (type) {
	case ENEMY_HORIZONTAL:
	{
		// Enemic groc caminant (fila 10): patrulla horitzontal
		int rY = 10 * 17;
		sprite->setAnimationSpeed(MOVE, 6);
		sprite->addKeyframe(MOVE, uvPx(1, rY));
		sprite->addKeyframe(MOVE, uvPx(18, rY));
		sprite->addKeyframe(MOVE, uvPx(35, rY));
		sprite->addKeyframe(MOVE, uvPx(18, rY));

		sprite->setAnimationSpeed(DIE, 8);
		sprite->addKeyframe(DIE, uvPx(86, rY));
		sprite->addKeyframe(DIE, uvPx(104, rY));
		break;
	}
	case ENEMY_VERTICAL:
	{
		// Peix (fila 9): patrulla vertical + mort
		int rY = 9 * 17;
		moveDir = -1; // Comença flotant amunt
		sprite->setAnimationSpeed(MOVE, 5);
		sprite->addKeyframe(MOVE, uvPx(30, rY));
		sprite->addKeyframe(MOVE, uvPx(44, rY));
		sprite->addKeyframe(MOVE, uvPx(59, rY));
		sprite->addKeyframe(MOVE, uvPx(44, rY));

		sprite->setAnimationSpeed(DIE, 8);
		sprite->addKeyframe(DIE, uvPx(74, rY));
		sprite->addKeyframe(DIE, uvPx(91, rY));
		sprite->addKeyframe(DIE, uvPx(108, rY));
		break;
	}
	case ENEMY_STATIC:
	{
		// Globus verd Valcom (fila 11, y = 187): idle vigilant + mort
		int rY = 11 * 17;
		moveDir = 0;
		sprite->setAnimationSpeed(MOVE, 3);
		sprite->addKeyframe(MOVE, uvPx(2, rY));
		sprite->addKeyframe(MOVE, uvPx(20, rY));

		sprite->setAnimationSpeed(DIE, 8);
		sprite->addKeyframe(DIE, uvPx(38, rY));
		sprite->addKeyframe(DIE, uvPx(56, rY));
		sprite->addKeyframe(DIE, uvPx(74, rY));
		break;
	}
	}

	sprite->changeAnimation(MOVE);
	if (type == ENEMY_HORIZONTAL)
		sprite->setFlippedHorizontally(moveDir > 0);
	else
		sprite->setFlippedHorizontally(false);

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x),
	                              float(tileMapDispl.y + posEnemy.y)));
}

void Enemy::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Enemy::setBombs(const std::vector<Bomb*> *bombsList)
{
	bombs = bombsList;
}

void Enemy::setPosition(const glm::vec2 &pos)
{
	posEnemy = glm::ivec2(int(pos.x), int(pos.y));
	spawnOriginY = posEnemy.y;
	if (sprite != NULL) {
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x),
		                              float(tileMapDispl.y + posEnemy.y)));
	}
}

void Enemy::kill()
{
	if (!alive || bDying)
		return;
	bDying = true;
	dyingTime = 500;
	if (type == ENEMY_STATIC && !exploded) {
		triggerExplosion = true;
		exploded = true;
	}
	if (sprite != NULL)
		sprite->changeAnimation(DIE);
}

bool Enemy::checkCollision(const glm::ivec2 &targetPos, const glm::ivec2 &targetSize) const
{
	if (!alive || bDying)
		return false;

	int eLeft = posEnemy.x + 5;
	int eRight = posEnemy.x + 27;
	int eTop = posEnemy.y + 5;
	int eBottom = posEnemy.y + 27;

	int tLeft = targetPos.x + 4;
	int tRight = targetPos.x + targetSize.x - 4;
	int tTop = targetPos.y + 4;
	int tBottom = targetPos.y + targetSize.y - 4;

	bool overlapX = (tRight > eLeft) && (tLeft < eRight);
	bool overlapY = (tBottom > eTop) && (tTop < eBottom);
	return overlapX && overlapY;
}

bool Enemy::wouldFall(int nextX) const
{
	if (map == NULL)
		return false;
	// Comprova el tile sota els peus en la direcció de moviment
	int footY = posEnemy.y + 32 + 1;
	int probeX = (moveDir > 0) ? (nextX + 28) : (nextX + 4);
	return !map->isSolidTile(glm::ivec2(probeX, footY));
}

void Enemy::updateHorizontal(int deltaTime)
{
	if (map == NULL)
		return;

	// Gravetat suau cap a terra
	posEnemy.y += 2;
	bool onGround = false;
	int correctedY = posEnemy.y;
	if (map->collisionMoveDown(posEnemy, glm::ivec2(32, 32), &correctedY)) {
		posEnemy.y = correctedY;
		onGround = true;
	}

	const float speed = 35.0f; // px per segon
	float dt = float(deltaTime) / 1000.f;
	moveAccum += speed * dt;
	int step = int(moveAccum);
	if (step == 0)
		return;
	if (step > 4) step = 4;
	moveAccum -= float(step);

	posEnemy.x += moveDir * step;

	bool hitWall = false;
	if (moveDir < 0)
		hitWall = map->collisionMoveLeft(posEnemy, glm::ivec2(32, 32));
	else
		hitWall = map->collisionMoveRight(posEnemy, glm::ivec2(32, 32));

	bool hitBomb = false;
	if (bombs != NULL) {
		for (size_t b = 0; b < bombs->size(); ++b) {
			if ((*bombs)[b] != NULL && (*bombs)[b]->isActive()) {
				glm::ivec2 bPos = glm::ivec2((*bombs)[b]->getPosition());
				if (posEnemy.x + 28 > bPos.x + 4 && posEnemy.x + 4 < bPos.x + 28 &&
				    posEnemy.y + 28 > bPos.y + 4 && posEnemy.y + 4 < bPos.y + 28) {
					hitBomb = true;
					break;
				}
			}
		}
	}

	// Comprova si cauria pel marge d'una plataforma (només si està tocant a terra)
	bool dropLedge = onGround && wouldFall(posEnemy.x);

	if (hitWall || hitBomb || dropLedge) {
		posEnemy.x -= moveDir * step;
		moveDir = -moveDir;
		if (sprite != NULL)
			sprite->setFlippedHorizontally(moveDir > 0);
	}
}

void Enemy::updateVertical(int deltaTime)
{
	const float speed = 30.0f; // px per segon (moviment suau de volador)
	float dt = float(deltaTime) / 1000.f;
	moveAccum += speed * dt;
	int step = int(moveAccum);
	if (step == 0)
		return;
	if (step > 4) step = 4;
	moveAccum -= float(step);

	posEnemy.y += moveDir * step;

	bool hit = false;
	if (moveDir < 0) {
		int probeY = posEnemy.y;
		hit = (map != NULL && map->collisionMoveUp(posEnemy, glm::ivec2(32, 32), &probeY));
		if (hit)
			posEnemy.y = probeY;
	} else {
		int probeY = posEnemy.y;
		hit = (map != NULL && map->collisionMoveDown(posEnemy, glm::ivec2(32, 32), &probeY));
		if (hit)
			posEnemy.y = probeY;
	}

	// Límit de patrulla vertical des de la posició inicial (±56 px = ~3.5 tiles)
	const int maxPatrolDistance = 56;
	if (moveDir < 0 && posEnemy.y <= spawnOriginY - maxPatrolDistance) {
		posEnemy.y = spawnOriginY - maxPatrolDistance;
		hit = true;
	} else if (moveDir > 0 && posEnemy.y >= spawnOriginY + maxPatrolDistance) {
		posEnemy.y = spawnOriginY + maxPatrolDistance;
		hit = true;
	}

	// Col·lisió amb bombes actives
	if (!hit && bombs != NULL) {
		for (size_t b = 0; b < bombs->size(); ++b) {
			if ((*bombs)[b] != NULL && (*bombs)[b]->isActive()) {
				glm::ivec2 bPos = glm::ivec2((*bombs)[b]->getPosition());
				if (posEnemy.x + 28 > bPos.x + 4 && posEnemy.x + 4 < bPos.x + 28 &&
				    posEnemy.y + 28 > bPos.y + 4 && posEnemy.y + 4 < bPos.y + 28) {
					hit = true;
					posEnemy.y -= moveDir * step;
					break;
				}
			}
		}
	}

	if (hit) {
		moveDir = -moveDir;
	}
}

void Enemy::updateStatic(int deltaTime, const glm::ivec2 &playerPos)
{
	// Assegurar contacte amb terra/plataforma en caiguda si ha aparegut a l'aire
	if (map != NULL) {
		posEnemy.y += 2;
		int probeY = posEnemy.y;
		if (map->collisionMoveDown(posEnemy, glm::ivec2(32, 32), &probeY)) {
			posEnemy.y = probeY;
			spawnOriginY = probeY;
		}
	}

	bobTimer += deltaTime;
	float t = float(bobTimer) / 250.f;
	int bobOffset = int(round(1.5f * sin(t)));
	posEnemy.y = spawnOriginY + bobOffset;

	// Girar la mirada cap al jugador (el sprite base a bomber.png mira cap a l'esquerra)
	if (sprite != NULL) {
		bool faceRight = (playerPos.x > posEnemy.x);
		sprite->setFlippedHorizontally(faceRight);

		// Si el jugador s'apropa a menys de 96px, l'animació idle s'accelera (vigilant)
		float distToPlayer = glm::length(glm::vec2(playerPos - posEnemy));
		if (distToPlayer < 96.f) {
			sprite->setAnimationSpeed(MOVE, 6);
		} else {
			sprite->setAnimationSpeed(MOVE, 3);
		}
	}
}

void Enemy::update(int deltaTime, const glm::ivec2 &playerPos)
{
	if (!alive)
		return;

	if (sprite != NULL)
		sprite->update(deltaTime);

	if (bDying) {
		dyingTime -= deltaTime;
		if (dyingTime <= 0) {
			alive = false;
			bDying = false;
		}
		if (sprite != NULL) {
			sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x),
			                              float(tileMapDispl.y + posEnemy.y)));
		}
		return;
	}

	switch (type) {
	case ENEMY_HORIZONTAL:
		updateHorizontal(deltaTime);
		break;
	case ENEMY_VERTICAL:
		updateVertical(deltaTime);
		break;
	case ENEMY_STATIC:
		updateStatic(deltaTime, playerPos);
		break;
	}

	if (sprite != NULL) {
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x),
		                              float(tileMapDispl.y + posEnemy.y)));
	}
}

void Enemy::render()
{
	if (!alive || sprite == NULL)
		return;
	sprite->render();
}
