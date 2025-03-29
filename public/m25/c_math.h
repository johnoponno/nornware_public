#pragma once

#if 1

#include "c_vector.h"

#define C_AABBI_CLEAR { { INT_MAX, INT_MAX, INT_MAX }, { INT_MIN, INT_MIN, INT_MIN } }
#define C_AABBF_CLEAR { { FLT_MAX, FLT_MAX, FLT_MAX }, { -FLT_MAX, -FLT_MAX, -FLT_MAX } }
#define C_AABBI_CALC(point, aabbi) { c_vec3i_minimum(aabbi.min, point), c_vec3i_maximum(aabbi.max, point) }
#define C_AABBF_CALC(point, aabbf) { c_vec3f_minimum((aabbf).min, point), c_vec3f_maximum((aabbf).max, point) }
#define C_AABBF_ELEMENTS_CALC(x, y, z, aabbf) { c_vec3f_minimum(aabbf.min, {x, y, z}), c_vec3f_maximum(aabbf.max, {x, y, z}) }

#define C_COLLISION_EPSILON .01f

#define C_PLANE_EPSILON .001f

enum struct c_ray_vs_t
{
	RAY_STARTS_INSIDE,
	NO_INTERSECTION,
	INTERSECTION,
};

struct c_aabbi_t
{
	c_vec3i_t min;
	c_vec3i_t max;
};

struct c_aabbf_t
{
	c_vec3f_t min;
	c_vec3f_t max;
};

struct c_explicit_corner_box_3f_t
{
	//0 - 3 = bottom
	//4 - 7 = top
	c_vec3f_t corners[8];
};

struct c_point_normal_t
{
	c_vec3f_t p;
	c_vec3f_t n;
};

struct c_line_line_intersect_t
{
	c_vec3f_t pa;
	c_vec3f_t pb;
	float mua;
	float mub;
	uint32_t eh;
};

struct c_ray_t
{
	c_vec3f_t ori;
	c_vec3f_t dir;
};

struct c_sphere_t
{
	c_vec3f_t center;
	float radius;
};

struct c_plane_t
{
	c_vec3f_t normal;
	float determinant;
};

/*
   Calculate the line segment PaPb that is the shortest route between
   two lines P1P2 and P3P4. Calculate also the values of mua and mub where
	  Pa = P1 + mua (P2 - P1)
	  Pb = P3 + mub (P4 - P3)
   Return FALSE if no solution exists.
*/
c_line_line_intersect_t c_line_line_intersect(const c_vec3f_t& p1, const c_vec3f_t& p2, const c_vec3f_t& p3, const c_vec3f_t& p4);

float c_gain(const float x, const float k);
float c_scale_and_saturate(const float anEdge0, const float anEdge1, const float aValue);
float c_smoother_step(const float anEdge0, const float anEdge1, const float aValue);
float c_parabola(const float x, const float k);
float c_lerp_angle_0_2pi(const float speed, const float target, const float current);

c_explicit_corner_box_3f_t c_aabbf_make_explicit(const c_aabbf_t& in_box, const c_vec3f_t* in_offset, const float* in_scale);
c_aabbf_t c_aabbf_from_explicit(const c_explicit_corner_box_3f_t& in_box);
void c_aabbf_add(const c_explicit_corner_box_3f_t& aBox, c_aabbf_t& anInstance);
c_vec3f_t c_aabbf_center(const c_aabbf_t& anInstance);
bool c_aabbf_point_inside_eh(const c_vec3f_t& aPoint, const c_aabbf_t& anInstance);
bool c_aabbf_overlaps_eh(const c_aabbf_t& a, const c_aabbf_t& b);

bool c_ray_vs_ray(
	const c_vec2f_t& aStart1, const c_vec2f_t& anEnd1, const c_vec2f_t& aStart2, const c_vec2f_t& anEnd2,
	c_vec2f_t& anIntersection);
bool c_ray_vs_line(
	const c_vec2f_t& in_ray_start, const c_vec2f_t& in_ray_end,
	const c_vec2f_t& in_line_start, const c_vec2f_t& in_line_end,//points on line, but the line is technically infinite...
	c_vec2f_t& out_intersection);
