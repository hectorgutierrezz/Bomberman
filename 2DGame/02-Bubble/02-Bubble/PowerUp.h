#ifndef _POWERUP_INCLUDE
#define _POWERUP_INCLUDE

#include <glm/glm.hpp>
#include "Sprite.h"
#include "Texture.h"
#include "ShaderProgram.h"

class Player;

enum PowerUpType
{
	POWERUP_BOMB_UP = 0, // +1 bomba simultània
	POWERUP_FIRE_UP = 1, // +1 radi d'explosió
	POWERUP_HEART   = 2, // +1 cor / vida de nivell
	POWERUP_SKATES  = 3  // Patins (augmenta velocitat de moviment)
};

class PowerUp
{
public:
	PowerUp(PowerUpType type, const glm::vec2 &pos);
	~PowerUp();

	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();

	bool checkCollision(const glm::ivec2 &playerPos, const glm::ivec2 &playerSize) const;
	void applyEffect(Player *player);

	bool isActive() const { return active; }
	void setActive(bool a) { active = a; }
	PowerUpType getType() const { return type; }
	const glm::vec2 &getPosition() const { return position; }

	static const char *getTypeName(PowerUpType type);

private:
	static Texture powerUpTexture;
	static bool textureLoaded;

	PowerUpType type;
	glm::vec2 position;
	glm::ivec2 tileMapDispl;
	Sprite *sprite;
	bool active;
	float bobTime;
};

#endif // _POWERUP_INCLUDE
