#ifndef _STEPPABLE_BLOCK_INCLUDE
#define _STEPPABLE_BLOCK_INCLUDE

#include <glm/glm.hpp>
#include "TileMap.h"

enum SteppableBlockState {
	STEPPABLE_STATE_INTACT = 0,
	STEPPABLE_STATE_CRACKED = 1,
	STEPPABLE_STATE_DESTROYED = 2
};

class SteppableBlock {
public:
	SteppableBlock(int tileX, int tileY);
	~SteppableBlock();

	// Updates contact and respawn timers.
	// Returns true if visual state changed so TileMap mesh should be updated.
	bool update(int deltaTime, const glm::ivec2 &playerPos, const glm::ivec2 &playerSize, bool playerOnGround, TileMap *map);

	void reset(TileMap *map);

	int getTileX() const { return tileX; }
	int getTileY() const { return tileY; }
	SteppableBlockState getState() const { return state; }
	int getContactTimer() const { return contactTimer; }
	int getRespawnTimer() const { return respawnTimer; }

private:
	int tileX;
	int tileY;
	SteppableBlockState state;
	int contactTimer; // ms: acumula fins a 350 ms (0,35s) mentre el jugador el trepitja
	int unsteppedTimer; // ms: manté l'aspecte agrietat 250 ms després de deixar-lo
	int respawnTimer; // ms: compte enrere de 3000 ms (3s) un cop destruït
};

#endif // _STEPPABLE_BLOCK_INCLUDE
