#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "TileMap.h"


using namespace std;


TileMap *TileMap::createTileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program)
{
	TileMap *map = new TileMap(levelFile, minCoords, program);
	
	return map;
}


TileMap::TileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program)
{
	map = NULL;
	nTiles = 0;
	tileSize = 16;
	blockSize = 16;
	tilesheetSize = glm::ivec2(16, 16);
	mapSize = glm::ivec2(0, 0);
	tileTexSize = glm::vec2(1.f / 16.f, 1.f / 16.f);
	storedMinCoords = minCoords;
	shaderProgram = &program;
	vao = 0;
	vbo = 0;

	if(loadLevel(levelFile))
		prepareArrays(minCoords, program);
}

TileMap::~TileMap()
{
	if(map != NULL)
		delete [] map;
	free();
}


void TileMap::render() const
{
	if(nTiles <= 0)
		return;
	glEnable(GL_TEXTURE_2D);
	tilesheet.use();
	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);
	glDrawArrays(GL_TRIANGLES, 0, 6 * nTiles);
	glDisable(GL_TEXTURE_2D);
}

void TileMap::free()
{
	if(vao != 0)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
	if(vbo != 0)
	{
		glDeleteBuffers(1, &vbo);
		vbo = 0;
	}
	nTiles = 0;
}

bool TileMap::loadLevel(const string &levelFile)
{
	ifstream fin;
	string line, tilesheetFile;
	
	fin.open(levelFile.c_str());
	if(!fin.is_open())
	{
		cerr << "Error: No s'ha pogut obrir el fitxer de nivell: " << levelFile << endl;
		return false;
	}
	getline(fin, line);
	if(line.compare(0, 7, "TILEMAP") != 0)
	{
		cerr << "Error: Format de mapa no vàlid a " << levelFile << endl;
		fin.close();
		return false;
	}

	getline(fin, line);
	{
		stringstream ss(line);
		ss >> mapSize.x >> mapSize.y;
	}

	getline(fin, line);
	{
		stringstream ss(line);
		ss >> tileSize >> blockSize;
	}

	getline(fin, line);
	{
		stringstream ss(line);
		ss >> tilesheetFile;
	}
	tilesheet.loadFromFile(tilesheetFile, TEXTURE_PIXEL_FORMAT_RGBA);
	tilesheet.setWrapS(GL_CLAMP_TO_EDGE);
	tilesheet.setWrapT(GL_CLAMP_TO_EDGE);
	tilesheet.setMinFilter(GL_NEAREST);
	tilesheet.setMagFilter(GL_NEAREST);

	getline(fin, line);
	{
		stringstream ss(line);
		ss >> tilesheetSize.x >> tilesheetSize.y;
	}
	if(tilesheetSize.x <= 0) tilesheetSize.x = 16;
	if(tilesheetSize.y <= 0) tilesheetSize.y = 16;
	if(tileSize <= 0) tileSize = 16;
	if(blockSize <= 0) blockSize = 16;
	tileTexSize = glm::vec2(1.f / float(tilesheetSize.x), 1.f / float(tilesheetSize.y));
	
	if(mapSize.x <= 0 || mapSize.y <= 0)
	{
		cerr << "Error: Dimensions de mapa incorrectes: " << mapSize.x << "x" << mapSize.y << endl;
		fin.close();
		return false;
	}
	
	map = new int[mapSize.x * mapSize.y];
	for(int j=0; j<mapSize.y; j++)
	{
		getline(fin, line);
		if(!line.empty() && line.back() == '\r')
			line.pop_back();

		for(int i=0; i<mapSize.x; i++)
		{
			char c = (i < int(line.size())) ? line[i] : ' ';
			if(c == ' ' || c == '0')
				map[j*mapSize.x+i] = 0;
			else if(c >= '1' && c <= '9')
				map[j*mapSize.x+i] = c - '0';
			else if(c >= 'A' && c <= 'Z')
				map[j*mapSize.x+i] = 10 + (c - 'A');
			else if(c >= 'a' && c <= 'z')
				map[j*mapSize.x+i] = 36 + (c - 'a');
			else
				map[j*mapSize.x+i] = 0;
		}
	}
	fin.close();
	
	return true;
}

