#include "Enemy.h"
#include <cmath>
#include <iostream>

Texture Enemy::spritesheet;
bool Enemy::textureLoaded = false;

namespace {
enum EnemyAnimations {
	MOVE,
	DIE
};
}

Enemy::Enemy()
	: type(ENEMY_HORIZONTAL), alive(true), bDying(false), dyingTime(0),
	  moveDir(1), bobTimer(0), spawnOriginY(0), moveAccum(0.f),
	  triggerExplosion(false), exploded(false), tileMapDispl(0),
	  posEnemy(0), sprite(NULL), map(NULL), bombs(NULL)
{
}

Enemy::~Enemy()
{
	if (sprite != NULL)
		delete sprite;
}

void Enemy::init(EnemyType enemyType, const glm::ivec2 &tileMapPos,
                 ShaderProgram &shaderProgram)
{
	type = enemyType;
	tileMapDispl = tileMapPos;
	alive = true;
	bDying = false;
	dyingTime = 0;
	moveDir = 1;
	bobTimer = 0;
	spawnOriginY = posEnemy.y;
	moveAccum = 0.f;
	triggerExplosion = false;
	exploded = false;

	if (sprite != NULL) {
		delete sprite;
		sprite = NULL;
	}

	if (!textureLoaded) {
		if (!spritesheet.loadFromFile(
		        "images/Sprites/Original/Color/Characters/bomber.png",
		        TEXTURE_PIXEL_FORMAT_RGBA)) {
			std::cout << "Error loading enemy spritesheet" << std::endl;
			return;
		}
		textureLoaded = true;
	}

	const glm::vec2 frameSize(17.f / 253.f, 17.f / 632.f);
	sprite = Sprite::createSprite(glm::vec2(16.f, 16.f), frameSize,
	                              &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(2);

	const auto frame = [](int x, int y) {
		return glm::vec2(float(x) / 253.f, float(y) / 632.f);
	};

	switch (type) {
	case ENEMY_HORIZONTAL: {
		// El groc caminant patrulla horitzontalment.
		const int row = 10 * 17;
		sprite->setAnimationSpeed(MOVE, 6);
		sprite->addKeyframe(MOVE, frame(1, row));
		sprite->addKeyframe(MOVE, frame(18, row));
		sprite->addKeyframe(MOVE, frame(35, row));
		sprite->addKeyframe(MOVE, frame(18, row));

		sprite->setAnimationSpeed(DIE, 8);
		sprite->addKeyframe(DIE, frame(86, row));
		sprite->addKeyframe(DIE, frame(104, row));
		break;
	}
	case ENEMY_VERTICAL: {
		// El peix patrulla verticalment.
		const int row = 9 * 17;
		moveDir = -1;
		sprite->setAnimationSpeed(MOVE, 5);
		sprite->addKeyframe(MOVE, frame(30, row));
		sprite->addKeyframe(MOVE, frame(44, row));
		sprite->addKeyframe(MOVE, frame(59, row));
		sprite->addKeyframe(MOVE, frame(44, row));

		sprite->setAnimationSpeed(DIE, 8);
		sprite->addKeyframe(DIE, frame(74, row));
		sprite->addKeyframe(DIE, frame(91, row));
		sprite->addKeyframe(DIE, frame(108, row));
		break;
	}
	case ENEMY_STATIC: {
		const int row = 11 * 17;
		moveDir = 0;
		sprite->setAnimationSpeed(MOVE, 3);
		sprite->addKeyframe(MOVE, frame(2, row));
		sprite->addKeyframe(MOVE, frame(20, row));

		sprite->setAnimationSpeed(DIE, 8);
		sprite->addKeyframe(DIE, frame(38, row));
		sprite->addKeyframe(DIE, frame(56, row));
		sprite->addKeyframe(DIE, frame(74, row));
		break;
	}
	}

	sprite->changeAnimation(MOVE);
	sprite->setFlippedHorizontally(type == ENEMY_HORIZONTAL && moveDir > 0);
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

bool Enemy::checkCollision(const glm::ivec2 &targetPos,
                           const glm::ivec2 &targetSize) const
{
	if (!isAlive())
		return false;

	const int left = posEnemy.x + 2;
	const int right = posEnemy.x + 14;
	const int top = posEnemy.y + 2;
	const int bottom = posEnemy.y + 14;
	const int targetLeft = targetPos.x + 1;
	const int targetRight = targetPos.x + targetSize.x - 1;
	const int targetTop = targetPos.y + 1;
	const int targetBottom = targetPos.y + targetSize.y - 1;

	return targetRight > left && targetLeft < right &&
	       targetBottom > top && targetTop < bottom;
}

bool Enemy::overlapsBomb(const glm::ivec2 &position) const
{
	if (bombs == NULL)
		return false;

	for (size_t i = 0; i < bombs->size(); ++i) {
		const Bomb *bomb = (*bombs)[i];
		if (bomb == NULL || !bomb->isActive())
			continue;

		const glm::vec2 &bombPos = bomb->getPosition();
		if (position.x + 14 > int(bombPos.x) + 2 &&
		    position.x + 2 < int(bombPos.x) + 14 &&
		    position.y + 14 > int(bombPos.y) + 2 &&
		    position.y + 2 < int(bombPos.y) + 14)
			return true;
	}
	return false;
}

bool Enemy::wouldFall(int nextX) const
{
	if (map == NULL)
		return false;
	const int footY = posEnemy.y + 17;
	const int probeX = moveDir > 0 ? nextX + 14 : nextX + 2;
	return !map->isSolidTile(glm::ivec2(probeX, footY));
}

void Enemy::updateHorizontal(int deltaTime)
{
	if (map == NULL)
		return;

	posEnemy.y += 2;
	int correctedY = posEnemy.y;
	const bool onGround =
	    map->collisionMoveDown(posEnemy, glm::ivec2(16, 16), &correctedY);
	if (onGround)
		posEnemy.y = correctedY;

	moveAccum += 22.f * float(deltaTime) / 1000.f;
	int step = int(moveAccum);
	if (step == 0)
		return;
	if (step > 3)
		step = 3;
	moveAccum -= float(step);

	glm::ivec2 nextPos = posEnemy;
	nextPos.x += moveDir * step;
	const bool hitWall =
	    moveDir < 0 ? map->collisionMoveLeft(nextPos, glm::ivec2(16, 16))
	                : map->collisionMoveRight(nextPos, glm::ivec2(16, 16));
	const bool dropLedge = onGround && wouldFall(nextPos.x);
	if (hitWall || overlapsBomb(nextPos) || dropLedge) {
		moveDir = -moveDir;
		if (sprite != NULL)
			sprite->setFlippedHorizontally(moveDir > 0);
		return;
	}
	posEnemy.x = nextPos.x;
}

void Enemy::updateVertical(int deltaTime)
{
	moveAccum += 18.f * float(deltaTime) / 1000.f;
	int step = int(moveAccum);
	if (step == 0)
		return;
	if (step > 3)
		step = 3;
	moveAccum -= float(step);

	const int maxPatrolDistance = 56;
	glm::ivec2 nextPos = posEnemy;
	nextPos.y += moveDir * step;
	bool hit = false;

	if (map != NULL) {
		int correctedY = nextPos.y;
		hit = moveDir < 0
		          ? map->collisionMoveUp(nextPos, glm::ivec2(16, 16), &correctedY)
		          : map->collisionMoveDown(nextPos, glm::ivec2(16, 16), &correctedY);
		if (hit)
			nextPos.y = correctedY;
	}

	if (moveDir < 0 && nextPos.y <= spawnOriginY - maxPatrolDistance) {
		nextPos.y = spawnOriginY - maxPatrolDistance;
		hit = true;
	} else if (moveDir > 0 &&
	           nextPos.y >= spawnOriginY + maxPatrolDistance) {
		nextPos.y = spawnOriginY + maxPatrolDistance;
		hit = true;
	}

	if (!hit && overlapsBomb(nextPos))
		hit = true;

	if (hit)
		moveDir = -moveDir;
	else
		posEnemy.y = nextPos.y;
}

void Enemy::updateStatic(int deltaTime, const glm::ivec2 &playerPos)
{
	// Assegurar contacte amb terra/plataforma en caiguda si ha aparegut a l'aire
	if (map != NULL) {
		posEnemy.y += 2;
		int probeY = posEnemy.y;
		if (map->collisionMoveDown(posEnemy, glm::ivec2(16, 16), &probeY)) {
			posEnemy.y = probeY;
			spawnOriginY = probeY;
		}
	}
	bobTimer += deltaTime;
	const int bobOffset = int(round(1.5f * sin(float(bobTimer) / 250.f)));
	posEnemy.y = spawnOriginY + bobOffset;

	if (sprite != NULL) {
		sprite->setFlippedHorizontally(playerPos.x > posEnemy.x);
		const float distance =
		    glm::length(glm::vec2(playerPos - posEnemy));
		sprite->setAnimationSpeed(MOVE, distance < 96.f ? 6 : 3);
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
	} else {
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
	}

	if (sprite != NULL) {
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x),
		                              float(tileMapDispl.y + posEnemy.y)));
	}
}

void Enemy::render()
{
	if (alive && sprite != NULL)
		sprite->render();
}
