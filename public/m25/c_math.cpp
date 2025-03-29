#include "stdafx.h"

#if 1

#include "c_math.h"

static void __quaternion_multiply(
	const float* in_a, const float* in_b,
	float* out_r)
{
#if 0

	const float A = in_a[0];
	const float B = in_a[1];
	const float C = in_a[2];
	const float D = in_a[3];

	const float E = in_b[0];
	const float F = in_b[1];
	const float G = in_b[2];
	const float H = in_b[3];

	out_r[0] = A * H + D * E + B * G - C * F;
	out_r[1] = B * H + D * F + C * E - A * G;
	out_r[2] = C * H + D * G + A * F - B * E;
	out_r[3] = D * H - A * E - B * F - C * G;

#else

	out_r[0] = in_a[0] * in_b[3] + in_a[3] * in_b[0] + in_a[1] * in_b[2] - in_a[2] * in_b[1];
	out_r[1] = in_a[1] * in_b[3] + in_a[3] * in_b[1] + in_a[2] * in_b[0] - in_a[0] * in_b[2];
	out_r[2] = in_a[2] * in_b[3] + in_a[3] * in_b[2] + in_a[0] * in_b[1] - in_a[1] * in_b[0];
	out_r[3] = in_a[3] * in_b[3] - in_a[0] * in_b[0] - in_a[1] * in_b[1] - in_a[2] * in_b[2];

#endif
}

/*
static bool __right_of(const c_vec2f_t& in_start, const c_vec2f_t& in_end, const c_vec2f_t& in_point)
{
	const c_vec2f_t TO_POINT = in_point - in_start;
	const c_vec2f_t TO_END = in_end - in_start;

	return c_right_of(TO_POINT, TO_END);
}
*/

static void __float_min_max(float& in_min, float& in_max)
{
	if (in_min > in_max)
	{
		const float TEMP = in_min;
		in_min = in_max;
		in_max = TEMP;
	}
}

static bool __float_in_range(const float in_value, const float in_min, const float in_max)
{
	return in_min <= in_value && in_value <= in_max;
}

static bool __int32_in_range(const int32_t in_value, const int32_t in_min, const int32_t in_max)
{
	return in_min <= in_value && in_value <= in_max;
}

static bool __legacy_sphere_vs_line(
	const c_sphere_t& in_sphere, const c_vec3f_t& in_ray_start, const c_vec3f_t& in_ray_end,
	float& out_param)
{
	//special case for start == end
	if (in_ray_start == in_ray_end)
	{
		if (c_length(in_ray_start - in_sphere.center) < in_sphere.radius)
		{
			out_param = 0.f;
			return true;
		}

		return false;
	}

	//START != END
	//START != END
	//START != END
	//START != END

	//check if start is inside circle
	if (c_length(in_ray_start - in_sphere.center) < in_sphere.radius)
	{
		out_param = 0.f;
		return true;
	}

	//check if end is inside circle
	if (c_length(in_ray_end - in_sphere.center) < in_sphere.radius)
	{
		out_param = 1.f;
		return true;
	}

	//CALC INTERSECTION
	//CALC INTERSECTION
	//CALC INTERSECTION
	//CALC INTERSECTION
	{
		//see http://astronomy.swin.edu.au/~pbourke/geometry/sphereline/
		const c_vec3f_t LINE = in_ray_end - in_ray_start;

		const float A = c_length_squared(LINE);
		const float B = 2.f * (c_dot_product(LINE, in_ray_start - in_sphere.center));
		const float C = c_length_squared(in_ray_start) + c_length_squared(in_sphere.center) - 2.f * (c_dot_product(in_ray_start, in_sphere.center)) - (in_sphere.radius * in_sphere.radius);

		const float D = B * B - 4.f * A * C;

		if (D < 0.f)
			return false;

		if (D == 0.f)
		{
			out_param = -B / 2.f * A;
		}
		else
		{
			const float U1 = (-B + ::sqrtf(D)) / (2.f * A);
			const float U2 = (-B - ::sqrtf(D)) / (2.f * A);

			if (U1 < U2)
				out_param = U1;
			else
				out_param = U2;
		}

		return true;
	}
}

//public
//public
//public
//public

/*
   Calculate the line segment PaPb that is the shortest route between
   two lines P1P2 and P3P4. Calculate also the values of mua and mub where
	  Pa = P1 + mua (P2 - P1)
	  Pb = P3 + mub (P4 - P3)
   Return FALSE if no solution exists.
*/
c_line_line_intersect_t c_line_line_intersect(const c_vec3f_t& p1, const c_vec3f_t& p2, const c_vec3f_t& p3, const c_vec3f_t& p4)
{
	const float EPSILON = .001f;

	const c_vec3f_t p13 = p1 - p3;
	const c_vec3f_t p43 = p4 - p3;
	if (::fabsf(p43.x) < EPSILON && ::fabsf(p43.y) < EPSILON && ::fabsf(p43.z) < EPSILON)
		return {};

	const c_vec3f_t p21 = p2 - p1;
	if (::fabsf(p21.x) < EPSILON && ::fabsf(p21.y) < EPSILON && ::fabsf(p21.z) < EPSILON)
		return {};

	const float d1343 = p13.x * p43.x + p13.y * p43.y + p13.z * p43.z;
	const float d4321 = p43.x * p21.x + p43.y * p21.y + p43.z * p21.z;
	const float d1321 = p13.x * p21.x + p13.y * p21.y + p13.z * p21.z;
	const float d4343 = p43.x * p43.x + p43.y * p43.y + p43.z * p43.z;
	const float d2121 = p21.x * p21.x + p21.y * p21.y + p21.z * p21.z;

	const float denom = d2121 * d4343 - d4321 * d4321;
	if (::fabsf(denom) < EPSILON)
		return {};

	const float numer = d1343 * d4321 - d1321 * d4343;

	c_line_line_intersect_t result;

	result.mua = numer / denom;
	result.mub = (d1343 + d4321 * (result.mua)) / d4343;

	result.pa.x = p1.x + result.mua * p21.x;
	result.pa.y = p1.y + result.mua * p21.y;
	result.pa.z = p1.z + result.mua * p21.z;
	result.pb.x = p3.x + result.mub * p43.x;
	result.pb.y = p3.y + result.mub * p43.y;
	result.pb.z = p3.z + result.mub * p43.z;

	result.eh = 1;

	return result;
}

