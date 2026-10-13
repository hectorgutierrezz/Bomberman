#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE

#include <glm/glm.hpp>
#include <vector>
#include "Sprite.h"
#include "TileMap.h"
#include "Texture.h"
#include "Bomb.h"

enum EnemyType {
	ENEMY_HORIZONTAL, // Patrulla esquerra/dreta sobre el terra o plataformes
	ENEMY_VERTICAL,   // Patrulla amunt/avall en eix vertical (volador)
	ENEMY_STATIC      // Guardià estàtic vigilant amb animació idle
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
	glm::ivec2 getSize() const { return glm::ivec2(32, 32); }

	EnemyType getType() const { return type; }
	bool isAlive() const { return alive && !bDying; }
	bool isFinished() const { return !alive; } // Ja es pot eliminar de la llista

	void kill();
	bool checkCollision(const glm::ivec2 &targetPos, const glm::ivec2 &targetSize) const;

	// Explosió en morir (per a l'enemic estàtic)
	bool shouldSpawnExplosion() const { return triggerExplosion; }
	void clearExplosionTrigger() { triggerExplosion = false; }

private:
	void updateHorizontal(int deltaTime);
	void updateVertical(int deltaTime);
	void updateStatic(int deltaTime, const glm::ivec2 &playerPos);
	bool wouldFall(int nextX) const;

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

	glm::ivec2 tileMapDispl, posEnemy;
	Sprite *sprite;
	TileMap *map;
	const std::vector<Bomb*> *bombs;
};

#endif // _ENEMY_INCLUDE
