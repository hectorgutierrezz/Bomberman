#include "Bomb.h"

Texture Bomb::bombTexture;
bool Bomb::textureLoaded = false;

Bomb::Bomb()
{
	sprite = NULL;
	active = false;
	bExploded = false;
	fuseTime = 2200.f;
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
	fuseTime = 2200.f;

	// Centrar bomba en bloc de 32x32: la bomba fa 18x21 px centrada a (28, 39) al quad de 52x56
	glm::vec2 quadOffset = glm::vec2(16.f - 28.f, 32.f - 39.f);
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + position.x + quadOffset.x), float(tileMapDispl.y + position.y + quadOffset.y)));
}

void Bomb::setPosition(const glm::vec2 &pos)
{
	position = pos;
	if(sprite != NULL)
	{
		glm::vec2 quadOffset = glm::vec2(16.f - 28.f, 32.f - 39.f);
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

bool Bomb::isSolidOnTop(const glm::ivec2 &playerPos, const glm::ivec2 &playerSize) const
{
	if(!active)
		return false;

	int bombLeft = int(position.x);
	int bombRight = bombLeft + 32;
	int bombTop = int(position.y);

	int playerLeft = playerPos.x;
	int playerRight = playerPos.x + playerSize.x;
	int playerBottom = playerPos.y + playerSize.y;

	// Solapament horitzontal amb marge de tolerància
	bool horizontalOverlap = (playerRight > bombLeft + 6) && (playerLeft < bombRight - 6);
	// El jugador està caient sobre la cara superior de la bomba
	bool onTop = (playerBottom >= bombTop - 4) && (playerBottom <= bombTop + 8);

	return horizontalOverlap && onTop;
}
