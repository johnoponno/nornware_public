#pragma once

#define MINYIN_ESCAPE 0x1B

/*
struct minyin_vec2i_t
{
	int32_t x;
	int32_t y;

};
*/

struct minyin_sound_request_t
{
	const char* asset;
	uint32_t id;
};

struct minyin_t
{
	bool key_is_down(const int32_t in_key) const;
	bool key_downflank(const int32_t in_key) const;

	struct
	{
		bool down_current;
		bool down_last;
	} _keys[256];
	int32_t _screen_cursor_x;
	int32_t _screen_cursor_y;
	int32_t _canvas_cursor_x;
	int32_t _canvas_cursor_y;
	/*
	int32_t cursor_pos_x;
	int32_t cursor_pos_y;
	int32_t cursor_movement_x;
	int32_t cursor_movement_y;
	struct
	{
		int32_t delta;
		uint32_t event_consumed;
	} mouse_wheel;
	*/
};
