#include "Door.h"
#include <iostream>

enum DoorAnims {
	CLOSED, OPEN
};

Door::Door()
{
	sprite = NULL;
	open = false;
	position = glm::vec2(0.f, 0.f);
	tileMapDispl = glm::ivec2(0, 0);
}

Door::~Door()
{
	if(sprite != NULL)
		delete sprite;
}

void Door::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	tileMapDispl = tileMapPos;
	if(!texture.loadFromFile("images/door.png", TEXTURE_PIXEL_FORMAT_RGBA))
	{
		std::cout << "Error loading images/door.png" << std::endl;
	}

	// 2 frames horizontally: Closed (0.0 to 0.5), Open (0.5 to 1.0)
	glm::vec2 sizeInUV = glm::vec2(0.5f, 1.0f);
	sprite = Sprite::createSprite(glm::ivec2(32, 32), sizeInUV, &texture, &shaderProgram);
	sprite->setNumberAnimations(2);

	sprite->setAnimationSpeed(CLOSED, 1);
	sprite->addKeyframe(CLOSED, glm::vec2(0.0f, 0.0f));

	sprite->setAnimationSpeed(OPEN, 1);
	sprite->addKeyframe(OPEN, glm::vec2(0.5f, 0.0f));

	sprite->changeAnimation(CLOSED);
	open = false;
}

void Door::update(int deltaTime)
{
	if(sprite != NULL)
		sprite->update(deltaTime);
}

void Door::render()
{
	if(sprite != NULL)
		sprite->render();
}

void Door::setPosition(const glm::vec2 &pos)
{
	position = pos;
	if(sprite != NULL)
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + position.x), float(tileMapDispl.y + position.y)));
}

void Door::setOpen(bool isOpenDoor)
{
	open = isOpenDoor;
	if(sprite != NULL)
	{
		if(open)
			sprite->changeAnimation(OPEN);
		else
			sprite->changeAnimation(CLOSED);
	}
}

bool Door::checkCollision(const glm::ivec2 &playerPos, const glm::ivec2 &playerSize) const
{
	if(!open)
		return false;

	// Check bounding box intersection between player and door
	int dLeft = int(position.x);
	int dRight = dLeft + 32;
	int dTop = int(position.y);
	int dBottom = dTop + 32;

	int pLeft = playerPos.x;
	int pRight = pLeft + playerSize.x;
	int pTop = playerPos.y;
	int pBottom = pTop + playerSize.y;

	// Overlap check with a small margin so player enters door center
	bool overlapX = (pRight > dLeft + 6) && (pLeft < dRight - 6);
	bool overlapY = (pBottom > dTop + 6) && (pTop < dBottom - 6);

	return overlapX && overlapY;
}
