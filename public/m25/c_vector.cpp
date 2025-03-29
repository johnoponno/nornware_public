#include "stdafx.h"

#if 1

#include "c_math.h"

#define LENGTH_2D(x, y) ::sqrtf((x) * (x) + (y) * (y))
#define LENGTH_SQUARED_2D(x, y) (x) * (x) + (y) * (y)

#define LENGTH_3D(x, y, z) ::sqrtf((x) * (x) + (y) * (y) + (z) * (z))

c_rotation_t c_rotation(const float in_angle)
{
	return{ ::sinf(in_angle), ::cosf(in_angle) };
}

c_pitch_yaw_t c_pitch_yaw(const c_vec3f_t& in_vector)
{
	c_pitch_yaw_t result;

	result.y = ::atan2f(in_vector.x, in_vector.z);

	//pitch trickery
	const c_vec3f_t BASE = c_normalize({ in_vector.x, 0.f, in_vector.z });
	const float LENGTH = c_dot_product(BASE, in_vector);
	result.x = ::atan2f(in_vector.y, -LENGTH) + C_PI;

	return result;
}

c_vec3f_t c_local_axes(const c_pitch_yaw_t& in_pitch_yaw, const c_vec3f_t& in_vector)
{
	c_vec3f_t result = in_vector;
	result = c_rotate_around_x(in_pitch_yaw.x, result);
	result = c_rotate_around_y(in_pitch_yaw.y, result);
	return result;
}

c_vec3f_t c_forward_vector(const c_pitch_yaw_t& in_pitch_yaw)
{
	return c_local_axes(in_pitch_yaw, { 0.f, 0.f, 1.f });
}

c_vec3f_t c_right_vector(const c_pitch_yaw_t& in_pitch_yaw)
{
	return c_local_axes(in_pitch_yaw, { 1.f, 0.f, 0.f });
}

c_vec3f_t c_up_vector(const c_pitch_yaw_t& in_pitch_yaw)
{
	return c_local_axes(in_pitch_yaw, { 0.f, 1.f, 0.f });
}

bool operator < (const c_xzi_t& a, const c_xzi_t& b)
{
	if (a.x < b.x)	return true;
	if (a.x > b.x)	return false;

	// Otherwise x are equal
	if (a.z < b.z)	return true;
	if (a.z > b.z)	return false;

	// Otherwise all are equal
	return false;
}

bool operator < (const c_xzf_t& a, const c_xzf_t& b)
{
	if (a.x < b.x)	return true;
	if (a.x > b.x)	return false;

	// Otherwise x are equal
	if (a.z < b.z)	return true;
	if (a.z > b.z)	return false;

	// Otherwise all are equal
	return false;
}

c_vec2f_t c_vec2f(const int32_t x, const int32_t y)
{
	return{ (float)x, (float)y };
}

c_vec2f_t c_vec2f(const c_vec2i_t& in_vector)
{
	return{ (float)in_vector.x, (float)in_vector.y };
}

c_vec2f_t c_vec2f(const c_vec2ui_t& in_vector)
{
	return{ (float)in_vector.x, (float)in_vector.y };
}

c_xzf_t c_xzf(const c_vec3f_t& a)
{
	return { a.x, a.z };
}

//c_length overloads
//c_length overloads
//c_length overloads
//c_length overloads
float c_length(const float x, const float y)
{
	return LENGTH_2D(x, y);
}
float c_length(const c_xzf_t& in_vector)
{
	return LENGTH_2D(in_vector.x, in_vector.z);
}
float c_length(const c_vec2f_t& in_vector)
{
	return LENGTH_2D(in_vector.x, in_vector.y);
}
float c_length(const c_vec3f_t& in_vector)
{
	return ::sqrtf(
		in_vector.x * in_vector.x +
		in_vector.y * in_vector.y +
		in_vector.z * in_vector.z
	);
}

