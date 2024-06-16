#include "stdafx.h"
#include "minyin.h"

#include "sd_bitmap.h"
#include "fs.h"

static bool __allocate(
	const uint32_t in_size,
	minyin_bitmap_t& out_bitmap)
{
	assert(!out_bitmap.pixels);
	out_bitmap.pixels = new uint8_t[in_size];
	if (!out_bitmap.pixels)
		return false;

	return true;
}

void __deallocate(
	const uint32_t in_size,
	minyin_bitmap_t& out_bitmap)
{
	if (in_size)
	{
		assert(out_bitmap.width && out_bitmap.height && out_bitmap.pixels && (uint32_t)(out_bitmap.width * out_bitmap.height) == in_size);
		delete[] out_bitmap.pixels;
		out_bitmap.pixels = nullptr;
	}
	else
	{
		assert(!out_bitmap.width && !out_bitmap.height && !out_bitmap.pixels);
	}
}

static void __min_max(int32_t& aMin, int32_t& aMax)
{
	if (aMin > aMax)
	{
		const int32_t temp = aMin;
		aMin = aMax;
		aMax = temp;
	}
}

/*
static uint8_t __palettize(
	const uint16_t in_color,
	minyin_bitmap_t& out_bitmap)
{
	assert(out_bitmap.palette_used <= 0xff);
	for (uint32_t i = 0; i < out_bitmap.palette_used; ++i)
	{
		if (in_color == out_bitmap.palette[i])
			return (uint8_t)i;
	}
	if (out_bitmap.palette_used < 0xff)
	{
		out_bitmap.palette[out_bitmap.palette_used] = in_color;
		++out_bitmap.palette_used;
		assert(out_bitmap.palette_used);
		return uint8_t(out_bitmap.palette_used - 1);
	}
	else
	{
		//overflow
		return 0;
	}
}
*/

static bool __string_has_extension(const char* in_extension, const char* in_string)
{
	const char* LAST_DOT = ::strrchr(in_string, '.');
	return nullptr != LAST_DOT && 0 == ::strcmp(LAST_DOT + 1, in_extension);
}

/*
static bool __flip_vertical(
	const int32_t aWidth, const int32_t aHeight, const uint8_t* somePixels,
	minyin_bitmap_t& bm)
{
	if (!aWidth || !aHeight || !somePixels)
		return false;

	//init bitmap
	if (!minyin_bitmap_init(bm, aWidth, aHeight))
		return false;

	//convert to correct format
	const uint8_t* SRC = somePixels + (aHeight - 1) * aWidth;
	uint8_t* dst = bm.pixels;

	for (int32_t y = 0; y < aHeight; ++y)
	{
		for (int32_t x = 0; x < aWidth; ++x)
		{
			*dst = *SRC;
			++dst;
			++SRC;
		}
		SRC -= aWidth * 2;
	}

	return true;
}
*/

//from project ninja - for loading 8 bit .tga files "properly" (with palette support)
static struct octamap_t
{
	fs_tga_header_t* header;
	uint8_t* palette;
	uint8_t* pixels;
} __octamap_make(const fs_blob_t& blob)
{
	octamap_t result{};

	result.header = (fs_tga_header_t*)blob.data;
	assert(0 == result.header->id_length);
	assert(1 == result.header->color_map_type);
	assert(1 == result.header->image_type);
	assert(0 == result.header->color_map_origin);
	assert(256 == result.header->color_map_length);
	assert(24 == result.header->color_map_entry_size);
	assert(768 == (uint32_t)result.header->color_map_length * ((uint32_t)result.header->color_map_entry_size / 8));

	result.palette = (uint8_t*)blob.data + sizeof(fs_tga_header_t);
	assert(result.palette + 768 < (uint8_t*)blob.data + blob.size);

	result.pixels = result.palette + 768;
	assert(result.pixels + result.header->image_spec_width * result.header->image_spec_height < (uint8_t*)blob.data + blob.size);

	return result;
}

static bool __clip(

	//these are legacy, but here in case you would want to clip to some other rect than the whole bitmap
	const int32_t in_cliprect_x,
	const int32_t in_cliprect_y,
	const int32_t in_cliprect_z,
	const int32_t in_cliprect_w,
	//these are legacy, but here in case you would want to clip to some other rect than the whole bitmap

	int32_t& out_src_x, int32_t& out_src_y, int32_t& out_dst_x, int32_t& out_dst_y, int32_t& out_width, int32_t& out_height)
{
	//REJECTION TEST
	if ((out_dst_x + out_width) < in_cliprect_x ||
		out_dst_x >= in_cliprect_z ||
		(out_dst_y + out_height) < in_cliprect_y ||
		out_dst_y >= in_cliprect_w)
	{
		return false;
	}

	int32_t clip;

	//CLIP
	//x
	if (out_dst_x < in_cliprect_x)
	{
		clip = out_dst_x - in_cliprect_x;
		out_src_x -= clip;
		out_width += clip;
		out_dst_x = in_cliprect_x;
	}

	//return if no width
	if (out_width <= 0)
		return false;

	//z
	if ((out_dst_x + out_width) >= in_cliprect_z)
		out_width = in_cliprect_z - out_dst_x;

	//return if no width
	if (out_width <= 0)
		return false;

	//y
	if (out_dst_y < in_cliprect_y)
	{
		clip = out_dst_y - in_cliprect_y;
		out_src_y -= clip;
		out_height += clip;
		out_dst_y = in_cliprect_y;
	}

	//return if no height
	if (out_height <= 0)
		return false;

	//w
	if ((out_dst_y + out_height) >= in_cliprect_w)
		out_height = in_cliprect_w - out_dst_y;

	return out_height > 0;
}