void TileMap::prepareArrays(const glm::vec2 &minCoords, ShaderProgram &program)
{
	if(map == NULL || tilesheetSize.x <= 0 || tilesheetSize.y <= 0)
		return;

	int tile;
	glm::vec2 posTile, texCoordTile[2];
	vector<float> vertices;
	
	nTiles = 0;
	for(int j=0; j<mapSize.y; j++)
	{
		for(int i=0; i<mapSize.x; i++)
		{
			tile = map[j * mapSize.x + i];
			if(tile != 0)
			{
				// Non-empty tile
				nTiles++;
				posTile = glm::vec2(minCoords.x + i * tileSize, minCoords.y + j * tileSize);
				texCoordTile[0] = glm::vec2(float((tile-1)%tilesheetSize.x) / float(tilesheetSize.x), float((tile-1)/tilesheetSize.x) / float(tilesheetSize.y));
				texCoordTile[1] = texCoordTile[0] + tileTexSize;
				// First triangle
				vertices.push_back(posTile.x); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
				// Second triangle
				vertices.push_back(posTile.x); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
				vertices.push_back(posTile.x); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[1].y);
			}
		}
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	if(nTiles > 0)
		glBufferData(GL_ARRAY_BUFFER, 24 * nTiles * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	else
		glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
	posLocation = program.bindVertexAttribute("position", 2, 4*sizeof(float), 0);
	texCoordLocation = program.bindVertexAttribute("texCoord", 2, 4*sizeof(float), (void *)(2*sizeof(float)));
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);
}

// Collision tests for axis aligned bounding boxes.
// Method collisionMoveDown also corrects Y coordinate if the box is
// already intersecting a tile below.

bool TileMap::collisionMoveLeft(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int x, y0, y1;
	
	x = pos.x / tileSize;
	y0 = pos.y / tileSize;
	y1 = (pos.y + size.y - 1) / tileSize;
	for(int y=y0; y<=y1; y++)
	{
		if(isTileSolid(x, y) && !isTileJumpThrough(x, y))
			return true;
	}
	
	return false;
}

bool TileMap::collisionMoveRight(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int x, y0, y1;
	
	x = (pos.x + size.x - 1) / tileSize;
	y0 = pos.y / tileSize;
	y1 = (pos.y + size.y - 1) / tileSize;
	for(int y=y0; y<=y1; y++)
	{
		if(isTileSolid(x, y) && !isTileJumpThrough(x, y))
			return true;
	}
	
	return false;
}

bool TileMap::collisionMoveDown(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY, bool ignoreJumpThrough) const
{
	int x0, x1, y;
	
	x0 = pos.x / tileSize;
	x1 = (pos.x + size.x - 1) / tileSize;
	y = (pos.y + size.y - 1) / tileSize;
	for(int x=x0; x<=x1; x++)
	{
		if(isTileSolid(x, y))
		{
			if(ignoreJumpThrough && isTileJumpThrough(x, y))
				continue;
			if(*posY - tileSize * y + size.y <= 10)
			{
				*posY = tileSize * y - size.y;
				return true;
			}
		}
	}
	
	return false;
}

bool TileMap::collisionMoveUp(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const
{
	int x0, x1, y;

	x0 = pos.x / tileSize;
	x1 = (pos.x + size.x - 1) / tileSize;
	y = pos.y / tileSize;
	for(int x=x0; x<=x1; x++)
	{
		if(isTileSolid(x, y) && !isTileJumpThrough(x, y))
		{
			*posY = tileSize * (y + 1);
			return true;
		}
	}

	return false;
}

bool TileMap::isTileJumpThrough(int tileX, int tileY) const
{
	if(tileX < 0 || tileX >= mapSize.x || tileY < 0 || tileY >= mapSize.y)
		return false;
	int t = map[tileY * mapSize.x + tileX];
	// 'A' (10) és la plataforma de pont que es pot travessar saltant des de sota
	return (t == 10 || t == 11 || t == 12);
}

bool TileMap::isTileSolid(int tileX, int tileY) const
{
	if(tileX < 0 || tileX >= mapSize.x || tileY < 0 || tileY >= mapSize.y)
		return true;
	return map[tileY * mapSize.x + tileX] != 0;
}

bool TileMap::isSolidTile(const glm::ivec2 &pos) const
{
	int x = pos.x / tileSize;
	int y = pos.y / tileSize;
	return isTileSolid(x, y);
}

bool TileMap::isDestructibleTile(int tileX, int tileY) const
{
	if(tileX < 0 || tileX >= mapSize.x || tileY < 0 || tileY >= mapSize.y)
		return false;
	return map[tileY * mapSize.x + tileX] == DESTRUCTIBLE_TILE;
}

bool TileMap::isDestructibleTileAt(const glm::ivec2 &pos) const
{
	return isDestructibleTile(pos.x / tileSize, pos.y / tileSize);
}

bool TileMap::destroyTile(int tileX, int tileY, bool rebuild)
{
	if(!isDestructibleTile(tileX, tileY))
		return false;

	map[tileY * mapSize.x + tileX] = 0;
	if(rebuild)
		rebuildArrays();
	return true;
}

void TileMap::rebuildMesh()
{
	rebuildArrays();
}

void TileMap::rebuildArrays()
{
	if(shaderProgram == NULL)
		return;
	free();
	prepareArrays(storedMinCoords, *shaderProgram);
}