c_vec2f_t c_ray_vs_vertical_line(const c_vec2f_t& in_ray_start, const c_vec2f_t& in_ray_end, const float in_line_x);
c_vec2f_t c_ray_vs_horizontal_line(const c_vec2f_t& in_ray_start, const c_vec2f_t& in_ray_end, const float in_line_y);
bool c_line_vs_line(
	const c_vec2f_t& aStart1, const c_vec2f_t& anEnd1, const c_vec2f_t& aStart2, const c_vec2f_t& anEnd2,
	c_vec2f_t& anIntersection);
bool c_line_vs_circle(
	const c_vec2f_t& aStart, const c_vec2f_t& anEnd, const c_vec2f_t& aCircleCenter, const float aRadius,
	float& aParam);
bool c_ray_vs_circle(const c_vec2f_t& aStart, const c_vec2f_t& anEnd, const c_vec2f_t& aCircleCenter, const float aRadius);
bool c_ray_vs_circle(
	const c_vec2f_t& aStart, const c_vec2f_t& anEnd, const c_vec2f_t& aCircleCenter, const float aRadius,
	float& aParam);
bool c_ray_vs_circle(
	const c_vec2f_t& aStart, const c_vec2f_t& anEnd, const c_vec2f_t& aCircleCenter, const float aRadius,
	c_vec2f_t& anIntersection);
bool c_ray_vs_circle(const float aStartX, const float aStartY, const float anEndX, const float anEndY, const float aCircleX, const float aCircleY, const float aRadius);
c_ray_vs_t c_ray_vs_triangle(
	const c_vec2f_t& in_ray_start, const c_vec2f_t& in_ray_end, const c_vec2f_t* in_tri,
	c_vec2f_t& out_intersection);
c_ray_vs_t c_ray_vs_box(
	const c_aabbf_t& in_aabb, const c_ray_t& in_ray,
	c_point_normal_t& out_result);
bool c_ray_vs_cylinder(
	const c_vec3f_t& aCylinderBase, const float aRadius, const float aHeight, const c_vec3f_t& aStart, const c_vec3f_t& anEnd,
	c_point_normal_t& aResult);
bool c_ray_vs_triangle(
	const c_vec3f_t& aV0, const c_vec3f_t& aV1, const c_vec3f_t& aV2, const c_vec3f_t& aStart, const c_vec3f_t& anEnd,
	c_point_normal_t& aResult);
float c_ray_distance(const c_vec2f_t& in_ray_start, const c_vec2f_t& in_ray_end, const c_vec2f_t& in_point);
float c_signed_line_distance(const c_vec2f_t& in_line_a, const c_vec2f_t& in_line_b, const c_vec2f_t& in_point);

float c_angle_bound(const float angle);

bool c_inside_recti(const c_vec2i_t& aPosition, const c_vec2i_t& aMin, const c_vec2i_t& aMax);
bool c_inside_rectf(const c_vec2f_t& aPosition, const c_vec2f_t& aMin, const c_vec2f_t& aMax);
bool c_inside_circle(const c_vec2f_t& aCenter, const float aRadius, const c_vec2f_t& aPointToTest);
bool c_inside_cylinder(const c_vec3f_t& aCylinderBase, const float aRadius, const float aHeight, const c_vec3f_t& aPointToTest);
//bool c_point_inside_triangle(const c_vec2f_t& V0, const c_vec2f_t& V1, const c_vec2f_t& V2, const c_vec2f_t& aPoint);
bool c_point_in_triangle(const c_vec2f_t& in_point, const c_vec2f_t* in_tri);

c_vec3f_t c_vec3f_rotated_around_axis(const c_vec3f_t& axis, const float angle, const c_vec3f_t& vector);

c_vec3f_t c_triangle_normal(const c_vec3f_t& v0, const c_vec3f_t& v1, const c_vec3f_t& v2);

float c_triangle_area(const float a, const float b, const float c);//inputs are side lengths...

c_vec2i_t c_vec2i_minimum(const c_vec2i_t& a, const c_vec2i_t& b);
c_vec2f_t c_vec2f_minimum(const c_vec2f_t& a, const c_vec2f_t& b);
c_vec3i_t c_vec3i_minimum(const c_vec3i_t& a, const c_vec3i_t& b);
c_vec3f_t c_vec3f_minimum(const c_vec3f_t& a, const c_vec3f_t& b);

