#include "stdafx.h"
#include "paletas.h"

#include "micron.h"
#include "fs.h"
#include "sd_bitmap.h"

static int32_t __sort_r(const void* elem1, const void* elem2)
{
	const micron_color_t* ELEM1 = (micron_color_t*)elem1;
	const micron_color_t* ELEM2 = (micron_color_t*)elem2;

	if (ELEM1->r < ELEM2->r)
		return -1;

	if (ELEM1->r > ELEM2->r)
		return 1;

	return 0;
}

static int32_t __sort_g(const void* elem1, const void* elem2)
{
	const micron_color_t* ELEM1 = (micron_color_t*)elem1;
	const micron_color_t* ELEM2 = (micron_color_t*)elem2;

	if (ELEM1->g < ELEM2->g)
		return -1;

	if (ELEM1->g > ELEM2->g)
		return 1;

	return 0;
}

static int32_t __sort_b(const void* elem1, const void* elem2)
{
	const micron_color_t* ELEM1 = (micron_color_t*)elem1;
	const micron_color_t* ELEM2 = (micron_color_t*)elem2;

	if (ELEM1->b < ELEM2->b)
		return -1;

	if (ELEM1->b > ELEM2->b)
		return 1;

	return 0;
}

static void __sort(std::vector<micron_color_t>& out_bucket)
{
	int32_t low_r = INT_MAX;
	int32_t low_g = INT_MAX;
	int32_t low_b = INT_MAX;
	int32_t high_r = INT_MIN;
	int32_t high_g = INT_MIN;
	int32_t high_b = INT_MIN;
	for (const micron_color_t& PIXEL : out_bucket)
	{
		low_r = __min(low_r, PIXEL.r);
		low_g = __min(low_g, PIXEL.g);
		low_b = __min(low_b, PIXEL.b);
		high_r = __max(high_r, PIXEL.r);
		high_g = __max(high_g, PIXEL.g);
		high_b = __max(high_b, PIXEL.b);
	}
	const int32_t RED = high_r - low_r;
	const int32_t GREEN = high_g - low_g;
	const int32_t BLUE = high_b - low_b;

	//3) sort by greatest range
	const int32_t GREATEST = __max(RED, __max(GREEN, BLUE));
	if (GREATEST == RED)
		::qsort(out_bucket.data(), out_bucket.size(), sizeof(micron_color_t), __sort_r);
	else if (GREATEST == GREEN)
		::qsort(out_bucket.data(), out_bucket.size(), sizeof(micron_color_t), __sort_g);
	else if (GREATEST == BLUE)
		::qsort(out_bucket.data(), out_bucket.size(), sizeof(micron_color_t), __sort_b);
	else
		assert(0);
}

static void __split(
	const std::vector<micron_color_t>& in_bucket,
	std::vector<std::vector<micron_color_t>>& out_new_buckets)
{
	{
		std::vector<micron_color_t> a;
		for (uint32_t i = 0; i < in_bucket.size() / 2; ++i)
			a.push_back(in_bucket[i]);
		out_new_buckets.push_back(a);
	}

	{
		std::vector<micron_color_t> b;
		for (uint32_t i = in_bucket.size() / 2; i < in_bucket.size(); ++i)
			b.push_back(in_bucket[i]);
		out_new_buckets.push_back(b);
	}
}

static uint32_t __nearest_palette_entry(
	const micron_color_t* in_palette, const uint32_t in_palette_size, const micron_color_t& in_source_image_pixel,
	std::map<micron_color_t, uint32_t>& out_cache)
{
	assert(in_palette_size <= 256);

	//search in cache
	auto find = out_cache.find(in_source_image_pixel);
	if (out_cache.cend() != find)
		return find->second;

	//search for the nearest color (euclidean)
	float nd = FLT_MAX;
	uint32_t npe = UINT32_MAX;
	for (uint32_t pe = 0; pe < in_palette_size; ++pe)
	{
		const float R = (float)in_palette[pe].r - (float)in_source_image_pixel.r;
		const float G = (float)in_palette[pe].g - (float)in_source_image_pixel.g;
		const float B = (float)in_palette[pe].b - (float)in_source_image_pixel.b;
		const float D = ::sqrtf(R * R + G * G + B * B);
		if (D < nd)
		{
			nd = D;
			npe = pe;
		}
	}
	assert(nd < FLT_MAX);
	assert(npe < in_palette_size);

	out_cache.insert({ in_source_image_pixel, npe });

	return npe;
}

//public
//public
//public
//public

void paletas_t::item(
	const char* in_file,
	micron_bitmap_t& out_bitmap)
{
	_items.push_back({ in_file, &out_bitmap });
}

