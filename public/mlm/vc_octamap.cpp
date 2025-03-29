#include "stdafx.h"
#include "vc_octamap.h"

#include "../microlib/fs.h"
#include "../micron.h"
#include "vc_fatpack.h"

namespace mlm
{
	//these need to be as big as the biggest destination sizes for stretched blits
	static struct
	{
		int32_t x;
		int32_t y;
	} __stretch_table[64]{};

	static bool __clip(const int32_t in_cliprect_x, const int32_t in_cliprect_y, const int32_t in_cliprect_z, const int32_t in_cliprect_w, const int32_t in_edge, const int32_t in_x, const int32_t in_y)
	{
		return
			in_x >= (in_cliprect_x + in_edge) &&
			in_y >= (in_cliprect_y + in_edge) &&
			in_x < (in_cliprect_z - in_edge) &&
			in_y < (in_cliprect_w - in_edge);
	}

	//generic clip, returns false if totally rejected, otherwise modifies references
	static bool __clip(
		const int32_t in_cliprect_x, const int32_t in_cliprect_y, const int32_t in_cliprect_z, const int32_t in_cliprect_w,
		int32_t& out_src_x, int32_t& out_src_y, int32_t& out_dst_x, int32_t& out_dst_y, int32_t& out_width, int32_t& out_height)
	{
		//REJECTION TEST
		if (
			(out_dst_x + out_width) < in_cliprect_x ||
			out_dst_x >= in_cliprect_z ||
			(out_dst_y + out_height) < in_cliprect_y ||
			out_dst_y >= in_cliprect_w
			)
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
		{
			out_width = in_cliprect_z - out_dst_x;
		}

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
		{
			out_height = in_cliprect_w - out_dst_y;
		}

		return out_height > 0;
	}

	static bool __clip(const int32_t in_cliprect_x, const int32_t in_cliprect_y, const int32_t in_cliprect_z, const int32_t in_cliprect_w, const int32_t in_x, const int32_t in_y)
	{
		return
			in_x >= in_cliprect_x &&
			in_y >= in_cliprect_y &&
			in_x < in_cliprect_z &&
			in_y < in_cliprect_w;
	}

	static void __h_mirror_modify_src_x(
		const int32_t in_original_copy_width, const int32_t in_current_copy_width, const int32_t in_original_src_x,
		int32_t& out_src_x)
	{
		//right clip / no clip
		if (in_original_src_x == out_src_x)
		{
			out_src_x += in_original_copy_width - 1;
		}
		//left clip
		else
		{
			const int32_t DIFF = in_original_copy_width - in_current_copy_width;
			out_src_x += in_original_copy_width - DIFF - DIFF - 1;
		}
	}

	void vc_octamap_blit_key(
		const int32_t in_dst_x, const int32_t in_dst_y,
		int32_t in_copy_width, int32_t in_copy_height,
		const int32_t in_src_x, const int32_t in_src_y,
		const c_blob_t& in_src,
		micron_t& out_micron)
	{
		//check defaults
		if (
			0 == in_copy_width ||
			in_copy_width > FS_TGA_HEADER(in_src)->image_spec_width
			)
			in_copy_width = FS_TGA_HEADER(in_src)->image_spec_width;
		if (
			0 == in_copy_height ||
			in_copy_height > FS_TGA_HEADER(in_src)->image_spec_height
			)
			in_copy_height = FS_TGA_HEADER(in_src)->image_spec_height;

		//we are keying and flipping in y...
		const uint8_t* BYTE_SRC = FS_TGA_PIXELS_PALETTIZED(in_src) + in_src_x + (FS_TGA_HEADER(in_src)->image_spec_height - 1 - in_src_y) * FS_TGA_HEADER(in_src)->image_spec_width;
		uint8_t* byte_dst = out_micron.canvas + in_dst_x + in_dst_y * out_micron.canvas_width;
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
				if (*BYTE_SRC != MLM_VC_OCTAMAP_COLOR_KEY_INDEX)
#if OCTAMAP_DEPTH_COMPLEXITY
					if (byte_dst[0] < 255)
						++byte_dst[0];
#else
					* byte_dst = *BYTE_SRC;
#endif
				++BYTE_SRC;
				++byte_dst;
			}