uint32_t c_frame(const float aNow, const float anFps, const uint32_t aNumFrames)
{
	if (anFps == 0.f)
		return 0;

	return (uint32_t)(aNow * anFps) % aNumFrames;
}

float c_scale_and_saturate(const float anEdge0, const float anEdge1, const float aValue)
{
	const float v = (aValue - anEdge0) / (anEdge1 - anEdge0);

	if (v > 1)
		return 1;

	if (v < 0)
		return 0;

	assert(0 <= v && v <= 1);
	return v;
}

float c_smoother_step(const float anEdge0, const float anEdge1, const float aValue)
{
	// Scale, and saturate aValue to 0..1 range
	const float v = (aValue - anEdge0) / (anEdge1 - anEdge0);
	if (v > 1.f)
		return 1.f;
	if (v < 0.f)
		return 0.f;

	// Evaluate polynomial
	const float result = v * v * v * (v * (v * 6.f - 15.f) + 10.f);
	if (result > 1.f)
		return 1.f;
	if (result < 0.f)
		return 0.f;
	return result;
}

float c_gain(const float x, const float k)
{
	const float a = .5f * ::powf(2.f * ((x < .5f) ? x : 1.f - x), k);
	return (x < .5f) ? a : 1.f - a;
}

float c_parabola(const float x, const float k)
{
	return ::powf(4.f * x * (1.f - x), k);
}

float c_lerp_angle_0_2pi(const float speed, const float target, const float current)
{
	float delta = target - current;
	float result = current;

	if (delta > C_PI)
	{
		delta -= C_PI2;
		result += delta * speed;
		if (result < 0.f)
			result += C_PI2;
	}
	else if (delta < -C_PI)
	{
		delta += C_PI2;
		result += delta * speed;
		if (result > C_PI2)
			result -= C_PI2;
	}
	else
	{
		result += delta * speed;
	}

	return result;
}

c_explicit_corner_box_3f_t c_aabbf_make_explicit(const c_aabbf_t& in_box, const c_vec3f_t* in_offset, const float* in_scale)
{
	c_explicit_corner_box_3f_t result;

	//bottom
	result.corners[0] = in_box.min;
	result.corners[1] = { in_box.min.x, in_box.min.y, in_box.max.z };
	result.corners[2] = { in_box.max.x, in_box.min.y, in_box.max.z };
	result.corners[3] = { in_box.max.x, in_box.min.y, in_box.min.z };

	//top
	result.corners[4] = { in_box.min.x, in_box.max.y, in_box.min.z };
	result.corners[5] = { in_box.min.x, in_box.max.y, in_box.max.z };
	result.corners[6] = in_box.max;
	result.corners[7] = { in_box.max.x, in_box.max.y, in_box.min.z };

	if (in_scale)
	{
		for (c_vec3f_t& corner : result.corners)
			corner *= *in_scale;
	}

	if (in_offset)
	{
		for (c_vec3f_t& corner : result.corners)
			corner += *in_offset;
	}

	assert(result.corners[0].x == result.corners[1].x);
	assert(result.corners[2].x == result.corners[3].x);
	assert(result.corners[0].x <= result.corners[2].x);
	assert(result.corners[0].z == result.corners[3].z);
	assert(result.corners[1].z == result.corners[2].z);
	assert(result.corners[0].z <= result.corners[1].z);

	return result;
}

c_aabbf_t c_aabbf_from_explicit(const c_explicit_corner_box_3f_t& in_box)
{
	c_aabbf_t result = C_AABBF_CLEAR;
	for (const c_vec3f_t& C : in_box.corners)
		result = C_AABBF_CALC(C, result);
	assert(result.min.x <= result.max.x);
	assert(result.min.y <= result.max.y);
	assert(result.min.z <= result.max.z);
	return result;
}

void c_aabbf_add(
	const c_explicit_corner_box_3f_t& in_box,
	c_aabbf_t& out)
{
	for (const c_vec3f_t& CORNER : in_box.corners)
		out = C_AABBF_CALC(CORNER, out);
}

c_vec3f_t c_aabbf_center(const c_aabbf_t& anInstance)
{
	return anInstance.min + (anInstance.max - anInstance.min) * .5f;
}

bool c_aabbf_point_inside_eh(const c_vec3f_t& aPoint, const c_aabbf_t& anInstance)
{
	assert(anInstance.min.x < anInstance.max.x);
	assert(anInstance.min.y < anInstance.max.y);
	assert(anInstance.min.z < anInstance.max.z);

	return
		__float_in_range(aPoint.x, anInstance.min.x, anInstance.max.x) &&
		__float_in_range(aPoint.y, anInstance.min.y, anInstance.max.y) &&
		__float_in_range(aPoint.z, anInstance.min.z, anInstance.max.z);
}

bool c_aabbf_overlaps_eh(const c_aabbf_t& in_a, const c_aabbf_t& in_b)
{
	const c_vec3f_t ASPECT = in_a.max - in_a.min;
	const c_vec3f_t CENTER = in_a.min + ASPECT * .5f;

	return
		CENTER.x > (in_b.min.x - ASPECT.x * .5f) && CENTER.x < in_b.max.x + ASPECT.x * .5f &&
		CENTER.y >(in_b.min.y - ASPECT.y * .5f) && CENTER.y < in_b.max.y + ASPECT.y * .5f &&
		CENTER.z >(in_b.min.z - ASPECT.z * .5f) && CENTER.z < in_b.max.z + ASPECT.z * .5f;
}

