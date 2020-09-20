
#include "Vectors.h"

Vec2 Vec2::operator+=(const Vec2& other)
{
	X += other.X;
	Y += other.Y;
	return *this;
}

Vec2 Vec2::operator-=(const Vec2& other)
{
	X -= other.X;
	Y -= other.Y;
	return *this;
}

Vec2 Vec2::operator*=(const float& other)
{
	X *= other;
	Y *= other;
	return *this;
}

Vec2 Vec2::operator/=(const float& other)
{
	X /= other;
	Y /= other;
	return *this;
}

Vec2 Vec2::operator+(const Vec2& other) const
{
	Vec2 result = *this;
	result += other;
	return result;
}

Vec2 Vec2::operator-(const Vec2& other) const
{
	Vec2 result = *this;
	result -= other;
	return result;
}

Vec2 Vec2::operator*(const float& other) const
{
	Vec2 result = *this;
	result *= other;
	return result;
}

Vec2 Vec2::operator/(const float& other) const
{
	Vec2 result = *this;
	result /= other;
	return result;
}

Vec2 operator*(const float& other, const Vec2 vec)
{
	Vec2 result = vec;
	result *= other;
	return result;
}

float Vec2::SqrLength() const
{
	return Dot(*this, *this);
}

float Vec2::Length() const
{
	return sqrt(SqrLength());
}

Vec2 Vec2::Normalized() const
{
	return *this / Length();
}

void Vec2::Normalize()
{
	*this = Normalized();
}

float Vec2::Dot(const Vec2& lhs, const Vec2& rhs)
{
	return lhs.X * rhs.X + lhs.Y * rhs.Y;
}

float Vec2::Cross(const Vec2& lhs, const Vec2& rhs)
{
	return lhs.X * rhs.Y - lhs.Y * rhs.X;
}

float Clamp(float value, float min, float max)
{
	return value < min ? min : value > max ? max : value;
}

float Repeat(float value, float period)
{
	return fmod(value, period);
}