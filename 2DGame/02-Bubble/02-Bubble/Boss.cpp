#include "Boss.h"
#include "SoundManager.h"
#include <cmath>
#include <cstdlib>
#include <iostream>

Boss::Boss() {
	sprite = NULL;
	projSprite = NULL;
	map = NULL;
	tileMapDispl = glm::ivec2(0, 0);
	posBoss = glm::vec2(0.f, 0.f);

	maxHealth = 3;
	health = maxHealth;
	alive = true;
	bDying = false;
	dyingTimer = 0;
	bInvulnerable = false;
	invulnerableTimer = 0;

	state = BOSS_CHASE;
	stateTimer = 3000;
	facingRight = false;
	bobAngle = 0.0f;

	hasFiredInCurrentAttack = false;
	hasFiredMidChase = false;

	spawnMinionPending = false;
	hasSummonedInCurrentState = false;
	minionTypeToSpawn = ENEMY_HORIZONTAL;
	minionSpawnPos = glm::vec2(0.f);
	summonCooldown = 6000; // Primer summon possible als 6 segons

	triggerDeathExplosion = false;
	deathExplosionPos = glm::vec2(0.f);
	deathExplosionTimer = 0;
}

Boss::~Boss() {
	if (sprite != NULL) {
		delete sprite;
		sprite = NULL;
	}
	if (projSprite != NULL) {
		delete projSprite;
		projSprite = NULL;
	}
}

