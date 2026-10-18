#ifndef _TILE_MAP_INCLUDE
#define _TILE_MAP_INCLUDE


#include <glm/glm.hpp>
#include "Texture.h"
#include "ShaderProgram.h"


// Class Tilemap is capable of loading a tile map from a text file in a very
// simple format (see level01.txt for an example). With this information
// it builds a single VBO that contains all tiles. As a result the render
// method draws the whole map independently of what is visible.


class TileMap
{

private:
	TileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program);

public:
	// Identificador del tile destruïble (caràcter 'D' als fitxers de nivell)
	static const int DESTRUCTIBLE_TILE = 13;

	// Tile maps can only be created inside an OpenGL context
	static TileMap *createTileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program);

	~TileMap();

	void render() const;
	void free();
	
	bool collisionMoveLeft(const glm::ivec2 &pos, const glm::ivec2 &size) const;
	bool collisionMoveRight(const glm::ivec2 &pos, const glm::ivec2 &size) const;
	bool collisionMoveDown(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY, bool ignoreJumpThrough = false) const;
	bool collisionMoveUp(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const;
	bool isSolidTile(const glm::ivec2 &pos) const;
	bool isTileSolid(int tileX, int tileY) const;
	bool isTileJumpThrough(int tileX, int tileY) const;
	bool isDestructibleTile(int tileX, int tileY) const;
	bool isDestructibleTileAt(const glm::ivec2 &pos) const;
	// Destrueix un bloc D: el treu del mapa i regenera la malla. Retorna true si s'ha trencat.
	bool destroyTile(int tileX, int tileY, bool rebuild = true);
	void rebuildMesh();
	int getTileSize() const { return tileSize; }
	int getBlockSize() const { return blockSize; }
	// Mida total del mapa en píxels de món (per al clamping de la càmera)
	int getMapWidth()  const { return mapSize.x * tileSize; }
	int getMapHeight() const { return mapSize.y * tileSize; }
	
private:
	bool loadLevel(const string &levelFile);
	void prepareArrays(const glm::vec2 &minCoords, ShaderProgram &program);
	void rebuildArrays();

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
	glm::vec2 storedMinCoords;
	ShaderProgram *shaderProgram;

};


#endif // _TILE_MAP_INCLUDE