bool c_ray_vs_ray(const c_vec2f_t& aStart1, const c_vec2f_t& anEnd1, const c_vec2f_t& aStart2, const c_vec2f_t& anEnd2, c_vec2f_t& anIntersection)
{
	const float A2 = (anEnd2.y - aStart2.y) * (anEnd1.x - aStart1.x) - (anEnd2.x - aStart2.x) * (anEnd1.y - aStart1.y);
	if (A2 == 0.f)
		return false;

	const float A1 = (anEnd2.x - aStart2.x) * (aStart1.y - aStart2.y) - (anEnd2.y - aStart2.y) * (aStart1.x - aStart2.x);
	const float AU1 = A1 / A2;

	if (AU1 >= 0.f && AU1 <= 1.f)
	{
		const float A3 = (anEnd1.x - aStart1.x) * (aStart1.y - aStart2.y) - (anEnd1.y - aStart1.y) * (aStart1.x - aStart2.x);
		const float AU2 = A3 / A2;
		if (AU2 >= 0.f && AU2 <= 1.f)
		{
			anIntersection.x = aStart1.x + (anEnd1.x - aStart1.x) * AU1;
			anIntersection.y = aStart1.y + (anEnd1.y - aStart1.y) * AU1;
			return true;
		}
	}

	return false;
}

bool c_ray_vs_line(
	const c_vec2f_t& in_ray_start, const c_vec2f_t& in_ray_end,
	const c_vec2f_t& in_line_start, const c_vec2f_t& in_line_end,//points on line, but the line is technically infinite...
	c_vec2f_t& out_intersection)
{
	const float A2 = (in_line_end.y - in_line_start.y) * (in_ray_end.x - in_ray_start.x) - (in_line_end.x - in_line_start.x) * (in_ray_end.y - in_ray_start.y);
	if (A2 == 0.f)
		return false;

	const float A1 = (in_line_end.x - in_line_start.x) * (in_ray_start.y - in_line_start.y) - (in_line_end.y - in_line_start.y) * (in_ray_start.x - in_line_start.x);
	const float AU1 = A1 / A2;

	if (AU1 >= 0.f && AU1 <= 1.f)
	{
		out_intersection.x = in_ray_start.x + (in_ray_end.x - in_ray_start.x) * AU1;
		out_intersection.y = in_ray_start.y + (in_ray_end.y - in_ray_start.y) * AU1;
		return true;
	}

	return false;
}

c_vec2f_t c_ray_vs_vertical_line(const c_vec2f_t& in_ray_start, const c_vec2f_t& in_ray_end, const float in_line_x)
{
	const float T = (in_line_x - in_ray_start.x) / (in_ray_end.x - in_ray_start.x);
	c_vec2f_t result;
	result.x = in_line_x;
	result.y = in_ray_start.y + (in_ray_end.y - in_ray_start.y) * T;
	return result;
}

c_vec2f_t c_ray_vs_horizontal_line(const c_vec2f_t& in_ray_start, const c_vec2f_t& in_ray_end, const float in_line_y)
{
	const float T = (in_line_y - in_ray_start.y) / (in_ray_end.y - in_ray_start.y);
	c_vec2f_t result;
	result.y = in_line_y;
	result.x = in_ray_start.x + (in_ray_end.x - in_ray_start.x) * T;
	return result;
}

bool c_line_vs_line(const c_vec2f_t& aStart1, const c_vec2f_t& anEnd1, const c_vec2f_t& aStart2, const c_vec2f_t& anEnd2, c_vec2f_t& anIntersection)
{
	const float A2 = (anEnd2.y - aStart2.y) * (anEnd1.x - aStart1.x) - (anEnd2.x - aStart2.x) * (anEnd1.y - aStart1.y);
	if (A2 == 0.f)
		return false;

	const float A1 = (anEnd2.x - aStart2.x) * (aStart1.y - aStart2.y) - (anEnd2.y - aStart2.y) * (aStart1.x - aStart2.x);
	const float AU1 = A1 / A2;

	anIntersection.x = aStart1.x + (anEnd1.x - aStart1.x) * AU1;
	anIntersection.y = aStart1.y + (anEnd1.y - aStart1.y) * AU1;

	return true;
}

bool c_line_vs_circle(const c_vec2f_t& aStart, const c_vec2f_t& anEnd, const c_vec2f_t& aCircleCenter, const float aRadius, float& aParam)
{
	//special case for start == end
	if (aStart == anEnd)
	{
		if (c_length(aStart - aCircleCenter) < aRadius)
		{
			aParam = 0;
			return true;
		}
		else
		{
			return false;
		}
	}
	//start != end
	else
	{
		//check if start is inside circle
		if (c_length(aStart - aCircleCenter) < aRadius)
		{
			aParam = 0;
			return true;
		}
		//check if end is inside circle
		else if (c_length(anEnd - aCircleCenter) < aRadius)
		{
			aParam = 1;
			return true;
		}
		//calc intersection
		else
		{
			//see http://astronomy.swin.edu.au/~pbourke/geometry/sphereline/
			const c_vec2f_t LINE = anEnd - aStart;

			const float a = c_length_squared(LINE);
			const float b = 2 * c_dot_product(LINE, aStart - aCircleCenter);
			const float c = c_length_squared(aStart) + c_length_squared(aCircleCenter) - 2 * c_dot_product(aStart, aCircleCenter) - (aRadius * aRadius);

			const float d = b * b - 4 * a * c;

			if (d < 0)
			{
				return false;
			}
			else if (d == 0.0f)
			{
				aParam = -b / 2 * a;
			}
			else
			{
				const float u1 = (-b + ::sqrtf(d)) / (2 * a);
				const float u2 = (-b - ::sqrtf(d)) / (2 * a);

				if (u1 < u2)
				{
					aParam = u1;
				}
				else
				{
					aParam = u2;
				}
			}

			return true;
		}
	}
}

bool c_ray_vs_circle(const c_vec2f_t& aStart, const c_vec2f_t& anEnd, const c_vec2f_t& aCircleCenter, const float aRadius)
{
	float p;
	return c_ray_vs_circle(aStart, anEnd, aCircleCenter, aRadius, p);
}

bool c_ray_vs_circle(const c_vec2f_t& aStart, const c_vec2f_t& anEnd, const c_vec2f_t& aCircleCenter, const float aRadius, float& aParam)
{
	if (c_line_vs_circle(aStart, anEnd, aCircleCenter, aRadius, aParam))
		return aParam >= 0.f && aParam <= 1.f;

	return false;
}

