#include "Utils.h"



Utils::Utils()
{
}


Utils::~Utils()
{
}

void Utils::setTriData(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices)
{
	std::vector<Vertex> Vertices;
	Vertices.push_back(Vertex(-1.0f, 0.0f, 0.0f,	 1.0f, 0.0f, 0.0f,		 0.0f, 1.0f));
	Vertices.push_back(Vertex(0.0f, 1.0f, 0.0f,		 0.0f, 1.0f, 0.0f,		 0.0f, 0.0f));
	Vertices.push_back(Vertex(1.0f, 0.0f, 0.0f,		 0.0f, 0.0f, 1.0f,		 1.0f, 1.0f));

	Vertices.push_back(Vertex(-0.5f, -0.5f, 0.0f, 0.7f, 0.2f, 1.0f, 0.0f, 1.0f));
	Vertices.push_back(Vertex(0.5f, -0.5f, 0.0f, 0.1f, 0.7f, 0.2f, 1.0f, 1.0f));
	Vertices.push_back(Vertex(-0.5f, 0.5f, 0.0f, 0.7f, 1.0f, 1.0f, 0.0f, 0.0f));

	std::vector<uint16_t> Indices;
	Indices.push_back(0);
	Indices.push_back(2);
	Indices.push_back(1);

	vertices.clear(); indices.clear();

	vertices = Vertices;
	indices = Indices;
}