#include "stdafx.h"
#include "vc_assets.h"

namespace mlm
{
	vc_assets_t::vc_assets_t()
	{
		//tiles
		{
			for (
				uint32_t i = 0;
				i < VC_NUM_GFX;
				++i
				)
				gfx[i] = VC_GFX_NAME_DEFAULT[i].default;

			for (auto& info : tile_info)
			{
				info.anim_target = 0;
				info.wang = 0;
			}
		}

		for (vc_screen_t& s : screens)
		{
			s.ambience = "none";
			s.restart = 0;
		}

#if 0
		assert(nullptr == terminal_content);
		assert(0 == num_terminal_content);

		assert(nullptr == terminal);
		assert(0 == num_terminal);
#endif
	}

	vc_assets_t::~vc_assets_t()
	{
		//delete[] gui_big_font.data;

#if 0
		for (
			uint32_t i = 0;
			i < _countof(farplane_assets);
			++i
			)
			delete[] farplane_assets[i].data;
#else
		delete[] tempcave.data;
#endif

		delete[] tiles.data;
		//delete[] cursor.data;
		delete[] blades.data;
		delete[] gibs.data;
		delete[] impalement.data;
		delete[] drowning.data;
		delete[] burning.data;
		delete[] electrination.data;
		delete[] special_icons.data;
		//delete[] fish.data;

		delete[] hero.bitmap.data;
		delete[] heavy.data;
		delete[] walker.bitmap.data;
		delete[] walker2.bitmap.data;
		delete[] jumper.bitmap.data;
		delete[] jumper2.bitmap.data;
		delete[] swimmer.bitmap.data;
		delete[] swimmer2.bitmap.data;

#if 0
		for (
			uint32_t i = 0;
			i < num_terminal_content;
			++i
			)
			delete[] terminal_content[i].bitmap.data;
		delete[] terminal_content;

		delete[] terminal;
#endif

		//sounds.cleanup();
		//engine.cleanup();
	}

	/*
	uint8_t vc_assets_t::palettize_color(const uint8_t in_r, const uint8_t in_g, const uint8_t in_b) const
	{
		float nd = FLT_MAX;
		uint8_t result = 0;

		for (
			uint32_t i = 0;
			i < 256;
			++i
			)
		{
			const c_vec3uc_t PUC = sd_color_decode(sd_palette[i]);
			const c_vec3f_t PF{ (float)PUC.x, (float)PUC.y, (float)PUC.z };
			const c_vec3f_t CF{ (float)in_r, (float)in_g, (float)in_b };
			const c_vec3f_t VF = PF - CF;
			const float D = c_length(VF);
			if (D < nd)
			{
				nd = D;
				result = (uint8_t)i;
			}
		}

		return result;
	}

	uint8_t vc_assets_t::palettize_color(const uint16_t in_color) const
	{
		const c_vec3uc_t PUC = sd_color_decode(in_color);
		return palettize_color(PUC.x, PUC.y, PUC.z);
	}
	*/
}
