#pragma once

#include <cmath>

struct Fixed
{
	signed int Value;

	static const int Precision = 16;

	static Fixed FromFloat(float value)
	{
		Fixed result;
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

struct Vec2
{
	float X, Y;

	Vec2(): Vec2(0, 0) {}

	Vec2(float x, float y)
		: X(x), Y(y) {}

	Vec2 operator+=(const Vec2& other)
	{
		X += other.X;
		Y += other.Y;
		return *this;
	}

	Vec2 operator-=(const Vec2& other)
	{
		X -= other.X;
		Y -= other.Y;
		return *this;
	}

	Vec2 operator*=(const float& other)
	{
		X *= other;
		Y *= other;
		return *this;
	}

	Vec2 operator/=(const float& other)
	{
		X /= other;
		Y /= other;
		return *this;
	}

	Vec2 operator+(const Vec2& other) const
	{
		Vec2 result = *this;
		result += other;
		return result;
	}

	Vec2 operator-(const Vec2& other) const
	{
		Vec2 result = *this;
		result -= other;
		return result;
	}

	Vec2 operator*(const float& other) const
	{
		Vec2 result = *this;
		result *= other;
		return result;
	}

	Vec2 operator/(const float& other) const
	{
		Vec2 result = *this;
		result /= other;
		return result;
	}

	float SqrLength() const
	{
		return Dot(*this, *this);
	}

	float Length() const
	{
		return sqrt(SqrLength());
	}

	Vec2 Normalized() const
	{
		return *this / Length();
	}

	void Normalize()
	{
		*this = Normalized();
	}

	static float Dot(const Vec2& lhs, const Vec2& rhs)
	{
		return lhs.X * rhs.X + lhs.Y * rhs.Y;
	}

	static float Cross(const Vec2& lhs, const Vec2& rhs)
	{
		return lhs.X * rhs.Y - lhs.Y * rhs.X;
	}
};

Vec2 operator*(const float& other, const Vec2 vec)
{
	Vec2 result = vec;
	result *= other;
	return result;
}

float Clamp(float value, float min, float max)
{
	return value < min ? min : value > max ? max : value;
}

float Repeat(float value, float period)
{
	return fmod(value, period);
}

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