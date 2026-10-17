#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"


#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT 96
#define FALL_STEP 4
#define GRAVITY 0.4f
#define MAX_FALL_SPEED 8.0f


enum PlayerAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, IDLE, FALL, HURT, DIE
};


Player::Player()
{
	sprite = NULL;
	map = NULL;
	bombs = NULL;
	facingDirection = 1;
	internalLives = 1;
	maxBombs = 1;
	bombRadius = 1;
	moveSpeed = 2.0f;
	moveAccumX = 0.0f;
	invulnerableTime = 0;
	fallSpeed = 0.0f;
	godMode = false;
	bDying = false;
	bDead = false;
	dyingTime = 0;
}

Player::~Player()
{
	if (sprite != NULL)
		delete sprite;
}

void Player::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	bJumping = false;
	bDying = false;
	bDead = false;
	dyingTime = 0;
	internalLives = 1;
	maxBombs = 1;
	bombRadius = 1;
	moveSpeed = 2.0f;
	moveAccumX = 0.0f;
	invulnerableTime = 0;
	fallSpeed = 0.0f;
	godMode = false;
	if(sprite != NULL)
	{
		delete sprite;
		sprite = NULL;
	}
	spritesheet.loadFromFile("images/Sprites/Original/Color/Characters/bomber.png", TEXTURE_PIXEL_FORMAT_RGBA);
	
	glm::vec2 sizeInUV = glm::vec2(17.f / 253.f, 17.f / 632.f); // 17x17 pixels normalitzats
	
	sprite = Sprite::createSprite(glm::ivec2(32, 32), sizeInUV, &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(8); // Mateix nombre d'animacions que a l'enum

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

	// HURT (Expressió de dolor / impacte en rebre dany)
	sprite->setAnimationSpeed(HURT, 8);
	sprite->addKeyframe(HURT, uv(1, 1));

	// DIE (Animació completa de mort oficial de Pocket Bomberman)
	sprite->setAnimationSpeed(DIE, 6);
	sprite->addKeyframe(DIE, uv(1, 1)); // Xoc / impacte
	sprite->addKeyframe(DIE, uv(2, 1)); // Gir / mareig
	sprite->addKeyframe(DIE, uv(3, 1)); // Tombarella
	sprite->addKeyframe(DIE, uv(7, 1)); // Caigut derrotat a terra
	sprite->addKeyframe(DIE, uv(4, 1)); // Puf / flash
	sprite->addKeyframe(DIE, uv(5, 1)); // Anell d'esvaïment
	sprite->addKeyframe(DIE, uv(6, 1)); // Desaparició final

	sprite->changeAnimation(IDLE);
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::startDying()
{
	if(bDying || bDead)
		return;
	bDying = true;
	bDead = false;
	dyingTime = 1200; // 1.2 segons de durada de l'animació de mort
	bJumping = false;
	if(sprite != NULL)
		sprite->changeAnimation(DIE);
}

bool Player::stepFall(int step)
{
	bool onGround = false;
	for(int s = 0; s < step; ++s)
	{
		posPlayer.y += 1;
		if(map != NULL && map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y))
		{
			onGround = true;
			break;
		}
		if(bombs != NULL)
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
			if(onGround)
				break;
		}
	}
	return onGround;
}

