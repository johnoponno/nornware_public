#include "stdafx.h"
#include "fs.h"

static uint32_t __total_file_size(::FILE* out_handle)
{
	//seek to end
	if (0 != ::fseek(out_handle, 0, SEEK_END))
		return 0;

	//get filesize
	const long fp = ::ftell(out_handle);
	if (-1 == fp)
		return 0;

	//store size
	const uint32_t result = (const uint32_t)fp;

	//seek back to beginning
	if (0 != ::fseek(out_handle, 0, SEEK_SET))
		return 0;

	return result;
}

static bool __rle_decode(c_blob_t& out)
{
	//only support compressed RGB images here!
	assert(10 == FS_TGA_HEADER(out)->image_type);

	//COPY now manages the original memory (compressed)
	const c_blob_t COPY = out;

	assert(COPY.data && COPY.size);
	const uint32_t BYTES_PER_PIXEL = FS_TGA_HEADER(COPY)->image_spec_bpp / 8;
	assert(BYTES_PER_PIXEL < 256);
	const uint32_t DECODED_IMAGE_SIZE = BYTES_PER_PIXEL * FS_TGA_HEADER(COPY)->image_spec_width * FS_TGA_HEADER(COPY)->image_spec_height;

	//get new memory for the uncompressed version
	out.size = sizeof(fs_tga_header_t) + DECODED_IMAGE_SIZE;
	out.data = new uint8_t[out.size];
	if (!out.data)
	{
		delete[] COPY.data;
		return false;
	}

	//copy back the original header
	*FS_TGA_HEADER(out) = *FS_TGA_HEADER(COPY);

	//we are uncompressing, so we are effectively changing the type of the image
	FS_TGA_HEADER(out)->image_type = 2;

	//construct the decompressed image
	const uint8_t* SRC = FS_TGA_PIXELS(COPY);
	uint8_t* dst = FS_TGA_PIXELS(out);

	while (dst < FS_TGA_PIXELS(out) + DECODED_IMAGE_SIZE)
	{
		const uint8_t packet_header = *SRC;
		const uint8_t is_rle_packet = 0 != (packet_header & 128);
		const uint8_t packet_length = (packet_header & 127) + 1;

		++SRC;

		//rle
		if (is_rle_packet)
		{
			for (uint8_t i = 0; i < packet_length; ++i)
			{
				for (uint32_t byte = 0; byte < BYTES_PER_PIXEL; ++byte)
					dst[byte] = SRC[byte];
				dst += BYTES_PER_PIXEL;
			}
			SRC += BYTES_PER_PIXEL;
		}
		//raw
		else
		{
			for (uint8_t i = 0; i < packet_length; ++i)
			{
				for (uint32_t byte = 0; byte < BYTES_PER_PIXEL; ++byte)
					dst[byte] = SRC[byte];
				dst += BYTES_PER_PIXEL;
				SRC += BYTES_PER_PIXEL;
			}
		}
	}

	//cleanup original compressed memory
	delete[] COPY.data;
	return true;
}

//public
//public
//public
//public

c_blob_t fs_file_contents(const char* in_file)
{
	::FILE* handle{};
	if (0 != ::fopen_s(&handle, in_file, "rb"))
		return {};

	c_blob_t result{};
	result.size = __total_file_size(handle);
	if (!result.size)
		return{};

	result.data = new uint8_t[result.size];
	if (!result.data)
		return{};

	if (1 != ::fread(result.data, result.size, 1, handle))
	{
		delete[] result.data;
		return{};
	}

	::fclose(handle);

	return result;
}

c_blob_t fs_file_contents_null_terminated(const char* in_file)
{
	::FILE* handle{};
	if (0 != ::fopen_s(&handle, in_file, "rb"))
		return {};

	c_blob_t result{};
	result.size = __total_file_size(handle);
	if (!result.size)
		return{};

	result.data = new uint8_t[result.size + 1];
	if (!result.data)
		return{};

	if (1 != ::fread(result.data, result.size, 1, handle))
	{
		delete[] result.data;
		return{};
	}

	{
		uint8_t* data = (uint8_t*)result.data;
		data[result.size] = 0;
	}

	::fclose(handle);

	return result;
}


c_blob_t fs_tga_read_24(const char* in_file)
{
	c_blob_t result = fs_file_contents(in_file);
	if (!result.data || !result.size)
		return {};

	if (
		(2 != FS_TGA_HEADER(result)->image_type && 10 != FS_TGA_HEADER(result)->image_type) ||
		24 != FS_TGA_HEADER(result)->image_spec_bpp
		)
	{
		delete[] result.data;
		return {};
	}

	if (
		10 == FS_TGA_HEADER(result)->image_type &&
		!__rle_decode(result)
		)
	{
		delete[] result.data;
		return {};
	}

	return result;
}

uint8_t* fs_tga_pixels_palettized(const c_blob_t& in_image)
{
	const fs_tga_header_t* HEADER = FS_TGA_HEADER(in_image);
	assert(1 == HEADER->image_type);
	assert(0 == HEADER->color_map_entry_size % 8);
	return (uint8_t*)in_image.data + sizeof(fs_tga_header_t) + HEADER->color_map_length * HEADER->color_map_entry_size / 8;
}

const uint8_t* fs_tga_src(const fs_tga_header_t* in_header, const uint8_t* in_pixels, const int32_t in_src_x, const int32_t in_src_y)
{
	if (in_header->image_spec_descriptor & (1 << 5))
		return in_pixels + in_src_x + in_src_y * in_header->image_spec_width;
	else
		return in_pixels + in_src_x + (in_header->image_spec_height - 1 - in_src_y) * in_header->image_spec_width;
}

const uint8_t* fs_tga_row_advance(const fs_tga_header_t* in_header, const uint8_t* in_scan_src)
{
	if (in_header->image_spec_descriptor & (1 << 5))
		return in_scan_src + in_header->image_spec_width;
	else
		return in_scan_src - in_header->image_spec_width;
}
