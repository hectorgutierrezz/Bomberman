#ifndef _BOMB_INCLUDE
#define _BOMB_INCLUDE

#include <glm/glm.hpp>
#include "Sprite.h"
#include "Texture.h"

class Bomb
{
public:
	Bomb();
	~Bomb();

	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();

	void setPosition(const glm::vec2 &pos);
	bool isActive() const { return active; }
	bool shouldExplode() const { return bExploded; }
	void explode();

	// Gestió de solapament i solidesa amb el jugador
	void updatePlayerOverlap(const glm::ivec2 &playerPos, const glm::ivec2 &playerSize);
	bool isSolidForPlayer() const { return active && !playerInside; }
	bool isPlayerInside() const { return playerInside; }

	// Col·lisions físiques
	bool collisionDown(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const;
	bool collisionLeft(const glm::ivec2 &pos, const glm::ivec2 &size) const;
	bool collisionRight(const glm::ivec2 &pos, const glm::ivec2 &size) const;

	bool isSolidOnTop(const glm::ivec2 &playerPos, const glm::ivec2 &playerSize) const;
	int getTop() const { return int(position.y); }
	const glm::vec2 &getPosition() const { return position; }

private:
	bool checkOverlap(const glm::ivec2 &pPos, const glm::ivec2 &pSize) const;

private:
	static Texture bombTexture;
	static bool textureLoaded;

	Sprite *sprite;
	glm::ivec2 tileMapDispl;
	glm::vec2 position;
	float fuseTime;
	bool active;
	bool bExploded;
	bool playerInside;
};

#endif // _BOMB_INCLUDE
