#ifndef _PLAYER_INCLUDE
#define _PLAYER_INCLUDE


#include <vector>
#include "Sprite.h"
#include "TileMap.h"
#include "Bomb.h"


// Player is basically a Sprite that represents the player. As such it has
// all properties it needs to track its movement, jumping, and collisions.


#define PLAYER_WIDTH 16
#define PLAYER_HEIGHT 16

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
	glm::ivec2 getSize() const { return glm::ivec2(PLAYER_WIDTH, PLAYER_HEIGHT); }
	bool isOnJumpThroughPlatform() const;
	int getFacingDirection() const { return facingDirection; }
	int getInternalLives() const { return internalLives; }
	void setInternalLives(int l) { internalLives = l; }
	void addInternalLife() { internalLives++; }
	int getLives() const { return internalLives; } // Alias

	int getMaxBombs() const { return maxBombs; }
	void setMaxBombs(int b) { maxBombs = b; }
	void addMaxBomb() { maxBombs++; }

	int getBombRadius() const { return bombRadius; }
	void setBombRadius(int r) { bombRadius = r; }
	void addBombRadius() { bombRadius++; }

	float getSpeed() const { return moveSpeed; }
	void setSpeed(float s) { moveSpeed = s; }
	void addSpeed() { moveSpeed = std::min(moveSpeed + 0.25f, 2.1f); }

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
	int bombRadius;    // Radi de les explosions (augmenta amb Fire Up)
	float moveSpeed;   // Velocitat horitzontal (augmenta amb Patins)
	float moveAccumX;
	int invulnerableTime;
	float fallSpeed;
	int dropDownTimer;
	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;
	const std::vector<Bomb*> *bombs;

	bool stepFall(int step);
};


#endif // _PLAYER_INCLUDE


