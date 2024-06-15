#pragma once

#if 0

void win32_input_poll(const ::HWND window);
bool win32_key_is_down(const int32_t aKey);
bool win32_key_down_flank(const int32_t aKey);
bool win32_key_up_flank(const int32_t aKey);
bool win32_key_up_flank_any();

int32_t win32_mouse_wheel_delta();
void win32_mouse_wheel_delta_set(const int32_t aDelta);
void win32_mouse_cursor_position_set(const ::HWND window, const int32_t anX, const int32_t aY);

#endif
