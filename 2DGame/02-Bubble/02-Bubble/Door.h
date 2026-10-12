#ifndef _DOOR_INCLUDE
#define _DOOR_INCLUDE

#include <glm/glm.hpp>
#include "Sprite.h"
#include "Texture.h"
#include "ShaderProgram.h"

class Door
{
public:
	Door();
	~Door();

	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();

	void setPosition(const glm::vec2 &pos);
	glm::vec2 getPosition() const { return position; }

	void setOpen(bool open);
	bool isOpen() const { return open; }

	bool checkCollision(const glm::ivec2 &playerPos, const glm::ivec2 &playerSize) const;

private:
	glm::vec2 position;
	glm::ivec2 tileMapDispl;
	Texture texture;
	Sprite *sprite;
	bool open;
};

#endif // _DOOR_INCLUDE