bool c_ray_vs_circle(const c_vec2f_t& aStart, const c_vec2f_t& anEnd, const c_vec2f_t& aCircleCenter, const float aRadius, c_vec2f_t& anIntersection)
{
	float p;
	if (c_ray_vs_circle(aStart, anEnd, aCircleCenter, aRadius, p))
	{
		anIntersection = aStart + (anEnd - aStart) * p;
		return true;
	}

	return false;
}

bool c_ray_vs_circle(const float aStartX, const float aStartY, const float anEndX, const float anEndY, const float aCircleX, const float aCircleY, const float aRadius)
{
	float param;
	return c_ray_vs_circle(
		{ aStartX, aStartY },
		{ anEndX, anEndY },
		{ aCircleX, aCircleY },
		aRadius,
		param);
}

c_ray_vs_t c_ray_vs_triangle(
	const c_vec2f_t& in_ray_start, const c_vec2f_t& in_ray_end, const c_vec2f_t* in_tri,
	c_vec2f_t& out_intersection)
{
	c_vec2f_t b[3];
	c_vec2f_t a;
	bool s[3];
	bool e[3];

	for (uint32_t i = 0; i < 3; ++i)
	{
		b[i] = in_tri[(i + 1) % 3] - in_tri[i];

		//is start point "left of" / outside?
		a = in_ray_start - in_tri[i];
		s[i] = a.x * b[i].y > a.y * b[i].x;

		//is end point "left of" / outside?
		a = in_ray_end - in_tri[i];
		e[i] = a.x * b[i].y > a.y * b[i].x;

		//if start and end are both "left of" / outside a triangle edge then we cannot possibly intersect
		if (s[i] && e[i])
			return c_ray_vs_t::NO_INTERSECTION;
	}

	//if start point is inside all lines, this is a hit
	if (!s[0] && !s[1] && !s[2])
	{
		out_intersection = in_ray_start;
		return c_ray_vs_t::RAY_STARTS_INSIDE;
	}

	for (uint32_t i = 0; i < 3; ++i)
	{
		//only test the lines where the start point is outside
		if (s[i])
		{
			assert(!e[i]);//endpoint should be inside...

			//FIXME: inline this function?
#ifdef _DEBUG
			const bool RVL = 
#endif
				c_ray_vs_line(in_ray_start, in_ray_end, in_tri[i], in_tri[(i + 1) % 3], out_intersection);
			assert(RVL);

			//the intersection point must be inside the triangle
			{
				bool point_in_triangle = true;
				for (uint32_t j = 0; j < 3; ++j)
				{
					//skip the currently intersected line (we want to test the others)
					if (j != i)
					{
						//is intersection point "left of" / outside?
						a = out_intersection - in_tri[j];
						if (a.x * b[j].y > a.y * b[j].x)
						{
							point_in_triangle = false;
							break;//skip the remaining tests (inside triangle) for this intersection point
						}
					}
				}
				if (point_in_triangle)
					return c_ray_vs_t::INTERSECTION;
			}
		}
	}

	return c_ray_vs_t::NO_INTERSECTION;
}

c_ray_vs_t c_ray_vs_box(
	const c_aabbf_t& in_box, const c_ray_t& in_ray,
	c_point_normal_t& out_result)
{
	enum
	{
		RIGHT,
		LEFT,
		MIDDLE,
	};
	static_assert(0 == RIGHT, "wtf!?");
	static_assert(1 == LEFT, "wtf!?");
	static_assert(2 == MIDDLE, "wtf!?");

	struct
	{
		const float* RAY_ORIGIN;
		const float* RAY_DIR;
		const float* BOX_MIN;
		const float* BOX_MAX;
		float* result_position;
		float* result_normal;
	} help =
	{
		(float*)&in_ray.ori,
		(float*)&in_ray.dir,
		(float*)&in_box.min,
		(float*)&in_box.max,
		(float*)&out_result.p,
		(float*)&out_result.n,
	};

	//Find candidate planes; this loop can be avoided if
	//rays cast all from the eye(assume perspective view)
	bool inside = true;
	uint32_t quadrant[3];
	float candidate_plane[3];
	for (int32_t i = 0; i < 3; ++i)
	{
		if (help.RAY_ORIGIN[i] < help.BOX_MIN[i])
		{
			quadrant[i] = LEFT;
			candidate_plane[i] = help.BOX_MIN[i];
			inside = false;
		}
		else if (help.RAY_ORIGIN[i] > help.BOX_MAX[i])
		{
			quadrant[i] = RIGHT;
			candidate_plane[i] = help.BOX_MAX[i];
			inside = false;
		}
		else
		{
			quadrant[i] = MIDDLE;
		}
	}

	//Ray origin inside bounding box
	if (inside)
	{
		out_result.p = in_ray.ori;
		out_result.n = {};
		return c_ray_vs_t::RAY_STARTS_INSIDE;
	}

	//Calculate T distances to candidate planes
	float max_t[3];
	for (int32_t i = 0; i < 3; ++i)
	{
		if (quadrant[i] != MIDDLE && help.RAY_DIR[i] != 0.0f)
			max_t[i] = (candidate_plane[i] - help.RAY_ORIGIN[i]) / help.RAY_DIR[i];
		else
			max_t[i] = -1.f;
	}

	//get largest of the maxT's for final choice of intersection
	int32_t which_plane = 0;
	for (int32_t i = 1; i < 3; ++i)
	{
		if (max_t[which_plane] < max_t[i])
			which_plane = i;
	}

	//Check final candidate actually inside box OR t past end of ray
	if (max_t[which_plane] < 0.f || max_t[which_plane] > 1.f)
	{
		//		assert(aNormal.length() == 1.f);
		return c_ray_vs_t::NO_INTERSECTION;
	}

	for (int32_t i = 0; i < 3; ++i)
	{
		if (which_plane != i)
		{
			help.result_position[i] = help.RAY_ORIGIN[i] + max_t[which_plane] * help.RAY_DIR[i];
			if (help.result_position[i] < help.BOX_MIN[i] || help.result_position[i] > help.BOX_MAX[i])
				return c_ray_vs_t::NO_INTERSECTION;
		}
		else
		{
			help.result_position[i] = candidate_plane[i];
		}
	}

	out_result.n = {};
	assert(MIDDLE != quadrant[which_plane]);
	help.result_normal[which_plane] = quadrant[which_plane] == LEFT ? -1 : 1.f;

	assert(1.f == c_length(out_result.n));
	return c_ray_vs_t::INTERSECTION;				//ray hits box
}

