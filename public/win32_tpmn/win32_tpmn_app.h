#pragma once

#if 0

#include "../../win32/win32_d3d9_app.h"
#include "tpmn_game.h"

struct tpmn_app_t : public win32_d3d9_simpleapp_i
{
	bool win32_d3d9_simpleapp_tick(const win32_cursor_position_t& in_cursor_position) override;

	explicit tpmn_app_t();

	tpmn_game_t _game;
};

#endif