//public
//public
//public
//public

bool minyin_t::key_is_down(const int32_t in_key) const
{
	if (in_key >= 0 && in_key < _countof(_keys))
		return _keys[in_key].down_current;
	return false;
}

bool minyin_t::key_downflank(const int32_t in_key) const
{
	if (in_key >= 0 && in_key < _countof(_keys))
		return _keys[in_key].down_current && !_keys[in_key].down_last;
	return false;
}

minyin_bitmap_t::minyin_bitmap_t()
{
	pixels = nullptr;
	width = 0;
	height = 0;
}

minyin_bitmap_t::~minyin_bitmap_t()
{
	minyin_bitmap_relinquish(*this);
}

bool minyin_bitmap_init(
	minyin_bitmap_t& out_bitmap,
	const int32_t in_width, const int32_t in_height)
{
	//cleanup
	minyin_bitmap_relinquish(out_bitmap);

	//get new memory
	if (!__allocate(in_width * in_height, out_bitmap))
		return false;

	//store new settings
	out_bitmap.width = in_width;
	out_bitmap.height = in_height;

	return true;
}

bool minyin_bitmap_load_8(
	minyin_bitmap_t& out_bm,
	const char* in_filename)
{
	minyin_bitmap_relinquish(out_bm);

	if (!in_filename || 0 == *in_filename)
		return false;

	if (!__string_has_extension("tga", in_filename))
		return false;

	const fs_blob_t BLOB = fs_file_contents(in_filename);
	if (!BLOB.data || !BLOB.size)
		return false;
	const octamap_t OCTAMAP = __octamap_make(BLOB);

	//FIXME: remove?
	{
		struct pe_t
		{
			uint8_t r;
			uint8_t g;
			uint8_t b;
		};
		for (
			uint32_t i = 0;
			i < 256;
			++i
			)
		{
			const pe_t* PE = (pe_t*)OCTAMAP.palette + i;
			minyin_palette[i] = sd_color_encode(PE->b, PE->g, PE->r);
		}
	}

	bool result = false;
	if (minyin_bitmap_init(out_bm, OCTAMAP.header->image_spec_width, OCTAMAP.header->image_spec_height))
	{
		const uint8_t* SRC = OCTAMAP.pixels + (OCTAMAP.header->image_spec_height - 1) * OCTAMAP.header->image_spec_width;
		uint8_t* dst = out_bm.pixels;

		for (int32_t y = 0; y < OCTAMAP.header->image_spec_height; ++y)
		{
			for (int32_t x = 0; x < OCTAMAP.header->image_spec_width; ++x)
			{
				*dst = *SRC;
				++dst;
				++SRC;
			}
			SRC -= OCTAMAP.header->image_spec_width * 2;
		}

		result = true;
	}
	delete[] BLOB.data;
	return result;
}

void minyin_bitmap_relinquish(minyin_bitmap_t& out_bitmap)
{
	__deallocate(out_bitmap.width * out_bitmap.height, out_bitmap);
	out_bitmap.width = 0;
	out_bitmap.height = 0;
}

void minyin_cross(
	minyin_bitmap_t& out_bitmap,
	const int32_t in_x, const int32_t in_y, const int32_t in_size, const uint8_t in_color)
{
	minyin_h_line(out_bitmap, in_x - in_size / 2, in_x + in_size / 2 + 1, in_y, in_color);
	minyin_v_line(out_bitmap, in_x, in_y - in_size / 2, in_y + in_size / 2 + 1, in_color);
}

void minyin_h_line(
	minyin_bitmap_t& out_bitmap,
	int32_t in_x1, int32_t in_x2, const int32_t in_y1, const uint8_t in_color)
{
	//clip
	if (in_y1 < 0 || in_y1 >= out_bitmap.height)
		return;

	__min_max(in_x1, in_x2);
	in_x1 = __max(0, in_x1);
	in_x2 = __min(out_bitmap.width, in_x2);
	uint8_t* dst = out_bitmap.pixels + in_x1 + in_y1 * out_bitmap.width;

	for (
		int32_t x = in_x1;
		x < in_x2;
		++x
		)
	{
		assert(dst >= out_bitmap.pixels && dst < out_bitmap.pixels + out_bitmap.width * out_bitmap.height);
		*dst++ = in_color;
	}
}

