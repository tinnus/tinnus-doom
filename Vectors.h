#pragma once

#include <cmath>

/*
struct Fixed
{
	signed int Value;
		result.Value = (int)(double(value) * (1 << Precision));
		return result;
	}

	float ToFloat()
	{
		return (float)Value / (1 << Precision);
	}

	Fixed operator+=(const Fixed& other)
	{
		Value += other.Value;
		return *this;
	}

	Fixed operator+(const Fixed& other)
	{
		Fixed result = *this;
		return result += other;
	}

	Fixed operator-=(const Fixed& other)
	{
		Value -= other.Value;
		return *this;
	}

	Fixed operator-(const Fixed& other)
	{
		Fixed result = *this;
		return result -= other;
	}

	Fixed operator*(const Fixed& other)
	{
		Fixed result = *this;
		auto resultExtended = (signed long long)result.Value * other.Value;
		result.Value = resultExtended >> Precision;
		return result;
	}
};
*/

struct Vec2
{
	float X, Y;

	Vec2(): Vec2(0, 0) {}

	Vec2(float x, float y)
		: X(x), Y(y) {}

	Vec2 operator+=(const Vec2& other);
	Vec2 operator-=(const Vec2& other);
	Vec2 operator*=(const float& other);
	Vec2 operator/=(const float& other);

	Vec2 operator+(const Vec2& other) const;
	Vec2 operator-(const Vec2& other) const;
	Vec2 operator*(const float& other) const;
	Vec2 operator/(const float& other) const;

	float SqrLength() const;
	float Length() const;

	Vec2 Normalized() const;
	void Normalize();

	static float Dot(const Vec2& lhs, const Vec2& rhs);
	static float Cross(const Vec2& lhs, const Vec2& rhs);
};

Vec2 operator*(const float& other, const Vec2 vec);

float Clamp(float value, float min, float max);
float Repeat(float value, float period);

template<typename T>
T Lerp(T a, T b, float alpha)
{
	return a + alpha * (b - a);
}

template<typename T>
float InverseLerp(T a, T b, T value)
{
	return (float)(value - a) / (b - a);
}