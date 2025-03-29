#pragma once

struct c_blob_t;
struct c_vec2i_t;
struct c_vec2f_t;
struct c_xorshift128_t;

struct micron_t;

namespace m25
{
	struct m_events_t;
	struct m_immutable_t;
	struct m_mutable_t;

	struct vc_assets_t;
	struct vc_fatpack_t;
	struct vc_sound_request_t;

	const c_vec2i_t vc_world_to_screen_int(const c_vec2f_t& aWorld, const c_vec2i_t& c);
	const c_vec2f_t vc_world_to_screen_float(const c_vec2f_t& aWorld, const c_vec2i_t& c);

	c_vec2i_t vc_hero_screen(const m_mutable_t& m_mu);

	bool vc_view_init(vc_assets_t& out_assets, vc_fatpack_t& out_fatpack, micron_t& out_micron);

	c_vec2f_t vc_screen_local(const c_vec2f_t& world);

	void vc_tiles_clear_animations(vc_assets_t& im, vc_fatpack_t& mu);
	void vc_tiles_update_animations(
		const vc_assets_t& im,
		vc_fatpack_t& mu);

	void vc_fx_draw_all(
		const uint32_t in_paused, const uint32_t in_tick, const m_immutable_t& in_im, const c_vec2i_t& in_camera, const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack, micron_t& out_micron);

	void vc_visualize_events(
		const m_immutable_t& in_im, const uint32_t in_tick, const m_events_t& in_events, const m_mutable_t& in_mu, const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack, micron_t& out_micron);

	void vc_gui_draw_and_clear(
		const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack, micron_t& out_micron);

	void vc_draw_hero(
		const uint32_t in_tick, const m_immutable_t& in_im, const m_mutable_t& in_mu, const c_vec2i_t& in_camera, const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack, micron_t& out_micron);

	void vc_do_hero_movement(
		const micron_t& in_micron, const m_immutable_t& in_im,
		m_mutable_t& out_mu);

	void vc_draw_plax(
		const uint32_t tick, const m_immutable_t& m_im, const m_mutable_t& m_mu, const vc_assets_t& im, const c_vec2i_t& camera,
		c_xorshift128_t& prng, micron_t& out_micron);

	void vc_reset_animations(vc_fatpack_t& mu);
	/*
	void vc_do_ambience(
		const uint32_t tick, const char* aFile, const float max_volume, const vc_assets_t& im,
		vc_fatpack_t& mu);
		*/
	void vc_calculate_flowers(
		const m_immutable_t& m_im, const m_mutable_t& m_mu, 
		vc_fatpack_t& mu);

	uint32_t vc_gui_big_text(
		const micron_t& in_micron, const int32_t in_x, const int32_t in_y, const int32_t in_key, const char* in_text,
		vc_fatpack_t& out_fatpack);

	int32_t vc_tiles_on_source_x(const c_blob_t& tiles);
	void vc_tiles_draw(
		const m_immutable_t& in_im, const m_mutable_t& in_mu, const vc_assets_t& in_assets, const c_vec2i_t& in_camera, const uint8_t in_flags,
		vc_fatpack_t& out_fatpack, micron_t& out_micron);
#if 0
	void vc_draw_terminals(const vc_immutable_t& im, const c_vec2i_t& aCamera, const uint32_t anOffset, vc_canvas_t& canvas);
#endif

	//void vc_do_hero_sound(const uint32_t tick, const m_immutable_t& m_im, const m_mutable_t& m_mu, const vc_assets_t& im);

	c_vec2i_t vc_tile_src(const int32_t tiles_on_source_x, const int32_t tile_index);

	/*
	enum
	{
		VC_KEY_ESCAPE,
		VC_KEY_RETURN,
		VC_KEY_LMB,
		VC_KEY_RMB,
	};
	bool vc_key_upflank(const int32_t in_key);
	bool vc_key_is_down(const int32_t in_key);
	bool vc_input_down();
	bool vc_input_left();
	bool vc_input_right();
	bool vc_input_jump();
	bool vc_input_sprint();
	bool vc_input_melee();
	*/

	void vc_canvas_clear_set(
		const uint8_t& aColor, const int32_t aDstX, const int32_t aDstY, const int32_t aClearWidth, const int32_t aClearHeight,
		micron_t& out_micron);
	void vc_canvas_clear_stipple(
		const uint8_t& in_color, int32_t in_dst_x, int32_t in_dst_y, int32_t in_clear_width, int32_t in_clear_height,
		micron_t& out_micron);
	void vc_canvas_set_pixel(
		const int32_t aX, const int32_t aY, const uint8_t& aColor,
		micron_t& out_micron);
	void vc_canvas_h_line(
		int32_t in_x1, int32_t in_x2, const int32_t in_y1, const uint8_t& in_color,
		micron_t& out_micron);
	void vc_canvas_v_line(
		const int32_t in_x1, int32_t in_y1, int32_t in_y2, const uint8_t& in_color,
		micron_t& out_micron);
	void vc_canvas_rect(
		const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2, const uint8_t& aColor,
		micron_t& out_micron);
	void vc_canvas_atascii_print(
		const int32_t in_x, const int32_t in_y, const uint8_t in_color, const void* in_string,
		micron_t& out_micron);
}
