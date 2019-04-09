//
// Bachelor of Software Engineering
// Media Design School
// Auckland
// New Zealand
//
// (c) 2005 - 2017 Media Design School
//
// File Name	:	Utils.h
// Description	:	header file outlining the Utils
// Author		:	Jasper Lyons & James Monk
// Mail			:	jjlyonsschool@gmail.com & james.mon7482@mediadesign.school.nz
//

#pragma once

#ifndef _UTILS_H__
#define _UTILS_H__

// Library Includes //
#include <vector>
#include <gnmx.h>

// Local Includes //
#include "common/allocator.h"
#include "api_gnm/toolkit/toolkit.h"
#include "Terrain.h"

// Types //
typedef struct Vertex
{
	float x, y, z; // Position
	float r, g, b;// Color
	float u, v;// UVs
	Vertex() {}
	Vertex(float _x, float _y, float _z, float _r, float _g, float _b, float _u, float _v) {
		x = _x; y = _y; z = _z; r = _r; g = _g; b = _b; u = _u; v = _v;
	}
} Vertex;

enum VertexElements
{
	kVertexPosition = 0,
	kVertexColor,
	kVertexUv,
	kVertexElemCount // element means attribute
};

// Constants //

// Prototypes //

class Utils
{
public:
	Utils();
	~Utils();

	static void setTriData(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	static void setQuadData(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	static void setCubeData(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	static void setSphereData(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	static void setTerrainData(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	
};

#endif // _UTILS_H__