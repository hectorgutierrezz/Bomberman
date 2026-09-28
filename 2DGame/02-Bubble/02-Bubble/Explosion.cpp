#include "Explosion.h"

Texture Explosion::explosionTexture;
bool Explosion::textureLoaded = false;

Explosion::Explosion()
{
	active = false;
	lifetime = 600.f;
	tileMapDispl = glm::ivec2(0);
}

Explosion::~Explosion()
{
	for(size_t i = 0; i < sprites.size(); ++i)
	{
		if(sprites[i] != NULL)
			delete sprites[i];
	}
	sprites.clear();
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
	sprites.clear();

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
			glm::vec2 nextPos = centerPos + glm::vec2(float(dirs[d].x * 32 * step), float(dirs[d].y * 32 * step));
			
			// Si hi ha xoc amb un bloc sòlid del mapa, la flama s'atura
			if(map != NULL && map->isSolidTile(glm::ivec2(int(nextPos.x + 16.f), int(nextPos.y + 16.f))))
			{
				break;
			}
			cells.push_back(nextPos);
		}
	}

	// 3. Crear els sprites per a cada casella de l'explosió
	glm::vec2 sizeInUV = glm::vec2(52.f / 312.f, 56.f / 56.f); // 6 frames = 1/6 ample UV
	glm::vec2 quadOffset = glm::vec2(16.f - 26.f, 16.f - 28.f); // Centrar quad de 52x56 en cel·la de 32x32

	for(size_t i = 0; i < cells.size(); ++i)
	{
		Sprite *sp = Sprite::createSprite(glm::vec2(52.f, 56.f), sizeInUV, &explosionTexture, &shaderProgram);
		sp->setNumberAnimations(1);
		sp->setAnimationSpeed(0, 10); // 10 fps
		for(int k = 0; k < 6; ++k)
		{
			sp->addKeyframe(0, glm::vec2(float(k) / 6.f, 0.f));
		}
		sp->changeAnimation(0);

		glm::vec2 spPos = glm::vec2(float(tileMapDispl.x + cells[i].x + quadOffset.x),
		                           float(tileMapDispl.y + cells[i].y + quadOffset.y));
		sp->setPosition(spPos);
		sprites.push_back(sp);
	}
}

void Explosion::update(int deltaTime)
{
	if(!active)
		return;

	for(size_t i = 0; i < sprites.size(); ++i)
	{
		if(sprites[i] != NULL)
			sprites[i]->update(deltaTime);
	}

	lifetime -= float(deltaTime);
	if(lifetime <= 0.f)
	{
		active = false;
	}
}

void Explosion::render()
{
	if(!active)
		return;

	for(size_t i = 0; i < sprites.size(); ++i)
	{
		if(sprites[i] != NULL)
			sprites[i]->render();
	}
}

bool Explosion::checkCollision(const glm::ivec2 &targetPos, const glm::ivec2 &targetSize) const
{
	if(!active)
		return false;

	for(size_t i = 0; i < cells.size(); ++i)
	{
		int cellLeft = int(cells[i].x);
		int cellRight = cellLeft + 32;
		int cellTop = int(cells[i].y);
		int cellBottom = cellTop + 32;

		int tLeft = targetPos.x;
		int tRight = targetPos.x + targetSize.x;
		int tTop = targetPos.y;
		int tBottom = targetPos.y + targetSize.y;

		bool overlapX = (tRight > cellLeft + 4) && (tLeft < cellRight - 4);
		bool overlapY = (tBottom > cellTop + 4) && (tTop < cellBottom - 4);

		if(overlapX && overlapY)
			return true;
	}

	return false;
}
