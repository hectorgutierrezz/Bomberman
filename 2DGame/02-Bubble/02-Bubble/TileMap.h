#ifndef _TILE_MAP_INCLUDE
#define _TILE_MAP_INCLUDE

#include <glm/glm.hpp>
#include "Texture.h"
#include "ShaderProgram.h"

// Class Tilemap is capable of loading a tile map from a text file in a very
// simple format (see level01.txt for an example). With this information
// it builds a single VBO that contains all tiles. As a result the render
// method draws the whole map independently of what is visible.

class TileMap {
public:
  enum TileType {
    TILE_EMPTY = 0,
    TILE_DESTRUCTIBLE = 56
  };

private:
  TileMap(const string &levelFile, const glm::vec2 &minCoords,
          ShaderProgram &program);

public:
  static TileMap *createTileMap(const string &levelFile,
                                const glm::vec2 &minCoords,
                                ShaderProgram &program);

  ~TileMap();

  void render() const;
  void free();

  bool collisionMoveLeft(const glm::ivec2 &pos, const glm::ivec2 &size) const;
  bool collisionMoveRight(const glm::ivec2 &pos, const glm::ivec2 &size) const;
  bool collisionMoveDown(const glm::ivec2 &pos, const glm::ivec2 &size,
                         int *posY, bool ignoreJumpThrough = false) const;
  bool collisionMoveUp(const glm::ivec2 &pos, const glm::ivec2 &size,
                       int *posY) const;
  bool isSolidTile(const glm::ivec2 &pos) const;
  bool isTileSolid(int tileX, int tileY) const;
  bool isTileJumpThrough(int tileX, int tileY) const;
  bool isTileDestructible(int tileX, int tileY) const;
  bool destroyTile(int tileX, int tileY);
  int getTileType(int tileX, int tileY) const;

  int getTileSize() const { return tileSize; }
  int getBlockSize() const { return blockSize; }
  int getMapWidthTiles() const { return mapSize.x; }
  int getMapHeightTiles() const { return mapSize.y; }
  // Mida total del mapa en pixels de mon (per al clamping de la camera)
  int getMapWidth() const { return mapSize.x * tileSize; }
  int getMapHeight() const { return mapSize.y * tileSize; }

private:
  bool loadLevel(const string &levelFile);
  void prepareArrays(const glm::vec2 &minCoords, ShaderProgram &program);

private:
  GLuint vao;
  GLuint vbo;
  GLint posLocation, texCoordLocation;
  int nTiles;
  glm::ivec2 position, mapSize, tilesheetSize;
  int tileSize, blockSize;
  Texture tilesheet;
  glm::vec2 tileTexSize;
  int *map;
  glm::vec2 mapMinCoords;
  ShaderProgram *shaderProgram;
};

#endif // _TILE_MAP_INCLUDE