void minyin_v_line(
	minyin_bitmap_t& out_bitmap,
	const int32_t in_x1, int32_t in_y1, int32_t in_y2, const uint8_t in_color)
{
	//clip
	if (in_x1 < 0 || in_x1 >= out_bitmap.width)
		return;

	__min_max(in_y1, in_y2);
	in_y1 = __max(0, in_y1);
	in_y2 = __min(out_bitmap.height, in_y2);
	uint8_t* dst = out_bitmap.pixels + in_x1 + in_y1 * out_bitmap.width;

	for (
		int32_t y = in_y1;
		y < in_y2;
		++y
		)
	{
		assert(dst >= out_bitmap.pixels && dst < out_bitmap.pixels + out_bitmap.width * out_bitmap.height);
		*dst = in_color;
		dst += out_bitmap.width;
	}
}

void minyin_blit(
	minyin_bitmap_t& out_bitmap,
	const minyin_bitmap_t& in_src, int32_t in_dst_x, int32_t in_dst_y, int32_t in_copy_width, int32_t in_copy_height, int32_t in_src_x, int32_t in_src_y)
{
	if (0 == in_copy_width || in_copy_width > in_src.width)
		in_copy_width = in_src.width;
	if (0 == in_copy_height || in_copy_height > in_src.height)
		in_copy_height = in_src.height;

	const uint8_t* BYTE_SRC = in_src.pixels + in_src_x + in_src_y * in_src.width;
	uint8_t* byte_dst = out_bitmap.pixels + in_dst_x + in_dst_y * out_bitmap.width;

	for (
		int32_t y = 0;
		y < in_copy_height;
		++y
		)
	{
		const uint8_t* BYTE_SCAN_SRC = BYTE_SRC;
		uint8_t* byte_scan_dst = byte_dst;

		for (
			int32_t x = 0;
			x < in_copy_width;
			++x
			)
		{
			assert(BYTE_SRC >= in_src.pixels && BYTE_SRC < (in_src.pixels + in_src.width * in_src.height));
			assert(byte_dst >= out_bitmap.pixels && byte_dst < (out_bitmap.pixels + out_bitmap.width * out_bitmap.height));
			*byte_dst = *BYTE_SRC;
			++byte_dst;
			++BYTE_SRC;
		}

		BYTE_SRC = BYTE_SCAN_SRC + in_src.width;
		byte_dst = byte_scan_dst + out_bitmap.width;
	}
}

void minyin_blit_key(
	minyin_bitmap_t& out_bitmap,
	const minyin_bitmap_t& in_src, int32_t in_dst_x, int32_t in_dst_y, int32_t in_copy_width, int32_t in_copy_height, int32_t in_src_x, int32_t in_src_y)
{
	minyin_blit(out_bitmap, in_src, in_dst_x, in_dst_y, in_copy_width, in_copy_height, in_src_x, in_src_y);
}

void minyin_blit_key_clip(
	minyin_bitmap_t& out_bitmap,
	const minyin_bitmap_t& in_src, int32_t in_dst_x, int32_t in_dst_y, int32_t in_copy_width, int32_t in_copy_height, int32_t in_src_x, int32_t in_src_y)
{
	if (0 == in_copy_width || in_copy_width > in_src.width)
		in_copy_width = in_src.width;
	if (0 == in_copy_height || in_copy_height > in_src.height)
		in_copy_height = in_src.height;

	if (__clip(

		//these are legacy, but here in case you would want to clip to some other rect than the whole bitmap
		0, 0, out_bitmap.width, out_bitmap.height,
		//these are legacy, but here in case you would want to clip to some other rect than the whole bitmap

		in_src_x, in_src_y, in_dst_x, in_dst_y, in_copy_width, in_copy_height))
	{
		minyin_blit(out_bitmap, in_src, in_dst_x, in_dst_y, in_copy_width, in_copy_height, in_src_x, in_src_y);
	}
}

void minyin_print(
	minyin_bitmap_t& out_bitmap,
	const sd_fontv_t& in_font, const int32_t in_x, const int32_t in_y, const char* in_text)
{
	out_bitmap;
	in_font;
	in_x;
	in_y;
	in_text;
}

void minyin_print_color_not_black(
	minyin_bitmap_t& out_bitmap,
	const sd_fontv_t& in_font, const uint16_t in_color, const int32_t in_dst_x, const int32_t in_dst_y, const char* in_message)
{
	out_bitmap;
	in_font;
	in_color;
	in_dst_x;
	in_dst_y;
	in_message;
}

uint16_t minyin_palette[256];
