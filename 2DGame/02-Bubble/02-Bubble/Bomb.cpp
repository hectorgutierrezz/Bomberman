#include "Bomb.h"

Texture Bomb::bombTexture;
bool Bomb::textureLoaded = false;

Bomb::Bomb()
{
	sprite = NULL;
	active = false;
	bExploded = false;
	playerInside = true;
	fuseTime = 2200.f;
	explosionRadius = 1;
	position = glm::vec2(0.f);
	tileMapDispl = glm::ivec2(0);
}

Bomb::~Bomb()
{
	if(sprite != NULL)
		delete sprite;
}

void Bomb::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	tileMapDispl = tileMapPos;
	if(!textureLoaded)
	{
		bombTexture.loadFromFile("images/Bomberman/Bomb On (52x56).png", TEXTURE_PIXEL_FORMAT_RGBA);
		textureLoaded = true;
	}

	// 4 frames horizontals de 52x56 cadascun en una textura de 208x56
	// Quad de 52x56 per mantenir el pixel-art 1:1 natiu (el gràfic de la bomba fa 16x19 px amb espurnes)
	glm::vec2 sizeInUV = glm::vec2(52.f / 208.f, 56.f / 56.f); // 0.25f, 1.0f
	sprite = Sprite::createSprite(glm::vec2(52.f, 56.f), sizeInUV, &bombTexture, &shaderProgram);
	sprite->setNumberAnimations(1);
	sprite->setAnimationSpeed(0, 8); // 8 frames per segon (animació de parpelleig/metxa)
	sprite->addKeyframe(0, glm::vec2(0.00f, 0.f));
	sprite->addKeyframe(0, glm::vec2(0.25f, 0.f));
	sprite->addKeyframe(0, glm::vec2(0.50f, 0.f));
	sprite->addKeyframe(0, glm::vec2(0.75f, 0.f));
	sprite->changeAnimation(0);

	active = true;
	bExploded = false;
	playerInside = true;
	fuseTime = 2200.f;

	// Dins la textura de 52x56, la bomba és a x=[20..35] (centre 27.5) i y=[20..38] (base a fila 38).
	// Perquè la base reposi exactament sobre la superfície del terra (y + 16) i centrada al tile de 16x16 (x + 8):
	// quadOffset.x = 8 - 27.5 = -19.5f; quadOffset.y = 16 - 38 = -22.f
	glm::vec2 quadOffset = glm::vec2(-19.5f, -22.f);
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + position.x + quadOffset.x), float(tileMapDispl.y + position.y + quadOffset.y)));
}

void Bomb::setPosition(const glm::vec2 &pos)
{
	position = pos;
	if(sprite != NULL)
	{
		glm::vec2 quadOffset = glm::vec2(-19.5f, -22.f);
		sprite->setPosition(glm::vec2(float(tileMapDispl.x + position.x + quadOffset.x), float(tileMapDispl.y + position.y + quadOffset.y)));
	}
}

void Bomb::update(int deltaTime)
{
	if(!active || sprite == NULL)
		return;

	sprite->update(deltaTime);
	fuseTime -= float(deltaTime);
	if(fuseTime <= 0.f)
	{
		active = false;
		bExploded = true;
	}
}

void Bomb::render()
{
	if(active && sprite != NULL)
		sprite->render();
}

void Bomb::explode()
{
	fuseTime = 0.f;
	active = false;
	bExploded = true;
}

bool Bomb::checkOverlap(const glm::ivec2 &pPos, const glm::ivec2 &pSize) const
{
	int bLeft = int(position.x);
	int bRight = bLeft + 16;
	int bTop = int(position.y);
	int bBottom = bTop + 16;

	int pLeft = pPos.x;
	int pRight = pPos.x + pSize.x;
	int pTop = pPos.y;
	int pBottom = pPos.y + pSize.y;

	bool overlapX = (pRight > bLeft + 1) && (pLeft < bRight - 1);
	bool overlapY = (pBottom > bTop + 1) && (pTop < bBottom - 1);

	return overlapX && overlapY;
}

void Bomb::updatePlayerOverlap(const glm::ivec2 &playerPos, const glm::ivec2 &playerSize)
{
	if(playerInside)
	{
		if(!checkOverlap(playerPos, playerSize))
		{
			playerInside = false;
		}
	}
}

bool Bomb::collisionDown(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const
{
	if(!isSolidForPlayer())
		return false;

	int bLeft = int(position.x);
	int bRight = bLeft + 16;
	int bTop = int(position.y);

	int pLeft = pos.x;
	int pRight = pos.x + size.x;
	int pBottom = pos.y + size.y;

	// Solapament horitzontal amb marge
	bool overlapX = (pRight > bLeft + 2) && (pLeft < bRight - 2);
	// El jugador cau o reposa sobre la cara superior
	if(overlapX && pBottom >= bTop && pBottom <= bTop + 8)
	{
		*posY = bTop - size.y;
		return true;
	}
	return false;
}

bool Bomb::collisionLeft(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	if(!isSolidForPlayer())
		return false;

	int bRight = int(position.x) + 16;
	int bTop = int(position.y);
	int bBottom = bTop + 16;

	int pLeft = pos.x;
	int pTop = pos.y;
	int pBottom = pTop + size.y;

	bool overlapY = (pBottom > bTop + 2) && (pTop < bBottom - 2);
	if(overlapY && pLeft <= bRight && pLeft >= bRight - 3)
	{
		return true;
	}
	return false;
}

bool Bomb::collisionRight(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	if(!isSolidForPlayer())
		return false;

	int bLeft = int(position.x);
	int bTop = int(position.y);
	int bBottom = bTop + 16;

	int pRight = pos.x + size.x;
	int pTop = pos.y;
	int pBottom = pTop + size.y;

	bool overlapY = (pBottom > bTop + 2) && (pTop < bBottom - 2);
	if(overlapY && pRight >= bLeft && pRight <= bLeft + 3)
	{
		return true;
	}
	return false;
}

bool Bomb::isSolidOnTop(const glm::ivec2 &playerPos, const glm::ivec2 &playerSize) const
{
	int dummyY = 0;
	return collisionDown(playerPos, playerSize, &dummyY);
}
