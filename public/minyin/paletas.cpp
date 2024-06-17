#include "stdafx.h"
#include "paletas.h"

#include "minyin.h"
#include "fs.h"
#include "sd_bitmap.h"

static int32_t __sort_r(const void* elem1, const void* elem2)
{
	const pixel_t* ELEM1 = (pixel_t*)elem1;
	const pixel_t* ELEM2 = (pixel_t*)elem2;

	if (ELEM1->r < ELEM2->r)
		return -1;

	if (ELEM1->r > ELEM2->r)
		return 1;

	return 0;
}

static int32_t __sort_g(const void* elem1, const void* elem2)
{
	const pixel_t* ELEM1 = (pixel_t*)elem1;
	const pixel_t* ELEM2 = (pixel_t*)elem2;

	if (ELEM1->g < ELEM2->g)
		return -1;

	if (ELEM1->g > ELEM2->g)
		return 1;

	return 0;
}

static int32_t __sort_b(const void* elem1, const void* elem2)
{
	const pixel_t* ELEM1 = (pixel_t*)elem1;
	const pixel_t* ELEM2 = (pixel_t*)elem2;

	if (ELEM1->b < ELEM2->b)
		return -1;

	if (ELEM1->b > ELEM2->b)
		return 1;

	return 0;
}

//public
//public
//public
//public

void paletas_item(
	const char* in_file,
	minyin_bitmap_t& out_bitmap, paletas_t& out_paletas)
{
	out_paletas.items.push_back({ in_file, &out_bitmap });
}

