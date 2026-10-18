#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE

#include <glm/glm.hpp>
#include <vector>
#include "Bomb.h"
#include "Sprite.h"
#include "Texture.h"
#include "TileMap.h"

enum EnemyType {
	ENEMY_HORIZONTAL,
	ENEMY_VERTICAL,
	ENEMY_STATIC
};

class Enemy
{
public:
	Enemy();
	~Enemy();

	void init(EnemyType type, const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime, const glm::ivec2 &playerPos);
	void render();

	void setTileMap(TileMap *tileMap);
	void setBombs(const std::vector<Bomb*> *bombsList);
	void setPosition(const glm::vec2 &pos);
	glm::ivec2 getPosition() const { return posEnemy; }
	glm::ivec2 getSize() const { return glm::ivec2(16, 16); }

	EnemyType getType() const { return type; }
	bool isAlive() const { return alive && !bDying; }
	bool isFinished() const { return !alive; } // Ja es pot eliminar de la llista

	void kill();
	bool checkCollision(const glm::ivec2 &targetPos, const glm::ivec2 &targetSize) const;

	bool shouldSpawnExplosion() const { return triggerExplosion; }
	void clearExplosionTrigger() { triggerExplosion = false; }

private:
	void updateHorizontal(int deltaTime);
	void updateVertical(int deltaTime);
	void updateStatic(int deltaTime, const glm::ivec2 &playerPos);
	bool wouldFall(int nextX) const;
	bool overlapsBomb(const glm::ivec2 &position) const;

private:
	static Texture spritesheet;
	static bool textureLoaded;

	EnemyType type;
	bool alive;
	bool bDying;
	int dyingTime;
	int moveDir;        // +1 / -1
	int bobTimer;       // Per al bobbing de l'enemic estàtic
	int spawnOriginY;   // Posició vertical d'origen per a patrulla i bobbing
	float moveAccum;    // Acumulador subpíxel per moviment suau
	bool triggerExplosion; // Flag per generar explosió al morir
	bool exploded;         // Evitar múltiples explosions

	glm::ivec2 tileMapDispl;
	glm::ivec2 posEnemy;
	Sprite *sprite;
	TileMap *map;
	const std::vector<Bomb*> *bombs;
};

#endif // _ENEMY_INCLUDE
