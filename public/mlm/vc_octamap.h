#pragma once

struct c_blob_t;

struct micron_t;

//#define NINJA_VC_OCTAFONT_HEIGHT 14
//#define NINJA_VC_OCTAFONT_SPACE_WIDTH 7
//#define NINJA_VC_OCTAFONT_CHAR_SPACING -1
//#define NINJA_VC_OCTAFONT_CHARBEGIN 33
//#define NINJA_VC_OCTAFONT_CHAREND 128
//#define NINJA_VC_OCTAFONT_NUMCHARS 95
//static_assert(NINJA_VC_OCTAFONT_NUMCHARS == NINJA_VC_OCTAFONT_CHAREND - NINJA_VC_OCTAFONT_CHARBEGIN, "wtf?");

#define MLM_VC_OCTAMAP_COLOR_KEY_INDEX 15

namespace mlm
{
	/*
	struct vc_octafont_character_t
	{
		int32_t s;
		int32_t t;
		int32_t w;
	};
	*/

	//assumes src
	void vc_octamap_blit_key(
		const int32_t dst_x, const int32_t dst_y,
		int32_t copy_width, int32_t copy_height,
		const int32_t src_x, const int32_t src_y,
		const c_blob_t& src, micron_t& out_micron);

	void vc_octamap_blit_key_clip(
		int32_t dst_x, int32_t dst_y,
		int32_t copy_width, int32_t copy_height,
		int32_t src_x, int32_t src_y,
		const c_blob_t& src, micron_t& out_micron);

	void vc_octamap_blit_key_clip_flip_x(
		int32_t aDstX, int32_t aDstY,
		int32_t aCopyWidth, int32_t aCopyHeight,
		int32_t aSrcX, int32_t aSrcY,
		const c_blob_t& src, micron_t& out_micron);

	void vc_octamap_blit(
		const int32_t aDstX, const int32_t aDstY,
		int32_t copy_width, int32_t copy_height,
		const int32_t aSrcX, const int32_t aSrcY,
		const c_blob_t& src, micron_t& out_micron);

	void vc_octamap_blit_clip(
		int32_t dst_x, int32_t dst_y,
		int32_t copy_width, int32_t copy_height,
		int32_t src_x, int32_t src_y,
		const c_blob_t& src, micron_t& out_micron);

	void vc_octamap_blit_stretch(
		const int32_t aDstX, const int32_t aDstY,
		const int32_t aDstWidth, const int32_t aDstHeight,
		const c_blob_t& src, micron_t& out_micron);

	//vc_canvas_t vc_canvas_make(const int32_t width, const int32_t height);
	void vc_canvas_clear(const uint8_t color, int32_t aDstX, int32_t aDstY, int32_t aClearWidth, int32_t aClearHeight, micron_t& out_micron);
	void vc_canvas_fill_circle(const int32_t x, const int32_t y, const int32_t aRadius, const uint8_t color, micron_t& out_micron);
	void vc_canvas_line(const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2, const uint8_t color, micron_t& out_micron);

#if 0
	constexpr vc_octafont_character_t VC_FONT_TABLES[NINJA_VC_OCTAFONT_NUMCHARS] =
	{
		{ 0, 0, 7 },
		{ 10, 0, 8 },
		{ 22, 0, 7 },
		{ 34, 0, 7 },
		{ 44, 0, 12 },
		{ 57, 0, 12 },
		{ 73, 0, 5 },
		{ 83, 0, 7 },
		{ 97, 0, 7 },
		{ 107, 0, 7 },
		{ 118, 0, 9 },
		{ 133, 0, 5 },
		{ 142, 0, 6 },
		{ 158, 0, 4 },
		{ 167, 0, 8 },
		{ 0, 14, 10 },
		{ 12, 14, 8 },
		{ 24, 14, 10 },
		{ 36, 14, 10 },
		{ 48, 14, 10 },
		{ 60, 14, 10 },
		{ 72, 14, 10 },
		{ 84, 14, 10 },
		{ 96, 14, 9 },
		{ 108, 14, 10 },
		{ 124, 14, 5 },
		{ 135, 14, 6 },
		{ 147, 14, 7 },
		{ 159, 14, 8 },
		{ 169, 14, 7 },
		{ 182, 14, 9 },
		{ 0, 28, 11 },
		{ 12, 28, 13 },
		{ 26, 28, 12 },
		{ 39, 28, 10 },
		{ 51, 28, 12 },
		{ 64, 28, 11 },
		{ 76, 28, 10 },
		{ 88, 28, 11 },
		{ 100, 28, 13 },
		{ 114, 28, 7 },
		{ 126, 28, 10 },
		{ 138, 28, 12 },
		{ 151, 28, 11 },
		{ 163, 28, 13 },
		{ 177, 28, 12 },
		{ 190, 28, 11 },
		{ 0, 42, 12 },
		{ 13, 42, 11 },
		{ 25, 42, 13 },
		{ 39, 42, 11 },
		{ 51, 42, 12 },
		{ 64, 42, 12 },
		{ 77, 42, 13 },
		{ 91, 42, 13 },
		{ 105, 42, 13 },
		{ 119, 42, 13 },
		{ 133, 42, 11 },
		{ 146, 42, 9 },
		{ 159, 42, 6 },
		{ 171, 42, 8 },
		{ 183, 42, 7 },
		{ 195, 42, 7 },
		{ 0, 56, 8 },
		{ 12, 56, 9 },
		{ 24, 56, 9 },
		{ 36, 56, 8 },
		{ 48, 56, 9 },
		{ 60, 56, 8 },
		{ 72, 56, 7 },
		{ 84, 56, 8 },
		{ 96, 56, 9 },
		{ 110, 56, 6 },
		{ 121, 56, 8 },
		{ 132, 56, 10 },
		{ 144, 56, 6 },
		{ 156, 56, 13 },
		{ 170, 56, 9 },
		{ 182, 56, 8 },
		{ 0, 70, 9 },
		{ 12, 70, 9 },
		{ 24, 70, 8 },
		{ 36, 70, 8 },
		{ 48, 70, 7 },
		{ 60, 70, 10 },
		{ 72, 70, 9 },
		{ 84, 70, 13 },
		{ 98, 70, 10 },
		{ 110, 70, 9 },
		{ 122, 70, 8 },
		{ 137, 70, 7 },
		{ 151, 70, 3 },
		{ 161, 70, 7 },
		{ 172, 70, 9 },
		{ -858993460, -858993460, -858993460 },
	};
#endif
}
