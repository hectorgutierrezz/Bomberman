#ifndef _BOSS_INCLUDE
#define _BOSS_INCLUDE

#include <glm/glm.hpp>
#include <vector>
#include "Enemy.h"
#include "Player.h"
#include "Sprite.h"
#include "Texture.h"
#include "TileMap.h"

enum BossState {
	BOSS_CHASE,
	BOSS_SHOOT,
	BOSS_SUMMON,
	BOSS_PAUSE,
	BOSS_HURT,
	BOSS_DYING,
	BOSS_DEAD
};

enum BossAnim {
	BOSS_ANIM_FLY,
	BOSS_ANIM_ATTACK,
	BOSS_ANIM_HURT,
	BOSS_ANIM_DYING
};

struct BossProjectile {
	glm::vec2 pos;
	glm::vec2 vel;
	int lifeTime;
	bool active;
};

class Boss {
public:
	Boss();
	~Boss();

	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime, const glm::ivec2 &playerPos);
	void render();

	void setTileMap(TileMap *tileMap) { map = tileMap; }
	void setPosition(const glm::vec2 &pos);
	glm::ivec2 getPosition() const { return glm::ivec2(int(posBoss.x), int(posBoss.y)); }
	glm::ivec2 getSize() const { return glm::ivec2(32, 32); }

	int getHealth() const { return health; }
	int getMaxHealth() const { return maxHealth; }
	bool isAlive() const { return alive && !bDying; }
	bool isDying() const { return bDying; }
	bool isDead() const { return !alive; }
	bool isInvulnerable() const { return bInvulnerable; }

	bool hit();
	void kill();
	bool checkCollision(const glm::ivec2 &targetPos, const glm::ivec2 &targetSize) const;
	bool checkProjectilesPlayerCollision(Player *player);

	bool shouldSpawnMinion() const { return spawnMinionPending; }
	void clearMinionTrigger() { spawnMinionPending = false; }
	EnemyType getMinionType() const { return minionTypeToSpawn; }
	glm::vec2 getMinionSpawnPos() const { return minionSpawnPos; }

	bool shouldSpawnExplosion() const { return triggerDeathExplosion; }
	void clearExplosionTrigger() { triggerDeathExplosion = false; }
	glm::vec2 getExplosionPos() const { return deathExplosionPos; }

private:
	void updateAI(int deltaTime, const glm::ivec2 &playerPos);
	void updateProjectiles(int deltaTime);
	void shootProjectile(const glm::ivec2 &playerPos);
	void changeState(BossState newState);

private:
	Texture spritesheet;
	Sprite *sprite;
	Sprite *projSprite;
	TileMap *map;

	glm::ivec2 tileMapDispl;
	glm::vec2 posBoss;
	int health;
	int maxHealth;

	bool alive;
	bool bDying;
	int dyingTimer;
	bool bInvulnerable;
	int invulnerableTimer;

	BossState state;
	int stateTimer;
	bool facingRight;
	float bobAngle;

	// Trets i projectils
	bool hasFiredInCurrentAttack;
	bool hasFiredMidChase;
	std::vector<BossProjectile> projectiles;

	// Invocació de sequaços enemics
	bool spawnMinionPending;
	bool hasSummonedInCurrentState;
	EnemyType minionTypeToSpawn;
	glm::vec2 minionSpawnPos;
	int summonCooldown;

	// Efectes d'explosió en morir
	bool triggerDeathExplosion;
	glm::vec2 deathExplosionPos;
	int deathExplosionTimer;
};

#endif // _BOSS_INCLUDE
