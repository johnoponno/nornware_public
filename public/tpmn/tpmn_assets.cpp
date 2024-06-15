#include "stdafx.h"
#include "tpmn_assets.h"

//#include "../win32/win32_d3d9_state.h"
#include "../softdraw/fs.h"
#include "../softdraw/minyin.h"

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

#define ASSET_DUST_FAR "dust_far.tga"
#define ASSET_DUST_NEAR "dust_near.tga"
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

//#define SOUND(asset, id) if (!out_assets.container.add_sound(out_assets.engine, asset, id, 1)) return false;

/*
tpmn_assets_t::tpmn_assets_t()
	:container(TPMN_NUM_SOUNDS)
{
}
*/

bool tpmn_assets_init(tpmn_assets_t& out_assets, std::vector<minyin_sound_request_t>& out_sounds)
{
	{
		const fs_blob_t CONTENTS = fs_file_contents(ASSET_COMMON_INFO);
		const bool RESULT = CONTENTS.data && sizeof(out_assets.anim_target) == CONTENTS.size;
		if (RESULT)
		{
			::memcpy(&out_assets.anim_target, CONTENTS.data, CONTENTS.size);
			for (uint32_t* at = out_assets.anim_target; at < out_assets.anim_target + TPMN_MAX_TILE; ++at)
				*at = tpmn_change_endianness(*at);
		}
		delete[] CONTENTS.data;
		if (!RESULT)
			return false;
	}

	BITMAP(ASSET_BG00, out_assets.backgrounds[0]);
	BITMAP(ASSET_BG01, out_assets.backgrounds[1]);
	BITMAP(ASSET_BG02, out_assets.backgrounds[2]);
	BITMAP(ASSET_BG03, out_assets.backgrounds[3]);
	BITMAP(ASSET_BG04, out_assets.backgrounds[4]);
	BITMAP(ASSET_BG05, out_assets.backgrounds[5]);
	BITMAP(ASSET_BG06, out_assets.backgrounds[6]);

	BITMAP(ASSET_PORTAL, out_assets.portal);
	BITMAP(ASSET_SERVER, out_assets.server);
	BITMAP(ASSET_ARCS, out_assets.arcs);
	BITMAP(ASSET_SPIKYGREEN, out_assets.spikygreen);
	BITMAP(ASSET_BLUEBLOB, out_assets.blueblob);
	BITMAP(ASSET_BROWNBLOB, out_assets.brownblob);
	BITMAP(ASSET_PENGUIN, out_assets.penguin);
	BITMAP(ASSET_PLANT, out_assets.plant);
	BITMAP(ASSET_SCORPION, out_assets.scorpion);
	BITMAP(ASSET_FIREDUDE, out_assets.firedude);
	BITMAP(ASSET_BAT, out_assets.bat);
	BITMAP(ASSET_GUISERVERFIXED, out_assets.gui_server_fixed);
	BITMAP(ASSET_GUISERVERBROKEN, out_assets.gui_server_broken);
	BITMAP(ASSET_IDLE, out_assets.idle);

	BITMAP(ASSET_TILES, out_assets.tiles);
	BITMAP(ASSET_HERO, out_assets.hero);
	BITMAP(ASSET_WHIP, out_assets.whip);

	BITMAP(ASSET_DUST_FAR, out_assets.dust_far);
	BITMAP(ASSET_DUST_NEAR, out_assets.dust_near);
	BITMAP(ASSET_FLAKE, out_assets.flake);

	if (!sd_fontv_load_24(out_assets.font, ASSET_FONT, false))
		return false;
	out_assets.font.char_spacing = -1;

	/*
	if (!out_assets.engine.init(win32_d3d9_hwnd()))
		return false;

	if (!out_assets.container.init())
		return false;

	SOUND(ASSET_SPAWN, TPMN_SND_SPAWN);
	SOUND(ASSET_FIXSERVER, TPMN_SND_FIXSERVER);
	SOUND(ASSET_HEROWHIP, TPMN_SND_HEROWHIP);
	SOUND(ASSET_HEROJUMP01, TPMN_SND_HEROJUMP01);
	SOUND(ASSET_HEROJUMP02, TPMN_SND_HEROJUMP02);
	SOUND(ASSET_HEROJUMP03, TPMN_SND_HEROJUMP03);
	SOUND(ASSET_HEROJUMP04, TPMN_SND_HEROJUMP04);
	SOUND(ASSET_HEROJUMP05, TPMN_SND_HEROJUMP05);
	SOUND(ASSET_HEROLAND01, TPMN_SND_HEROLAND01);
	SOUND(ASSET_HEROLAND02, TPMN_SND_HEROLAND02);
	SOUND(ASSET_HEROLAND03, TPMN_SND_HEROLAND03);
	SOUND(ASSET_HEROLAND04, TPMN_SND_HEROLAND04);
	SOUND(ASSET_HERODIE01, TPMN_SND_HERODIE01);
	SOUND(ASSET_HERODIE02, TPMN_SND_HERODIE02);
	SOUND(ASSET_HERODIE03, TPMN_SND_HERODIE03);
	SOUND(ASSET_CHECKPOINT, TPMN_SND_CHECKPOINT);
	SOUND(ASSET_SND_SPIKYGREEN, TPMN_SND_SPIKYGREEN);
	SOUND(ASSET_SND_BLUEBLOB, TPMN_SND_BLUEBLOB);
	SOUND(ASSET_SND_BROWNBLOB, TPMN_SND_BROWNBLOB);
	SOUND(ASSET_SND_PLANT, TPMN_SND_PLANT);
	SOUND(ASSET_SND_SCORPION, TPMN_SND_SCORPION);
	SOUND(ASSET_SLIDERDEATH, TPMN_SND_SLIDERDEATH);
	SOUND(ASSET_BATFLEE, TPMN_SND_BATFLEE);
	SOUND(ASSET_KEY, TPMN_SND_KEY);
	SOUND(ASSET_SLIDER, TPMN_SND_SLIDER);
	*/
	out_sounds.push_back({ ASSET_SPAWN, TPMN_SND_SPAWN });
	out_sounds.push_back({ ASSET_FIXSERVER, TPMN_SND_FIXSERVER });
	out_sounds.push_back({ ASSET_HEROWHIP, TPMN_SND_HEROWHIP });
	out_sounds.push_back({ ASSET_HEROJUMP01, TPMN_SND_HEROJUMP01 });
	out_sounds.push_back({ ASSET_HEROJUMP02, TPMN_SND_HEROJUMP02 });
	out_sounds.push_back({ ASSET_HEROJUMP03, TPMN_SND_HEROJUMP03 });
	out_sounds.push_back({ ASSET_HEROJUMP04, TPMN_SND_HEROJUMP04 });
	out_sounds.push_back({ ASSET_HEROJUMP05, TPMN_SND_HEROJUMP05 });
	out_sounds.push_back({ ASSET_HEROLAND01, TPMN_SND_HEROLAND01 });
	out_sounds.push_back({ ASSET_HEROLAND02, TPMN_SND_HEROLAND02 });
	out_sounds.push_back({ ASSET_HEROLAND03, TPMN_SND_HEROLAND03 });
	out_sounds.push_back({ ASSET_HEROLAND04, TPMN_SND_HEROLAND04 });
	out_sounds.push_back({ ASSET_HERODIE01, TPMN_SND_HERODIE01 });
	out_sounds.push_back({ ASSET_HERODIE02, TPMN_SND_HERODIE02 });
	out_sounds.push_back({ ASSET_HERODIE03, TPMN_SND_HERODIE03 });
	out_sounds.push_back({ ASSET_CHECKPOINT, TPMN_SND_CHECKPOINT });
	out_sounds.push_back({ ASSET_SND_SPIKYGREEN, TPMN_SND_SPIKYGREEN });
	out_sounds.push_back({ ASSET_SND_BLUEBLOB, TPMN_SND_BLUEBLOB });
	out_sounds.push_back({ ASSET_SND_BROWNBLOB, TPMN_SND_BROWNBLOB });
	out_sounds.push_back({ ASSET_SND_PLANT, TPMN_SND_PLANT });
	out_sounds.push_back({ ASSET_SND_SCORPION, TPMN_SND_SCORPION });
	out_sounds.push_back({ ASSET_SLIDERDEATH, TPMN_SND_SLIDERDEATH });
	out_sounds.push_back({ ASSET_BATFLEE, TPMN_SND_BATFLEE });
	out_sounds.push_back({ ASSET_KEY, TPMN_SND_KEY });
	out_sounds.push_back({ ASSET_SLIDER, TPMN_SND_SLIDER });

	return true;
}

void tpmn_assets_reload(tpmn_assets_t& out_assets)
{
	sd_bitmap_load_24(ASSET_TILES, out_assets.tiles);
	sd_bitmap_load_24(ASSET_BG00, out_assets.backgrounds[0]);
	sd_bitmap_load_24(ASSET_BG01, out_assets.backgrounds[1]);
	sd_bitmap_load_24(ASSET_BG03, out_assets.backgrounds[3]);
	sd_bitmap_load_24(ASSET_BG04, out_assets.backgrounds[4]);
	sd_bitmap_load_24(ASSET_BG05, out_assets.backgrounds[5]);
	sd_bitmap_load_24(ASSET_DUST_NEAR, out_assets.dust_near);
}

void tpmn_assets_cleanup(tpmn_assets_t& in_assets)
{
	in_assets;
	//out_assets.container.cleanup();
	//out_assets.engine.cleanup();
}

/*
void tpmn_sound_play(const tpmn_assets_t& in_assets, const uint32_t in_id)
{
	out_assets.container.play(id, 1.f, 0.f, 1.f, nullptr);
}
*/
