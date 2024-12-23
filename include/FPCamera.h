#pragma once

#include "DirectXMath.h"
#include <iostream>
#include "noise.h"
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
	float previous_y_sin = 0.0f;

	float height = 0.0f;

	float forward_s = 0.0f;
	float right_s = 0.0f;

	float velocity = 0.0f;
	float max_velocity = 0.2f;
	float sensitivity = 1.0f;
	float acceleration = 1.0f;
	float smoothing_factor = 0.9f;
	
	bool fly = false;

	float bob_t = 0.0f;
	float bob_depth = 1.0f;
	float bob_speed = 7.5f;
	float bob_height = 0;

	FPCamera();

	void UpdateTargetPosition(int x, int y);
	void MoveCamera(float dt);
	DirectX::XMMATRIX GetViewMatrix();
};