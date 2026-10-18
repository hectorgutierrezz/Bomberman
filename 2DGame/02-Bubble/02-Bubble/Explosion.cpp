#include "Explosion.h"

Texture Explosion::explosionTexture;
bool Explosion::textureLoaded = false;

Explosion::Explosion()
{
	active = false;
	lifetime = 600.f;
	tileMapDispl = glm::ivec2(0);
	sprite = NULL;
}

Explosion::~Explosion()
{
	if(sprite != NULL)
	{
		delete sprite;
		sprite = NULL;
	}
	cells.clear();
}

void Explosion::init(const glm::vec2 &centerPos, const glm::ivec2 &tileMapPos, int range, TileMap *map, ShaderProgram &shaderProgram)
{
	tileMapDispl = tileMapPos;
	active = true;
	lifetime = 600.f; // 6 frames a 10 fps = 600 ms

	if(!textureLoaded)
	{
		explosionTexture.loadFromFile("images/Bomberman/Boooooom (52x56).png", TEXTURE_PIXEL_FORMAT_RGBA);
		textureLoaded = true;
	}

	cells.clear();

	// 1. Casella central de la bomba
	cells.push_back(centerPos);

	// 2. Propagació en creu (4 direccions)
	glm::ivec2 dirs[4] = {
		glm::ivec2(1, 0),   // Dreta
		glm::ivec2(-1, 0),  // Esquerra
		glm::ivec2(0, 1),   // Avall
		glm::ivec2(0, -1)   // Amunt
	};

	for(int d = 0; d < 4; ++d)
	{
		for(int step = 1; step <= range; ++step)
		{
			glm::vec2 nextPos = centerPos + glm::vec2(float(dirs[d].x * 16 * step), float(dirs[d].y * 16 * step));
			glm::ivec2 samplePos(int(nextPos.x + 8.f), int(nextPos.y + 8.f));

			// La flama s'atura amb els blocs sòlids; si és destruïble, l'abasta i no continua
			if(map != NULL && map->isSolidTile(samplePos))
			{
				if(map->isDestructibleTileAt(samplePos))
					cells.push_back(nextPos);
				break;
			}
			cells.push_back(nextPos);
		}
	}

	// 3. Crear un únic sprite reutilitzat per a totes les cel·les de l'explosió
	glm::vec2 sizeInUV = glm::vec2(52.f / 312.f, 56.f / 56.f); // 6 frames = 1/6 ample UV

	if(sprite != NULL)
	{
		delete sprite;
		sprite = NULL;
	}

	sprite = Sprite::createSprite(glm::vec2(32.f, 32.f), sizeInUV, &explosionTexture, &shaderProgram);
	sprite->setNumberAnimations(1);
	sprite->setAnimationSpeed(0, 10); // 10 fps
	for(int k = 0; k < 6; ++k)
	{
		sprite->addKeyframe(0, glm::vec2(float(k) / 6.f, 0.f));
	}
	sprite->changeAnimation(0);
}

void Explosion::update(int deltaTime)
{
	if(!active)
		return;

	if(sprite != NULL)
		sprite->update(deltaTime);

	lifetime -= float(deltaTime);
	if(lifetime <= 0.f)
	{
		active = false;
	}
}

void Explosion::render()
{
	if(!active || sprite == NULL)
		return;

	glm::vec2 quadOffset = glm::vec2(-8.f, -8.f); // Centrar flama de 32x32 en cel·la de 16x16
	for(size_t i = 0; i < cells.size(); ++i)
	{
		glm::vec2 spPos = glm::vec2(float(tileMapDispl.x + cells[i].x + quadOffset.x),
		                           float(tileMapDispl.y + cells[i].y + quadOffset.y));
		sprite->setPosition(spPos);
		sprite->render();
	}
}

bool Explosion::checkCollision(const glm::ivec2 &targetPos, const glm::ivec2 &targetSize) const
{
	if(!active)
		return false;

	for(size_t i = 0; i < cells.size(); ++i)
	{
		int cellLeft = int(cells[i].x);
		int cellRight = cellLeft + 16;
		int cellTop = int(cells[i].y);
		int cellBottom = cellTop + 16;

		int tLeft = targetPos.x;
		int tRight = targetPos.x + targetSize.x;
		int tTop = targetPos.y;
		int tBottom = targetPos.y + targetSize.y;

		bool overlapX = (tRight > cellLeft + 1) && (tLeft < cellRight - 1);
		bool overlapY = (tBottom > cellTop + 1) && (tTop < cellBottom - 1);

		if(overlapX && overlapY)
			return true;
	}

	return false;
}
