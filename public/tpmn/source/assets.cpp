#include "stdafx.h"
#include "assets.h"

#include "../../dx9/state.h"
#include "../../softdraw/tga.h"

namespace tpmn
{
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

#define BITMAP(a, b) if(!bitmap_load_24(a, b)) return false;
#define SOUND(asset, id) if (!container_add_sound(assets.engine, asset, id, 1, assets.container)) return false;

	assets_t::assets_t()
		:container(num_sounds)
	{
	}

	bool assets_init(assets_t& assets)
	{
		{
			fs::blob_t contents = fs::file_contents(ASSET_COMMON_INFO);
			const bool result = contents.data && sizeof(assets.anim_target) == contents.size;
			if (result)
			{
				::memcpy(&assets.anim_target, contents.data, contents.size);
				for (uint32_t* at = assets.anim_target; at < assets.anim_target + MAX_TILE; ++at)
					*at = change_endianness(*at);
			}
			delete[] contents.data;
			if (!result)
				return false;
		}

		BITMAP(ASSET_BG00, assets.myBackgrounds[0]);
		BITMAP(ASSET_BG01, assets.myBackgrounds[1]);
		BITMAP(ASSET_BG02, assets.myBackgrounds[2]);
		BITMAP(ASSET_BG03, assets.myBackgrounds[3]);
		BITMAP(ASSET_BG04, assets.myBackgrounds[4]);
		BITMAP(ASSET_BG05, assets.myBackgrounds[5]);
		BITMAP(ASSET_BG06, assets.myBackgrounds[6]);

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

		if (!fontv_load_24(assets.font, ASSET_FONT, false))
			return false;
		assets.font.char_spacing = -1;

		if (!engine_init(dx9::hwnd(), assets.engine))
			return false;

		if (!container_init(assets.container))
			return false;

		SOUND(ASSET_SPAWN, snd_spawn);
		SOUND(ASSET_FIXSERVER, snd_fixserver);
		SOUND(ASSET_HEROWHIP, snd_herowhip);
		SOUND(ASSET_HEROJUMP01, snd_herojump01);
		SOUND(ASSET_HEROJUMP02, snd_herojump02);
		SOUND(ASSET_HEROJUMP03, snd_herojump03);
		SOUND(ASSET_HEROJUMP04, snd_herojump04);
		SOUND(ASSET_HEROJUMP05, snd_herojump05);
		SOUND(ASSET_HEROLAND01, snd_heroland01);
		SOUND(ASSET_HEROLAND02, snd_heroland02);
		SOUND(ASSET_HEROLAND03, snd_heroland03);
		SOUND(ASSET_HEROLAND04, snd_heroland04);
		SOUND(ASSET_HERODIE01, snd_herodie01);
		SOUND(ASSET_HERODIE02, snd_herodie02);
		SOUND(ASSET_HERODIE03, snd_herodie03);
		SOUND(ASSET_CHECKPOINT, snd_checkpoint);
		SOUND(ASSET_SND_SPIKYGREEN, snd_spikygreen);
		SOUND(ASSET_SND_BLUEBLOB, snd_blueblob);
		SOUND(ASSET_SND_BROWNBLOB, snd_brownblob);
		SOUND(ASSET_SND_PLANT, snd_plant);
		SOUND(ASSET_SND_SCORPION, snd_scorpion);
		SOUND(ASSET_SLIDERDEATH, snd_sliderdeath);
		SOUND(ASSET_BATFLEE, snd_batflee);
		SOUND(ASSET_KEY, snd_key);
		SOUND(ASSET_SLIDER, snd_slider);

		return true;
	}

	void assets_reload(assets_t& assets)
	{
		//FS_LOG("reloading tiles");
		bitmap_load_24(ASSET_TILES, assets.tiles);

		//FS_LOG("reloading bg00");
		bitmap_load_24(ASSET_BG00, assets.myBackgrounds[0]);

		//FS_LOG("reloading bg01");
		bitmap_load_24(ASSET_BG01, assets.myBackgrounds[1]);

		//FS_LOG("reloading bg03");
		bitmap_load_24(ASSET_BG03, assets.myBackgrounds[3]);

		//FS_LOG("reloading bg04");
		bitmap_load_24(ASSET_BG04, assets.myBackgrounds[4]);

		//FS_LOG("reloading bg05");
		bitmap_load_24(ASSET_BG05, assets.myBackgrounds[5]);

		//FS_LOG("reloading dust_near");
		bitmap_load_24(ASSET_DUST_NEAR, assets.dust_near);
	}

	void assets_cleanup(assets_t& assets)
	{
		container_cleanup(assets.container);
		engine_cleanup(assets.engine);
	}

	void sound_play(const assets_t& assets, const uint32_t id)
	{
		sound::container_play(id, 1.f, 0.f, 1.f, nullptr, assets.container);
	}

	const uint16_t TITLE_COLOR = 0xffff;
	const uint16_t TEXT_COLOR = 0xc7bf;
}