//c_length_squared overloads
//c_length_squared overloads
//c_length_squared overloads
//c_length_squared overloads
float c_length_squared(const float x, const float y)
{
	return LENGTH_SQUARED_2D(x, y);
}
float c_length_squared(const c_xzf_t& in_vector)
{
	return LENGTH_SQUARED_2D(in_vector.x, in_vector.z);
}
float c_length_squared(const c_vec2f_t& in_vector)
{
	return LENGTH_SQUARED_2D(in_vector.x, in_vector.y);
}
float c_length_squared(const c_vec3f_t& in_vector)
{
	return
		in_vector.x * in_vector.x +
		in_vector.y * in_vector.y +
		in_vector.z * in_vector.z;
}

bool c_left_of(const c_vec2f_t& a, const c_vec2f_t& b)
{
	return (a.x * b.y) > (a.y * b.x);
}

bool c_right_of(const c_vec2f_t& a, const c_vec2f_t& b)
{
	return (a.x * b.y) < (a.y * b.x);
}

c_vec2f_t c_left_normal(const c_vec2f_t& in_vector)
{
	return{ in_vector.y, -in_vector.x };
}

c_vec2f_t c_right_normal(const c_vec2f_t& in_vector)
{
	return{ -in_vector.y, in_vector.x };
}

float c_dot_product(const c_vec2f_t& a, const c_vec2f_t& b)
{
	return
		b.x * a.x +
		b.y * a.y;
}

c_vec2f_t c_reflection(const c_vec2f_t& in_normal, const c_vec2f_t& in_vector_to_reflect)
{
	return in_vector_to_reflect + in_normal * (c_dot_product(in_normal, in_vector_to_reflect) * -2.f);
}

c_vec2f_t c_projection(const c_vec2f_t& in_normal, const c_vec2f_t& in_vector_to_project)
{
	return in_vector_to_project - in_normal * c_dot_product(in_normal, in_vector_to_project);
}

c_vec2f_t c_rotate(const c_rotation_t& in_rotation, const c_vec2f_t& in_vector)
{
	return
	{
		in_vector.x * in_rotation.cos + in_vector.y * in_rotation.sin,
		in_vector.y * in_rotation.cos - in_vector.x * in_rotation.sin
	};
}

c_vec2f_t c_rotate(const float in_angle, const c_vec2f_t& in_vector)
{
	if (0.f == in_angle)
		return in_vector;

	return c_rotate(c_rotation(in_angle), in_vector);
}

c_vec2f_t c_normalize(const float in_x, const float in_y)
{
	const float LENGTH = LENGTH_2D(in_x, in_y);
	if (LENGTH)
	{
		const float OOL = 1.f / LENGTH;
		return { in_x * OOL, in_y * OOL };
	}

	//if no length, don't change the input
	return { in_x, in_y };
}

c_vec2f_t c_normalize(const c_vec2f_t& in_vector)
{
	const float LENGTH = LENGTH_2D(in_vector.x, in_vector.y);
	if (LENGTH)
	{
		const float OOL = 1.f / LENGTH;
		return { in_vector.x * OOL, in_vector.y * OOL };
	}

	//if no length, don't change the input
	return in_vector;
}

c_vec2i_t c_floor_to_int(const c_vec2f_t& in_vector)
{
	c_vec2i_t i
	{
		(int32_t)in_vector.x,
		(int32_t)in_vector.y
	};

	if (in_vector.x < 0.f)
		--i.x;

	if (in_vector.y < 0.f)
		--i.y;

	return i;
}

c_vec3f_t c_vec3f(const c_vec3i_t& in_vector)
{
	return
	{
		(float)in_vector.x,
		(float)in_vector.y,
		(float)in_vector.z
	};
}

c_vec3f_t c_reflection(const c_vec3f_t& in_normal, const c_vec3f_t& in_vector_to_reflect)
{
	return in_vector_to_reflect + in_normal * (c_dot_product(in_normal, in_vector_to_reflect) * -2.f);
}

