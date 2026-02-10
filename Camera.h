#pragma once
#include <DirectXMath.h>
#include "Transform.h"

class Camera
{
public:
	Camera(float x, float y, float z, float asR, bool pers = true);
	void UpdateProjMatrix(float aspectRatio);
	void UpdateViewMatrix();

	//Getters
	DirectX::XMFLOAT4X4 GetProj(), GetView();
	DirectX::XMFLOAT3 GetPos();

	void Update(float dt);

private:
	Transform transform;
	DirectX::XMFLOAT4X4 proj, view;
	float nearClip, farClip, movementSpeed, mouseLookSpeed, FOV, aspectRatio;
	bool perspective;

};
