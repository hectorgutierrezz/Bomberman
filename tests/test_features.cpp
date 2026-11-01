#include <iostream>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <map>

#include "SteppableBlock.h"
#include "TileMap.h"
#include "PowerUp.h"
#include "Player.h"

void runSteppableBlockTests() {
	std::cout << "[TEST] 1. SteppableBlock State Machine, Timing & Collisions..." << std::endl;

	TileMap map;
	map.initForTest(36, 28, 16);

	int tileX = 12;
	int tileY = 22;
	int tileSize = 16;
	glm::ivec2 playerSize(16, 16);

	// Setup initial tile as STEPPABLE_INTACT_TILE
	map.setTile(tileX, tileY, TileMap::STEPPABLE_INTACT_TILE, false);

	SteppableBlock block(tileX, tileY);

	assert(block.getTileX() == tileX);
	assert(block.getTileY() == tileY);
	assert(block.getState() == STEPPABLE_STATE_INTACT);
	assert(block.getContactTimer() == 0);
	assert(block.getRespawnTimer() == 0);
	assert(map.getTile(tileX, tileY) == TileMap::STEPPABLE_INTACT_TILE);
	assert(map.isTileSolid(tileX, tileY) == true);
	std::cout << "  ✓ Initial state is INTACT, tile is solid and timers are 0" << std::endl;

	// Positions:
	// footY = posPlayer.y + 16 == tileY * 16 -> posPlayer.y = (tileY - 1) * 16
	glm::ivec2 standingPos(tileX * tileSize, (tileY - 1) * tileSize);
	glm::ivec2 awayPos((tileX + 5) * tileSize, (tileY - 1) * tileSize);

	// Test 1: Player steps on block -> immediately transitions to CRACKED
	bool changed = block.update(16, standingPos, playerSize, true, &map);
	assert(changed == true);
	assert(block.getState() == STEPPABLE_STATE_CRACKED);
	assert(map.getTile(tileX, tileY) == TileMap::STEPPABLE_CRACKED_TILE);
	assert(map.isTileSolid(tileX, tileY) == true);
	assert(block.getContactTimer() == 0);
	std::cout << "  ✓ Stepping on block immediately shows CRACKED visual state and maintains solid collision" << std::endl;

	// Test 2: Stepping for 200 ms (contactTimer increases, does not break)
	changed = block.update(200, standingPos, playerSize, true, &map);
	assert(changed == false);
	assert(block.getState() == STEPPABLE_STATE_CRACKED);
	assert(block.getContactTimer() == 200);
	std::cout << "  ✓ Contact timer accumulates while continuously stepping" << std::endl;

	// Test 3: Cracks remain for 250 ms after leaving, but contact resets immediately.
	changed = block.update(16, awayPos, playerSize, true, &map);
	assert(changed == false);
	assert(block.getState() == STEPPABLE_STATE_CRACKED);
	assert(block.getContactTimer() == 0);
	assert(map.getTile(tileX, tileY) == TileMap::STEPPABLE_CRACKED_TILE);
	assert(map.isTileSolid(tileX, tileY) == true);
	std::cout << "  ✓ Stepping off resets contact while cracks remain visible" << std::endl;

	// Stepping back on keeps the cracked visual and resets its grace timer.
	changed = block.update(16, standingPos, playerSize, true, &map);
	assert(changed == false);
	assert(block.getState() == STEPPABLE_STATE_CRACKED);
	assert(block.getContactTimer() == 16);

	changed = block.update(16, awayPos, playerSize, true, &map);
	assert(changed == false);
	assert(block.getContactTimer() == 0);

	// Cracks remain for 249 ms after leaving and disappear at 250 ms.
	changed = block.update(233, awayPos, playerSize, true, &map);
	assert(changed == false);
	assert(block.getState() == STEPPABLE_STATE_CRACKED);
	changed = block.update(1, awayPos, playerSize, true, &map);
	assert(changed == true);
	assert(block.getState() == STEPPABLE_STATE_INTACT);
	assert(map.getTile(tileX, tileY) == TileMap::STEPPABLE_INTACT_TILE);

	// Continuous contact still needs exactly 350 ms to break the block.
	changed = block.update(16, standingPos, playerSize, true, &map);
	assert(changed == true);
	assert(block.getState() == STEPPABLE_STATE_CRACKED);
	changed = block.update(175, standingPos, playerSize, true, &map);
	assert(changed == false);
	assert(block.getContactTimer() == 175);
	changed = block.update(174, standingPos, playerSize, true, &map);
	assert(changed == false);
	assert(block.getContactTimer() == 349);
	assert(block.getState() == STEPPABLE_STATE_CRACKED);

	// The 350th millisecond breaks it.
	changed = block.update(1, standingPos, playerSize, true, &map);
	assert(changed == true);
	assert(block.getState() == STEPPABLE_STATE_DESTROYED);
	assert(block.getContactTimer() == 0);
	assert(block.getRespawnTimer() == 3000);
	assert(map.getTile(tileX, tileY) == 0);
	assert(map.isTileSolid(tileX, tileY) == false); // Tile has no collision!
	std::cout << "  ✓ 350 ms continuous contact breaks block; cracks linger for 250 ms when stepped off" << std::endl;

	// Test 5: While DESTROYED, stepping does NOT activate anything
	changed = block.update(100, standingPos, playerSize, true, &map);
	assert(changed == false);
	assert(block.getState() == STEPPABLE_STATE_DESTROYED);
	assert(block.getContactTimer() == 0);
	assert(block.getRespawnTimer() == 2900);
	std::cout << "  ✓ Block cannot be activated while destroyed or waiting to respawn" << std::endl;

	// Test 6: Advance 1900 ms -> respawnTimer becomes 1000 ms
	changed = block.update(1900, awayPos, playerSize, false, &map);
	assert(changed == false);
	assert(block.getState() == STEPPABLE_STATE_DESTROYED);
	assert(block.getRespawnTimer() == 1000);

	// Test 7: Advance remaining 1000 ms -> RESPAWNS!
	changed = block.update(1000, awayPos, playerSize, false, &map);
	assert(changed == true);
	assert(block.getState() == STEPPABLE_STATE_INTACT);
	assert(block.getRespawnTimer() == 0);
	assert(block.getContactTimer() == 0);
	assert(map.getTile(tileX, tileY) == TileMap::STEPPABLE_INTACT_TILE);
	assert(map.isTileSolid(tileX, tileY) == true);
	std::cout << "  ✓ After 3 seconds, block automatically respawns intact with solid collision restored" << std::endl;

	// Test 8: Reset functionality
	block.update(16, standingPos, playerSize, true, &map); // Crack it
	assert(block.getState() == STEPPABLE_STATE_CRACKED);
	block.reset(&map);
	assert(block.getState() == STEPPABLE_STATE_INTACT);
	assert(map.getTile(tileX, tileY) == TileMap::STEPPABLE_INTACT_TILE);
	std::cout << "  ✓ Reset restores block to pristine INTACT state" << std::endl;
}