bool paletas_t::calculate(
	const uint32_t in_palette_size,
	micron_color_t* out_palette_256)
{
	assert(in_palette_size <= 256);

	//load all source images (24 bit)
	std::vector<fs_blob_t> source_images;
	for (const paletas_t::item_t& ITEM : _items)
	{
		fs_blob_t source_image = fs_tga_read_24(ITEM.file);
		if (!source_image.data)
			return false;
		source_images.push_back(source_image);
	}

	//clear the palette (max size)
	::memset(out_palette_256, 0, sizeof(micron_color_t) * 256);

	//calculate the final palette
	{
		//get all unique pixels
		uint32_t total_source_pixels = 0;
		std::vector<std::vector<micron_color_t>> all_buckets;
		{
			std::set<micron_color_t> unique_pixels_set;
			for (const fs_blob_t& SOURCE_IMAGE : source_images)
			{
				for (
					micron_color_t* pixel = (micron_color_t*)FS_TGA_PIXELS(SOURCE_IMAGE);
					pixel < (micron_color_t*)FS_TGA_PIXELS(SOURCE_IMAGE) + FS_TGA_HEADER(SOURCE_IMAGE)->image_spec_width * FS_TGA_HEADER(SOURCE_IMAGE)->image_spec_height;
					++pixel
					)
				{
					++total_source_pixels;
					unique_pixels_set.insert(*pixel);
				}
			}
			std::vector<micron_color_t> first_bucket;
			for (const micron_color_t& UP : unique_pixels_set)
				first_bucket.push_back(UP);
			assert(unique_pixels_set.size() == first_bucket.size());
			all_buckets.push_back(first_bucket);
		}

		//figure out which channel has the greatest range, sort, and split until we have as many buckets as we want palette indices
		while (all_buckets.size() < in_palette_size)
		{
			std::vector<std::vector<micron_color_t>> new_buckets;
			for (std::vector<micron_color_t>& bucket : all_buckets)
			{
				__sort(bucket);
				__split(bucket, new_buckets);
			}
			all_buckets.clear();
			for (const std::vector<micron_color_t>& NEW_BUCKET : new_buckets)
				all_buckets.push_back(NEW_BUCKET);
		}
		assert(all_buckets.size() == in_palette_size);

		//average buckets to a single color (these colors will be final palette colors)
		for (
			uint32_t bucket_index = 0;
			bucket_index < all_buckets.size();
			++bucket_index
			)
		{
			const std::vector<micron_color_t>& BUCKET = all_buckets[bucket_index];
			float r = 0;
			float g = 0;
			float b = 0;
			for (const micron_color_t& PIXEL : BUCKET)
			{
				r += PIXEL.r;
				g += PIXEL.g;
				b += PIXEL.b;
			}
			r /= (float)BUCKET.size();
			g /= (float)BUCKET.size();
			b /= (float)BUCKET.size();
			assert(r >= 0 && r < 256);
			assert(g >= 0 && g < 256);
			assert(b >= 0 && b < 256);
			out_palette_256[bucket_index].r = (uint8_t)r;
			out_palette_256[bucket_index].g = (uint8_t)g;
			out_palette_256[bucket_index].b = (uint8_t)b;
		}
	}

	//create all the palettize bitmaps by remapping all pixels to the final palette of averaged colors (use the nearest color)
	{
		std::map<micron_color_t, uint32_t> remap_cache;
		for (
			uint32_t i = 0;
			i < _items.size();
			++i
			)
		{
			paletas_t::item_t& item = _items[i];

			assert(!item.bitmap->width);
			item.bitmap->width = FS_TGA_HEADER(source_images[i])->image_spec_width;

			assert(!item.bitmap->height);
			item.bitmap->height = FS_TGA_HEADER(source_images[i])->image_spec_height;

			assert(!item.bitmap->pixels);
			item.bitmap->pixels = new uint8_t[item.bitmap->width * item.bitmap->height];
			assert(item.bitmap->pixels);

			for (int32_t y = 0; y < FS_TGA_HEADER(source_images[i])->image_spec_height; ++y)
			{
				for (int32_t x = 0; x < FS_TGA_HEADER(source_images[i])->image_spec_width; ++x)
				{
					const micron_color_t* SOURCE_IMAGE_I_PIXEL = (micron_color_t*)FS_TGA_PIXELS(source_images[i]) + x + y * FS_TGA_HEADER(source_images[i])->image_spec_width;
					const uint32_t NEAREST_PALETTE_ENTRY = __nearest_palette_entry(out_palette_256, in_palette_size, *SOURCE_IMAGE_I_PIXEL, remap_cache);
					assert(NEAREST_PALETTE_ENTRY < in_palette_size);
					item.bitmap->pixels[x + (item.bitmap->height - 1 - y) * item.bitmap->width] = (uint8_t)NEAREST_PALETTE_ENTRY;
				}
			}
		}
	}

	//cleanup loaded source images
	for (fs_blob_t& source_image : source_images)
		delete[] source_image.data;

	return true;
}
