#include "stdafx.h"
#include "tga.h"

namespace fs
{
	static uint32_t __total_file_size(::FILE* handle)
	{
		//seek to end
		if (0 != ::fseek(handle, 0, SEEK_END))
			return 0;

		//get filesize
		const long fp = ::ftell(handle);
		if (-1 == fp)
			return 0;

		//store size
		const uint32_t result = (const uint32_t)fp;

		//seek back to beginning
		if (0 != ::fseek(handle, 0, SEEK_SET))
			return 0;

		return result;
	}

	namespace tga
	{
		static bool __read(const char* aFile, image_t& anImage)
		{
			delete[] anImage.memory;
			anImage = {};

			/*
			read_file_t handle;
			if (!open_binary(aFile, handle))
				return false;
				*/
			::FILE* handle{};
			if (0 != ::fopen_s(&handle, aFile, "rb"))
				return false;

			/*
			if (handle.impl->total_file_size() < sizeof(header_t))
				return false;
				*/
			//anImage.memory_size = handle.impl->total_file_size();
			anImage.memory_size = __total_file_size(handle);
			if (anImage.memory_size < sizeof(header_t))
				return false;

			anImage.memory = new uint8_t[anImage.memory_size];
			if (!anImage.memory)
				return false;
			anImage.header = (header_t*)anImage.memory;
			anImage.pixels = anImage.memory + sizeof(header_t);

			//if (!read(anImage.memory, handle.impl->total_file_size(), handle))
			if (1 != ::fread(anImage.memory, anImage.memory_size, 1, handle))
			{
				delete[] anImage.memory;
				return false;
			}

			return true;
		}