c_vec3f_t c_projection(const c_vec3f_t& in_normal, const c_vec3f_t& in_vector_to_project)
{
	return in_vector_to_project - in_normal * c_dot_product(in_normal, in_vector_to_project);
}

c_vec3f_t c_normalize(const float in_x, const float in_y, const float in_z)
{
	const float LENGTH = LENGTH_3D(in_x, in_y, in_z);
	if (LENGTH)
	{
		const float OOL = 1.f / LENGTH;
		return
		{
			in_x * OOL,
			in_y * OOL,
			in_z * OOL
		};
	}

	//if no length, don't change the input
	return { in_x, in_y, in_z };
}

c_vec3f_t c_normalize(const c_vec3f_t& in_vector)
{
	const float LENGTH = LENGTH_3D(in_vector.x, in_vector.y, in_vector.z);
	if (LENGTH)
	{
		const float OOL = 1.f / LENGTH;
		return
		{
			in_vector.x * OOL,
			in_vector.y * OOL,
			in_vector.z * OOL
		};
	}

	//if no length, don't change the input
	return in_vector;
}

c_vec3f_t c_rotate_around_x(const float in_angle, const c_vec3f_t& in_vector)
{
	if (0 == in_angle)
		return in_vector;

	return c_rotate_around_x(c_rotation(in_angle), in_vector);
}

c_vec3f_t c_rotate_around_x(const c_rotation_t& in_rotation, const c_vec3f_t& in_vector)
{
	return
	{
		in_vector.x,
		in_vector.y * in_rotation.cos + in_vector.z * in_rotation.sin,
		in_vector.z * in_rotation.cos - in_vector.y * in_rotation.sin
	};
}

c_vec3f_t c_rotate_around_y(const float in_angle, const c_vec3f_t& in_vector)
{
	if (0 == in_angle)
		return in_vector;

	return c_rotate_around_y(c_rotation(in_angle), in_vector);
}

c_vec3f_t c_rotate_around_y(const c_rotation_t& in_rotation, const c_vec3f_t& in_vector)
{
	return
	{
		in_vector.x * in_rotation.cos + in_vector.z * in_rotation.sin,
		in_vector.y,
		in_vector.z * in_rotation.cos - in_vector.x * in_rotation.sin
	};
}

c_vec3f_t c_rotate_around_z(const float in_angle, const c_vec3f_t& in_vector)
{
	if (0 == in_angle)
		return in_vector;

	return c_rotate_around_z(c_rotation(in_angle), in_vector);
}

c_vec3f_t c_rotate_around_z(const c_rotation_t& in_rotation, const c_vec3f_t& in_vector)
{
	return
	{
		in_vector.x * in_rotation.cos + in_vector.y * in_rotation.sin,
		in_vector.y * in_rotation.cos - in_vector.x * in_rotation.sin,
		in_vector.z
	};
}

float c_dot_product(const c_vec3f_t& in_a, const c_vec3f_t& in_b)
{
	return
		in_b.x * in_a.x +
		in_b.y * in_a.y +
		in_b.z * in_a.z;
}

c_vec3f_t c_cross_product(const c_vec3f_t& in_a, const c_vec3f_t& in_b)
{
	return
	{
		in_a.y * in_b.z - in_a.z * in_b.y,
		in_a.z * in_b.x - in_a.x * in_b.z,
		in_a.x * in_b.y - in_a.y * in_b.x
	};
}

c_vec3i_t c_floor_to_int(const c_vec3f_t& in_vector)
{
	c_vec3i_t i{ (int32_t)in_vector.x, (int32_t)in_vector.y, (int32_t)in_vector.z };

	if (in_vector.x < 0.f)
		--i.x;

	if (in_vector.y < 0.f)
		--i.y;

	if (in_vector.z < 0.f)
		--i.z;

	return i;
}

#endif
