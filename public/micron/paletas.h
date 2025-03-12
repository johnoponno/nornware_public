#pragma once

struct micron_bitmap_t;

struct pixel_t
{
	bool operator < (const pixel_t& in_other) const;
	bool operator >= (const pixel_t& in_other) const;

	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct paletas_t
{
	struct item_t
	{
		const char* file;
		micron_bitmap_t* bitmap;
	};
	std::vector<item_t> items;
};

void paletas_item(
	const char* in_file,
	micron_bitmap_t& out_bitmap, paletas_t& out_paletas);
bool paletas_calculate(
	const uint32_t in_palette_size,
	paletas_t& out_paletas, pixel_t* out_palette);
