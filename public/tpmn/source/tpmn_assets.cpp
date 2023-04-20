#include "stdafx.h"
#include "tpmn_assets.h"

#include "../../dx9/state.h"
#include "../../softdraw/tga.h"

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
#define SOUND(asset, id) if (!container_add_sound(assets.engine, asset, id, 1, assets.container)) return false;

tpmn_assets_t::tpmn_assets_t()
	:container(TPMN_NUM_SOUNDS)
{
}

bool tpmn_assets_init(tpmn_assets_t& assets)
{
	{
		fs::blob_t contents = fs::file_contents(ASSET_COMMON_INFO);
		const bool result = contents.data && sizeof(assets.anim_target) == contents.size;
		if (result)
		{
			::memcpy(&assets.anim_target, contents.data, contents.size);
			for (uint32_t* at = assets.anim_target; at < assets.anim_target + TPMN_MAX_TILE; ++at)
				*at = tpmn_change_endianness(*at);
		}
		delete[] contents.data;
		if (!result)
			return false;
	}

	BITMAP(ASSET_BG00, assets.backgrounds[0]);
	BITMAP(ASSET_BG01, assets.backgrounds[1]);
	BITMAP(ASSET_BG02, assets.backgrounds[2]);
	BITMAP(ASSET_BG03, assets.backgrounds[3]);
	BITMAP(ASSET_BG04, assets.backgrounds[4]);
	BITMAP(ASSET_BG05, assets.backgrounds[5]);
	BITMAP(ASSET_BG06, assets.backgrounds[6]);

	BITMAP(ASSET_PORTAL, assets.portal);
	BITMAP(ASSET_SERVER, assets.server);
	BITMAP(ASSET_ARCS, assets.arcs);
	BITMAP(ASSET_SPIKYGREEN, assets.spikygreen);
	BITMAP(ASSET_BLUEBLOB, assets.blueblob);
	BITMAP(ASSET_BROWNBLOB, assets.brownblob);
	BITMAP(ASSET_PENGUIN, assets.penguin);
	BITMAP(ASSET_PLANT, assets.plant);
	BITMAP(ASSET_SCORPION, assets.scorpion);
	BITMAP(ASSET_FIREDUDE, assets.firedude);
	BITMAP(ASSET_BAT, assets.bat);
	BITMAP(ASSET_GUISERVERFIXED, assets.gui_server_fixed);
	BITMAP(ASSET_GUISERVERBROKEN, assets.gui_server_broken);
	BITMAP(ASSET_IDLE, assets.idle);

	BITMAP(ASSET_TILES, assets.tiles);
	BITMAP(ASSET_HERO, assets.hero);
	BITMAP(ASSET_WHIP, assets.whip);

	BITMAP(ASSET_DUST_FAR, assets.dust_far);
	BITMAP(ASSET_DUST_NEAR, assets.dust_near);
	BITMAP(ASSET_FLAKE, assets.flake);

	if (!sd_fontv_load_24(assets.font, ASSET_FONT, false))
		return false;
	assets.font.char_spacing = -1;

	if (!engine_init(win32_d3d9_hwnd(), assets.engine))
		return false;

	if (!container_init(assets.container))
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

	return true;
}

void tpmn_assets_reload(tpmn_assets_t& assets)
{
	sd_bitmap_load_24(ASSET_TILES, assets.tiles);
	sd_bitmap_load_24(ASSET_BG00, assets.backgrounds[0]);
	sd_bitmap_load_24(ASSET_BG01, assets.backgrounds[1]);
	sd_bitmap_load_24(ASSET_BG03, assets.backgrounds[3]);
	sd_bitmap_load_24(ASSET_BG04, assets.backgrounds[4]);
	sd_bitmap_load_24(ASSET_BG05, assets.backgrounds[5]);
	sd_bitmap_load_24(ASSET_DUST_NEAR, assets.dust_near);
}

void tpmn_assets_cleanup(tpmn_assets_t& assets)
{
	container_cleanup(assets.container);
	engine_cleanup(assets.engine);
}

void tpmn_sound_play(const tpmn_assets_t& assets, const uint32_t id)
{
	sound::container_play(id, 1.f, 0.f, 1.f, nullptr, assets.container);
}
