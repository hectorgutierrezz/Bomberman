#ifndef _EXPLOSION_INCLUDE
#define _EXPLOSION_INCLUDE

#include <vector>
#include <glm/glm.hpp>
#include "Sprite.h"
#include "Texture.h"
#include "TileMap.h"

class Explosion
{
public:
	Explosion();
	~Explosion();

	void init(const glm::vec2 &centerPos, const glm::ivec2 &tileMapPos, int range, TileMap *map, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();

	bool isActive() const { return active; }
	bool checkCollision(const glm::ivec2 &targetPos, const glm::ivec2 &targetSize) const;
	const std::vector<glm::vec2> &getCells() const { return cells; }

private:
	static Texture explosionTexture;
	static bool textureLoaded;

	bool active;
	float lifetime;
	glm::ivec2 tileMapDispl;
	std::vector<glm::vec2> cells;
	std::vector<Sprite*> sprites;
};

#endif // _EXPLOSION_INCLUDE
