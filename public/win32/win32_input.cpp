#include "stdafx.h"
#include "win32_input.h"

#include "../softdraw/minyin.h"

static struct
{
	struct
	{
		bool down_current;
		bool down_last;
	} keys[256];
	int32_t cursor_pos_x;
	int32_t cursor_pos_y;
	int32_t cursor_movement_x;
	int32_t cursor_movement_y;
	struct
	{
		int32_t delta;
		uint32_t event_consumed;
	} mouse_wheel;
} __state{};

void win32_input_poll(const ::HWND window)
{
	for (uint32_t key = 0; key < _countof(__state.keys); ++key)
	{
		__state.keys[key].down_last = __state.keys[key].down_current;
		__state.keys[key].down_current = 0 != ::GetAsyncKeyState(key);
	}

	{
		::POINT cursorPos;
		::GetCursorPos(&cursorPos);
		::ScreenToClient(window, &cursorPos);

		__state.cursor_movement_x = cursorPos.x - __state.cursor_pos_x;
		__state.cursor_movement_y = cursorPos.y - __state.cursor_pos_y;
		__state.cursor_pos_x = cursorPos.x;
		__state.cursor_pos_y = cursorPos.y;
	}

	if (__state.mouse_wheel.event_consumed)
	{
		__state.mouse_wheel.delta = 0;
		__state.mouse_wheel.event_consumed = false;
	}
}

bool win32_key_is_down(const int32_t aKey)
{
	if (aKey >= 0 && aKey < _countof(__state.keys))
		return __state.keys[aKey].down_current;
	return false;
}

bool win32_key_down_flank(const int32_t aKey)
{
	if (aKey >= 0 && aKey < _countof(__state.keys))
		return __state.keys[aKey].down_current && !__state.keys[aKey].down_last;
	return false;
}

bool win32_key_up_flank(const int32_t aKey)
{
	if (aKey >= 0 && aKey < _countof(__state.keys))
		return !__state.keys[aKey].down_current && __state.keys[aKey].down_last;
	return false;
}

bool win32_key_up_flank_any()
{
	for (uint32_t i = 0; i < _countof(__state.keys); ++i)
	{
		if (win32_key_up_flank(i))
			return true;
	}

	return false;
}

void win32_mouse_wheel_delta_set(const int32_t aDelta)
{
	__state.mouse_wheel.delta = aDelta;
}

int32_t win32_mouse_wheel_delta()
{
	if (0 != __state.mouse_wheel.delta)
		__state.mouse_wheel.event_consumed = true;
	return __state.mouse_wheel.delta;
}

void win32_mouse_cursor_position_set(const ::HWND window, const int32_t anX, const int32_t aY)
{
	__state.cursor_pos_x = anX;
	__state.cursor_pos_y = aY;

	if (::IsZoomed(window))
	{
		::SetCursorPos(anX, aY);
	}
	else
	{
		::POINT zero = { 0, 0 };
		::ClientToScreen(window, &zero);
		::SetCursorPos(zero.x + anX, zero.y + aY);
	}
}

/*
minyin_vec2i_t win32_mouse_cursor_position()
{
	return { __state.cursor_pos_x, __state.cursor_pos_y };
}
*/