void Player::update(int deltaTime)
{
	sprite->update(deltaTime);

	if(bDying)
	{
		dyingTime -= deltaTime;
		if(dyingTime <= 0)
		{
			dyingTime = 0;
			bDying = false;
			bDead = true; // Senyalitza que l'animació de mort ha finalitzat
		}
		// Cau amb acceleració si estava en l'aire
		fallSpeed = std::min(fallSpeed + GRAVITY, MAX_FALL_SPEED);
		int step = std::max(1, int(fallSpeed));
		if(stepFall(step))
			fallSpeed = 0.0f;
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
		return;
	}

	if(invulnerableTime > 0)
	{
		invulnerableTime -= deltaTime;
		if(invulnerableTime < 0)
			invulnerableTime = 0;
	}

	// Reacció immediata en rebre dany: mostrar cara de ferit (HURT) durant els primers 350ms
	if(invulnerableTime > 1650)
	{
		if(sprite->animation() != HURT)
			sprite->changeAnimation(HURT);
		fallSpeed = std::min(fallSpeed + GRAVITY, MAX_FALL_SPEED);
		int step = std::max(1, int(fallSpeed));
		if(stepFall(step))
			fallSpeed = 0.0f;
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
		return;
	}

	// Actualitzar solapament del jugador amb les bombes actives
	if(bombs != NULL)
	{
		for(size_t i = 0; i < bombs->size(); ++i)
		{
			if((*bombs)[i] != NULL && (*bombs)[i]->isActive())
				(*bombs)[i]->updatePlayerOverlap(posPlayer, glm::ivec2(32, 32));
		}
	}

	if(Game::instance().getKey(GLFW_KEY_LEFT) || Game::instance().getKey(GLFW_KEY_A))
	{
		facingDirection = -1;
		if(sprite->animation() != MOVE_LEFT)
			sprite->changeAnimation(MOVE_LEFT);
		sprite->setFlippedHorizontally(false);

		moveAccumX += moveSpeed;
		int step = int(moveAccumX);
		moveAccumX -= float(step);

		for(int s = 0; s < step; ++s)
		{
			posPlayer.x -= 1;
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
				posPlayer.x += 1;
				sprite->changeAnimation(STAND_LEFT);
				break;
			}
		}
	}
	else if(Game::instance().getKey(GLFW_KEY_RIGHT) || Game::instance().getKey(GLFW_KEY_D))
	{
		facingDirection = 1;
		if(sprite->animation() != MOVE_RIGHT)
			sprite->changeAnimation(MOVE_RIGHT);
		sprite->setFlippedHorizontally(true);

		moveAccumX += moveSpeed;
		int step = int(moveAccumX);
		moveAccumX -= float(step);

		for(int s = 0; s < step; ++s)
		{
			posPlayer.x += 1;
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
				posPlayer.x -= 1;
				sprite->changeAnimation(STAND_RIGHT);
				break;
			}
		}
	}
	else
	{
		moveAccumX = 0.0f;
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
			bool onGround = (map != NULL && map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y));
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
				fallSpeed = 0.0f;
			else
				fallSpeed = 6.0f; // Continuïtat suau de la caiguda si s'acaba el salt en l'aire
		}
		else
		{
			posPlayer.y = int(startY - 96 * sin(3.14159f * jumpAngle / 180.f));
			if(jumpAngle < 90)
			{
				if(map->collisionMoveUp(posPlayer, glm::ivec2(32, 32), &posPlayer.y))
				{
					bJumping = false;
					fallSpeed = 0.0f;
				}
			}
			else if(jumpAngle > 90)
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
				{
					bJumping = false;
					fallSpeed = 0.0f;
				}
			}
		}
	}
	else
	{
		// Caiguda amb acceleració realista de gravetat
		fallSpeed = std::min(fallSpeed + GRAVITY, MAX_FALL_SPEED);
		int step = std::max(1, int(fallSpeed));
		bool onGround = stepFall(step);

		if(onGround)
		{
			fallSpeed = 0.0f;
			if(Game::instance().getKey(GLFW_KEY_UP) || Game::instance().getKey(GLFW_KEY_W))
			{
				bJumping = true;
				jumpAngle = 0;
				startY = posPlayer.y;
				fallSpeed = 0.0f;
			}
		}
	}

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::render()
{
	// Parpelleig durant la invulnerabilitat temporal després de rebre dany (només si no s'està morint)
	if(!bDying && invulnerableTime > 0 && ((invulnerableTime / 100) % 2 == 0))
		return;
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

void Player::resetPowerUps()
{
	maxBombs = 1;
	bombRadius = 1;
	internalLives = 1;
	moveSpeed = 2.0f;
	moveAccumX = 0.0f;
}

void Player::respawn(const glm::vec2 &pos)
{
	bJumping = false;
	bDying = false;
	bDead = false;
	dyingTime = 0;
	invulnerableTime = 0;
	internalLives = 1;
	fallSpeed = 0.0f;
	moveAccumX = 0.0f;
	setPosition(pos);
	if(sprite != NULL)
		sprite->changeAnimation(IDLE);
}

bool Player::hit()
{
	if(godMode || invulnerableTime > 0 || internalLives <= 0 || bDying || bDead)
		return false;

	internalLives--;
	if(internalLives > 0)
	{
		invulnerableTime = 2000;
		if(sprite != NULL)
			sprite->changeAnimation(HURT);
		return false;
	}

	// Ha perdut l'últim cor de nivell -> Comença l'animació de mort!
	startDying();
	return true;
}