bool paletas_process(
	const uint32_t in_palette_size,
	paletas_t& out_paletas)
{
	assert(in_palette_size <= 256);

	//load all source images (24 bit)
	std::vector<fs_tga_image_t*> source_images;
	for (const paletas_t::item_t& ITEM : out_paletas.items)
	{
		fs_tga_image_t* source_image = new fs_tga_image_t;
		if (!source_image)
			return false;
		*source_image = {};
		if (!fs_tga_read_24(ITEM.file, *source_image))
		{
			delete source_image;
			return false;
		}
		source_images.push_back(source_image);
	}

	pixel_t palette[256]{};
	{
		//1) get all unique pixels
		uint32_t total_source_pixels = 0;
		std::vector<pixel_t> unique_pixels_vector;
		{
			std::set<pixel_t> unique_pixels_set;
			for (const fs_tga_image_t* SOURCE_IMAGE : source_images)
			{
				for (
					pixel_t* pixel = (pixel_t*)SOURCE_IMAGE->pixels;
					pixel < (pixel_t*)SOURCE_IMAGE->pixels + SOURCE_IMAGE->header->image_spec_width * SOURCE_IMAGE->header->image_spec_height;
					++pixel
					)
				{
					++total_source_pixels;
					unique_pixels_set.insert(*pixel);
				}
			}
			for (const pixel_t& UP : unique_pixels_set)
				unique_pixels_vector.push_back(UP);
			assert(unique_pixels_set.size() == unique_pixels_vector.size());
		}

		//2) figure out which channel has the greatest range
		{
			int32_t low_r = INT_MAX;
			int32_t low_g = INT_MAX;
			int32_t low_b = INT_MAX;
			int32_t high_r = INT_MIN;
			int32_t high_g = INT_MIN;
			int32_t high_b = INT_MIN;
			for (const pixel_t& PIXEL : unique_pixels_vector)
			{
				low_r = __min(low_r, PIXEL.r);
				low_g = __min(low_g, PIXEL.g);
				low_b = __min(low_b, PIXEL.b);
				high_r = __max(high_r, PIXEL.r);
				high_g = __max(high_g, PIXEL.g);
				high_b = __max(high_b, PIXEL.b);
			}
			const int32_t RANGE_R = high_r - low_r;
			const int32_t RANGE_G = high_g - low_g;
			const int32_t RANGE_B = high_b - low_b;

			//3) sort by greatest range
			if (RANGE_R > RANGE_G && RANGE_R > RANGE_B)
			{
				::qsort(unique_pixels_vector.data(), unique_pixels_vector.size(), sizeof(pixel_t), __sort_r);
			}
			else if (RANGE_G > RANGE_R && RANGE_G > RANGE_B)
			{
				::qsort(unique_pixels_vector.data(), unique_pixels_vector.size(), sizeof(pixel_t), __sort_g);
			}
			else if (RANGE_B > RANGE_R && RANGE_B > RANGE_G)
			{
				::qsort(unique_pixels_vector.data(), unique_pixels_vector.size(), sizeof(pixel_t), __sort_b);
			}
			else
			{
				assert(RANGE_R == RANGE_G && RANGE_R == RANGE_B);
				::qsort(unique_pixels_vector.data(), unique_pixels_vector.size(), sizeof(pixel_t), __sort_r);
			}
		}

		//4) split into buckets (desired number of colors) and average buckets to a single color (these colors will be final palette colors)
		const float FLOAT_PIXELS_IN_BUCKET = (float)unique_pixels_vector.size() / (float)in_palette_size;
		const uint32_t PIXELS_IN_BUCKET = uint32_t(FLOAT_PIXELS_IN_BUCKET + .5f);//round
		for (uint32_t bucket = 0; bucket < in_palette_size; ++bucket)
		{
			float r = 0;
			float g = 0;
			float b = 0;
			for (uint32_t p = 0; p < PIXELS_IN_BUCKET; ++p)
			{
				const uint32_t PIXEL_INDEX = p + bucket * PIXELS_IN_BUCKET;
				if (PIXEL_INDEX < unique_pixels_vector.size())
				{
					r += unique_pixels_vector[PIXEL_INDEX].r;
					g += unique_pixels_vector[PIXEL_INDEX].g;
					b += unique_pixels_vector[PIXEL_INDEX].b;
				}
			}
			r /= (float)PIXELS_IN_BUCKET;
			g /= (float)PIXELS_IN_BUCKET;
			b /= (float)PIXELS_IN_BUCKET;
			assert(r >= 0 && r < 256);
			assert(g >= 0 && g < 256);
			assert(b >= 0 && b < 256);
			palette[bucket].r = (uint8_t)r;
			palette[bucket].g = (uint8_t)g;
			palette[bucket].b = (uint8_t)b;
		}
	}

	//5) create all the palettize bitmaps by remapping all pixels to the final palette of averaged colors (use the nearest color)
	for (uint32_t i = 0; i < out_paletas.items.size(); ++i)
	{
		paletas_t::item_t& item = out_paletas.items[i];

		assert(!item.bitmap->width);
		item.bitmap->width = source_images[i]->header->image_spec_width;

		assert(!item.bitmap->height);
		item.bitmap->height = source_images[i]->header->image_spec_height;

		assert(!item.bitmap->pixels);
		item.bitmap->pixels = new uint8_t[item.bitmap->width * item.bitmap->height];
		assert(item.bitmap->pixels);

		const pixel_t* SOURCE_IMAGE_I_PIXELS = (pixel_t*)source_images[i]->pixels;
		for (int32_t y = 0; y < source_images[i]->header->image_spec_height; ++y)
		{
			for (int32_t x = 0; x < source_images[i]->header->image_spec_width; ++x)
			{
				float nd = FLT_MAX;
				uint32_t npe = UINT32_MAX;
				for (uint32_t pe = 0; pe < in_palette_size; ++pe)
				{
					const float R = (float)palette[pe].r - (float)SOURCE_IMAGE_I_PIXELS[x + y * source_images[i]->header->image_spec_width].r;
					const float G = (float)palette[pe].g - (float)SOURCE_IMAGE_I_PIXELS[x + y * source_images[i]->header->image_spec_width].g;
					const float B = (float)palette[pe].b - (float)SOURCE_IMAGE_I_PIXELS[x + y * source_images[i]->header->image_spec_width].b;
					const float D = ::sqrtf(R * R + G * G + B * B);
					if (D < nd)
					{
						nd = D;
						npe = pe;
					}
				}
				assert(nd < FLT_MAX);
				assert(npe < in_palette_size);
				item.bitmap->pixels[x + (item.bitmap->height - 1 - y) * item.bitmap->width] = (uint8_t)npe;
			}
		}
	}

	//output final palette (sd format)
	for (uint32_t i = 0; i < in_palette_size; ++i)
		minyin_palette[i] = sd_color_encode(palette[i].b, palette[i].g, palette[i].r);

	//cleanup loaded source images
	for (fs_tga_image_t* source_image : source_images)
		delete source_image;

	return true;
}
