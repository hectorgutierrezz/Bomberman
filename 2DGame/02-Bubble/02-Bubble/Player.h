#ifndef _PLAYER_INCLUDE
#define _PLAYER_INCLUDE


#include <vector>
#include "Sprite.h"
#include "TileMap.h"
#include "Bomb.h"


// Player is basically a Sprite that represents the player. As such it has
// all properties it needs to track its movement, jumping, and collisions.


class Player
{
public:
	Player();
	~Player();

public:
	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();
	
	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);
	void setBombs(const std::vector<Bomb*> *bombsList);
	glm::ivec2 getPosition() const { return posPlayer; }
	int getFacingDirection() const { return facingDirection; }
	int getLives() const { return lives; }
	int getMaxBombs() const { return maxBombs; }
	bool isInvulnerable() const { return godMode || invulnerableTime > 0; }
	void toggleGodMode() { godMode = !godMode; }
	bool isGodMode() const { return godMode; }
	void hit();
	
private:
	bool bJumping;
	bool godMode;
	int facingDirection;
	glm::ivec2 tileMapDispl, posPlayer;
	int jumpAngle, startY;
	int lives;
	int maxBombs;
	int invulnerableTime;
	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;
	const std::vector<Bomb*> *bombs;

};


#endif // _PLAYER_INCLUDE


