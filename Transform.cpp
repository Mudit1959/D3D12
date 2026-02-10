#include "Transform.h"

Transform::Transform()
{
	SetPosition(0.0f, 0.0f, 0.0f);
	SetScale(1.0f, 1.0f, 1.0f);
	SetRotation(0.0f, 0.0f, 0.0f);
	DirectX::XMMATRIX initialMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMStoreFloat4x4(&world, initialMatrix);
	DirectX::XMStoreFloat4x4(&world, DirectX::XMMatrixInverse(0, DirectX::XMMatrixTranspose(initialMatrix)));
	relUp = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	relForward = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
	relRight = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	edited = 0;
}

void Transform::SetPosition(float x, float y, float z) { position = DirectX::XMFLOAT3(x, y, z); edited++; };
void Transform::SetPosition(DirectX::XMFLOAT3 pos) { position = pos; edited++; }
void Transform::SetRotation(float pitch, float yaw, float roll) { rotation = DirectX::XMFLOAT3(pitch, yaw, roll); edited++; }
void Transform::SetRotation(DirectX::XMFLOAT3 ro) { rotation = ro; edited++; } // XMFLOAT4 for quaternion
void Transform::SetScale(float x, float y, float z) { scale = DirectX::XMFLOAT3(x, y, z); edited++; }
void Transform::SetScale(DirectX::XMFLOAT3 s) { scale = s; edited++; }

//Getters
DirectX::XMFLOAT3 Transform::GetPosition() { return position; }
DirectX::XMFLOAT3 Transform::GetPitchYawRoll() { return rotation; } // XMFLOAT4 GetRotation() for quaternion
DirectX::XMFLOAT3 Transform::GetScale() { return scale; }
DirectX::XMFLOAT3 Transform::GetForward() { return relForward; }
DirectX::XMFLOAT3 Transform::GetRight() { return relRight; }
DirectX::XMFLOAT3 Transform::GetUp() { return relUp; }


//Movements - Simplified by performing Math and Load within the Store function
void Transform::MoveAbsolute(float x, float y, float z)
{
	DirectX::XMStoreFloat3(&position, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position), DirectX::XMVectorSet(x, y, z, 1.0f)));
	edited++;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	DirectX::XMStoreFloat3(&position, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position), DirectX::XMVectorSet(offset.x, offset.y, offset.z, 1.0f)));
	edited++;
}

void Transform::CalculateOrientation()
{
	DirectX::XMVECTOR quat = DirectX::XMQuaternionRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&rotation));
	relUp = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	relForward = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
	relRight = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	DirectX::XMStoreFloat3(&relUp, DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&relUp), quat));
	DirectX::XMStoreFloat3(&relRight, DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&relRight), quat));
	DirectX::XMStoreFloat3(&relForward, DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&relForward), quat));
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	DirectX::XMStoreFloat3(&rotation, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&rotation), DirectX::XMVectorSet(pitch, yaw, roll, 1.0f)));
	CalculateOrientation();
	edited++;
}

void Transform::Rotate(DirectX::XMFLOAT3 ro)
{
	DirectX::XMStoreFloat3(&rotation, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&rotation), DirectX::XMVectorSet(ro.x, ro.y, ro.z, 1.0f)));
	CalculateOrientation();
	edited++;
}

// Scale needs to be multiplied!
void Transform::Scale(float x, float y, float z)
{
	DirectX::XMStoreFloat3(&rotation, DirectX::XMVectorMultiply(DirectX::XMLoadFloat3(&scale), DirectX::XMVectorSet(x, y, z, 1.0f)));
	edited++;
}

void Transform::Scale(DirectX::XMFLOAT3 scale)
{
	DirectX::XMStoreFloat3(&rotation, DirectX::XMVectorMultiply(DirectX::XMLoadFloat3(&scale), DirectX::XMVectorSet(scale.x, scale.y, scale.z, 1.0f)));
	edited++;
}

// Checks if any edits have been made using a counter. If there are edits it will recalculate, otherwise, it will return the float4x4 as is. 
DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if (edited == 0) { return world; }
	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) * DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) * DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	DirectX::XMStoreFloat4x4(&world, worldMatrix);
	DirectX::XMStoreFloat4x4(&worldInverseT,
		DirectX::XMMatrixInverse(0, DirectX::XMMatrixTranspose(worldMatrix)));
	edited = 0;
	return world;
}
DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	if (edited == 0) { return worldInverseT; }
	GetWorldMatrix();
	edited = 0;
	return worldInverseT;
}

void Transform::MoveRelative(DirectX::XMFLOAT3 offset)
{
	DirectX::XMFLOAT3 iForward = DirectX::XMFLOAT3(relForward.x * offset.x, relForward.y * offset.x, relForward.z * offset.x);
	DirectX::XMFLOAT3 iRight = DirectX::XMFLOAT3(relRight.x * offset.y, relRight.y * offset.y, relRight.z * offset.y);
	DirectX::XMFLOAT3 iUp = DirectX::XMFLOAT3(relUp.x * offset.z, relUp.y * offset.z, relUp.z * offset.z);

	DirectX::XMStoreFloat3(&position, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),
		DirectX::XMVectorAdd(DirectX::XMVectorAdd(
			DirectX::XMLoadFloat3(&iForward),
			DirectX::XMLoadFloat3(&iRight)),
			DirectX::XMLoadFloat3(&iUp))
	));
	edited++;
}

void Transform::MoveRelative(float x, float y, float z)
{
	DirectX::XMFLOAT3 iForward = DirectX::XMFLOAT3(relForward.x * x, relForward.y * x, relForward.z * x);
	DirectX::XMFLOAT3 iRight = DirectX::XMFLOAT3(relRight.x * y, relRight.y * y, relRight.z * y);
	DirectX::XMFLOAT3 iUp = DirectX::XMFLOAT3(relUp.x * z, relUp.y * z, relUp.z * z);

	DirectX::XMStoreFloat3(&position, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),
		DirectX::XMVectorAdd(DirectX::XMVectorAdd(
			DirectX::XMLoadFloat3(&iForward),
			DirectX::XMLoadFloat3(&iRight)),
			DirectX::XMLoadFloat3(&iUp))
	));
	edited++;
}
