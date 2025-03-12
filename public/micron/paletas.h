#pragma once

struct micron_bitmap_t;
struct micron_color_t;

struct paletas_t
{
	void item(
		const char* in_file,
		micron_bitmap_t& out_bitmap);
	bool calculate(
		const uint32_t in_palette_size,
		micron_color_t* out_palette_256);

private:

	struct item_t
	{
		const char* file;
		micron_bitmap_t* bitmap;
	};
	std::vector<item_t> _items;
};