bool c_ray_vs_cylinder(
	const c_vec3f_t& aCylinderBase, const float aRadius, const float aHeight, const c_vec3f_t& aStart, const c_vec3f_t& anEnd,
	c_point_normal_t& aResult)
{
	float param;

	//vs infinite cylinder
	if (c_ray_vs_circle({ aStart.x, aStart.z }, { anEnd.x, anEnd.z }, { aCylinderBase.x, aCylinderBase.z }, aRadius, param))
	{
		//calc intersection
		aResult.p = aStart + (anEnd - aStart) * param;

		//within the vertical span means on the outside of the cylinder
		if (aResult.p.y >= aCylinderBase.y && aResult.p.y <= (aCylinderBase.y + aHeight))
		{
			//calc the normal
			aResult.n = aResult.p - aCylinderBase;
			aResult.n.y = 0.f;
			aResult.n = c_normalize(aResult.n);
			return true;
		}

		//otherwise we need to check top and bottom planes
		const c_plane_t TOP{ { 0.f, 1.f, 0.f }, -(aCylinderBase.y + aHeight) };
		if (c_plane_intersect_ray(true, aStart, anEnd - aStart, TOP, aResult))
		{
			//check if within cylinder circle
			return c_inside_circle({ aCylinderBase.x, aCylinderBase.z }, aRadius, { aResult.p.x, aResult.p.z });
		}

		const c_plane_t BOTTOM{ { 0.f, -1.f, 0.f }, aCylinderBase.y };
		if (c_plane_intersect_ray(true, aStart, anEnd - aStart, BOTTOM, aResult))
		{
			//check if within cylinder
			return c_inside_circle({ aCylinderBase.x, aCylinderBase.z }, aRadius, { aResult.p.x, aResult.p.z });
		}
	}

	return false;
}

bool c_ray_vs_triangle(
	const c_vec3f_t& aV0, const c_vec3f_t& aV1, const c_vec3f_t& aV2, const c_vec3f_t& aStart, const c_vec3f_t& anEnd,
	c_point_normal_t& aResult)
{
	//Calculate the parameters for the plane
	aResult.n.x = (aV1.y - aV0.y) * (aV2.z - aV0.z) - (aV1.z - aV0.z) * (aV2.y - aV0.y);
	aResult.n.y = (aV1.z - aV0.z) * (aV2.x - aV0.x) - (aV1.x - aV0.x) * (aV2.z - aV0.z);
	aResult.n.z = (aV1.x - aV0.x) * (aV2.y - aV0.y) - (aV1.y - aV0.y) * (aV2.x - aV0.x);
	aResult.n = c_normalize(aResult.n);

	//Calculate the position on the line that intersects the plane
	float denom =
		aResult.n.x * (anEnd.x - aStart.x) +
		aResult.n.y * (anEnd.y - aStart.y) +
		aResult.n.z * (anEnd.z - aStart.z);
	if (0.f == denom)
	{       // Line and plane don't intersect
		return false;
	}

	float d = -(aResult.n.x * aV0.x + aResult.n.y * aV0.y + aResult.n.z * aV0.z);
	float mu = -(d + aResult.n.x * aStart.x + aResult.n.y * aStart.y + aResult.n.z * aStart.z);
	if (::fabsf(mu) > 0.001f)
	{
		mu = mu / denom;
		if (mu < 0.0f || mu > 1.0f)
		{  // Intersection not along line segment
			return false;
		}
	}
	else
	{
		mu = 0;
	}
	aResult.p.x = aStart.x + mu * (anEnd.x - aStart.x);
	aResult.p.y = aStart.y + mu * (anEnd.y - aStart.y);
	aResult.p.z = aStart.z + mu * (anEnd.z - aStart.z);

	//Determine whether or not the intersection point is bounded by pa,pb,pc
	const c_vec3f_t pa1 = c_normalize(aV0 - aResult.p);

	const c_vec3f_t pa2 = c_normalize(aV1 - aResult.p);

	const c_vec3f_t pa3 = c_normalize(aV2 - aResult.p);

	float a1 = c_dot_product(pa1, pa2);
	float a2 = c_dot_product(pa2, pa3);
	float a3 = c_dot_product(pa3, pa1);
	if (a1 >= 1.0f)
	{
		return false;
	}
	if (a2 >= 1.0f)
	{
		return false;
	}
	if (a3 >= 1.0f)
	{
		return false;
	}
	constexpr float RTOD = 57.29578f;	//convert radians to degrees 
	float total = (::acosf(a1) + ::acosf(a2) + ::acosf(a3)) * RTOD;
	const float EPS(0.1f);
	if (::fabsf(total - 360.f) > EPS)
	{
		return false;
	}

	return true;
}

float c_ray_distance(const c_vec2f_t& in_ray_start, const c_vec2f_t& in_ray_end, const c_vec2f_t& in_point)
{
	const float abx = in_ray_end.x - in_ray_start.x;
	const float aby = in_ray_end.y - in_ray_start.y;

	const float bex = in_point.x - in_ray_end.x;
	const float bey = in_point.y - in_ray_end.y;

	//endpoint
	if (abx * bex + aby * bey > 0)
	{
		const float y = in_point.y - in_ray_end.y;
		const float x = in_point.x - in_ray_end.x;
		return ::sqrtf(x * x + y * y);
	}

	const float aex = in_point.x - in_ray_start.x;
	const float aey = in_point.y - in_ray_start.y;

	//endpoint
	if (abx * aex + aby * aey < 0)
	{
		const float y = in_point.y - in_ray_start.y;
		const float x = in_point.x - in_ray_start.x;
		return ::sqrtf(x * x + y * y);
	}

	//segment, finding the perpendicular distance
	return ::fabsf(abx * aey - aby * aex) / ::sqrtf(abx * abx + aby * aby);
}

