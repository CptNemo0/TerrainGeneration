#include "../include/FPCamera.h"

FPCamera::FPCamera()
{
	position = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	target_position = DirectX::XMVector3Normalize(DirectX::XMVectorSet(1.0f, 0.0f, 1.0f, 0.0f));
	up_direction = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
}

void FPCamera::UpdateTargetPosition(int dx, int dy)
{
	x += static_cast<float>(dx) * 0.001f * sensitivity;
	y += static_cast<float>(dy) * 0.001f * sensitivity;

	if (x >= 3.14159 * 2.0f)
	{
		x = 0.0f;
	}

	if (y > 3.14159 * 0.5f)
	{
		y = 3.14159 * 0.5f;
	}

	if (y < -3.14159 * 0.5f)
	{
		y = -3.14159 * 0.5f;
	}

	float x_cos = cosf(x);
	float z_sin = sinf(-x);
	float y_sin = -y / (3.14159 * 0.5f);
	
	forward = DirectX::XMVector3Normalize(DirectX::XMVectorSet(x_cos,y_sin, z_sin, 0.0f));
	target_position = DirectX::XMVectorAdd(position, forward);
	right = DirectX::XMVector3Cross(up_direction, forward);
}

void FPCamera::MoveCamera(float dt)
{
	if (forward_s || right_s)
	{
		velocity += acceleration * dt;
	}
	else
	{
		velocity *= 0.75f;
	}

	if (velocity > max_velocity) velocity = max_velocity;

	std::cout << velocity << std::endl;
	auto forward_move = DirectX::XMVectorScale(forward, forward_s);
	auto right_move = DirectX::XMVectorScale(right, right_s);
	auto move = DirectX::XMVectorAdd(forward_move, right_move);
	move = DirectX::XMVector3Normalize(move);
	move = DirectX::XMVectorScale(move, velocity);
	position = DirectX::XMVectorAdd(position, move);
}

DirectX::XMMATRIX FPCamera::GetViewMatrix()
{
	return DirectX::XMMatrixLookAtLH(position, target_position, up_direction);
}
