#pragma once
///#include "Dependencies\glew\glew.h"
///#include "Dependencies\freeglut\freeglut.h"
///#include "Dependencies\glm\glm.hpp"
#include <vector>
#include <fstream>

struct TerrainVertex
{
	Vector3 v3Pos;
	Vector3 v3Color;
};

class Terrain
{
public:
	Terrain();
	~Terrain();
	void Initialize();
	void BuildVertexBuffer();
	void BuildIndexBuffer();
	void LoadHeightMap();
	void LoadVegeMap();
	float GetHeight(float x, float z) const;
	void Smooth();
	bool InBounds(int _a, int _b);
	float Average(int _a, int _b);
	void Render();
	void RenderGrass();

private:		
	int m_vbo;
	int m_vao;
	int m_program;
	int m_grassProgram;
	int m_texture;
	
	std::vector<TerrainVertex> m_vecVertices;
	std::vector<int> m_vecIndices;
	std::vector<float> m_vecHeightMap;
	std::vector<float> m_vecVegeMap;
	int m_iNumIndices;
	
	int m_iNumCols;
	int m_iNumRows;

	std::string m_strFilePath;
	float m_fHeightScale;
	float m_fHeightOffset;
	float m_fWidth;
	float m_fDepth;
};