c_vec2i_t c_vec2i_maximum(const c_vec2i_t& a, const c_vec2i_t& b);
c_vec2f_t c_vec2f_maximum(const c_vec2f_t& a, const c_vec2f_t& b);
c_vec3i_t c_vec3i_maximum(const c_vec3i_t& a, const c_vec3i_t& b);
c_vec3f_t c_vec3f_maximum(const c_vec3f_t& a, const c_vec3f_t& b);

void c_int32_min_max(int32_t& aMin, int32_t& aMax);
void c_vec3i_min_max(c_vec3i_t& aMin, c_vec3i_t& aMax);
void c_vec3f_min_max(c_vec3f_t& aMin, c_vec3f_t& aMax);

int32_t c_int32_bound(const int32_t aMin, const int32_t aMax, const int32_t input);
float c_float_bound(const float min, const float max, const float input);
c_vec3f_t c_vec3f_bound(const c_vec3f_t& min, const c_vec3f_t& max, const c_vec3f_t& input);

c_vec3f_t c_vec3f_unit(const c_vec2f_t& v);

uint32_t c_frame(const float aNow, const float anFps, const uint32_t aNumFrames);

int8_t c_min_int8(const int8_t a, const int8_t b);
int16_t c_min_int16(const int16_t a, const int16_t b);
int32_t c_min_int32(const int32_t a, const int32_t b);
uint16_t c_min_uint16(const uint16_t a, const uint16_t b);
uint32_t c_min_uint32(const uint32_t a, const uint32_t b);
float c_min_float(const float a, const float b);

int8_t c_max_int8(const int8_t a, const int8_t b);
int16_t c_max_int16(const int16_t a, const int16_t b);
int32_t c_max_int32(const int32_t a, const int32_t b);
uint16_t c_max_uint16(const uint16_t a, const uint16_t b);
uint32_t c_max_uint32(const uint32_t a, const uint32_t b);
float c_max_float(const float a, const float b);
double c_max_double(const double a, const double b);

c_sphere_t c_sphere_from_box(const c_aabbf_t& aBox);
bool c_sphere_vs_ray(const c_sphere_t& aSphere, const c_vec3f_t& aRayStart, const c_vec3f_t& aRayEnd, float& aParam);
//bool c_sphere_vs_ray(const c_vec3f_t& aRayStart, const c_vec3f_t& aRayEnd, const c_vec3f_t& aSphereCenter, const float aRadius, float& aParam);
float c_sphere_vs_line(const c_vec3f_t& ray_origin, const c_vec3f_t& ray_unit_vector, const c_vec3f_t& sphere_center, const float radius);

c_plane_t c_plane_inversion(const c_plane_t& PLANE);
bool c_plane_is_flush(const c_plane_t& THAT_PLANE, const c_plane_t& THIS_PLANE);
bool c_plane_equals(const c_plane_t& THAT_PLANE, const c_plane_t& THIS_PLANE);
float c_plane_distance(const c_vec3f_t& POINT, const c_plane_t& PLANE);
bool c_plane_is_on(const c_vec3f_t& POINT, const c_plane_t& PLANE);
bool c_plane_intersect_line(const bool aRegardFacing, const c_vec3f_t& aPoint, const c_vec3f_t& aVector, const c_plane_t& PLANE, c_point_normal_t& aResult);
bool c_plane_intersect_ray(const bool aRegardFacing, const c_vec3f_t& aPoint, const c_vec3f_t& aVector, const c_plane_t& PLANE, c_point_normal_t& aResult);
c_plane_t c_plane_normalized(const c_plane_t& PLANE);
c_plane_t c_plane_from_points(const c_vec3f_t& v0, const c_vec3f_t& v1, const c_vec3f_t& v2);

#if 0
// http://realtimecollisiondetection.net/blog/?p=103
bool c_are_triangle_and_sphere_separated(
	const c_vec3f_t& in_t0,//triangle vertex 0
	const c_vec3f_t& in_t1,//triangle vertex 1
	const c_vec3f_t& in_t2,//triangle vertex 1
	const c_vec3f_t& P,//sphere center
	const double r);//sphere radius
#endif

template <typename type_t>
void c_generic_swap(type_t& one, type_t& two)
{
	type_t temp = one;
	one = two;
	two = temp;
}

constexpr float C_PI = (float)3.141592653589793238;
constexpr float C_PI2 = C_PI * 2.f;
constexpr float C_DEGREES_TO_RADIANS = C_PI / 180.f;
constexpr float C_RADIANS_TO_DEGREES = 180.f / C_PI;

#endif
