#include "stdafx.h"
#include "wmdl_assets.h"

#include "../minyin/fs.h"
#include "../minyin/minyin.h"
#include "../minyin/paletas.h"

#define ASSET_BG00 "bg00.tga"
#define ASSET_BG01 "bg01.tga"
#define ASSET_BG02 "bg02.tga"
#define ASSET_BG03 "bg03.tga"
#define ASSET_BG04 "bg04.tga"
#define ASSET_BG05 "bg05.tga"
#define ASSET_BG06 "bg06.tga"
#define ASSET_PORTAL "portal.tga"
#define ASSET_SERVER "server.tga"
#define ASSET_ARCS "arcs.tga"
#define ASSET_SPIKYGREEN "spikygreen.tga"
#define ASSET_BLUEBLOB "blueblob.tga"
#define ASSET_BROWNBLOB "brownblob.tga"
#define ASSET_PENGUIN "pingu.tga"
#define ASSET_PLANT "plantenemy.tga"
#define ASSET_SCORPION "scorpion.tga"
#define ASSET_FIREDUDE "firedude.tga"
#define ASSET_BAT "bat.tga"
#define ASSET_GUISERVERFIXED "gui_server_fixed.tga"
#define ASSET_GUISERVERBROKEN "gui_server_broken.tga"
#define ASSET_IDLE "idle.tga"
#define ASSET_TILES "tiles.tga"
#define ASSET_HERO "hero.tga"
#define ASSET_WHIP "whip.tga"
#define ASSET_FLAKE "snowflakes.tga"
#define ASSET_FONT "bigfont.tga"

#define ASSET_COMMON_INFO "view.common"

#define ASSET_SPAWN "spawn.wav"
#define ASSET_FIXSERVER "fixserver.wav"
#define ASSET_HEROWHIP "herowhip.wav"
#define ASSET_HEROJUMP01 "herojump01.wav"
#define ASSET_HEROJUMP02 "herojump02.wav"
#define ASSET_HEROJUMP03 "herojump03.wav"
#define ASSET_HEROJUMP04 "herojump04.wav"
#define ASSET_HEROJUMP05 "herojump05.wav"
#define ASSET_HEROLAND01 "heroland01.wav"
#define ASSET_HEROLAND02 "heroland02.wav"
#define ASSET_HEROLAND03 "heroland03.wav"
#define ASSET_HEROLAND04 "heroland04.wav"
#define ASSET_HERODIE01 "herodie01.wav"
#define ASSET_HERODIE02 "herodie02.wav"
#define ASSET_HERODIE03 "herodie03.wav"
#define ASSET_CHECKPOINT "checkpoint.wav"
#define ASSET_SND_SPIKYGREEN "spikygreen.wav"
#define ASSET_SND_BLUEBLOB "blueblob.wav"
#define ASSET_SND_BROWNBLOB "brownblob.wav"
#define ASSET_SND_PLANT "plant.wav"
#define ASSET_SND_SCORPION "scorpion.wav"
#define ASSET_SLIDERDEATH "sliderdeath.wav"
#define ASSET_BATFLEE "batflee.wav"
#define ASSET_KEY "key.wav"
#define ASSET_SLIDER "slider.wav"

#define BITMAP(a, b) if(!sd_bitmap_load_24(a, b)) return false;
#define BITMAP8(a, b) if(!minyin_bitmap_load_8(b, a)) return false;

