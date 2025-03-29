#include "stdafx.h"
#include "wmdl_assets.h"

#include "../micron.h"
#include "../microlib/fs.h"
#include "../microlib/paletas.h"

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

bool wmdl_assets_init(micron_t& out_micron, wmdl_assets_t& out_assets)
{
	//load animation info
	{
		const c_blob_t CONTENTS = fs_file_contents(ASSET_COMMON_INFO);
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

	//import bitmaps and calculate am 8-bit target palette
	{
		paletas_t p;

		//list all the incoming 24-bit .tga files, and their corresponding target (bitmaps for our convenience)
		p.item(ASSET_BG00, out_assets.backgrounds[0]);
		p.item(ASSET_BG01, out_assets.backgrounds[1]);
		p.item(ASSET_BG02, out_assets.backgrounds[2]);
		p.item(ASSET_BG03, out_assets.backgrounds[3]);
		p.item(ASSET_BG04, out_assets.backgrounds[4]);
		p.item(ASSET_BG05, out_assets.backgrounds[5]);
		p.item(ASSET_BG06, out_assets.backgrounds[6]);
		p.item(ASSET_PORTAL, out_assets.portal);
		p.item(ASSET_SERVER, out_assets.server);
		p.item(ASSET_ARCS, out_assets.arcs);
		p.item(ASSET_SPIKYGREEN, out_assets.spikygreen);
		p.item(ASSET_BLUEBLOB, out_assets.blueblob);
		p.item(ASSET_BROWNBLOB, out_assets.brownblob);
		p.item(ASSET_PENGUIN, out_assets.penguin);
		p.item(ASSET_PLANT, out_assets.plant);
		p.item(ASSET_SCORPION, out_assets.scorpion);
		p.item(ASSET_FIREDUDE, out_assets.firedude);
		p.item(ASSET_BAT, out_assets.bat);
		p.item(ASSET_GUISERVERFIXED, out_assets.gui_server_fixed);
		p.item(ASSET_GUISERVERBROKEN, out_assets.gui_server_broken);
		p.item(ASSET_IDLE, out_assets.idle);
		p.item(ASSET_TILES, out_assets.tiles);
		p.item(ASSET_HERO, out_assets.hero);
		p.item(ASSET_WHIP, out_assets.whip);
		p.item(ASSET_FLAKE, out_assets.flake);
		p.item(ASSET_FONT, out_assets.font.rep);

		//1) allocate contigous memory for all bitmaps specified above
		//2) calculate a palette (best-fit) based on the source data
		//3) assign memory to all output bitmaps
		//4) remap input 24-bit pixels to output 8-bit pixels

		//NOTE: it is possible to calculate to less than 256 colors if desired (for artistic effect)

		//NOTE: it would theoretically be possible to cache the outputs (palette and contiguous memory) and avoid the source files altogether
		//this would load much faster BUT would require information about what order (and size) the bitmaps are in order to reconstruct the current
		//runtime micron_bitmap_t objects
		if (!p.calculate(256, out_micron, out_assets.bitmap_memory))
			return false;

		//cache the key index (for transparent blitting) based on knowledge of the input assets (top left corner of hero image is "that pink")
		out_assets.key_index = out_assets.hero.pixels[0];

		//the same goes for the text edge index
		out_assets.text_edge_index = out_assets.font.rep.pixels[0];
	}

	//setup font character tables, spacing, etc
	if (!micron_font_init(out_assets.font, out_assets.key_index))
		return false;
	out_assets.font.char_spacing = -1;

	//request sounds from the implementation
	out_micron.sound_loads.push_back({ ASSET_SPAWN, WMDL_SND_SPAWN });
	out_micron.sound_loads.push_back({ ASSET_FIXSERVER, WMDL_SND_FIXSERVER });
	out_micron.sound_loads.push_back({ ASSET_HEROWHIP, WMDL_SND_HEROWHIP });
	out_micron.sound_loads.push_back({ ASSET_HEROJUMP01, WMDL_SND_HEROJUMP01 });
	out_micron.sound_loads.push_back({ ASSET_HEROJUMP02, WMDL_SND_HEROJUMP02 });
	out_micron.sound_loads.push_back({ ASSET_HEROJUMP03, WMDL_SND_HEROJUMP03 });
	out_micron.sound_loads.push_back({ ASSET_HEROJUMP04, WMDL_SND_HEROJUMP04 });
	out_micron.sound_loads.push_back({ ASSET_HEROJUMP05, WMDL_SND_HEROJUMP05 });
	out_micron.sound_loads.push_back({ ASSET_HEROLAND01, WMDL_SND_HEROLAND01 });
	out_micron.sound_loads.push_back({ ASSET_HEROLAND02, WMDL_SND_HEROLAND02 });
	out_micron.sound_loads.push_back({ ASSET_HEROLAND03, WMDL_SND_HEROLAND03 });
	out_micron.sound_loads.push_back({ ASSET_HEROLAND04, WMDL_SND_HEROLAND04 });
	out_micron.sound_loads.push_back({ ASSET_HERODIE01, WMDL_SND_HERODIE01 });
	out_micron.sound_loads.push_back({ ASSET_HERODIE02, WMDL_SND_HERODIE02 });
	out_micron.sound_loads.push_back({ ASSET_HERODIE03, WMDL_SND_HERODIE03 });
	out_micron.sound_loads.push_back({ ASSET_CHECKPOINT, WMDL_SND_CHECKPOINT });
	out_micron.sound_loads.push_back({ ASSET_SND_SPIKYGREEN, WMDL_SND_SPIKYGREEN });
	out_micron.sound_loads.push_back({ ASSET_SND_BLUEBLOB, WMDL_SND_BLUEBLOB });
	out_micron.sound_loads.push_back({ ASSET_SND_BROWNBLOB, WMDL_SND_BROWNBLOB });
	out_micron.sound_loads.push_back({ ASSET_SND_PLANT, WMDL_SND_PLANT });
	out_micron.sound_loads.push_back({ ASSET_SND_SCORPION, WMDL_SND_SCORPION });
	out_micron.sound_loads.push_back({ ASSET_SLIDERDEATH, WMDL_SND_SLIDERDEATH });
	out_micron.sound_loads.push_back({ ASSET_BATFLEE, WMDL_SND_BATFLEE });
	out_micron.sound_loads.push_back({ ASSET_KEY, WMDL_SND_KEY });
	out_micron.sound_loads.push_back({ ASSET_SLIDER, WMDL_SND_SLIDER });

	return true;
}

void wmdl_assets_cleanup(wmdl_assets_t& out_assets)
{
	delete[] out_assets.bitmap_memory.data;
}