void runPowerUpDropTests() {
	std::cout << "[TEST] 2. Random Power-up Drop Probability & Uniform Distribution..." << std::endl;

	const int TRIALS = 50000;
	int dropCount = 0;
	std::map<int, int> typeCounts;

	PowerUpType type;
	assert(!PowerUp::chooseRandomDrop(0.f, type));
	assert(PowerUp::chooseRandomDrop(1.f, type));
	for (int i = 0; i < TRIALS; ++i) {
		if (PowerUp::chooseRandomDrop(0.50f, type)) {
			dropCount++;
			typeCounts[int(type)]++;
		}
	}

	float dropRate = float(dropCount) / float(TRIALS);
	std::cout << "  Drop rate over " << TRIALS << " trials: " << (dropRate * 100.0f) << "% (expected: 50.0%)" << std::endl;
	assert(std::abs(dropRate - 0.50f) < 0.015f); // Within 1.5% of 50%
	std::cout << "  ✓ 50% independent drop probability verified" << std::endl;

	for (int t = 0; t < 4; ++t) {
		float typeRate = float(typeCounts[t]) / float(dropCount);
		std::cout << "    Type " << t << " (" << PowerUp::getTypeName(PowerUpType(t)) << "): "
		          << (typeRate * 100.0f) << "% (expected: 25.0%)" << std::endl;
		assert(std::abs(typeRate - 0.25f) < 0.02f); // Within 2% of 25%
	}
	std::cout << "  ✓ Uniform 25% selection among all 4 supported power-ups verified" << std::endl;
}

void runPowerUpPickupTests() {
	std::cout << "[TEST] 3. Power-up Positioning & Collection..." << std::endl;

	Player player;
	const glm::vec2 pickupPos(32.f, 48.f);
	PowerUp pickup(POWERUP_BOMB_UP, pickupPos);
	assert(pickup.getPosition() == pickupPos);
	assert(!pickup.checkCollision(glm::ivec2(0, 48), player.getSize()));
	assert(pickup.checkCollision(glm::ivec2(32, 48), player.getSize()));

	pickup.applyEffect(&player);
	assert(!pickup.isActive());
	assert(player.getMaxBombs() == 2);
	pickup.applyEffect(&player);
	assert(player.getMaxBombs() == 2);
	std::cout << "  ✓ Power-up occupies its tile, collides with the player, and applies its effect once" << std::endl;
}

int main() {
	std::cout << "========================================" << std::endl;
	std::cout << " RUNNING BOMBERMAN FEATURE UNIT TESTS   " << std::endl;
	std::cout << "========================================" << std::endl;

	runSteppableBlockTests();
	runPowerUpDropTests();
	runPowerUpPickupTests();

	std::cout << "========================================" << std::endl;
	std::cout << " ALL UNIT TESTS PASSED SUCCESSFULLY!    " << std::endl;
	std::cout << "========================================" << std::endl;
	return 0;
}
