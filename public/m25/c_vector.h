#pragma once

//cached "sin/cos for angle", for vector rotations
struct c_rotation_t
{
	float sin;
	float cos;
};

//euler angle rotation around these axes...
struct c_pitch_yaw_t
{
	float x;
	float y;
};

//2
//2
//2
//2

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
};//c_vec2i_t

struct c_vec2ui_t
{
	//strict weak ordering, to support use in std containers
	bool operator >= (const c_vec2ui_t& aVector) const
	{
		return !(*this < aVector);
	}

	bool operator < (const c_vec2ui_t& aVector) const
	{
		if (x < aVector.x)	return true;
		if (x > aVector.x)	return false;

		// Otherwise x are equal
		if (y < aVector.y)	return true;
		if (y > aVector.y)	return false;

		// Otherwise all are equal
		return false;
	}

	uint32_t x;
	uint32_t y;
};//c_vec2ui_t

//c_xzi_t
//c_xzi_t
//c_xzi_t
//c_xzi_t
struct c_xzi_t
{
	int32_t x;
	int32_t z;
};

//c_xzf_t
//c_xzf_t
//c_xzf_t
//c_xzf_t
struct c_vec3f_t;	//forward
struct c_xzf_t
{
	float x;
	float z;
};

//c_vec2f_t
//c_vec2f_t
//c_vec2f_t
//c_vec2f_t
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
};//c_vec2f_t

//3
//3
//3
//3
struct c_vec3uc_t
{
	uint8_t x;
	uint8_t y;
	uint8_t z;
};

struct c_vec3ui_t
{
	bool operator == (const c_vec3ui_t& aVector) const
	{
		return x == aVector.x && y == aVector.y && z == aVector.z;
	}

	bool operator != (const c_vec3ui_t& aVector) const
	{
		return !(*this == aVector);
	}

	uint32_t x;
	uint32_t y;
	uint32_t z;
};

struct c_vec3i_t
{
	c_vec3i_t& operator-=(const c_vec3i_t& aVector)
	{
		x -= aVector.x;
		y -= aVector.y;
		z -= aVector.z;

		return *this;
	}

	c_vec3i_t& operator+=(const c_vec3i_t& aVector)
	{
		x += aVector.x;
		y += aVector.y;
		z += aVector.z;

		return *this;
	}

	c_vec3i_t& operator*=(const int32_t aValue)
	{
		x *= aValue;
		y *= aValue;
		z *= aValue;

		return *this;
	}

	c_vec3i_t& operator/=(const int32_t aValue)
	{
		x /= aValue;
		y /= aValue;
		z /= aValue;

		return *this;
	}

	c_vec3i_t operator+(const c_vec3i_t& aVector) const
	{
		return{ x + aVector.x, y + aVector.y, z + aVector.z };
	}

	c_vec3i_t operator*(const int32_t aValue) const
	{
		return{ x * aValue, y * aValue, z * aValue };
	}

	c_vec3i_t operator/(const int32_t aValue) const
	{
		return{ x / aValue, y / aValue, z / aValue };
	}

	c_vec3i_t operator-(const c_vec3i_t& aVector) const
	{
		return{ x - aVector.x, y - aVector.y, z - aVector.z };
	}

	bool operator==(const c_vec3i_t& aVector) const
	{
		return x == aVector.x && y == aVector.y && z == aVector.z;
	}

	bool operator != (const c_vec3i_t& aVector) const
	{
		return !(*this == aVector);
	}

	//strict weak ordering, to support use in std containers
	bool operator >= (const c_vec3i_t& aVector) const
	{
		return !(*this < aVector);
	}

	bool operator < (const c_vec3i_t& aVector) const
	{
		if (x < aVector.x)	return true;
		if (x > aVector.x)	return false;

		// x are equal here
		if (y < aVector.y)	return true;
		if (y > aVector.y)	return false;

		// y are equal here
		if (z < aVector.z)	return true;
		if (z > aVector.z)	return false;

		// all are equal here
		return false;
	}