bool wmdl_assets_init(wmdl_assets_t& out_assets, std::vector<minyin_sound_request_t>& out_sounds)
{
	//common info
	{
		const fs_blob_t CONTENTS = fs_file_contents(ASSET_COMMON_INFO);
		const bool RESULT = CONTENTS.data && sizeof(out_assets.anim_target) == CONTENTS.size;
		if (RESULT)
		{
			::memcpy(&out_assets.anim_target, CONTENTS.data, CONTENTS.size);
			for (
				uint32_t* at = out_assets.anim_target;
				at < out_assets.anim_target + WMDL_MAX_TILE;
				++at
				)
			{
				*at = wmdl_change_endianness(*at);
			}
		}
		delete[] CONTENTS.data;
		if (!RESULT)
			return false;
	}

	//bitmaps
	{
		paletas_t p;
		paletas_item(ASSET_BG00, out_assets.backgrounds[0], p);
		paletas_item(ASSET_BG01, out_assets.backgrounds[1], p);
		paletas_item(ASSET_BG02, out_assets.backgrounds[2], p);
		paletas_item(ASSET_BG03, out_assets.backgrounds[3], p);
		paletas_item(ASSET_BG04, out_assets.backgrounds[4], p);
		paletas_item(ASSET_BG05, out_assets.backgrounds[5], p);
		paletas_item(ASSET_BG06, out_assets.backgrounds[6], p);
		paletas_item(ASSET_PORTAL, out_assets.portal, p);
		paletas_item(ASSET_SERVER, out_assets.server, p);
		paletas_item(ASSET_ARCS, out_assets.arcs, p);
		paletas_item(ASSET_SPIKYGREEN, out_assets.spikygreen, p);
		paletas_item(ASSET_BLUEBLOB, out_assets.blueblob, p);
		paletas_item(ASSET_BROWNBLOB, out_assets.brownblob, p);
		paletas_item(ASSET_PENGUIN, out_assets.penguin, p);
		paletas_item(ASSET_PLANT, out_assets.plant, p);
		paletas_item(ASSET_SCORPION, out_assets.scorpion, p);
		paletas_item(ASSET_FIREDUDE, out_assets.firedude, p);
		paletas_item(ASSET_BAT, out_assets.bat, p);
		paletas_item(ASSET_GUISERVERFIXED, out_assets.gui_server_fixed, p);
		paletas_item(ASSET_GUISERVERBROKEN, out_assets.gui_server_broken, p);
		paletas_item(ASSET_IDLE, out_assets.idle, p);
		paletas_item(ASSET_TILES, out_assets.tiles, p);
		paletas_item(ASSET_HERO, out_assets.hero, p);
		paletas_item(ASSET_WHIP, out_assets.whip, p);
		paletas_item(ASSET_FLAKE, out_assets.flake, p);
		paletas_item(ASSET_FONT, out_assets.font.rep, p);
		if (!paletas_calculate(256, p))
			return false;
		out_assets.key_index = out_assets.hero.pixels[0];
		out_assets.text_edge_index = out_assets.font.rep.pixels[0];
	}

	//font
	if (!minyin_font_init(out_assets.font, out_assets.key_index))
		return false;
	out_assets.font.char_spacing = -1;

	//request sounds
	out_sounds.push_back({ ASSET_SPAWN, WMDL_SND_SPAWN });
	out_sounds.push_back({ ASSET_FIXSERVER, WMDL_SND_FIXSERVER });
	out_sounds.push_back({ ASSET_HEROWHIP, WMDL_SND_HEROWHIP });
	out_sounds.push_back({ ASSET_HEROJUMP01, WMDL_SND_HEROJUMP01 });
	out_sounds.push_back({ ASSET_HEROJUMP02, WMDL_SND_HEROJUMP02 });
	out_sounds.push_back({ ASSET_HEROJUMP03, WMDL_SND_HEROJUMP03 });
	out_sounds.push_back({ ASSET_HEROJUMP04, WMDL_SND_HEROJUMP04 });
	out_sounds.push_back({ ASSET_HEROJUMP05, WMDL_SND_HEROJUMP05 });
	out_sounds.push_back({ ASSET_HEROLAND01, WMDL_SND_HEROLAND01 });
	out_sounds.push_back({ ASSET_HEROLAND02, WMDL_SND_HEROLAND02 });
	out_sounds.push_back({ ASSET_HEROLAND03, WMDL_SND_HEROLAND03 });
	out_sounds.push_back({ ASSET_HEROLAND04, WMDL_SND_HEROLAND04 });
	out_sounds.push_back({ ASSET_HERODIE01, WMDL_SND_HERODIE01 });
	out_sounds.push_back({ ASSET_HERODIE02, WMDL_SND_HERODIE02 });
	out_sounds.push_back({ ASSET_HERODIE03, WMDL_SND_HERODIE03 });
	out_sounds.push_back({ ASSET_CHECKPOINT, WMDL_SND_CHECKPOINT });
	out_sounds.push_back({ ASSET_SND_SPIKYGREEN, WMDL_SND_SPIKYGREEN });
	out_sounds.push_back({ ASSET_SND_BLUEBLOB, WMDL_SND_BLUEBLOB });
	out_sounds.push_back({ ASSET_SND_BROWNBLOB, WMDL_SND_BROWNBLOB });
	out_sounds.push_back({ ASSET_SND_PLANT, WMDL_SND_PLANT });
	out_sounds.push_back({ ASSET_SND_SCORPION, WMDL_SND_SCORPION });
	out_sounds.push_back({ ASSET_SLIDERDEATH, WMDL_SND_SLIDERDEATH });
	out_sounds.push_back({ ASSET_BATFLEE, WMDL_SND_BATFLEE });
	out_sounds.push_back({ ASSET_KEY, WMDL_SND_KEY });
	out_sounds.push_back({ ASSET_SLIDER, WMDL_SND_SLIDER });

	return true;
}
