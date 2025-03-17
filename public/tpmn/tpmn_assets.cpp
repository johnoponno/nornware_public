#include "stdafx.h"
#include "tpmn_assets.h"

#include "../micron.h"
#include "../micron/fs.h"

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

bool tpmn_assets_init(micron_t& out_micron, tpmn_assets_t& out_assets)
{
	//common info
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

	//bitmaps
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

	//font
	if (!sd_fontv_load_24(out_assets.font, ASSET_FONT, false))
		return false;
	out_assets.font.char_spacing = -1;

	//request sounds
	out_micron.sound_loads.push_back({ ASSET_SPAWN, TPMN_SND_SPAWN });
	out_micron.sound_loads.push_back({ ASSET_FIXSERVER, TPMN_SND_FIXSERVER });
	out_micron.sound_loads.push_back({ ASSET_HEROWHIP, TPMN_SND_HEROWHIP });
	out_micron.sound_loads.push_back({ ASSET_HEROJUMP01, TPMN_SND_HEROJUMP01 });
	out_micron.sound_loads.push_back({ ASSET_HEROJUMP02, TPMN_SND_HEROJUMP02 });
	out_micron.sound_loads.push_back({ ASSET_HEROJUMP03, TPMN_SND_HEROJUMP03 });
	out_micron.sound_loads.push_back({ ASSET_HEROJUMP04, TPMN_SND_HEROJUMP04 });
	out_micron.sound_loads.push_back({ ASSET_HEROJUMP05, TPMN_SND_HEROJUMP05 });
	out_micron.sound_loads.push_back({ ASSET_HEROLAND01, TPMN_SND_HEROLAND01 });
	out_micron.sound_loads.push_back({ ASSET_HEROLAND02, TPMN_SND_HEROLAND02 });
	out_micron.sound_loads.push_back({ ASSET_HEROLAND03, TPMN_SND_HEROLAND03 });
	out_micron.sound_loads.push_back({ ASSET_HEROLAND04, TPMN_SND_HEROLAND04 });
	out_micron.sound_loads.push_back({ ASSET_HERODIE01, TPMN_SND_HERODIE01 });
	out_micron.sound_loads.push_back({ ASSET_HERODIE02, TPMN_SND_HERODIE02 });
	out_micron.sound_loads.push_back({ ASSET_HERODIE03, TPMN_SND_HERODIE03 });
	out_micron.sound_loads.push_back({ ASSET_CHECKPOINT, TPMN_SND_CHECKPOINT });
	out_micron.sound_loads.push_back({ ASSET_SND_SPIKYGREEN, TPMN_SND_SPIKYGREEN });
	out_micron.sound_loads.push_back({ ASSET_SND_BLUEBLOB, TPMN_SND_BLUEBLOB });
	out_micron.sound_loads.push_back({ ASSET_SND_BROWNBLOB, TPMN_SND_BROWNBLOB });
	out_micron.sound_loads.push_back({ ASSET_SND_PLANT, TPMN_SND_PLANT });
	out_micron.sound_loads.push_back({ ASSET_SND_SCORPION, TPMN_SND_SCORPION });
	out_micron.sound_loads.push_back({ ASSET_SLIDERDEATH, TPMN_SND_SLIDERDEATH });
	out_micron.sound_loads.push_back({ ASSET_BATFLEE, TPMN_SND_BATFLEE });
	out_micron.sound_loads.push_back({ ASSET_KEY, TPMN_SND_KEY });
	out_micron.sound_loads.push_back({ ASSET_SLIDER, TPMN_SND_SLIDER });

	return true;
}