		static bool __rle_decode(image_t& anImage)
		{
			//only support compressed RGB images here!
			assert(10 == anImage.header->image_type);

			//copy now manages the original memory (compressed)
			const image_t COPY = anImage;

			assert(COPY.memory && COPY.header && COPY.pixels);
			const uint32_t BYTES_PER_PIXEL = COPY.header->image_spec_bpp / 8;
			const uint32_t DECODED_IMAGE_SIZE = BYTES_PER_PIXEL * COPY.header->image_spec_width * COPY.header->image_spec_height;

			//get new memory for the uncompressed version
			anImage.memory_size = sizeof(header_t) + DECODED_IMAGE_SIZE;
			anImage.memory = new uint8_t[anImage.memory_size];
			if (!anImage.memory)
			{
				delete[] COPY.memory;
				return false;
			}
			anImage.header = (header_t*)anImage.memory;
			anImage.pixels = anImage.memory + sizeof(header_t);

			//copy back the original header
			*anImage.header = *COPY.header;

			//we are uncompressing, so we are effectively changing the type of the image
			anImage.header->image_type = 2;

			//construct the decompressed image
			const uint8_t* SRC = COPY.pixels;
			uint32_t byte;
			uint8_t packetHeader;
			uint8_t isRLEPacket;
			uint8_t packetLength;
			uint8_t i;
			uint8_t* dst = anImage.pixels;

			while (dst < anImage.pixels + DECODED_IMAGE_SIZE)
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
		/*
		uint32_t image_type(const char* aFile)
		{
			header_t header;
			if (read_header(aFile, header))
				return header.image_type;

			assert(0);
			return 0xffffffff;
		}

		extern core::vec2i_t image_size(const char* aFile)
		{
			header_t header;
			if (read_header(aFile, header))
				return{ header.image_spec_width, header.image_spec_height };

			assert(0);
			return{};
		}

		bool read_header(const char* aFile, header_t& aHeader)
		{
			bool result = false;
			read_file_t handle;
			if (open_binary(aFile, handle))
			{
				result = FS_TEMPLATE_READ(aHeader, handle);
				close(handle);
			}
			return result;
		}
		*/

		bool create_24(const uint16_t aWidth, const uint16_t aHeight, image_t& anImage)
		{
			delete[] anImage.memory;
			anImage.memory_size = sizeof(header_t) + aWidth * aHeight * 3;
			anImage.memory = new uint8_t[anImage.memory_size];
			if (!anImage.memory)
				return false;

			anImage.header = (header_t*)anImage.memory;
			*anImage.header = {};
			anImage.header->image_type = 2;
			anImage.header->image_spec_width = aWidth;
			anImage.header->image_spec_height = aHeight;
			anImage.header->image_spec_bpp = 24;

			anImage.pixels = anImage.memory + sizeof(header_t);

			return true;
		}

		bool create_32(const uint16_t aWidth, const uint16_t aHeight, image_t& anImage)
		{
			delete[] anImage.memory;
			anImage.memory_size = sizeof(header_t) + aWidth * aHeight * 4;
			anImage.memory = new uint8_t[anImage.memory_size];
			if (!anImage.memory)
				return false;

			anImage.header = (header_t*)anImage.memory;
			*anImage.header = {};
			anImage.header->image_type = 2;
			anImage.header->image_spec_width = aWidth;
			anImage.header->image_spec_height = aHeight;
			anImage.header->image_spec_bpp = 32;

			anImage.pixels = anImage.memory + sizeof(header_t);

			return true;
		}

		bool read_8(const char* aFile, image_t& anImage)
		{
			if (!__read(aFile, anImage))
				return false;

			if ((1 != anImage.header->image_type && 3 != anImage.header->image_type) || 8 != anImage.header->image_spec_bpp)
			{
				delete[] anImage.memory;
				anImage = {};
			}

			return nullptr != anImage.memory;
		}

		bool read_24(const char* aFile, image_t& anImage)
		{
			if (!__read(aFile, anImage))
				return false;

			if ((2 != anImage.header->image_type && 10 != anImage.header->image_type) || 24 != anImage.header->image_spec_bpp)
			{
				delete[] anImage.memory;
				anImage = {};
				return false;
			}

			if (10 == anImage.header->image_type && !__rle_decode(anImage))
			{
				delete[] anImage.memory;
				anImage = {};
			}

			return nullptr != anImage.memory;
		}

		bool read_32(const char* aFile, image_t& anImage)
		{
			if (!__read(aFile, anImage))
				return false;

			if ((2 != anImage.header->image_type && 10 != anImage.header->image_type) || 32 != anImage.header->image_spec_bpp)
			{
				delete[] anImage.memory;
				anImage = {};
				return false;
			}

			if (10 == anImage.header->image_type && !__rle_decode(anImage))
			{
				delete[] anImage.memory;
				anImage = {};
			}

			return nullptr != anImage.memory;
		}

		void flip_vertical(image_t& anImage)
		{
			const uint32_t BYTES_PER_PIXEL = anImage.header->image_spec_bpp / 8;
			uint8_t* a;
			uint8_t* b;
			uint8_t tmp;

			for (int32_t y = 0; y < anImage.header->image_spec_height / 2; ++y)
			{
				for (int32_t x = 0; x < anImage.header->image_spec_width; ++x)
				{
					a = anImage.pixels + BYTES_PER_PIXEL * (x + y * anImage.header->image_spec_width);
					b = anImage.pixels + BYTES_PER_PIXEL * (x + (anImage.header->image_spec_height - 1 - y) * anImage.header->image_spec_width);

					for (uint32_t byte = 0; byte < BYTES_PER_PIXEL; ++byte)
					{
						tmp = a[byte];
						a[byte] = b[byte];
						b[byte] = tmp;
					}
				}
			}
		}

		/*
		bool write_24(const char* aFile, const image_t& anImage)
		{
			assert(2 == anImage.header->image_type && 24 == anImage.header->image_spec_bpp);

			fs::write_file_t handle;
			if (open_binary(aFile, handle))
			{
				write(anImage.memory, sizeof(header_t) + anImage.header->image_spec_width * anImage.header->image_spec_height * (anImage.header->image_spec_bpp / 8), handle);
				close(handle);
				return true;
			}
			return false;
		}

		bool write_32(const char* aFile, const image_t& anImage)
		{
			assert(2 == anImage.header->image_type && 32 == anImage.header->image_spec_bpp);

			fs::write_file_t handle;
			if (open_binary(aFile, handle))
			{
				write(anImage.memory, sizeof(header_t) + anImage.header->image_spec_width * anImage.header->image_spec_height * (anImage.header->image_spec_bpp / 8), handle);
				close(handle);
				return true;
			}
			return false;
		}
		*/
	}

	blob_t file_contents(const char* file)
	{
		::FILE* handle{};
		if (0 != ::fopen_s(&handle, file, "rb"))
			return {};

		blob_t result{};
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
}