			BYTE_SRC = BYTE_SCAN_SRC - FS_TGA_HEADER(in_src)->image_spec_width;
			byte_dst = byte_scan_dst + out_micron.canvas_width;
		}
	}

	void vc_octamap_blit_key_clip(
		int32_t in_dst_x, int32_t in_dst_y,
		int32_t in_copy_width, int32_t in_copy_height,
		int32_t in_src_x, int32_t in_src_y,
		const c_blob_t& in_src,
		micron_t& out_micron)
	{
		//check defaults
		if (
			0 == in_copy_width ||
			in_copy_width > FS_TGA_HEADER(in_src)->image_spec_width
			)
			in_copy_width = FS_TGA_HEADER(in_src)->image_spec_width;
		if (
			0 == in_copy_height ||
			in_copy_height > FS_TGA_HEADER(in_src)->image_spec_height
			)
			in_copy_height = FS_TGA_HEADER(in_src)->image_spec_height;

		//clip
		if (__clip(0, 0, out_micron.canvas_width, out_micron.canvas_height, in_src_x, in_src_y, in_dst_x, in_dst_y, in_copy_width, in_copy_height))
		{
			//call normal blit
			vc_octamap_blit_key(in_dst_x, in_dst_y, in_copy_width, in_copy_height, in_src_x, in_src_y, in_src, out_micron);
		}
	}

	void vc_octamap_blit_key_clip_flip_x(
		int32_t in_dst_x, int32_t in_dst_y,
		int32_t in_copy_width, int32_t in_copy_height,
		int32_t in_src_x, int32_t in_src_y,
		const c_blob_t& in_src,
		micron_t& out_micron)
	{
		//check defaults
		if (
			0 == in_copy_width ||
			in_copy_width > FS_TGA_HEADER(in_src)->image_spec_width
			)
			in_copy_width = FS_TGA_HEADER(in_src)->image_spec_width;
		if (
			0 == in_copy_height ||
			in_copy_height > FS_TGA_HEADER(in_src)->image_spec_height
			)
			in_copy_height = FS_TGA_HEADER(in_src)->image_spec_height;

		assert(in_src_x >= 0 && in_src_x < FS_TGA_HEADER(in_src)->image_spec_width);
		assert(in_src_y >= 0 && in_src_y < FS_TGA_HEADER(in_src)->image_spec_height);

		//clip
		const int32_t ORIGINAL_SRCX = in_src_x;
		const int32_t ORIGINAL_COPYWIDTH = in_copy_width;
		if (__clip(0, 0, out_micron.canvas_width, out_micron.canvas_height, in_src_x, in_src_y, in_dst_x, in_dst_y, in_copy_width, in_copy_height))
		{
			assert(in_src_x >= 0 && in_src_x < FS_TGA_HEADER(in_src)->image_spec_width);
			assert(in_src_y >= 0 && in_src_y < FS_TGA_HEADER(in_src)->image_spec_height);

			//we are flipping in x...
			__h_mirror_modify_src_x(ORIGINAL_COPYWIDTH, in_copy_width, ORIGINAL_SRCX, in_src_x);

			//we are keying and flipping in y...
			const uint8_t* BYTE_SRC = FS_TGA_PIXELS_PALETTIZED(in_src) + in_src_x + (FS_TGA_HEADER(in_src)->image_spec_height - 1 - in_src_y) * FS_TGA_HEADER(in_src)->image_spec_width;
			uint8_t* byte_dst = out_micron.canvas + in_dst_x + in_dst_y * out_micron.canvas_width;

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
					assert(BYTE_SRC >= FS_TGA_PIXELS_PALETTIZED(in_src) && BYTE_SRC < (FS_TGA_PIXELS_PALETTIZED(in_src) + FS_TGA_HEADER(in_src)->image_spec_width + FS_TGA_HEADER(in_src)->image_spec_height * FS_TGA_HEADER(in_src)->image_spec_width));
					if (*BYTE_SRC != MLM_VC_OCTAMAP_COLOR_KEY_INDEX)
#if OCTAMAP_DEPTH_COMPLEXITY
						if (byte_dst[0] < 255)
							++byte_dst[0];
#else
						* byte_dst = *BYTE_SRC;
#endif
					--BYTE_SRC;
					++byte_dst;
				}

				BYTE_SRC = BYTE_SCAN_SRC - FS_TGA_HEADER(in_src)->image_spec_width;
				byte_dst = byte_scan_dst + out_micron.canvas_width;
			}
		}
	}

	void vc_octamap_blit(
		const int32_t in_dst_x, const int32_t in_dst_y,
		int32_t in_copy_width, int32_t in_copy_height,
		const int32_t in_src_x, const int32_t in_src_y,
		const c_blob_t& in_src,
		micron_t& out_micron)
	{
		//check defaults
		if (
			0 == in_copy_width ||
			in_copy_width > FS_TGA_HEADER(in_src)->image_spec_width
			)
			in_copy_width = FS_TGA_HEADER(in_src)->image_spec_width;
		if (
			0 == in_copy_height ||
			in_copy_height > FS_TGA_HEADER(in_src)->image_spec_height
			)
			in_copy_height = FS_TGA_HEADER(in_src)->image_spec_height;

		//we are flipping in y...
		const uint8_t* BYTE_SRC = FS_TGA_PIXELS_PALETTIZED(in_src) + in_src_x + (FS_TGA_HEADER(in_src)->image_spec_height - 1 - in_src_y) * FS_TGA_HEADER(in_src)->image_spec_width;
		uint8_t* byte_dst = out_micron.canvas + in_dst_x + in_dst_y * out_micron.canvas_width;

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
				assert(BYTE_SRC >= FS_TGA_PIXELS_PALETTIZED(in_src) && BYTE_SRC < (FS_TGA_PIXELS_PALETTIZED(in_src) + FS_TGA_HEADER(in_src)->image_spec_width * FS_TGA_HEADER(in_src)->image_spec_height));
				assert(byte_dst >= out_micron.canvas && byte_dst < (out_micron.canvas + out_micron.canvas_width * out_micron.canvas_height));
#if OCTAMAP_DEPTH_COMPLEXITY
				if (byte_dst[0] < 255)
					++byte_dst[0];
#else
				* byte_dst = *BYTE_SRC;
#endif
				++byte_dst;
				++BYTE_SRC;
			}

			BYTE_SRC = BYTE_SCAN_SRC - FS_TGA_HEADER(in_src)->image_spec_width;
			byte_dst = byte_scan_dst + out_micron.canvas_width;
		}
	}

	void vc_octamap_blit_clip(
		int32_t in_dst_x, int32_t in_dst_y,
		int32_t in_copy_width, int32_t in_copy_height,
		int32_t in_src_x, int32_t in_src_y,
		const c_blob_t& in_src,
		micron_t& out_micron)
	{
		//check defaults
		if (
			0 == in_copy_width ||
			in_copy_width > FS_TGA_HEADER(in_src)->image_spec_width
			)
			in_copy_width = FS_TGA_HEADER(in_src)->image_spec_width;
		if (
			0 == in_copy_height ||
			in_copy_height > FS_TGA_HEADER(in_src)->image_spec_height
			)
			in_copy_height = FS_TGA_HEADER(in_src)->image_spec_height;

		//clip
		if (__clip(0, 0, out_micron.canvas_width, out_micron.canvas_height, in_src_x, in_src_y, in_dst_x, in_dst_y, in_copy_width, in_copy_height))
		{
			//call normal blit
			vc_octamap_blit(in_dst_x, in_dst_y, in_copy_width, in_copy_height, in_src_x, in_src_y, in_src, out_micron);
		}
	}

	void vc_octamap_blit_stretch(
		const int32_t in_dst_x, const int32_t in_dst_y,
		const int32_t in_dst_width, const int32_t in_dst_height,
		const c_blob_t& in_src,
		micron_t& out_micron)
	{
		if (
			!in_dst_width ||
			!in_dst_height
			)
			return;

		//fixed point multipliers
		const int32_t U_MUL = (FS_TGA_HEADER(in_src)->image_spec_width << 16) / in_dst_width;
		const int32_t V_MUL = (FS_TGA_HEADER(in_src)->image_spec_height << 16) / in_dst_height;

		//build texture tables
		assert(in_dst_width < _countof(__stretch_table));
		{
			int32_t u = 0;
			for (
				int32_t x = 0;
				x < in_dst_width;
				++x
				)
			{
				__stretch_table[x].x = u >> 16;
				u += U_MUL;
			}
		}

		assert(in_dst_height < _countof(__stretch_table));
		{
			int32_t v = 0;
			for (
				int32_t y = 0;
				y < in_dst_height;
				++y
				)
			{
				__stretch_table[y].y = (v >> 16) * FS_TGA_HEADER(in_src)->image_spec_width;
				v += V_MUL;
			}
		}

		//destination setup
		uint8_t* dst = out_micron.canvas + in_dst_x + in_dst_y * out_micron.canvas_width;

		//loop
		for (
			int32_t y = 0;
			y < in_dst_height;
			++y
			)
		{
			uint8_t* scan_dst = dst;

			for (
				int32_t x = 0;
				x < in_dst_width;
				++x
				)
			{
				//texture map
				const int32_t SRC_PIXEL = __stretch_table[x].x + __stretch_table[y].y;
				assert(SRC_PIXEL >= 0 && SRC_PIXEL < FS_TGA_HEADER(in_src)->image_spec_width * FS_TGA_HEADER(in_src)->image_spec_height);

				assert(dst >= out_micron.canvas && dst < out_micron.canvas + out_micron.canvas_width * out_micron.canvas_height);

#if OCTAMAP_DEPTH_COMPLEXITY
				if (dst[0] < 255)
					++dst[0];
#else
				* dst++ = FS_TGA_PIXELS_PALETTIZED(in_src)[SRC_PIXEL];
#endif
			}

			dst = scan_dst + out_micron.canvas_width;
		}
	}

	void vc_canvas_fill_circle(
		const int32_t in_x, const int32_t in_y, const int32_t in_radius, const uint8_t in_color,
		micron_t& out_micron)
	{
#if OCTAMAP_DEPTH_COMPLEXITY
		color;
#endif

		//check bounds (totally clipped)
		if (!__clip(0, 0, out_micron.canvas_width, out_micron.canvas_height, in_radius, in_x, in_y))
			return;

		//build table
		assert(in_radius <= _countof(__stretch_table));	//make sure fits inside table
		for (
			int32_t r = 0;
			r < in_radius;
			++r
			)
			__stretch_table[r].x = (int32_t)(::sqrtf((float)(in_radius * in_radius - r * r)) + .5f);

		//setup pointer
		uint8_t* tptr = out_micron.canvas + (in_x - in_radius) + (in_y - in_radius) * out_micron.canvas_width;
		uint8_t* bptr = out_micron.canvas + (in_x - in_radius) + in_y * out_micron.canvas_width;

		//fill
		int32_t start;
		int32_t end;
		for (
			int32_t y = 0;
			y < in_radius;
			++y
			)
		{
			start = in_radius - __stretch_table[in_radius - y - 1].x;
			end = __stretch_table[in_radius - y - 1].x + in_radius;
			for (
				int32_t x = start;
				x < end;
				++x
				)
			{
#if OCTAMAP_DEPTH_COMPLEXITY
				if (tptr[x] < 255)
					++tptr[x];
#else
				tptr[x] = in_color;
#endif
			}

			start = in_radius - __stretch_table[y].x;
			end = __stretch_table[y].x + in_radius;
			for (
				int32_t x = start;
				x < end;
				++x
				)
			{
#if OCTAMAP_DEPTH_COMPLEXITY
				if (bptr[x] < 255)
					++bptr[x];
#else
				bptr[x] = in_color;
#endif
			}

			tptr += out_micron.canvas_width;
			bptr += out_micron.canvas_width;
		}
	}

	/*
	vc_canvas_t vc_canvas_make(const int32_t in_width, const int32_t in_height)
	{
		vc_canvas_t result;

		result.pixels = new uint8_t[in_width * in_height];
		result.width = in_width;
		result.height = in_height;

		return result;
	}
	*/

	void vc_canvas_clear(
		const uint8_t in_color, int32_t in_dst_x, int32_t in_dst_y, int32_t in_clear_width, int32_t in_clear_height,
		micron_t& out_micron)
	{
#if OCTAMAP_DEPTH_COMPLEXITY
		color;
#endif

		//check defaults
		if (
			!in_clear_width ||
			in_clear_width > out_micron.canvas_width
			)
			in_clear_width = out_micron.canvas_width;
		if (
			!in_clear_height ||
			in_clear_height > out_micron.canvas_height
			)
			in_clear_height = out_micron.canvas_height;

		//clip
		int32_t src_x = 0;
		int32_t src_y = 0;
		if (__clip(0, 0, out_micron.canvas_width, out_micron.canvas_height, src_x, src_y, in_dst_x, in_dst_y, in_clear_width, in_clear_height))
		{
			uint8_t* dst = out_micron.canvas + in_dst_x + in_dst_y * out_micron.canvas_width;
			for (
				int32_t y = 0;
				y < in_clear_height;
				++y
				)
			{
				uint8_t* scan_dst = dst;

				for (
					int32_t x = 0;
					x < in_clear_width;
					++x
					)
				{
					assert(dst >= out_micron.canvas && dst < (out_micron.canvas + out_micron.canvas_width * out_micron.canvas_height));
#if OCTAMAP_DEPTH_COMPLEXITY
					if (dst[0] < 255)
						++dst[0];
#else
					* dst++ = in_color;
#endif
				}

				dst = scan_dst + out_micron.canvas_width;
			}
		}
	}

	/*
	void vc_canvas_to_bitmap(const uint16_t* sd_palette, const vc_canvas_t& canvas, sd_bitmap_t& output)
	{
		assert(canvas.width == output.width);
		assert(canvas.height == output.height);

		const uint8_t* SRC = canvas.pixels;
		uint16_t* dst = output.pixels;
		for (int32_t i = 0; i < canvas.width * canvas.height; ++i)
			*dst++ = sd_palette[*SRC++];
	}
	*/

	void vc_canvas_line(
		const int32_t in_x1, const int32_t in_y1, const int32_t in_x2, const int32_t in_y2, const uint8_t in_color,
		micron_t& out_micron)
	{
		//check if same point
		if (
			in_x1 == in_x2 &&
			in_y1 == in_y2
			)
		{
			if (__clip(0, 0, out_micron.canvas_width, out_micron.canvas_height, in_x1, in_y1))
				out_micron.canvas[in_x1 + in_y1 * out_micron.canvas_width] = in_color;
			return;
		}

		//line moves in the negative y direction
		int32_t y_diff = in_y2 - in_y1;			//difference between y coordinates
		int32_t y_unit;						//amount of change in x and y
		int32_t y_dir;
		if (y_diff < 0)
		{
			//get absolute value of difference
			y_diff = -y_diff;
			//set negative unit in y dimension
			y_unit = -out_micron.canvas_width;
			y_dir = -1;
		}
		//else positive unit in y dimension
		else
		{
			y_unit = out_micron.canvas_width;
			y_dir = 1;
		}

		//line moves in the negative x direction
		int32_t x_diff = in_x2 - in_x1;			//difference between x coordinates
		int32_t x_unit;
		int32_t x_dir;
		if (x_diff < 0)
		{
			//get absolute value of difference
			x_diff = -x_diff;
			//set negative unit in x dimension
			x_unit = -1;
			x_dir = -1;
		}
		//else set positive unit in x dimension
		else
		{
			x_unit = 1;
			x_dir = 1;
		}

		int32_t clip_x = in_x1;
		int32_t clip_y = in_y1;
		int32_t offset = in_y1 * out_micron.canvas_width + in_x1;	//offset into buffer
		int32_t error_term = 0;				//error term

		// If difference is bigger in x dimension
		if (x_diff > y_diff)
		{
			//prepare to count off in x direction
			const int32_t LENGTH = x_diff + 1;

			//loop through points in x direction
			for (
				int32_t i = 0;
				i < LENGTH;
				++i
				)
			{
				//set pixel
				if (__clip(0, 0, out_micron.canvas_width, out_micron.canvas_height, clip_x, clip_y))
				{
					out_micron.canvas[offset] = in_color;
				}

				//next x pixel
				offset += x_unit;
				error_term += y_diff;
				clip_x += x_dir;

				//check for y move
				//if so reset error_term and move offset to next
				if (error_term > x_diff)
				{
					error_term -= x_diff;
					offset += y_unit;
					clip_y += y_dir;
				}
			}
		}
		//if difference is bigger in y dimension
		else
		{
			//count off in y
			const int32_t LENGTH = y_diff + 1;

			//loop y points
			for (
				int32_t i = 0;
				i < LENGTH;
				++i
				)
			{
				//set pixel
				if (__clip(0, 0, out_micron.canvas_width, out_micron.canvas_height, clip_x, clip_y))
				{
					out_micron.canvas[offset] = in_color;
				}

				//next y pixel
				offset += y_unit;
				error_term += x_diff;
				clip_y += y_dir;

				//check for  x move
				//if so reset error_term and move offset to next
				if (error_term > 0)
				{
					error_term -= y_diff;
					offset += x_unit;
					clip_x += x_dir;
				}
			}
		}
	}
}
