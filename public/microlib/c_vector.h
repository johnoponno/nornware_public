#pragma once

struct c_vec2i_t
{
	c_vec2i_t& operator+=(const c_vec2i_t& aVector)
	{
		x += aVector.x;
		y += aVector.y;
		return *this;
	}

	c_vec2i_t& operator-=(const c_vec2i_t& aVector)
	{
		x -= aVector.x;
		y -= aVector.y;
		return *this;
	}

	c_vec2i_t& operator*=(const int32_t aValue)
	{
		x *= aValue;
		y *= aValue;
		return *this;
	}

	c_vec2i_t& operator/=(const int32_t aValue)
	{
		x /= aValue;
		y /= aValue;
		return *this;
	}

	c_vec2i_t operator+(const c_vec2i_t& aVector) const
	{
		return{ x + aVector.x, y + aVector.y };
	}

	c_vec2i_t operator-(const c_vec2i_t& aVector) const
	{
		return{ x - aVector.x, y - aVector.y };
	}

	c_vec2i_t operator*(const int32_t aValue) const
	{
		return{ x * aValue, y * aValue };
	}

	c_vec2i_t operator/(const int32_t aValue) const
	{
		return{ x / aValue, y / aValue };
	}

	c_vec2i_t operator-() const
	{
		return{ -x, -y };
	}

	bool operator==(const c_vec2i_t& aVector) const
	{
		return x == aVector.x && y == aVector.y;
	}

	bool operator!=(const c_vec2i_t& aVector) const
	{
		return !(*this == aVector);
	}

	//strict weak ordering, to support use in std containers
	bool operator >= (const c_vec2i_t& aVector) const
	{
		return !(*this < aVector);
	}

	bool operator < (const c_vec2i_t& aVector) const
	{
		if (x < aVector.x)	return true;
		if (x > aVector.x)	return false;

		// Otherwise x are equal
		if (y < aVector.y)	return true;
		if (y > aVector.y)	return false;

		// Otherwise all are equal
		return false;
	}

	int32_t x;
	int32_t y;
};

struct c_vec2f_t
{
	c_vec2f_t& operator-=(const c_vec2f_t& aVector)
	{
		x -= aVector.x;
		y -= aVector.y;

		return *this;
	}

	c_vec2f_t& operator+=(const c_vec2f_t& aVector)
	{
		x += aVector.x;
		y += aVector.y;

		return *this;
	}

	c_vec2f_t& operator*=(const float aValue)
	{
		x *= aValue;
		y *= aValue;

		return *this;
	}

	c_vec2f_t& operator/=(const float aValue)
	{
		x /= aValue;
		y /= aValue;

		return *this;
	}

	c_vec2f_t operator-() const
	{
		return{ -x, -y };
	}

	c_vec2f_t operator-(const c_vec2f_t& aVector) const
	{
		return{ x - aVector.x, y - aVector.y };
	}

	c_vec2f_t operator+(const c_vec2f_t& aVector) const
	{
		return{ x + aVector.x, y + aVector.y };
	}

	c_vec2f_t operator*(const float aValue) const
	{
		return{ x * aValue, y * aValue };
	}

	c_vec2f_t operator/(const float aValue) const
	{
		return{ x / aValue, y / aValue };
	}

	bool operator==(const c_vec2f_t& aVector) const
	{
		return x == aVector.x && y == aVector.y;
	}

	bool operator!=(const c_vec2f_t& aVector) const
	{
		return !(*this == aVector);
	}

	//strict weak ordering, to support use in std containers
	bool operator >= (const c_vec2f_t& aVector) const
	{
		return !(*this < aVector);
	}

	bool operator < (const c_vec2f_t& aVector) const
	{
		if (x < aVector.x)	return true;
		if (x > aVector.x)	return false;

		// x are equal here
		if (y < aVector.y)	return true;
		if (y > aVector.y)	return false;

		// all are equal here
		return false;
	}

	float x;
	float y;
};

struct c_vec4i_t
{
	bool operator == (const c_vec4i_t& aVector) const
	{
		return
			x == aVector.x &&
			y == aVector.y &&
			z == aVector.z &&
			w == aVector.w;
	}

	bool operator != (const c_vec4i_t& aVector) const
	{
		return !(*this == aVector);
	}

	bool operator < (const c_vec4i_t& aVector) const
	{
		if (x < aVector.x)	return true;
		if (x > aVector.x)	return false;

		// x are equal here
		if (y < aVector.y)	return true;
		if (y > aVector.y)	return false;

		// y are equal here
		if (z < aVector.z)	return true;
		if (z > aVector.z)	return false;

		// z are equal here
		if (w < aVector.w)	return true;
		if (w > aVector.w)	return false;

		// all are equal here
		return false;
	}

	int32_t x;
	int32_t y;
	union
	{
		int32_t z;
		int32_t width;
	};
	union
	{
		int32_t w;
		int32_t height;
	};
};

c_vec2f_t c_vec2f(const c_vec2i_t& v);
float c_length(const c_vec2f_t& v);
c_vec2f_t c_normalize(const c_vec2f_t& v);
c_vec2i_t c_floor_to_int(const c_vec2f_t& v);
c_vec2f_t c_rotate(const float anAngle, const c_vec2f_t& v);
