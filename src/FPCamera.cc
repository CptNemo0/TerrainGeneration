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
	float y_sin = -5.0f * y / (3.14159 * 0.5f);
	
	auto target_forward = DirectX::XMVector3Normalize(DirectX::XMVectorSet(x_cos, y_sin, z_sin, 1.0f));

	forward = DirectX::XMQuaternionSlerp(forward, target_forward, 0.1f);
	forward = DirectX::XMVectorSet(DirectX::XMVectorGetX(forward), y_sin, DirectX::XMVectorGetZ(forward), 0.0f);

	target_position = DirectX::XMVectorAdd(position, forward);
	right = DirectX::XMVector3Cross(up_direction, forward);
}

void FPCamera::MoveCamera(float dt)
{
	if (forward_s || right_s)
	{
		velocity += acceleration * dt;
		bob_t += bob_speed * dt;
		bob_height = sinf(bob_t) * bob_depth;
	}
	else
	{
		bob_height *= (1.0f - bob_speed);
		bob_t = 0.0f;
		velocity *= 0.75f;
	}

	if (velocity > max_velocity) 
	{
		if (fly)
		{
			if (velocity > 10.0f * max_velocity)
			{
				velocity = max_velocity * 10.0f;
			}
		}
		else
		{
			velocity = max_velocity;
		}
	}

	
	auto forward_move = DirectX::XMVectorScale(forward, forward_s);
	auto right_move = DirectX::XMVectorScale(right, right_s);
	auto move = DirectX::XMVectorAdd(forward_move, right_move);
	move = DirectX::XMVector3Normalize(move);
	move = DirectX::XMVectorScale(move, velocity);

	SimplexNoise noise;
	float local_y = noise.signedFBM(DirectX::XMVectorGetX(position), DirectX::XMVectorGetZ(position), 5, 1.5f, 10.0f, 0.0001);
	local_y *= 100.0f;
	height = height * 0.5f + local_y * 0.5f;
	height += 10.0f;
	
	//local_y *= 1500.0f + 5.0f;
	//height = height * 0.5f + local_y * 0.5f;
	

	position = DirectX::XMVectorAdd(position, move);

	if (fly) return;

	//height += bob_height;
	position = DirectX::XMVectorSet(DirectX::XMVectorGetX(position), height, DirectX::XMVectorGetZ(position), 0.0f);

	
}

DirectX::XMMATRIX FPCamera::GetViewMatrix()
{
	return DirectX::XMMatrixLookAtLH(position, target_position, up_direction);
}