float c_signed_line_distance(const c_vec2f_t& in_line_a, const c_vec2f_t& in_line_b, const c_vec2f_t& in_point)
{
	const float abx = in_line_b.x - in_line_a.x;
	const float aby = in_line_b.y - in_line_a.y;

	const float aex = in_point.x - in_line_a.x;
	const float aey = in_point.y - in_line_a.y;

	//return ::fabsf(abx * aey - aby * aex) / ::sqrtf(abx * abx + aby * aby);	//unsigned
	return (abx * aey - aby * aex) / ::sqrtf(abx * abx + aby * aby);	//signed
}

float c_angle_bound(const float angle)
{
	float result = angle;

	while (result < 0.f)
		result += C_PI2;

	while (result > C_PI2)
		result -= C_PI2;

	return result;
}

bool c_inside_recti(const c_vec2i_t& aPosition, const c_vec2i_t& aMin, const c_vec2i_t& aMax)
{
	return
		__int32_in_range(aPosition.x, c_min_int32(aMin.x, aMax.x), c_max_int32(aMin.x, aMax.x)) &&
		__int32_in_range(aPosition.y, c_min_int32(aMin.y, aMax.y), c_max_int32(aMin.y, aMax.y));
}

bool c_inside_rectf(const c_vec2f_t& aPosition, const c_vec2f_t& aMin, const c_vec2f_t& aMax)
{
	return
		__float_in_range(aPosition.x, c_min_float(aMin.x, aMax.x), c_max_float(aMin.x, aMax.x)) &&
		__float_in_range(aPosition.y, c_min_float(aMin.y, aMax.y), c_max_float(aMin.y, aMax.y));
}

bool c_inside_circle(const c_vec2f_t& aCenter, const float aRadius, const c_vec2f_t& aPointToTest)
{
	return c_length(aCenter - aPointToTest) < aRadius;
}

bool c_inside_cylinder(const c_vec3f_t& aCylinderBase, const float aRadius, const float aHeight, const c_vec3f_t& aPointToTest)
{
	if (!c_inside_circle({ aCylinderBase.x, aCylinderBase.z }, aRadius, { aPointToTest.x, aPointToTest.z }))
		return false;

	return aPointToTest.y > aCylinderBase.y && aPointToTest.y < (aCylinderBase.y + aHeight);
}

/*
bool c_point_inside_triangle(const c_vec2f_t& V0, const c_vec2f_t& V1, const c_vec2f_t& V2, const c_vec2f_t& aPoint)
{
	return
		__right_of(V0, V1, aPoint) &&
		__right_of(V1, V2, aPoint) &&
		__right_of(V2, V0, aPoint);
}
*/
bool c_point_in_triangle(const c_vec2f_t& in_point, const c_vec2f_t* in_tri)
{
	c_vec2f_t a;
	c_vec2f_t b;
	for (uint32_t i = 0; i < 3; ++i)
	{
		a = in_point - in_tri[i];
		b = in_tri[(i + 1) % 3] - in_tri[i];
		if (a.x * b.y > a.y * b.x)
			return false;
	}
	return true;
}

c_vec3f_t c_vec3f_rotated_around_axis(const c_vec3f_t& axis, const float angle, const c_vec3f_t& vector)
{
	//vector / point
	const float VECTOR[4] =
	{
		vector.x,
		vector.y,
		vector.z,
		1.f,
	};

	//rotation
	const float SIN_HALF_ANGLE = ::sinf(angle * .5f);
	const float ROTATION[4] =
	{
		axis.x * SIN_HALF_ANGLE,
		axis.y * SIN_HALF_ANGLE,
		axis.z * SIN_HALF_ANGLE,
		::cosf(angle * .5f),
	};

	//conjugate of rotation
	const float ROTATION_CONJUGATE[4] =
	{
		-ROTATION[0],
		-ROTATION[1],
		-ROTATION[2],
		ROTATION[3],
	};

	//this is basically: result = ROTATION * VECTOR * ROTATION_CONJUGATE
	float quat_prod[4];
	__quaternion_multiply(ROTATION, VECTOR, quat_prod);
	float result[4];
	__quaternion_multiply(quat_prod, ROTATION_CONJUGATE, result);

	return { result[0], result[1], result[2] };
}

c_vec3f_t c_triangle_normal(const c_vec3f_t& v0, const c_vec3f_t& v1, const c_vec3f_t& v2)
{
	return c_normalize(c_cross_product(v1 - v0, v2 - v1));
}

//inputs are side lengths...
float c_triangle_area(const float a, const float b, const float c)
{
	const float s = (a + b + c) * .5f;
	return ::sqrtf(s * (s - a) * (s - b) * (s - c));
}

c_vec2i_t c_vec2i_minimum(const c_vec2i_t& a, const c_vec2i_t& b)
{
	return{ c_min_int32(a.x, b.x), c_min_int32(a.y, b.y) };
}

c_vec2f_t c_vec2f_minimum(const c_vec2f_t& a, const c_vec2f_t& b)
{
	return{ c_min_float(a.x, b.x), c_min_float(a.y, b.y) };
}

c_vec3i_t c_vec3i_minimum(const c_vec3i_t& a, const c_vec3i_t& b)
{
	return{ c_min_int32(a.x, b.x), c_min_int32(a.y, b.y), c_min_int32(a.z, b.z) };
}

c_vec3f_t c_vec3f_minimum(const c_vec3f_t& a, const c_vec3f_t& b)
{
	return{ c_min_float(a.x, b.x), c_min_float(a.y, b.y), c_min_float(a.z, b.z) };
}

c_vec2i_t c_vec2i_maximum(const c_vec2i_t& a, const c_vec2i_t& b)
{
	return{ c_max_int32(a.x, b.x), c_max_int32(a.y, b.y) };
}

c_vec2f_t c_vec2f_maximum(const c_vec2f_t& a, const c_vec2f_t& b)
{
	return{ c_max_float(a.x, b.x), c_max_float(a.y, b.y) };
}

c_vec3i_t c_vec3i_maximum(const c_vec3i_t& a, const c_vec3i_t& b)
{
	return{ c_max_int32(a.x, b.x), c_max_int32(a.y, b.y), c_max_int32(a.z, b.z) };
}

