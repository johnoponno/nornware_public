#pragma once

struct c_vec2f_t;

bool c_inside_rectf(const c_vec2f_t& aPosition, const c_vec2f_t& aMin, const c_vec2f_t& aMax);
void c_int32_min_max(int32_t& aMin, int32_t& aMax);
uint32_t c_frame(const float aNow, const float anFps, const uint32_t aNumFrames);
double c_perlin_simplex_noise3(double aX, double aY, double aZ);

constexpr float C_PI = (float)3.141592653589793238;
constexpr float C_PI2 = C_PI * 2.f;
