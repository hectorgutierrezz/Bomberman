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
	int getInternalLives() const { return internalLives; }
	void setInternalLives(int l) { internalLives = l; }
	void addInternalLife() { internalLives++; }
	int getLives() const { return internalLives; } // Alias

	int getMaxBombs() const { return maxBombs; }
	void setMaxBombs(int b) { maxBombs = b; }
	void addMaxBomb() { maxBombs++; }

	void resetPowerUps();
	void respawn(const glm::vec2 &pos);

	bool isInvulnerable() const { return godMode || invulnerableTime > 0 || bDying || bDead; }
	void toggleGodMode() { godMode = !godMode; }
	bool isGodMode() const { return godMode; }

	bool isDying() const { return bDying; }
	bool isDead() const { return bDead; }
	void startDying();

	// Retorna true si el comptador intern arriba a 0 (mort), false si sobreviu
	bool hit();
	
private:
	bool bJumping;
	bool bDying;
	bool bDead;
	int dyingTime;
	bool godMode;
	int facingDirection;
	glm::ivec2 tileMapDispl, posPlayer;
	int jumpAngle, startY;
	int internalLives; // 1 de base per pantalla; augmenta amb power-ups
	int maxBombs;      // Es manté a través dels re-runs
	int invulnerableTime;
	float fallSpeed;
	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;
	const std::vector<Bomb*> *bombs;

	bool stepFall(int step);
};


#endif // _PLAYER_INCLUDE