c_vec3f_t c_vec3f_maximum(const c_vec3f_t& a, const c_vec3f_t& b)
{
	return{ c_max_float(a.x, b.x), c_max_float(a.y, b.y), c_max_float(a.z, b.z) };
}

void c_int32_min_max(int32_t& aMin, int32_t& aMax)
{
	if (aMin > aMax)
	{
		const int32_t TEMP = aMin;
		aMin = aMax;
		aMax = TEMP;
	}
}

void c_vec3i_min_max(c_vec3i_t& aMin, c_vec3i_t& aMax)
{
	c_int32_min_max(aMin.x, aMax.x);
	c_int32_min_max(aMin.y, aMax.y);
	c_int32_min_max(aMin.z, aMax.z);
}

void c_vec3f_min_max(c_vec3f_t& aMin, c_vec3f_t& aMax)
{
	__float_min_max(aMin.x, aMax.x);
	__float_min_max(aMin.y, aMax.y);
	__float_min_max(aMin.z, aMax.z);
}

int32_t c_int32_bound(const int32_t min, const int32_t max, const int32_t input)
{
	if (input < min)
		return min;

	if (input > max)
		return max;

	return input;
}

float c_float_bound(const float min, const float max, const float input)
{
	if (input < min)
		return min;

	if (input > max)
		return max;

	return input;
}

c_vec3f_t c_vec3f_bound(const c_vec3f_t& min, const c_vec3f_t& max, const c_vec3f_t& input)
{
	return{ c_float_bound(min.x, max.x, input.x), c_float_bound(min.y, max.y, input.y), c_float_bound(min.z, max.z, input.z) };
}

c_vec3f_t c_vec3f_unit(const c_vec2f_t& v)
{
	return { v.x, v.y, ::sqrtf(1.f - v.x * v.x - v.y * v.y) };
}

/*
bool core_distance_point_ray(const vec3f_t& aPoint, const vec3f_t& aRayStart, const vec3f_t& aRayEnd, float& aDistance)
{
	const float LENGTH = c_length(aRayEnd - aRayStart);

	const float U =
		(
			((aPoint.x - aRayStart.x) * (aRayEnd.x - aRayStart.x)) +
			((aPoint.y - aRayStart.y) * (aRayEnd.y - aRayStart.y)) +
			((aPoint.z - aRayStart.z) * (aRayEnd.z - aRayStart.z))
		) / (LENGTH * LENGTH);

	if (U < 0.f || U > 1.f)
		return false;   // closest point does not fall within the line segment

	const vec3f_t INTERSECTION = aRayStart + (aRayEnd - aRayStart) * U;

	aDistance = c_length(aPoint - INTERSECTION);

	return true;
}
*/

int8_t c_min_int8(const int8_t a, const int8_t b)
{
	if (a < b)
		return a;
	return b;
}

int16_t c_min_int16(const int16_t a, const int16_t b)
{
	if (a < b)
		return a;
	return b;
}

int32_t c_min_int32(const int32_t a, const int32_t b)
{
	if (a < b)
		return a;
	return b;
}

uint16_t c_min_uint16(const uint16_t a, const uint16_t b)
{
	if (a < b)
		return a;
	return b;
}

uint32_t c_min_uint32(const uint32_t a, const uint32_t b)
{
	if (a < b)
		return a;
	return b;
}

float c_min_float(const float a, const float b)
{
	if (a < b)
		return a;
	return b;
}

int8_t c_max_int8(const int8_t a, const int8_t b)
{
	if (a > b)
		return a;
	return b;
}

int16_t c_max_int16(const int16_t a, const int16_t b)
{
	if (a > b)
		return a;
	return b;
}

int32_t c_max_int32(const int32_t a, const int32_t b)
{
	if (a > b)
		return a;
	return b;
}

uint16_t c_max_uint16(const uint16_t a, const uint16_t b)
{
	if (a > b)
		return a;
	return b;
}

uint32_t c_max_uint32(const uint32_t a, const uint32_t b)
{
	if (a > b)
		return a;
	return b;
}

float c_max_float(const float a, const float b)
{
	if (a > b)
		return a;
	return b;
}

double c_max_double(const double a, const double b)
{
	if (a > b)
		return a;
	return b;
}

//c_sphere_t
//c_sphere_t
//c_sphere_t
//c_sphere_t
c_sphere_t c_sphere_from_box(const c_aabbf_t& in_box)
{
	c_sphere_t result{};
	result.center = in_box.min + (in_box.max - in_box.min) * .5f;
	result.radius = c_length(in_box.min - result.center);
	return result;
}

/*
bool c_sphere_vs_ray(
	const c_sphere_t& in_sphere, const c_vec3f_t& in_ray_start, const c_vec3f_t& in_ray_end,
	float& out_param)
{
	return c_sphere_vs_ray(in_ray_start, in_ray_end, in_sphere.center, in_sphere.radius, out_param);
}

bool c_sphere_vs_ray(
	const c_vec3f_t& in_ray_start, const c_vec3f_t& in_ray_end, const c_vec3f_t& in_sphere_center, const float in_radius,
	float& out_param)
{
	return __legacy_sphere_vs_line(in_ray_start, in_ray_end, in_sphere_center, in_radius, out_param) && out_param >= 0.f && out_param <= 1.f;
}
*/
bool c_sphere_vs_ray(
	const c_sphere_t& in_sphere, const c_vec3f_t& in_ray_start, const c_vec3f_t& in_ray_end,
	float& out_param)
{
	return __legacy_sphere_vs_line(in_sphere, in_ray_start, in_ray_end, out_param) && out_param >= 0.f && out_param <= 1.f;
}

float c_sphere_vs_line(const c_vec3f_t& in_ray_origin, const c_vec3f_t& in_ray_unit_vector, const c_vec3f_t& in_sphere_center, const float in_radius)
{
	const c_vec3f_t O_C = in_ray_origin - in_sphere_center;
	const float L_DOT_O_C = c_dot_product(in_ray_unit_vector, O_C);
	const float UNDER_THE_SQRT = L_DOT_O_C * L_DOT_O_C - c_length_squared(O_C) + in_radius * in_radius;
	if (UNDER_THE_SQRT < 0.f)
		return -1.f;

	if (UNDER_THE_SQRT == 0.f)
		return -L_DOT_O_C;

	const float SQRT = ::sqrtf(UNDER_THE_SQRT);
	const float SOLUTION1 = -L_DOT_O_C + SQRT;
	const float SOLUTION2 = -L_DOT_O_C - SQRT;

	if (SOLUTION1 < SOLUTION2)
		return SOLUTION1;

	return SOLUTION2;
}

