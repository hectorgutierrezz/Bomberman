#include "PowerUp.h"
#include "Player.h"
#include <cmath>
#include <cstdlib>
#include <iostream>

Texture PowerUp::powerUpTexture;
bool PowerUp::textureLoaded = false;

PowerUp::PowerUp(PowerUpType type, const glm::vec2 &pos)
{
	this->type = type;
	this->position = pos;
	this->active = true;
	this->sprite = NULL;
	this->bobTime = float(rand() % 1000) / 100.f;
	this->tileMapDispl = glm::ivec2(0);
}

PowerUp::~PowerUp()
{
	if(sprite != NULL)
	{
		delete sprite;
		sprite = NULL;
	}
}

void PowerUp::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	tileMapDispl = tileMapPos;
	if(!textureLoaded)
	{
		if(!powerUpTexture.loadFromFile("images/powerups.png", TEXTURE_PIXEL_FORMAT_RGBA))
		{
			std::cerr << "Error loading images/powerups.png" << std::endl;
		}
		powerUpTexture.setMinFilter(GL_NEAREST);
		powerUpTexture.setMagFilter(GL_NEAREST);
		powerUpTexture.setWrapS(GL_CLAMP_TO_EDGE);
		powerUpTexture.setWrapT(GL_CLAMP_TO_EDGE);
		textureLoaded = true;
	}

	// 4 frames a images/powerups.png (128x32 px) -> mida UV (0.25f, 1.0f)
	// Escala ajustada a 16x16 px per coincidir exactament amb la graella dels tiles i la mida del jugador
	glm::vec2 sizeInUV = glm::vec2(0.25f, 1.0f);
	sprite = Sprite::createSprite(glm::ivec2(16, 16), sizeInUV, &powerUpTexture, &shaderProgram);
	sprite->setNumberAnimations(4);

	for(int i = 0; i < 4; ++i)
	{
		sprite->setAnimationSpeed(i, 8);
		sprite->addKeyframe(i, glm::vec2(float(i) * 0.25f, 0.0f));
	}

	sprite->changeAnimation(int(type));
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + position.x), float(tileMapDispl.y + position.y)));
}

void PowerUp::update(int deltaTime)
{
	if(!active)
		return;

	bobTime += float(deltaTime) / 1000.f;
	float bobOffset = std::sin(bobTime * 4.0f) * 1.5f; // Efecte suau de flotació adaptat a 16px
	if(sprite != NULL)
	{
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + position.x), float(tileMapDispl.y + position.y + bobOffset)));
		sprite->update(deltaTime);
	}
}

void PowerUp::render()
{
	if(active && sprite != NULL)
		sprite->render();
}

bool PowerUp::checkCollision(const glm::ivec2 &playerPos, const glm::ivec2 &playerSize) const
{
	if(!active)
		return false;

	int pLeft = playerPos.x;
	int pRight = pLeft + playerSize.x;
	int pTop = playerPos.y;
	int pBottom = pTop + playerSize.y;

	// Bounding box de l'objecte (16x16) amb marge còmode de recol·lecció
	int iLeft = int(position.x);
	int iRight = int(position.x) + 16;
	int iTop = int(position.y);
	int iBottom = int(position.y) + 16;

	return (pRight > iLeft && pLeft < iRight && pBottom > iTop && pTop < iBottom);
}

void PowerUp::applyEffect(Player *player)
{
	if(player == NULL || !active)
		return;

	switch(type)
	{
	case POWERUP_BOMB_UP:
		player->addMaxBomb();
		break;
	case POWERUP_FIRE_UP:
		player->addBombRadius();
		break;
	case POWERUP_HEART:
		player->addInternalLife();
		break;
	case POWERUP_SKATES:
		player->addSpeed();
		break;
	}

	active = false;
}

const char *PowerUp::getTypeName(PowerUpType type)
{
	switch(type)
	{
	case POWERUP_BOMB_UP: return "BOMB UP";
	case POWERUP_FIRE_UP: return "FIRE UP";
	case POWERUP_HEART:   return "HEART";
	case POWERUP_SKATES:  return "SKATES";
	}
	return "UNKNOWN";
}
