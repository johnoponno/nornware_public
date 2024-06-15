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

static bool __read(
	const char* in_file,
	fs_tga_image_t& out_image)
{
	delete[] out_image.memory;
	out_image = {};

	::FILE* handle{};
	if (0 != ::fopen_s(&handle, in_file, "rb"))
		return false;

	out_image.memory_size = __total_file_size(handle);
	if (out_image.memory_size < sizeof(fs_tga_header_t))
		return false;

	out_image.memory = new uint8_t[out_image.memory_size];
	if (!out_image.memory)
		return false;
	out_image.header = (fs_tga_header_t*)out_image.memory;
	out_image.pixels = out_image.memory + sizeof(fs_tga_header_t);

	if (1 != ::fread(out_image.memory, out_image.memory_size, 1, handle))
	{
		delete[] out_image.memory;
		return false;
	}

	return true;
}

static bool __rle_decode(fs_tga_image_t& out_image)
{
	//only support compressed RGB images here!
	assert(10 == out_image.header->image_type);

	//copy now manages the original memory (compressed)
	const fs_tga_image_t COPY = out_image;

	assert(COPY.memory && COPY.header && COPY.pixels);
	const uint32_t BYTES_PER_PIXEL = COPY.header->image_spec_bpp / 8;
	const uint32_t DECODED_IMAGE_SIZE = BYTES_PER_PIXEL * COPY.header->image_spec_width * COPY.header->image_spec_height;

	//get new memory for the uncompressed version
	out_image.memory_size = sizeof(fs_tga_header_t) + DECODED_IMAGE_SIZE;
	out_image.memory = new uint8_t[out_image.memory_size];
	if (!out_image.memory)
	{
		delete[] COPY.memory;
		return false;
	}
	out_image.header = (fs_tga_header_t*)out_image.memory;
	out_image.pixels = out_image.memory + sizeof(fs_tga_header_t);

	//copy back the original header
	*out_image.header = *COPY.header;

	//we are uncompressing, so we are effectively changing the type of the image
	out_image.header->image_type = 2;

	//construct the decompressed image
	const uint8_t* SRC = COPY.pixels;
	uint32_t byte;
	uint8_t packetHeader;
	uint8_t isRLEPacket;
	uint8_t packetLength;
	uint8_t i;
	uint8_t* dst = out_image.pixels;

	while (dst < out_image.pixels + DECODED_IMAGE_SIZE)
	{
		packetHeader = *SRC;
		isRLEPacket = 0 != (packetHeader & 128);
		packetLength = (packetHeader & 127) + 1;

		++SRC;

		//rle
		if (isRLEPacket)
		{
			for (i = 0; i < packetLength; ++i)
			{
				for (byte = 0; byte < BYTES_PER_PIXEL; ++byte)
					dst[byte] = SRC[byte];
				dst += BYTES_PER_PIXEL;
			}
			SRC += BYTES_PER_PIXEL;
		}
		//raw
		else
		{
			for (i = 0; i < packetLength; ++i)
			{
				for (byte = 0; byte < BYTES_PER_PIXEL; ++byte)
					dst[byte] = SRC[byte];
				dst += BYTES_PER_PIXEL;
				SRC += BYTES_PER_PIXEL;
			}
		}
	}

	//cleanup original compressed memory
	delete[] COPY.memory;
	return true;
}

//public
//public
//public
//public

bool fs_tga_create_24(
	const uint16_t in_width, const uint16_t in_height, 
	fs_tga_image_t& out_image)
{
	delete[] out_image.memory;
	out_image.memory_size = sizeof(fs_tga_header_t) + in_width * in_height * 3;
	out_image.memory = new uint8_t[out_image.memory_size];
	if (!out_image.memory)
		return false;

	out_image.header = (fs_tga_header_t*)out_image.memory;
	*out_image.header = {};
	out_image.header->image_type = 2;
	out_image.header->image_spec_width = in_width;
	out_image.header->image_spec_height = in_height;
	out_image.header->image_spec_bpp = 24;

	out_image.pixels = out_image.memory + sizeof(fs_tga_header_t);

	return true;
}

bool fs_tga_create_32(
	const uint16_t in_width, const uint16_t in_height, 
	fs_tga_image_t& out_image)
{
	delete[] out_image.memory;
	out_image.memory_size = sizeof(fs_tga_header_t) + in_width * in_height * 4;
	out_image.memory = new uint8_t[out_image.memory_size];
	if (!out_image.memory)
		return false;

	out_image.header = (fs_tga_header_t*)out_image.memory;
	*out_image.header = {};
	out_image.header->image_type = 2;
	out_image.header->image_spec_width = in_width;
	out_image.header->image_spec_height = in_height;
	out_image.header->image_spec_bpp = 32;

	out_image.pixels = out_image.memory + sizeof(fs_tga_header_t);

	return true;
}

bool fs_tga_read_8(
	const char* in_file, 
	fs_tga_image_t& out_image)
{
	if (!__read(in_file, out_image))
		return false;

	if ((1 != out_image.header->image_type && 3 != out_image.header->image_type) || 8 != out_image.header->image_spec_bpp)
	{
		delete[] out_image.memory;
		out_image = {};
	}

	return nullptr != out_image.memory;
}

bool fs_tga_read_24(
	const char* in_file, 
	fs_tga_image_t& out_image)
{
	if (!__read(in_file, out_image))
		return false;

	if ((2 != out_image.header->image_type && 10 != out_image.header->image_type) || 24 != out_image.header->image_spec_bpp)
	{
		delete[] out_image.memory;
		out_image = {};
		return false;
	}

	if (10 == out_image.header->image_type && !__rle_decode(out_image))
	{
		delete[] out_image.memory;
		out_image = {};
	}

	return nullptr != out_image.memory;
}

bool fs_tga_read_32(const char* aFile, fs_tga_image_t& out_image)
{
	if (!__read(aFile, out_image))
		return false;

	if ((2 != out_image.header->image_type && 10 != out_image.header->image_type) || 32 != out_image.header->image_spec_bpp)
	{
		delete[] out_image.memory;
		out_image = {};
		return false;
	}

	if (10 == out_image.header->image_type && !__rle_decode(out_image))
	{
		delete[] out_image.memory;
		out_image = {};
	}

	return nullptr != out_image.memory;
}

void fs_tga_flip_vertical(fs_tga_image_t& out_image)
{
	const uint32_t BYTES_PER_PIXEL = out_image.header->image_spec_bpp / 8;
	uint8_t* a;
	uint8_t* b;
	uint8_t tmp;

	for (int32_t y = 0; y < out_image.header->image_spec_height / 2; ++y)
	{
		for (int32_t x = 0; x < out_image.header->image_spec_width; ++x)
		{
			a = out_image.pixels + BYTES_PER_PIXEL * (x + y * out_image.header->image_spec_width);
			b = out_image.pixels + BYTES_PER_PIXEL * (x + (out_image.header->image_spec_height - 1 - y) * out_image.header->image_spec_width);

			for (uint32_t byte = 0; byte < BYTES_PER_PIXEL; ++byte)
			{
				tmp = a[byte];
				a[byte] = b[byte];
				b[byte] = tmp;
			}
		}
	}
}

fs_blob_t fs_file_contents(const char* in_file)
{
	::FILE* handle{};
	if (0 != ::fopen_s(&handle, in_file, "rb"))
		return {};

	fs_blob_t result{};
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

	return result;
}
