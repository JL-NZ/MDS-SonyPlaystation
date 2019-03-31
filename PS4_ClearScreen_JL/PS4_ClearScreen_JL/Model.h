#pragma once
#include <string>
#include <gnmx.h>

#include "Utils.h"
#include "std_cbuffer.h"

using namespace sce;
using namespace sce::Gnmx;

enum ModelType
{
	kTriangle,
	kQuad,
	kCube,
	kSphere
};

//unistruct ShaderConstants
//{
//	Matrix4Unaligned m_WorldViewProj;
//};

#pragma once
class Model
{
public:
	Model(ModelType modelType);
	~Model();

	// Member Functions
	void Draw();
	bool genFetchShaderAndOffsetCache(std::string vertexBinary, std::string fragmentBinary);
		
	Gnm::Buffer vertexBuffers[kVertexElemCount];

	VsShader *vsShader;
	PsShader *psShader;

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;	
	
	void *fsMem;
	uint16_t *indexData;
	uint32_t shaderModifier;

	sce::Gnmx::InputOffsetsCache vsInputOffsetsCache; 
	sce::Gnmx::InputOffsetsCache psInputOffsetsCache;

	// Initialize a Gnm::Texture object
	Gnm::Texture texture;
	Gnm::SizeAlign textureSizeAlign;
	
	// Initialize the texture sampler
	Gnm::Sampler sampler;

	// Raw texture functions
	bool InitializeRawTextures();
	bool DrawRawTextures();

	// GNF texture functions
	bool InitializeGNFTextures();
	bool DrawGNFTextures();
};

