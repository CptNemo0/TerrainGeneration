#pragma once

#include "DirectXMath.h"
#include <iostream>
class FPCamera
{
public:
	DirectX::XMVECTOR up_direction;
	DirectX::XMVECTOR position;
	DirectX::XMVECTOR target_position;
	DirectX::XMVECTOR forward;
	DirectX::XMVECTOR right;

	int previous_x = 0;
	int previous_y = 0;

	float x = 0.0f;
	float y = 0.0f;

	float forward_s = 0.0f;
	float right_s = 0.0f;

	float velocity = 0.0f;
	float max_velocity = 0.005f;
	float sensitivity = 2.0f;
	float acceleration = 0.1f;
	float smoothing_factor = 0.9f;
	

	FPCamera();

	void UpdateTargetPosition(int x, int y);
	void MoveCamera(float dt);
	DirectX::XMMATRIX GetViewMatrix();
};