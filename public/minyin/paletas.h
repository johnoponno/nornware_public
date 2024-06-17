#pragma once

struct minyin_bitmap_t;

struct paletas_t
{
	struct item_t
	{
		const char* file;
		minyin_bitmap_t* bitmap;
	};
	std::vector<item_t> items;
};

void paletas_item(const char* in_file, minyin_bitmap_t& out_bitmap, paletas_t& out_palettizer);
bool paletas_process(const uint32_t in_palette_size, paletas_t& out_palettizer);