	int32_t x;
	int32_t y;
	int32_t z;
};//c_vec3i_t

struct c_vec3f_t
{
	c_vec3f_t& operator+=(const c_vec3f_t& in_vector)
	{
		x += in_vector.x;
		y += in_vector.y;
		z += in_vector.z;

		return *this;
	}
	c_vec3f_t& operator+=(const float in_scalar)
	{
		x += in_scalar;
		y += in_scalar;
		z += in_scalar;

		return *this;
	}

	c_vec3f_t& operator-=(const c_vec3f_t& in_vector)
	{
		x -= in_vector.x;
		y -= in_vector.y;
		z -= in_vector.z;

		return *this;
	}
	c_vec3f_t& operator-=(const float in_scalar)
	{
		x -= in_scalar;
		y -= in_scalar;
		z -= in_scalar;

		return *this;
	}

	c_vec3f_t& operator*=(const float in_scalar)
	{
		x *= in_scalar;
		y *= in_scalar;
		z *= in_scalar;

		return *this;
	}

	c_vec3f_t& operator/=(const float in_scalar)
	{
		x /= in_scalar;
		y /= in_scalar;
		z /= in_scalar;

		return *this;
	}

	c_vec3f_t operator+(const c_vec3f_t& in_vector) const
	{
		return{ x + in_vector.x, y + in_vector.y, z + in_vector.z };
	}
	c_vec3f_t operator+(const float in_scalar) const
	{
		return{ x + in_scalar, y + in_scalar, z + in_scalar };
	}

	c_vec3f_t operator-(const c_vec3f_t& aVector) const
	{
		return{ x - aVector.x, y - aVector.y, z - aVector.z };
	}

	bool operator == (const c_vec3f_t& aVector) const
	{
		return x == aVector.x && y == aVector.y && z == aVector.z;
	}

	bool operator != (const c_vec3f_t& aVector) const
	{
		return !(*this == aVector);
	}

	c_vec3f_t operator*(const float aValue) const
	{
		return{ x * aValue, y * aValue, z * aValue };
	}

	c_vec3f_t operator/(const float aValue) const
	{
		return{ x / aValue, y / aValue, z / aValue };
	}

	c_vec3f_t operator*(const c_vec3f_t& aVector) const
	{
		return{ x * aVector.x, y * aVector.y, z * aVector.z };
	}

	c_vec3f_t operator-() const
	{
		return{ -x, -y, -z };
	}

	//strict weak ordering, to support use in std containers
	bool operator >= (const c_vec3f_t& aVector) const
	{
		return !(*this < aVector);
	}

	bool operator < (const c_vec3f_t& aVector) const
	{
		if (x < aVector.x)	return true;
		if (x > aVector.x)	return false;

		// Otherwise x are equal
		if (y < aVector.y)	return true;
		if (y > aVector.y)	return false;

		// Otherwise y are equal
		if (z < aVector.z)	return true;
		if (z > aVector.z)	return false;

		// Otherwise all are equal
		return false;
	}

	float x;
	float y;
	float z;
};//c_vec3f_t

//4
//4
//4
//4
struct c_vec4uc_t
{
	uint8_t x;
	uint8_t y;
	uint8_t z;
	uint8_t w;
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

struct c_vec4f_t
{
	c_vec4f_t& operator+=(const c_vec4f_t& aVector)
	{
		x += aVector.x;
		y += aVector.y;
		z += aVector.z;
		w += aVector.w;

		return *this;
	}

	c_vec4f_t operator+(const c_vec4f_t& aVector) const
	{
		return{ x + aVector.x, y + aVector.y, z + aVector.z, w + aVector.w };
	}

	c_vec4f_t operator-(const c_vec4f_t& aVector) const
	{
		return{ x - aVector.x, y - aVector.y, z - aVector.z, w - aVector.w };
	}

	c_vec4f_t operator*(const float aScalar) const
	{
		return{ x * aScalar, y * aScalar, z * aScalar, w * aScalar };
	}

	bool operator==(const c_vec4f_t& aVector) const
	{
		return x == aVector.x && y == aVector.y && z == aVector.z && w == aVector.w;
	}

