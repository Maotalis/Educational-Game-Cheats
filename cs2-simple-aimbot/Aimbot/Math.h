#pragma once
#include <Windows.h>
#include <cmath>
#include <corecrt_math_defines.h>
#include <numbers>

float screenWidth = GetSystemMetrics(SM_CXSCREEN);
float screenHeight = GetSystemMetrics(SM_CYSCREEN);

struct view_matrix_t
{
	float* operator[](int index)
	{
		return matrix[index];
	}

	float matrix[4][4];
};

class Vector
{
public:
	constexpr Vector(
		const float x = 0.f,
		const float y = 0.f,
		const float z = 0.f) noexcept :
		x(x), y(y), z(z) {
	}

	constexpr const Vector& operator-(const Vector& other) const noexcept
	{
		return Vector{ x - other.x, y - other.y, z - other.z };
	}

	constexpr const Vector& operator+(const Vector& other) const noexcept
	{
		return Vector{ x + other.x, y + other.y, z + other.z };
	}

	constexpr const Vector& operator/(const float factor) const noexcept
	{
		return Vector{ x / factor, y / factor, z / factor };
	}

	constexpr const Vector& operator*(const float factor) const noexcept
	{
		return Vector{ x * factor, y * factor, z * factor };
	}

	const bool IsZero() {
		return x == 0.0f && y == 0.0f && z == 0.0f;
	}


	float x, y, z;
};

const bool WorldToScreen(view_matrix_t vm, Vector& in, Vector& out)
{
	out.x = vm[0][0] * in.x + vm[0][1] * in.y + vm[0][2] * in.z + vm[0][3];
	out.y = vm[1][0] * in.x + vm[1][1] * in.y + vm[1][2] * in.z + vm[1][3];

	float width = vm[3][0] * in.x + vm[3][1] * in.y + vm[3][2] * in.z + vm[3][3];

	if (width < 0.01f) {
		return false;
	}

	float inverseWidth = 1.f / width;

	out.x *= inverseWidth;
	out.y *= inverseWidth;

	float x = screenWidth / 2;
	float y = screenHeight / 2;

	x += 0.5f * out.x * screenWidth + 0.5f;
	y -= 0.5f * out.y * screenHeight + 0.5f;

	out.x = x;
	out.y = y;

	return true;
}

float Distance(Vector p1, Vector p2) {
	return std::sqrt(std::pow(p1.x - p2.x, 2) + std::pow(p1.y - p2.y, 2));
}


Vector RelativeAngles(Vector targetPos, Vector localEyePos)
{
	// Difference between target and local eye position
	float deltaX = targetPos.x - localEyePos.x;
	float deltaY = targetPos.y - localEyePos.y;
	float deltaZ = targetPos.z - localEyePos.z;

	// Calculate pitch (up/down) angle
	float pitch = std::atan2(-deltaZ, std::hypot(deltaX, deltaY)) * (180.0f / std::numbers::pi_v<float>);

	// Calculate yaw (left/right) angle
	float yaw = std::atan2(deltaY, deltaX) * (180.0f / std::numbers::pi_v<float>);

	return { pitch, yaw, 0.0f };
}
