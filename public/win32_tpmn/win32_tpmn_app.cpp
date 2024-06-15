#include "stdafx.h"

#if 0

#include "tpmn_app.h"

#include "tpmn_game.h"

bool tpmn_app_t::win32_d3d9_simpleapp_tick(const win32_cursor_position_t& in_cursor_position)
{
	return tpmn_game_tick(in_cursor_position, _game);
}

tpmn_app_t::tpmn_app_t()
	:win32_d3d9_simpleapp_i(TPMN_SECONDS_PER_TICK, _game.controller.canvas)
{
}

#endif