	bool operator != (const c_vec4f_t& aVector) const
	{
		return !(*this == aVector);
	}

	float x;
	float y;
	float z;
	float w;
};

struct c_pov_t
{
	c_vec3f_t position;
	c_pitch_yaw_t orientation;
};

c_rotation_t c_rotation(const float angle);

c_pitch_yaw_t c_pitch_yaw(const c_vec3f_t& aVector);
c_vec3f_t c_local_axes(const c_pitch_yaw_t& aPitchYaw, const c_vec3f_t& aVector);
c_vec3f_t c_forward_vector(const c_pitch_yaw_t& aPitchYaw);	//z
c_vec3f_t c_right_vector(const c_pitch_yaw_t& aPitchYaw);	//x
c_vec3f_t c_up_vector(const c_pitch_yaw_t& aPitchYaw);		//y

bool operator < (const c_xzi_t& a, const c_xzi_t& b);
bool operator < (const c_xzf_t& a, const c_xzf_t& b);

c_vec2f_t c_vec2f(const int32_t x, const int32_t y);
c_vec2f_t c_vec2f(const c_vec2i_t& v);
c_vec2f_t c_vec2f(const c_vec2ui_t& v);

c_xzf_t c_xzf(const c_vec3f_t& a);

float c_length(const float x, const float y);
float c_length(const c_xzf_t& v);
float c_length(const c_vec2f_t& v);
float c_length(const c_vec3f_t& v);

float c_length_squared(const float x, const float y);
float c_length_squared(const c_xzf_t& v);
float c_length_squared(const c_vec2f_t& v);
float c_length_squared(const c_vec3f_t& v);

float c_dot_product(const c_vec2f_t& a, const c_vec2f_t& b);
float c_dot_product(const c_vec3f_t& a, const c_vec3f_t& b);

c_vec2f_t c_normalize(const float x, const float y);
c_vec2f_t c_normalize(const c_vec2f_t& v);
c_vec3f_t c_normalize(const float x, const float y, const float z);
c_vec3f_t c_normalize(const c_vec3f_t& v);

c_vec2f_t c_reflection(const c_vec2f_t& normal, const c_vec2f_t& vector_to_reflect);
c_vec3f_t c_reflection(const c_vec3f_t& normal, const c_vec3f_t& vector_to_reflect);

c_vec2f_t c_projection(const c_vec2f_t& normal, const c_vec2f_t& vector_to_project);
c_vec3f_t c_projection(const c_vec3f_t& normal, const c_vec3f_t& vector_to_project);

c_vec2i_t c_floor_to_int(const c_vec2f_t& v);
c_vec3i_t c_floor_to_int(const c_vec3f_t& v);

bool c_left_of(const c_vec2f_t& a, const c_vec2f_t& b);
bool c_right_of(const c_vec2f_t& a, const c_vec2f_t& b);
c_vec2f_t c_left_normal(const c_vec2f_t& v);
c_vec2f_t c_right_normal(const c_vec2f_t& v);
c_vec2f_t c_rotate(const c_rotation_t& aRotation, const c_vec2f_t& v);
c_vec2f_t c_rotate(const float anAngle, const c_vec2f_t& v);

c_vec3f_t c_vec3f(const c_vec3i_t& v);

c_vec3f_t c_rotate_around_x(const float anAngle, const c_vec3f_t& v);
c_vec3f_t c_rotate_around_x(const c_rotation_t& aRotation, const c_vec3f_t& v);
c_vec3f_t c_rotate_around_y(const float anAngle, const c_vec3f_t& v);
c_vec3f_t c_rotate_around_y(const c_rotation_t& aRotation, const c_vec3f_t& v);
c_vec3f_t c_rotate_around_z(const float anAngle, const c_vec3f_t& v);
c_vec3f_t c_rotate_around_z(const c_rotation_t& aRotation, const c_vec3f_t& v);
c_vec3f_t c_cross_product(const c_vec3f_t& a, const c_vec3f_t& b);
