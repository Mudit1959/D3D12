#include "Camera.h"
#include "Input.h"

Camera::Camera(float x, float y, float z, float asR, bool pers)
{
	transform.SetPosition(x, y, z);
	aspectRatio = asR;
	nearClip = 0.01f;
	farClip = 400.0f;
	FOV = DirectX::XM_PIDIV4;
	perspective = perspective;
	movementSpeed = 10.0f;
	mouseLookSpeed = 1.5f;
	UpdateProjMatrix(asR);
	UpdateViewMatrix();
}

void Camera::UpdateProjMatrix(float aspectRatio)
{
	DirectX::XMStoreFloat4x4(&proj, DirectX::XMMatrixPerspectiveFovLH(FOV, aspectRatio, nearClip, farClip));
}
void Camera::UpdateViewMatrix()
{
	DirectX::XMFLOAT3 pos = transform.GetPosition();
	DirectX::XMFLOAT3 forward = transform.GetForward();
	DirectX::XMFLOAT3 worldUp = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	DirectX::XMStoreFloat4x4(&view, DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&forward),
		DirectX::XMLoadFloat3(&worldUp)));
}

DirectX::XMFLOAT4X4 Camera::GetProj() { return proj; }
DirectX::XMFLOAT4X4 Camera::GetView() { return view; }

void Camera::Update(float dt)
{
	if (Input::KeyDown('W')) { transform.MoveRelative(dt * movementSpeed, 0, 0); }
	if (Input::KeyDown('S')) { transform.MoveRelative(-dt * movementSpeed, 0, 0); }
	if (Input::KeyDown('A')) { transform.MoveRelative(0, -dt * movementSpeed, 0); }
	if (Input::KeyDown('D')) { transform.MoveRelative(0, dt * movementSpeed, 0); }
	if (Input::KeyDown('X')) { transform.MoveAbsolute(0, -dt * movementSpeed, 0); }
	if (Input::KeyDown(VK_SPACE)) { transform.MoveAbsolute(0, dt * movementSpeed, 0); }

	if (Input::MouseLeftDown())
	{
		int cursorMovementX = Input::GetMouseXDelta();
		int cursorMovementY = Input::GetMouseYDelta();
		float movX = cursorMovementX / 30.0f;
		float movY = cursorMovementY / 30.0f;
		float futurePitch = transform.GetPitchYawRoll().x + movY;
		if (futurePitch > DirectX::XM_PIDIV2) { futurePitch = DirectX::XM_PIDIV2; }
		if (futurePitch < -DirectX::XM_PIDIV2) { futurePitch = -DirectX::XM_PIDIV2; }
		transform.Rotate(movY, movX, 0);
		/* Other mouse movement code here */
	}

	UpdateViewMatrix();
}

DirectX::XMFLOAT3 Camera::GetPos() { return transform.GetPosition(); }
