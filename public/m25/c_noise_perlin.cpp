#include "stdafx.h"

static const struct constants_t
{
	explicit constants_t()
	{
		const int32_t PERMUTATION[] =
		{
			151, 160, 137, 91, 90, 15,
			131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
			190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
			88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
			77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
			102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
			135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
			5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
			223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
			129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
			251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
			49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
			138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
		};

		for (int32_t i = 0; i < 256; ++i)
			myPermutation[256 + i] = myPermutation[i] = PERMUTATION[i];
	}

	int32_t myPermutation[512];
} CONSTANTS;

static constexpr int32_t GRAD3[12][3] =
{
	{ 1, 1, 0 },
	{ -1, 1, 0 },
	{ 1, -1, 0 },
	{ -1, -1, 0 },
	{ 1, 0, 1 },
	{ -1, 0, 1 },
	{ 1, 0, -1 },
	{ -1, 0, -1 },
	{ 0, 1, 1 },
	{ 0, -1, 1 },
	{ 0, 1, -1 },
	{ 0, -1, -1 }
};

static int32_t __floor_double_to_int(const double x)
{
	return x > 0 ? (int32_t)x : (int32_t)x - 1;
}

static double __dot3(const int32_t* g, double x, double y, double z)
{
	return g[0] * x + g[1] * y + g[2] * z;
}

//public
//public
//public
//public

double c_perlin_simplex_noise3(double xin, double yin, double zin)
{
	//C_FUNCTION_CALL;

	double n0, n1, n2, n3; // Noise contributions from the four corners

	// Skew the input space to determine which simplex cell we're in
	const double F3 = 1.0 / 3.0;
	const double s = (xin + yin + zin) * F3; // Very nice and simple skew factor for 3D
	const int32_t i = __floor_double_to_int(xin + s);
	const int32_t j = __floor_double_to_int(yin + s);
	const int32_t k = __floor_double_to_int(zin + s);
	const double G3 = 1.0 / 6.0; // Very nice and simple unskew factor, too
	const double t = (i + j + k) * G3;
	const double X0 = i - t; // Unskew the cell origin back to (x,y,z) space
	const double Y0 = j - t;
	const double Z0 = k - t;
	const double x0 = xin - X0; // The x,y,z distances from the cell origin
	const double y0 = yin - Y0;
	const double z0 = zin - Z0;

	// For the 3D case, the simplex shape is a slightly irregular tetrahedron.
	// Determine which simplex we are in.
	int32_t i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
	int32_t i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords
	if (x0 >= y0)
	{
		if (y0 >= z0)
		{
			i1 = 1;
			j1 = 0;
			k1 = 0;
			i2 = 1;
			j2 = 1;
			k2 = 0;
		} // X Y Z order
		else if (x0 >= z0)
		{
			i1 = 1;
			j1 = 0;
			k1 = 0;
			i2 = 1;
			j2 = 0;
			k2 = 1;
		} // X Z Y order
		else
		{
			i1 = 0;
			j1 = 0;
			k1 = 1;
			i2 = 1;
			j2 = 0;
			k2 = 1;
		} // Z X Y order
	}
	else
	{ // x0<y0
		if (y0 < z0)
		{
			i1 = 0;
			j1 = 0;
			k1 = 1;
			i2 = 0;
			j2 = 1;
			k2 = 1;
		} // Z Y X order
		else if (x0 < z0)
		{
			i1 = 0;
			j1 = 1;
			k1 = 0;
			i2 = 0;
			j2 = 1;
			k2 = 1;
		} // Y Z X order
		else
		{
			i1 = 0;
			j1 = 1;
			k1 = 0;
			i2 = 1;
			j2 = 1;
			k2 = 0;
		} // Y X Z order
	}

	// A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
	// a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
	// a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
	// c = 1/6.
	const double x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
	const double y1 = y0 - j1 + G3;
	const double z1 = z0 - k1 + G3;
	const double x2 = x0 - i2 + 2.0 * G3; // Offsets for third corner in (x,y,z) coords
	const double y2 = y0 - j2 + 2.0 * G3;
	const double z2 = z0 - k2 + 2.0 * G3;
	const double x3 = x0 - 1.0 + 3.0 * G3; // Offsets for last corner in (x,y,z) coords
	const double y3 = y0 - 1.0 + 3.0 * G3;
	const double z3 = z0 - 1.0 + 3.0 * G3;

	// Work out the hashed gradient _indices of the four simplex corners
	const int32_t ii = i & 255;
	const int32_t jj = j & 255;
	const int32_t kk = k & 255;
	const int32_t gi0 = CONSTANTS.myPermutation[ii + CONSTANTS.myPermutation[jj + CONSTANTS.myPermutation[kk]]] % 12;
	const int32_t gi1 = CONSTANTS.myPermutation[ii + i1 + CONSTANTS.myPermutation[jj + j1 + CONSTANTS.myPermutation[kk + k1]]] % 12;
	const int32_t gi2 = CONSTANTS.myPermutation[ii + i2 + CONSTANTS.myPermutation[jj + j2 + CONSTANTS.myPermutation[kk + k2]]] % 12;
	const int32_t gi3 = CONSTANTS.myPermutation[ii + 1 + CONSTANTS.myPermutation[jj + 1 + CONSTANTS.myPermutation[kk + 1]]] % 12;

	// Calculate the contribution from the four corners
	double t0 = 0.6 - x0 * x0 - y0 * y0 - z0 * z0;
	if (t0 < 0)
	{
		n0 = 0.0;
	}
	else
	{
		t0 *= t0;
		n0 = t0 * t0 * __dot3(GRAD3[gi0], x0, y0, z0);
	}
	double t1 = 0.6 - x1 * x1 - y1 * y1 - z1 * z1;
	if (t1 < 0)
	{
		n1 = 0.0;
	}
	else
	{
		t1 *= t1;
		n1 = t1 * t1 * __dot3(GRAD3[gi1], x1, y1, z1);
	}
	double t2 = 0.6 - x2 * x2 - y2 * y2 - z2 * z2;
	if (t2 < 0)
	{
		n2 = 0.0;
	}
	else
	{
		t2 *= t2;
		n2 = t2 * t2 * __dot3(GRAD3[gi2], x2, y2, z2);
	}
	double t3 = 0.6 - x3 * x3 - y3 * y3 - z3 * z3;
	if (t3 < 0)
	{
		n3 = 0.0;
	}
	else
	{
		t3 *= t3;
		n3 = t3 * t3 * __dot3(GRAD3[gi3], x3, y3, z3);
	}

	// Add contributions from each corner to get the final noise value.
	// The result is scaled to stay just inside [-1,1]
	return 32.0 * (n0 + n1 + n2 + n3);
}
