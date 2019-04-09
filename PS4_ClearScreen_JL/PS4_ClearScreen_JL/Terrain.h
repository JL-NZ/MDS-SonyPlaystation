//
// Bachelor of Software Engineering
// Media Design School
// Auckland
// New Zealand
//
// (c) 2005 - 2017 Media Design School
//
// File Name	:	Terrain.h
// Description	:	header file outlining the Terrain
// Author		:	Jasper Lyons & James Monk
// Mail			:	jjlyonsschool@gmail.com & james.mon7482@mediadesign.school.nz
//

#pragma once

#ifndef _TERRAIN_H__
#define _TERRAIN_H__

// Library Includes //
#include <vector>
#include <fstream>
#include <gnmx.h>

// Local Includes //

// Types //

// Constants //

// Prototypes //

class Terrain
{
public:
	Terrain();
	~Terrain();
	void Initialize();
	void BuildVertexBuffer();
	void BuildIndexBuffer();
	void LoadHeightMap();
	float GetHeight(float x, float z) const;
	void Smooth();
	bool InBounds(int _a, int _b);
	float Average(int _a, int _b);

	std::vector<struct Vertex> m_vecVertices;
	std::vector<uint32_t> m_vecIndices;

private:		
	std::vector<float> m_vecHeightMap;	
	
	int m_iNumCols;
	int m_iNumRows;

	std::string m_strFilePath;
	float m_fHeightScale;
	float m_fHeightOffset;
	float m_fWidth;
	float m_fDepth;
};

#endif // _TERRAIN_H__