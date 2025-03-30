#pragma once

struct c_blob_t
{
	void* data;
	uint32_t size;
};

c_blob_t fs_file_contents(const char* in_file);
c_blob_t fs_file_contents_null_terminated(const char* in_file);

#pragma pack(push, 1)
struct fs_tga_header_t
{
	uint8_t id_length;
	uint8_t color_map_type;
	uint8_t image_type;
	uint16_t color_map_origin;
	uint16_t color_map_length;
	uint8_t color_map_entry_size;
	uint16_t image_spec_origin_x;
	uint16_t image_spec_origin_y;
	uint16_t image_spec_width;
	uint16_t image_spec_height;
	uint8_t image_spec_bpp;
	uint8_t image_spec_descriptor;
};
#pragma pack(pop)

c_blob_t fs_tga_read_24(const char* in_file);	//supports RLE
uint8_t* fs_tga_pixels_palettized(const c_blob_t& in_image);
const uint8_t* fs_tga_src(const fs_tga_header_t* in_header, const uint8_t* in_pixels, const int32_t in_src_x, const int32_t in_src_y);
const uint8_t* fs_tga_row_advance(const fs_tga_header_t* in_header, const uint8_t* in_scan_src);

#define FS_TGA_HEADER(image) ((fs_tga_header_t*)(image).data)
#define FS_TGA_PIXELS(image) ((uint8_t*)(image).data + sizeof(fs_tga_header_t))
//#define FS_TGA_PIXELS_PALETTIZED(image) ((uint8_t*)(image).data + sizeof(fs_tga_header_t) + 768)
#define FS_TGA_PALETTE(image) ((uint8_t*)(image).data + sizeof(fs_tga_header_t))
