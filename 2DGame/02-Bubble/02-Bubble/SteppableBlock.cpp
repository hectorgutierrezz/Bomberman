#include "SteppableBlock.h"
#include "SoundManager.h"
#include <cstdlib>
#include <cmath>

SteppableBlock::SteppableBlock(int tileX, int tileY)
{
	this->tileX = tileX;
	this->tileY = tileY;
	this->state = STEPPABLE_STATE_INTACT;
	this->contactTimer = 0;
	this->unsteppedTimer = 0;
	this->respawnTimer = 0;
}

SteppableBlock::~SteppableBlock()
{
}

void SteppableBlock::reset(TileMap *map)
{
	state = STEPPABLE_STATE_INTACT;
	contactTimer = 0;
	unsteppedTimer = 0;
	respawnTimer = 0;
	if (map != NULL)
	{
		map->setTile(tileX, tileY, TileMap::STEPPABLE_INTACT_TILE, false);
	}
}

bool SteppableBlock::update(int deltaTime, const glm::ivec2 &playerPos, const glm::ivec2 &playerSize, bool playerOnGround, TileMap *map)
{
	if (map == NULL)
		return false;

	bool stateChanged = false;
	int tileSize = map->getTileSize();

	// Comprovar si el jugador està dret trepitjant la cara superior del bloc
	bool isSteppedOn = false;
	if (state != STEPPABLE_STATE_DESTROYED && playerOnGround)
	{
		int pLeft = playerPos.x + 1;
		int pRight = playerPos.x + playerSize.x - 1;
		int bLeft = tileX * tileSize;
		int bRight = bLeft + tileSize;
		int footY = playerPos.y + playerSize.y;
		int blockTopY = tileY * tileSize;

		// Els peus toquen la cara superior del bloc i hi ha solapament horitzontal
		if (pRight > bLeft && pLeft < bRight && std::abs(footY - blockTopY) <= 1)
		{
			isSteppedOn = true;
		}
	}

	switch (state)
	{
	case STEPPABLE_STATE_INTACT:
		if (isSteppedOn)
		{
			// El jugador comença a trepitjar el bloc -> estat agrietat immediat
			state = STEPPABLE_STATE_CRACKED;
			contactTimer = 0;
			unsteppedTimer = 0;
			map->setTile(tileX, tileY, TileMap::STEPPABLE_CRACKED_TILE, false);
			stateChanged = true;
		}
		break;

	case STEPPABLE_STATE_CRACKED:
		if (isSteppedOn)
		{
			unsteppedTimer = 0;
			contactTimer += deltaTime;
			if (contactTimer >= 350) // 0,35 segons de contacte continu
			{
				// Es trenca el bloc!
				state = STEPPABLE_STATE_DESTROYED;
				contactTimer = 0;
				unsteppedTimer = 0;
				respawnTimer = 3000; // Reapareix aproximadament 3 segons després
				map->setTile(tileX, tileY, 0, false);
				stateChanged = true;
				SoundManager::instance().playSound(SFX_BOMB_EXPLOSION);
			}
		}
		else
		{
			contactTimer = 0;
			unsteppedTimer += deltaTime;
			if (unsteppedTimer >= 250) // Manté les esquerdes visibles un quart de segon
			{
				state = STEPPABLE_STATE_INTACT;
				unsteppedTimer = 0;
				map->setTile(tileX, tileY, TileMap::STEPPABLE_INTACT_TILE, false);
				stateChanged = true;
			}
		}
		break;

	case STEPPABLE_STATE_DESTROYED:
		// Evita que es torni a activar mentre està destruït o esperant a reaparèixer
		respawnTimer -= deltaTime;
		if (respawnTimer <= 0)
		{
			// Reaparició després de ~3 segons
			state = STEPPABLE_STATE_INTACT;
			respawnTimer = 0;
			contactTimer = 0;
			unsteppedTimer = 0;
			map->setTile(tileX, tileY, TileMap::STEPPABLE_INTACT_TILE, false);
			stateChanged = true;
		}
		break;
	}

	return stateChanged;
}