//c_plane_t
//c_plane_t
//c_plane_t
//c_plane_t
c_plane_t c_plane_inversion(const c_plane_t& PLANE)
{
	return{ -PLANE.normal, -PLANE.determinant };
}

bool c_plane_is_flush(const c_plane_t& THAT, const c_plane_t& THIS)
{
	return THIS.normal == -THAT.normal && THIS.determinant == -THAT.determinant;
}

c_plane_t c_plane_from_points(const c_vec3f_t& v0, const c_vec3f_t& v1, const c_vec3f_t& v2)
{
	c_plane_t result;
	result.normal = c_triangle_normal(v0, v1, v2);
	result.determinant = -c_dot_product(result.normal, v0);

	return result;
}

c_plane_t c_plane_normalized(const c_plane_t& PLANE)
{
	const float MAGNITUDE = c_length(PLANE.normal);
	return{ { PLANE.normal.x / MAGNITUDE, PLANE.normal.y / MAGNITUDE, PLANE.normal.z / MAGNITUDE }, PLANE.determinant / MAGNITUDE };
}

bool c_plane_equals(const c_plane_t& THAT, const c_plane_t& THIS)
{
	return THIS.normal == THAT.normal && THIS.determinant == THAT.determinant;
}

float c_plane_distance(const c_vec3f_t& POINT, const c_plane_t& PLANE)
{
	return c_dot_product(PLANE.normal, POINT) + PLANE.determinant;
}

bool c_plane_is_on(const c_vec3f_t& POINT, const c_plane_t& PLANE)
{
	return ::fabsf(c_plane_distance(POINT, PLANE)) < C_PLANE_EPSILON;
}

bool c_plane_intersect_line(
	const bool aRegardFacing, const c_vec3f_t& aPoint, const c_vec3f_t& aVector, const c_plane_t& PLANE,
	c_point_normal_t& aResult)
{
	const float DOT = c_dot_product(PLANE.normal, aVector);
	if (aRegardFacing && DOT >= 0.f)
		return false;	//parallell or backfacing

	const float T = -(c_dot_product(PLANE.normal, aPoint) + PLANE.determinant) / DOT;
	aResult.p = aPoint + aVector * T;
	aResult.n = PLANE.normal;

	return true;
}

bool c_plane_intersect_ray(
	const bool aRegardFacing, const c_vec3f_t& aPoint, const c_vec3f_t& aVector, const c_plane_t& PLANE,
	c_point_normal_t& aResult)
{
	const float DOT = c_dot_product(PLANE.normal, aVector);
	if (aRegardFacing && DOT >= 0.f)
		return false;	//parallell or backfacing

	const float T = -(c_dot_product(PLANE.normal, aPoint) + PLANE.determinant) / DOT;
	if (T < 0.f || T > 1.f)
		return false;	//outside ray

	aResult.p = aPoint + aVector * T;
	aResult.n = PLANE.normal;

	return true;
}

#if 0
// http://realtimecollisiondetection.net/blog/?p=103
bool c_are_triangle_and_sphere_separated(
	const c_vec3f_t& in_t0,//triangle vertex 0
	const c_vec3f_t& in_t1,//triangle vertex 1
	const c_vec3f_t& in_t2,//triangle vertex 1
	const c_vec3f_t& P,//sphere center
	const float r)//sphere radius
{
	const c_vec3f_t A = in_t0 - P;
	const c_vec3f_t B = in_t1 - P;
	const c_vec3f_t C = in_t2 - P;

	const float rr = r * r;
	const c_vec3f_t V = c_cross_product(B - A, C - A);
	const float d = c_dot_product(A, V);
	const float e = c_dot_product(V, V);
	const int sep1 = d * d > rr * e;
	if (sep1)
		return true;

	const float aa = c_dot_product(A, A);
	const float ab = c_dot_product(A, B);
	const float ac = c_dot_product(A, C);
	const int sep2 = (aa > rr) & (ab > aa) & (ac > aa);
	if (sep2)
		return true;

	const float bb = c_dot_product(B, B);
	const float bc = c_dot_product(B, C);
	const int sep3 = (bb > rr) & (ab > bb) & (bc > bb);
	if (sep3)
		return true;

	const float cc = c_dot_product(C, C);
	const int sep4 = (cc > rr) & (ac > cc) & (bc > cc);
	if (sep4)
		return true;

	const c_vec3f_t AB = B - A;
	const c_vec3f_t BC = C - B;
	const c_vec3f_t CA = A - C;

	const float d1 = ab - aa;
	const float e1 = c_dot_product(AB, AB);

	const c_vec3f_t Q1 = A * e1 - AB * d1;
	const c_vec3f_t QC = C * e1 - Q1;
	const int sep5 = (c_dot_product(Q1, Q1) > rr * e1 * e1) & (c_dot_product(Q1, QC) > 0);
	if (sep5)
		return true;

	const float d2 = bc - bb;
	const float e2 = c_dot_product(BC, BC);

	const c_vec3f_t Q2 = B * e2 - BC * d2;
	const c_vec3f_t QA = A * e2 - Q2;
	const int sep6 = (c_dot_product(Q2, Q2) > rr * e2 * e2) & (c_dot_product(Q2, QA) > 0);
	if (sep6)
		return true;

	const float d3 = ac - cc;
	const float e3 = c_dot_product(CA, CA);

	const c_vec3f_t Q3 = C * e3 - CA * d3;
	const c_vec3f_t QB = B * e3 - Q3;
	const int sep7 = (c_dot_product(Q3, Q3) > rr * e3 * e3) & (c_dot_product(Q3, QB) > 0);
	if (sep7)
		return true;

	return false;
}
#endif

#endif
