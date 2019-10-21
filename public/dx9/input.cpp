#include "stdafx.h"
#include "input.h"

namespace dx9
{
#define NUM_KEYS 256
	struct state_t
	{
		struct
		{
			bool down_current;
			bool down_last;
		} keys[NUM_KEYS];
		int32_t cursor_pos_x;
		int32_t cursor_pos_y;
		int32_t cursor_movement_x;
		int32_t cursor_movement_y;
		struct
		{
			int32_t delta;
			uint32_t event_consumed;
		} mouse_wheel;
	};
	static state_t __state{};

	void input_poll(const ::HWND window)
	{
		for (int32_t key = 0; key < NUM_KEYS; ++key)
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

	bool key_is_down(const int32_t aKey)
	{
		if (aKey >= 0 && aKey < NUM_KEYS)
			return __state.keys[aKey].down_current;
		return false;
	}

	bool key_down_flank(const int32_t aKey)
	{
		if (aKey >= 0 && aKey < NUM_KEYS)
			return __state.keys[aKey].down_current && !__state.keys[aKey].down_last;
		return false;
	}

	bool key_up_flank(const int32_t aKey)
	{
		if (aKey >= 0 && aKey < NUM_KEYS)
			return !__state.keys[aKey].down_current && __state.keys[aKey].down_last;
		return false;
	}

	bool key_up_flank_any()
	{
		for (int32_t i = 0; i < NUM_KEYS; ++i)
		{
			if (key_up_flank(i))
				return true;
		}

		return false;
	}

	void mouse_wheel_delta_set(const int32_t aDelta)
	{
		__state.mouse_wheel.delta = aDelta;
	}

	int32_t mouse_wheel_delta()
	{
		if (0 != __state.mouse_wheel.delta)
			__state.mouse_wheel.event_consumed = true;
		return __state.mouse_wheel.delta;
	}

	void mouse_cursor_position_set(const ::HWND window, const int32_t anX, const int32_t aY)
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

	cursor_position_t mouse_cursor_position()
	{
		return { __state.cursor_pos_x, __state.cursor_pos_y };
	}
}
