#include "stdafx.h"
#include "vc_fatpack.h"

#include "../microlib/c_math.h"
#include "../microlib/microlib.h"
#include "../micron.h"
#include "vc_assets.h"

namespace m25
{
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

	void vc_canvas_clear_set(
		const uint8_t& in_color, const int32_t in_dst_x, const int32_t in_dst_y, const int32_t in_clear_width, const int32_t in_clear_height,
		micron_t& out_micron)
	{
		uint8_t* dst = out_micron.canvas + in_dst_x + in_dst_y * out_micron.canvas_width;
		uint8_t* scan_dst;
		for (
			int32_t y = 0;
			y < in_clear_height;
			++y
			)
		{
			scan_dst = dst;

			for (
				int32_t x = 0;
				x < in_clear_width;
				++x
				)
			{
				assert(
					dst >= out_micron.canvas &&
					dst < out_micron.canvas + out_micron.canvas_width * out_micron.canvas_height
				);
				*dst++ = in_color;
			}

			dst = scan_dst + out_micron.canvas_width;
		}
	}

	void vc_canvas_clear_stipple(
		const uint8_t& in_color, int32_t in_dst_x, int32_t in_dst_y, int32_t in_clear_width, int32_t in_clear_height,
		micron_t& out_micron)
	{
		//clip
		int32_t src_x = 0;
		int32_t src_y = 0;
		if (__clip(

			//these are legacy, but here in case you would want to clip to some other rect than the whole bitmap
			0, 0, out_micron.canvas_width, out_micron.canvas_height,
			//these are legacy, but here in case you would want to clip to some other rect than the whole bitmap

			src_x, src_y, in_dst_x, in_dst_y, in_clear_width, in_clear_height))
		{
			uint8_t* dst = out_micron.canvas + in_dst_x + in_dst_y * out_micron.canvas_width;
			uint8_t* scan_dst;
			for (
				int32_t y = 0;
				y < in_clear_height;
				++y
				)
			{
				scan_dst = dst;
				dst += y % 2;

				for (
					int32_t x = 0;
					x < in_clear_width;
					x += 2
					)
				{
					assert(
						dst >= out_micron.canvas &&
						dst < out_micron.canvas + out_micron.canvas_width * out_micron.canvas_height
					);
					*dst = in_color;
					dst += 2;
				}

				dst = scan_dst + out_micron.canvas_width;
			}
		}
	}

	void vc_canvas_set_pixel(
		const int32_t in_x, const int32_t in_y, const uint8_t& in_color,
		micron_t& out_micron)
	{
		uint8_t* dst = out_micron.canvas + in_x + in_y * out_micron.canvas_width;
		assert(
			dst >= out_micron.canvas &&
			dst < out_micron.canvas + out_micron.canvas_width * out_micron.canvas_height
		);
		*dst = in_color;
	}

	void vc_canvas_h_line(
		int32_t in_x1, int32_t in_x2, const int32_t in_y1, const uint8_t& in_color,
		micron_t& out_micron)
	{
		//clip
		if (in_y1 < 0 || in_y1 >= out_micron.canvas_height)
			return;

		c_int32_min_max(in_x1, in_x2);
		in_x1 = __max(0, in_x1);
		in_x2 = __min(out_micron.canvas_width, in_x2);
		uint8_t* dst = out_micron.canvas + in_x1 + in_y1 * out_micron.canvas_width;

		for (
			int32_t x = in_x1;
			x < in_x2;
			++x
			)
		{
			assert(
				dst >= out_micron.canvas &&
				dst < out_micron.canvas + out_micron.canvas_width * out_micron.canvas_height
			);
			*dst++ = in_color;
		}
	}

	void vc_canvas_v_line(
		const int32_t in_x1, int32_t in_y1, int32_t in_y2, const uint8_t& in_color,
		micron_t& out_micron)
	{
		//clip
		if (in_x1 < 0 || in_x1 >= out_micron.canvas_width)
			return;

		c_int32_min_max(in_y1, in_y2);
		in_y1 = __max(0, in_y1);
		in_y2 = __min(out_micron.canvas_height, in_y2);
		uint8_t* dst = out_micron.canvas + in_x1 + in_y1 * out_micron.canvas_width;

		for (
			int32_t y = in_y1;
			y < in_y2;
			++y
			)
		{
			assert(
				dst >= out_micron.canvas &&
				dst < out_micron.canvas + out_micron.canvas_width * out_micron.canvas_height
			);
			*dst = in_color;
			dst += out_micron.canvas_width;
		}
	}

	void vc_canvas_rect(
		const int32_t in_x1, const int32_t in_y1, const int32_t in_x2, const int32_t in_y2, const uint8_t& in_color,
		micron_t& out_micron)
	{
		vc_canvas_h_line(in_x1, in_x2 + 1, in_y1, in_color, out_micron);
		vc_canvas_h_line(in_x1, in_x2 + 1, in_y2, in_color, out_micron);
		vc_canvas_v_line(in_x1, in_y1 + 1, in_y2, in_color, out_micron);
		vc_canvas_v_line(in_x2, in_y1 + 1, in_y2, in_color, out_micron);
	}

	void vc_canvas_atascii_print(
		const int32_t in_x, const int32_t in_y, const uint8_t in_color, const void* in_string,
		micron_t& out_micron)
	{
		int32_t x = in_x;
		int32_t signed_offset;
		const uint8_t* c = (uint8_t*)in_string;
		while (*c)
		{
			for (int32_t bit_y = 0; bit_y < 8; ++bit_y)
			{
				for (int32_t bit_x = 0; bit_x < 8; ++bit_x)
				{
					signed_offset = x + bit_x + (in_y + bit_y) * out_micron.canvas_width;
					assert(signed_offset >= 0 && signed_offset < out_micron.canvas_width * out_micron.canvas_height);
					if (MICRON_ATASCII_BITS[*c * 8 + bit_y] & (1 << bit_x))
						out_micron.canvas[signed_offset] = 15;
					else
						out_micron.canvas[signed_offset] = in_color;
				}
			}
			x += 8;
			if (x >= out_micron.canvas_width)
				return;
			++c;
		}
	}

	vc_fatpack_t::vc_fatpack_t()
	{
		tile_anim_tick = 0.f;

		bit_display_visited = 0;
		bit_display_hero_tests = 0;
		bit_camera_lerp = 0;
		bit_display_cambits = 0;
		bit_play_music = 1;

		idle_screen = VC_SCREEN_MAIN;

		last_visit_offset = UINT32_MAX;

		hero_anim = 0.f;

		//assert(0 == fx.used);
		assert(0 == gui_num_text);

		prng = c_xorshift128_t::make();
	}

	vc_fx_t* vc_fatpack_t::fx_acquire()
	{
		fx.resize(fx.size() + 1);
		return fx.data() + fx.size() - 1;
	}
}
