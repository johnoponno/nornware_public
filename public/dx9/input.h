#pragma once

namespace dx9
{
	struct cursor_position_t
	{
		int32_t x;
		int32_t y;
	};

	void input_poll(const ::HWND window);

	bool key_is_down(const int32_t aKey);
	bool key_down_flank(const int32_t aKey);
	bool key_up_flank(const int32_t aKey);
	bool key_up_flank_any();

	int32_t mouse_wheel_delta();
	void mouse_wheel_delta_set(const int32_t aDelta);
	cursor_position_t mouse_cursor_position();
	void mouse_cursor_position_set(const ::HWND window, const int32_t anX, const int32_t aY);
}
