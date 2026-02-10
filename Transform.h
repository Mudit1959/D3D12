#pragma once
#include <DirectXMath.h>
class Transform
{
public:
	//Setters
	void SetPosition(float x, float y, float z);
	void SetPosition(DirectX::XMFLOAT3 pos);
	void SetRotation(float pitch, float yaw, float roll);
	void SetRotation(DirectX::XMFLOAT3 ro);  // XMFLOAT4 for quaternion
	void SetScale(float x, float y, float z);
	void SetScale(DirectX::XMFLOAT3 s);

	//Getters
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetPitchYawRoll();  // XMFLOAT4 GetRotation() for quaternion
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();
	DirectX::XMFLOAT3 GetForward();
	DirectX::XMFLOAT3 GetRight();
	DirectX::XMFLOAT3 GetUp();

	//Movements
	void MoveAbsolute(float x, float y, float z);
	void MoveAbsolute(DirectX::XMFLOAT3 offset);
	void Rotate(float pitch, float yaw, float roll);
	void Rotate(DirectX::XMFLOAT3 rotation);
	void Scale(float x, float y, float z);
	void Scale(DirectX::XMFLOAT3 scale);
	void MoveRelative(float x, float y, float z);
	void MoveRelative(DirectX::XMFLOAT3 offset);
	void CalculateOrientation();

	Transform();

private:
	DirectX::XMFLOAT3 position, scale, rotation;
	DirectX::XMFLOAT3 relUp, relForward, relRight;
	DirectX::XMFLOAT4X4 world, worldInverseT;
	int edited;
};