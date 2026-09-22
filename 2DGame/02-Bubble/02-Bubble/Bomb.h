#ifndef _BOMB_INCLUDE
#define _BOMB_INCLUDE

#include <glm/glm.hpp>
#include "Sprite.h"
#include "Texture.h"

class Bomb
{
public:
    Bomb();
    ~Bomb();

    void init(ShaderProgram &shaderProgram);
    void update(int deltaTime);
    void render();

    void setPosition(const glm::vec2 &pos);
    bool isActive() const { return active; }
    bool isSolidOnTop(const glm::ivec2 &playerPos, const glm::ivec2 &playerSize) const;
    int getTop() const { return int(position.y); }
    const glm::vec2 &getPosition() const { return position; }

private:
    Texture texture;
    Sprite *sprite;
    glm::vec2 position;
    float fuseTime;
    bool active;
};

#endif // _BOMB_INCLUDE
