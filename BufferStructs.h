#pragma once
#include <DirectXMath.h>
#include "Light.h"


struct VSConstants
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 worldInv;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 proj;
};

struct PSConstants 
{
	unsigned int albedoIndex;
	unsigned int normalIndex;
	unsigned int roughnessIndex;
	unsigned int metalnessIndex;

	DirectX::XMFLOAT2 UVScale;
	DirectX::XMFLOAT2 UVOffset;

	DirectX::XMFLOAT3 cameraWorldPos;
	int lights;

	Light light[MAX_LIGHTS];

};
