#pragma once
#include <d3d12.h>
#include<wrl/client.h>
#include "Vertex.h"
#include "Graphics.h"
#include <DirectXMath.h>
#include <fstream>
#include <stdexcept>
#include <vector>

/*
* This Mesh class makes use of the predefined Vertex struct.
* If you want to draw images that use vertices of a different format, e.g, only RGBA or some depth factor, or some shade value, need to rework this class/
*
*
* GetVertexBuffer(): Returns the vertex buffer ComPtr
* GetIndexBuffer(): Returns the index buffer ComPtr
* GetIndexCount(): Returns the number of indices this mesh contains
* GetVertexCount(): Returns the number of vertices this mesh contains
* Draw(): Sets the buffers and draws using the correct number of indices
* Refer to Game::Draw() to see the code necessary for setting buffers and drawing
*/


using namespace DirectX;

class Mesh
{
public:

	// ~ Constructor, Copy Constructor, Copy Assignment, Destructor
	Mesh(Vertex* v, int vCount, unsigned int* i, int iCount); // Constructor
	Mesh(const char* objFilePath);
	void CreateBuffers(Vertex* v, int vCount, unsigned int* i, int iCount);
	//Mesh(const Mesh& other); // Copy Constructor
	//Mesh& operator= (const Mesh& other); // Copy Assignment
	~Mesh();

	Microsoft::WRL::ComPtr<ID3D12Resource> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D12Resource> GetIndexBuffer();
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);
	int GetIndexCount();
	int GetVertexCount();
	

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer; // Contains all the necessary vertices
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer; // Contains all the indices - drawn in groups of 3 (triangle drawing mode)
	D3D12_VERTEX_BUFFER_VIEW vbView;
	D3D12_INDEX_BUFFER_VIEW ibView;
	int indexCount, vertexCount;
};
