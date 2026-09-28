#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"


#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT 96
#define FALL_STEP 4


enum PlayerAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, IDLE, FALL
};


Player::Player()
{
	sprite = NULL;
	map = NULL;
	bombs = NULL;
	facingDirection = 1;
}

Player::~Player()
{
	if (sprite != NULL)
		delete sprite;
}

void Player::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	bJumping = false;
	spritesheet.loadFromFile("images/Sprites/Original/Color/Characters/bomber.png", TEXTURE_PIXEL_FORMAT_RGBA);
	
	glm::vec2 sizeInUV = glm::vec2(17.f / 253.f, 17.f / 632.f); // 17x17 pixels normalitzats
	
	sprite = Sprite::createSprite(glm::ivec2(32, 32), sizeInUV, &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(6); // Mateix nombre d'animacions que a l'enum

	auto uv = [](int col, int row) {
		return glm::vec2(float(col * 17.f / 253.f), float(row * 17.f / 632.f));
	};
	
	// IDLE (Frontal / Parat)
	sprite->setAnimationSpeed(IDLE, 8);
	sprite->addKeyframe(IDLE, uv(0, 0));

	// STAND_RIGHT (Parat cara a la dreta)
	sprite->setAnimationSpeed(STAND_RIGHT, 8);
	sprite->addKeyframe(STAND_RIGHT, uv(3, 0));

	// STAND_LEFT (Parat cara a l'esquerra)
	sprite->setAnimationSpeed(STAND_LEFT, 8);
	sprite->addKeyframe(STAND_LEFT, uv(3, 0));

	// MOVE_RIGHT (Caminar cap a la dreta)
	sprite->setAnimationSpeed(MOVE_RIGHT, 8);
	sprite->addKeyframe(MOVE_RIGHT, uv(3, 0));
	sprite->addKeyframe(MOVE_RIGHT, uv(4, 0));
	sprite->addKeyframe(MOVE_RIGHT, uv(3, 0));
	sprite->addKeyframe(MOVE_RIGHT, uv(5, 0));

	// MOVE_LEFT (Caminar cap a l'esquerra)
	sprite->setAnimationSpeed(MOVE_LEFT, 8);
	sprite->addKeyframe(MOVE_LEFT, uv(3, 0));
	sprite->addKeyframe(MOVE_LEFT, uv(4, 0));
	sprite->addKeyframe(MOVE_LEFT, uv(3, 0));
	sprite->addKeyframe(MOVE_LEFT, uv(5, 0));

	// FALL (En l'aire / Caiguda)
	sprite->setAnimationSpeed(FALL, 8);
	sprite->addKeyframe(FALL, uv(1, 0));

	sprite->changeAnimation(IDLE);
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::update(int deltaTime)
{
	sprite->update(deltaTime);

	// Actualitzar solapament del jugador amb les bombes actives
	if(bombs != NULL)
	{
		for(size_t i = 0; i < bombs->size(); ++i)
		{
			if((*bombs)[i] != NULL && (*bombs)[i]->isActive())
				(*bombs)[i]->updatePlayerOverlap(posPlayer, glm::ivec2(32, 32));
		}
	}

	if(Game::instance().getKey(GLFW_KEY_LEFT))
	{
		facingDirection = -1;
		if(sprite->animation() != MOVE_LEFT)
			sprite->changeAnimation(MOVE_LEFT);
		sprite->setFlippedHorizontally(false);
		posPlayer.x -= 2;

		bool hitBomb = false;
		if(bombs != NULL)
		{
			for(size_t i = 0; i < bombs->size(); ++i)
			{
				if((*bombs)[i] != NULL && (*bombs)[i]->isSolidForPlayer())
				{
					if((*bombs)[i]->collisionLeft(posPlayer, glm::ivec2(32, 32)))
					{
						hitBomb = true;
						break;
					}
				}
			}
		}

		if(map->collisionMoveLeft(posPlayer, glm::ivec2(32, 32)) || hitBomb)
		{
			posPlayer.x += 2;
			sprite->changeAnimation(STAND_LEFT);
		}
	}
	else if(Game::instance().getKey(GLFW_KEY_RIGHT))
	{
		facingDirection = 1;
		if(sprite->animation() != MOVE_RIGHT)
			sprite->changeAnimation(MOVE_RIGHT);
		sprite->setFlippedHorizontally(true);
		posPlayer.x += 2;

		bool hitBomb = false;
		if(bombs != NULL)
		{
			for(size_t i = 0; i < bombs->size(); ++i)
			{
				if((*bombs)[i] != NULL && (*bombs)[i]->isSolidForPlayer())
				{
					if((*bombs)[i]->collisionRight(posPlayer, glm::ivec2(32, 32)))
					{
						hitBomb = true;
						break;
					}
				}
			}
		}

		if(map->collisionMoveRight(posPlayer, glm::ivec2(32, 32)) || hitBomb)
		{
			posPlayer.x -= 2;
			sprite->changeAnimation(STAND_RIGHT);
		}
	}
	else
	{
		if(sprite->animation() == MOVE_LEFT)
		{
			sprite->changeAnimation(STAND_LEFT);
			sprite->setFlippedHorizontally(false);
		}
		else if(sprite->animation() == MOVE_RIGHT)
		{
			sprite->changeAnimation(STAND_RIGHT);
			sprite->setFlippedHorizontally(true);
		}
	}
	
	if(bJumping)
	{
		jumpAngle += JUMP_ANGLE_STEP;
		if(jumpAngle == 180)
		{
			bJumping = false;
			posPlayer.y = startY;
		}
		else
		{
			posPlayer.y = int(startY - 96 * sin(3.14159f * jumpAngle / 180.f));
			if(jumpAngle > 90)
			{
				bool hitGround = map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y);
				if(!hitGround && bombs != NULL)
				{
					for(size_t i = 0; i < bombs->size(); ++i)
					{
						if((*bombs)[i] != NULL && (*bombs)[i]->isSolidForPlayer())
						{
							if((*bombs)[i]->collisionDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y))
							{
								hitGround = true;
								break;
							}
						}
					}
				}
				if(hitGround)
					bJumping = false;
			}
		}
	}
	else
	{
		posPlayer.y += FALL_STEP;
		bool onGround = map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y);
		if(!onGround && bombs != NULL)
		{
			for(size_t i = 0; i < bombs->size(); ++i)
			{
				if((*bombs)[i] != NULL && (*bombs)[i]->isSolidForPlayer())
				{
					if((*bombs)[i]->collisionDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y))
					{
						onGround = true;
						break;
					}
				}
			}
		}

		if(onGround)
		{
			if(Game::instance().getKey(GLFW_KEY_UP))
			{
				bJumping = true;
				jumpAngle = 0;
				startY = posPlayer.y;
			}
		}
	}

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::render()
{
	sprite->render();
}

void Player::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Player::setPosition(const glm::vec2 &pos)
{
	posPlayer = pos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::setBombs(const std::vector<Bomb*> *bombsList)
{
	bombs = bombsList;
}

void Player::hit()
{
	posPlayer = glm::ivec2(4 * 16, 25 * 16);
	bJumping = false;
	jumpAngle = 0;
	if(sprite != NULL)
	{
		sprite->changeAnimation(IDLE);
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	}
}