void Boss::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram) {
	tileMapDispl = tileMapPos;

	if (!spritesheet.loadFromFile("images/boss.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
		std::cerr << "Error: No s'ha pogut carregar images/boss.png" << std::endl;
		return;
	}
	spritesheet.setWrapS(GL_CLAMP_TO_EDGE);
	spritesheet.setWrapT(GL_CLAMP_TO_EDGE);
	spritesheet.setMinFilter(GL_NEAREST);
	spritesheet.setMagFilter(GL_NEAREST);

	// 1. Sprite principal del Boss: 32x32 px per frame a la meitat superior (full de 128x64 px)
	glm::vec2 quadSize(32.f, 32.f);
	glm::vec2 sizeInSpritesheet(32.f / 128.f, 32.f / 64.f); // (0.25f, 0.5f)
	sprite = Sprite::createSprite(quadSize, sizeInSpritesheet, &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(4);

	auto frameUV = [](int idx) {
		return glm::vec2(float(idx) * 0.25f, 0.0f);
	};

	// FLY / CHASE: aleteig continu d'ales (Frames 0 i 1)
	sprite->setAnimationSpeed(BOSS_ANIM_FLY, 6);
	sprite->addKeyframe(BOSS_ANIM_FLY, frameUV(0));
	sprite->addKeyframe(BOSS_ANIM_FLY, frameUV(1));

	// ATTACK / SHOOT: braços amunt i rugit de tret (Frame 2)
	sprite->setAnimationSpeed(BOSS_ANIM_ATTACK, 5);
	sprite->addKeyframe(BOSS_ANIM_ATTACK, frameUV(2));

	// HURT: ulls en X i expressió de dolor (Frame 3)
	sprite->setAnimationSpeed(BOSS_ANIM_HURT, 4);
	sprite->addKeyframe(BOSS_ANIM_HURT, frameUV(3));

	// DYING: parpelleig dolor / rugit (Frames 3 i 2)
	sprite->setAnimationSpeed(BOSS_ANIM_DYING, 6);
	sprite->addKeyframe(BOSS_ANIM_DYING, frameUV(3));
	sprite->addKeyframe(BOSS_ANIM_DYING, frameUV(2));

	sprite->changeAnimation(BOSS_ANIM_FLY);
	sprite->setFlippedHorizontally(false);

	// 2. Sprite de raig / projectil d'energia dirigit (16x16 px a y=32)
	glm::vec2 projQuadSize(16.f, 16.f);
	glm::vec2 projUVSize(16.f / 128.f, 16.f / 64.f); // (0.125f, 0.25f)
	projSprite = Sprite::createSprite(projQuadSize, projUVSize, &spritesheet, &shaderProgram);
	projSprite->setNumberAnimations(1);
	projSprite->setAnimationSpeed(0, 8);
	projSprite->addKeyframe(0, glm::vec2(0.000f, 0.5f));
	projSprite->addKeyframe(0, glm::vec2(0.125f, 0.5f));
	projSprite->addKeyframe(0, glm::vec2(0.250f, 0.5f));
	projSprite->addKeyframe(0, glm::vec2(0.375f, 0.5f));
	projSprite->changeAnimation(0);

	changeState(BOSS_CHASE);
}

void Boss::setPosition(const glm::vec2 &pos) {
	posBoss = pos;
	if (sprite != NULL) {
		sprite->setPosition(glm::vec2(float(tileMapDispl.x) + posBoss.x, float(tileMapDispl.y) + posBoss.y));
	}
}

void Boss::changeState(BossState newState) {
	state = newState;
	switch (state) {
	case BOSS_CHASE:
		stateTimer = 3000; // Persegueix durant ~3 segons
		hasFiredMidChase = false;
		if (sprite != NULL)
			sprite->changeAnimation(BOSS_ANIM_FLY);
		break;
	case BOSS_SHOOT:
		stateTimer = 800; // 800ms de preparació i disparament de raig dirigit
		hasFiredInCurrentAttack = false;
		if (sprite != NULL)
			sprite->changeAnimation(BOSS_ANIM_ATTACK);
		break;
	case BOSS_SUMMON:
		stateTimer = 900; // 900ms d'invocació amb braços amunt
		hasSummonedInCurrentState = false;
		if (sprite != NULL)
			sprite->changeAnimation(BOSS_ANIM_ATTACK);
		break;
	case BOSS_PAUSE:
		stateTimer = 1000; // Espera / descansa durant 1 segon
		if (sprite != NULL)
			sprite->changeAnimation(BOSS_ANIM_FLY);
		break;
	case BOSS_HURT:
		stateTimer = 600; // 600ms de reacció al dany
		if (sprite != NULL)
			sprite->changeAnimation(BOSS_ANIM_HURT);
		break;
	case BOSS_DYING:
		dyingTimer = 2000;
		if (sprite != NULL)
			sprite->changeAnimation(BOSS_ANIM_DYING);
		SoundManager::instance().playSound(SFX_BOMBERMAN_KILLED);
		break;
	case BOSS_DEAD:
		break;
	}
}

void Boss::update(int deltaTime, const glm::ivec2 &playerPos) {
	if (state == BOSS_DEAD)
		return;

	// Gestió d'invulnerabilitat temporal post-impacte
	if (bInvulnerable) {
		invulnerableTimer -= deltaTime;
		if (invulnerableTimer <= 0) {
			bInvulnerable = false;
			invulnerableTimer = 0;
		}
	}

	// Mort o actualització d'IA
	if (bDying) {
		dyingTimer -= deltaTime;
		deathExplosionTimer += deltaTime;
		if (deathExplosionTimer >= 220) {
			deathExplosionTimer = 0;
			triggerDeathExplosion = true;
			float offsetX = float((rand() % 24) - 4);
			float offsetY = float((rand() % 24) - 4);
			deathExplosionPos = posBoss + glm::vec2(offsetX, offsetY);
			SoundManager::instance().playSound(SFX_BOMB_EXPLOSION);
		}
		if (dyingTimer <= 0) {
			bDying = false;
			alive = false;
			changeState(BOSS_DEAD);
		}
	} else {
		updateAI(deltaTime, playerPos);
	}

	// Actualitzar projectils actius
	updateProjectiles(deltaTime);

	// Actualitzar sprite
	if (sprite != NULL) {
		sprite->setFlippedHorizontally(facingRight);
		sprite->update(deltaTime);
		sprite->setPosition(glm::vec2(float(tileMapDispl.x) + posBoss.x, float(tileMapDispl.y) + posBoss.y));
	}
	if (projSprite != NULL) {
		projSprite->update(deltaTime);
	}
}

void Boss::updateAI(int deltaTime, const glm::ivec2 &playerPos) {
	float dtFactor = float(deltaTime) / 16.666f;

	if (summonCooldown > 0)
		summonCooldown -= deltaTime;

	// Límit de l'arena (deixem marge de 2 tiles de paret i terra)
	float minX = 32.f;
	float maxX = (map != NULL ? float(map->getMapWidth()) : 384.f) - 32.f - 32.f;
	float minY = 32.f;
	float maxY = (map != NULL ? float(map->getMapHeight()) : 192.f) - 32.f - 32.f;

	stateTimer -= deltaTime;

	switch (state) {
	case BOSS_CHASE: {
		// Persecució moderada del jugador
		float targetX = float(playerPos.x) - 8.f;
		float targetY = float(playerPos.y) - 8.f;
		float dx = targetX - posBoss.x;
		float dy = targetY - posBoss.y;

		// Orientació cap al jugador
		if (dx > 4.f)
			facingRight = true;
		else if (dx < -4.f)
			facingRight = false;

		// Efecte suau de levitació
		bobAngle += 0.04f * dtFactor;
		float bob = std::sin(bobAngle) * 0.20f * dtFactor;

		float speedX = 0.65f;
		float speedY = 0.45f;

		if (std::abs(dx) > 3.f)
			posBoss.x += (dx > 0.f ? 1.f : -1.f) * speedX * dtFactor;

		if (std::abs(dy) > 3.f)
			posBoss.y += (dy > 0.f ? 1.f : -1.f) * speedY * dtFactor + bob;
		else
			posBoss.y += bob;

		// Disparament a mig camí de la persecució (així dispara amb més freqüència)
		if (stateTimer <= 1500 && !hasFiredMidChase) {
			shootProjectile(playerPos);
			hasFiredMidChase = true;
		}

		// Mantenir dins els límits de l'arena
		if (posBoss.x < minX) posBoss.x = minX;
		if (posBoss.x > maxX) posBoss.x = maxX;
		if (posBoss.y < minY) posBoss.y = minY;
		if (posBoss.y > maxY) posBoss.y = maxY;

		if (stateTimer <= 0) {
			// En finalitzar la persecució: si toca invocar esbirros, ho fa; si no, dispara
			if (summonCooldown <= 0) {
				changeState(BOSS_SUMMON);
			} else {
				changeState(BOSS_SHOOT);
			}
		}
		break;
	}

	case BOSS_SHOOT: {
		// El cap s'atura breument, alça els braços i dispara un raig dirigit al jugador
		bobAngle += 0.02f * dtFactor;
		posBoss.y += std::sin(bobAngle) * 0.12f * dtFactor;

		float dx = float(playerPos.x) - (posBoss.x + 8.f);
		if (dx > 4.f)
			facingRight = true;
		else if (dx < -4.f)
			facingRight = false;

		if (stateTimer <= 400 && !hasFiredInCurrentAttack) {
			shootProjectile(playerPos);
			hasFiredInCurrentAttack = true;
		}

		if (stateTimer <= 0) {
			changeState(BOSS_PAUSE);
		}
		break;
	}

	case BOSS_SUMMON: {
		// El cap invoca un enemic bàsic (ocasionalment, cada ~15 segons)
		bobAngle += 0.02f * dtFactor;
		posBoss.y += std::sin(bobAngle) * 0.12f * dtFactor;

		float dx = float(playerPos.x) - (posBoss.x + 8.f);
		if (dx > 4.f)
			facingRight = true;
		else if (dx < -4.f)
			facingRight = false;

		if (stateTimer <= 450 && !hasSummonedInCurrentState) {
			spawnMinionPending = true;
			hasSummonedInCurrentState = true;
			summonCooldown = 15000; // Recàrrega de 15 segons

			minionTypeToSpawn = (rand() % 2 == 0) ? ENEMY_HORIZONTAL : ENEMY_VERTICAL;

			float spawnX = facingRight ? (posBoss.x + 32.f) : (posBoss.x - 20.f);
			if (spawnX < 48.f) spawnX = 48.f;
			if (spawnX > 320.f) spawnX = 320.f;

			if (minionTypeToSpawn == ENEMY_HORIZONTAL) {
				minionSpawnPos = glm::vec2(spawnX, 144.f); // Sobre el terra
			} else {
				minionSpawnPos = glm::vec2(spawnX, posBoss.y); // Enemic flotant
			}

			SoundManager::instance().playSound(SFX_BOMB_LAY);
		}

		if (stateTimer <= 0) {
			changeState(BOSS_PAUSE);
		}
		break;
	}

	case BOSS_PAUSE: {
		// Pausa de descans immòbil (finestra d'atac pel jugador)
		bobAngle += 0.03f * dtFactor;
		float bob = std::sin(bobAngle) * 0.15f * dtFactor;
		posBoss.y += bob;
		if (posBoss.y < minY) posBoss.y = minY;
		if (posBoss.y > maxY) posBoss.y = maxY;

		// Gira cap al jugador mentre espera
		float dx = float(playerPos.x) - (posBoss.x + 8.f);
		if (dx > 4.f)
			facingRight = true;
		else if (dx < -4.f)
			facingRight = false;

		if (stateTimer <= 0) {
			changeState(BOSS_CHASE);
		}
		break;
	}

	case BOSS_HURT: {
		// Retrocedir breument davant l'impacte
		float recoilX = facingRight ? -0.8f : 0.8f;
		posBoss.x += recoilX * dtFactor;
		if (posBoss.x < minX) posBoss.x = minX;
		if (posBoss.x > maxX) posBoss.x = maxX;

		if (stateTimer <= 0) {
			changeState(BOSS_CHASE);
		}
		break;
	}

	default:
		break;
	}
}

void Boss::shootProjectile(const glm::ivec2 &playerPos) {
	glm::vec2 origin = posBoss + glm::vec2(8.f, 8.f);
	glm::vec2 target(float(playerPos.x + 8), float(playerPos.y + 8));

	glm::vec2 dir = target - origin;
	float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
	if (len > 0.001f) {
		dir /= len;
	} else {
		dir = glm::vec2(facingRight ? 1.0f : -1.0f, 0.0f);
	}

	float raySpeed = 1.75f;
	BossProjectile p;
	p.pos = origin;
	p.vel = dir * raySpeed;
	p.lifeTime = 3000; // 3 segons de vol màxim
	p.active = true;

	projectiles.push_back(p);
	SoundManager::instance().playSound(SFX_BOMB_LAY);
}

void Boss::updateProjectiles(int deltaTime) {
	float dtFactor = float(deltaTime) / 16.666f;
	int tileSize = (map != NULL) ? map->getTileSize() : 16;

	for (size_t i = 0; i < projectiles.size(); ++i) {
		if (!projectiles[i].active)
			continue;

		projectiles[i].lifeTime -= deltaTime;
		if (projectiles[i].lifeTime <= 0) {
			projectiles[i].active = false;
			continue;
		}

		projectiles[i].pos += projectiles[i].vel * dtFactor;

		// Col·lisió del projectil amb rajoles sòlides de l'escenari
		if (map != NULL) {
			int tx = int(projectiles[i].pos.x + 8.f) / tileSize;
			int ty = int(projectiles[i].pos.y + 8.f) / tileSize;
			if (map->isTileSolid(tx, ty) && !map->isTileJumpThrough(tx, ty)) {
				projectiles[i].active = false;
				continue;
			}
		}

		// Desaparició fora dels marges de la pantalla
		if (projectiles[i].pos.x < 16.f || projectiles[i].pos.x > 380.f ||
		    projectiles[i].pos.y < 16.f || projectiles[i].pos.y > 190.f) {
			projectiles[i].active = false;
		}
	}
}

bool Boss::checkProjectilesPlayerCollision(Player *player) {
	if (player == NULL || player->isDying() || player->isDead() || player->isInvulnerable())
		return false;

	glm::ivec2 pPos = player->getPosition();
	glm::ivec2 pSize = player->getSize();

	for (size_t i = 0; i < projectiles.size(); ++i) {
		if (!projectiles[i].active)
			continue;

		int px = int(projectiles[i].pos.x) + 3;
		int py = int(projectiles[i].pos.y) + 3;
		int pw = 10;
		int ph = 10;

		if (px < pPos.x + pSize.x && px + pw > pPos.x &&
		    py < pPos.y + pSize.y && py + ph > pPos.y) {
			projectiles[i].active = false;
			player->hit();
			return true;
		}
	}
	return false;
}

bool Boss::hit() {
	if (!alive || bDying || bInvulnerable)
		return false;

	health--;
	bInvulnerable = true;
	invulnerableTimer = 1500; // 1.5 segons d'invulnerabilitat temporal

	if (health <= 0) {
		health = 0;
		bDying = true;
		changeState(BOSS_DYING);
	} else {
		changeState(BOSS_HURT);
		SoundManager::instance().playSound(SFX_BOMBERMAN_KILLED);
	}
	return true;
}

void Boss::kill() {
	if (!alive || bDying)
		return;
	health = 0;
	bDying = true;
	bInvulnerable = false;
	changeState(BOSS_DYING);
}

bool Boss::checkCollision(const glm::ivec2 &targetPos, const glm::ivec2 &targetSize) const {
	if (!alive || bDying || bInvulnerable)
		return false;

	// Caixa de col·lisió centrada de 24x24 px dins el sprite de 32x32 px
	int bx = int(posBoss.x) + 4;
	int by = int(posBoss.y) + 4;
	int bw = 24;
	int bh = 24;

	return (bx < targetPos.x + targetSize.x && bx + bw > targetPos.x &&
	        by < targetPos.y + targetSize.y && by + bh > targetPos.y);
}

void Boss::render() {
	if (state == BOSS_DEAD)
		return;

	// Efecte de parpelleig quan és invulnerable
	if (bInvulnerable && !bDying) {
		if ((invulnerableTimer / 90) % 2 == 0)
			return;
	}

	if (sprite != NULL)
		sprite->render();

	// Renderitzar rajos / projectils actius
	if (projSprite != NULL) {
		for (size_t i = 0; i < projectiles.size(); ++i) {
			if (projectiles[i].active) {
				projSprite->setPosition(glm::vec2(float(tileMapDispl.x) + projectiles[i].pos.x,
				                                  float(tileMapDispl.y) + projectiles[i].pos.y));
				projSprite->render();
			}
		}
	}
}
