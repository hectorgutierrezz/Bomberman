#include "Bomb.h"

Bomb::Bomb()
{
    sprite = NULL;
    active = false;
    fuseTime = 1800.f;
    position = glm::vec2(0.f);
}

Bomb::~Bomb()
{
    if(sprite != NULL)
        delete sprite;
}

void Bomb::init(ShaderProgram &shaderProgram)
{
    texture.loadFromFile("images/Bomberman/Bomb On (52x56).png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(glm::vec2(20.f, 22.f), glm::vec2(1.f, 1.f), &texture, &shaderProgram);
    sprite->setPosition(position + glm::vec2(6.f, 8.f));
    active = true;
    fuseTime = 1800.f;
}

void Bomb::setPosition(const glm::vec2 &pos)
{
    position = pos;
    if(sprite != NULL)
        sprite->setPosition(position + glm::vec2(6.f, 8.f));
}

void Bomb::update(int deltaTime)
{
    if(!active || sprite == NULL)
        return;

    sprite->update(deltaTime);
    fuseTime -= float(deltaTime);
    if(fuseTime <= 0.f)
        active = false;
}

void Bomb::render()
{
    if(active && sprite != NULL)
        sprite->render();
}

bool Bomb::isSolidOnTop(const glm::ivec2 &playerPos, const glm::ivec2 &playerSize) const
{
    if(!active)
        return false;

    int bombLeft = int(position.x);
    int bombRight = bombLeft + 32;
    int bombTop = int(position.y);
    int bombBottom = bombTop + 32;

    int playerLeft = playerPos.x;
    int playerRight = playerPos.x + playerSize.x;
    int playerBottom = playerPos.y + playerSize.y;

    bool horizontalOverlap = playerRight > bombLeft + 4 && playerLeft < bombRight - 4;
    bool onTop = playerBottom <= bombBottom + 8 && playerBottom >= bombTop - 4;

    return horizontalOverlap && onTop;
}
