#pragma once

struct c_vec2i_t;
struct c_vec2f_t;

namespace m25
{
	enum struct m_mode_t;
	struct m_immutable_t;
	struct m_mutable_t;
	struct m_events_t;
	struct m_mob_t;

	bool m_in_world(const int32_t aX, const int32_t aY);
	uint32_t m_world_to_offset(const c_vec2f_t& aWorldPos);
	c_vec2i_t m_world_to_grid(const c_vec2f_t& aWorld);
	c_vec2f_t m_grid_to_world(const int32_t aX, const int32_t aY);
	c_vec2f_t m_offset_to_world(const uint32_t anOffset);
	uint32_t m_grid_to_offset(const c_vec2i_t& aGrid);

	void m_spawn(const uint32_t tick, const m_immutable_t& im, m_mutable_t& mu);
	void m_set_start(const c_vec2i_t& aGrid, m_immutable_t& im);
	void m_restart(const uint32_t tick, const m_mode_t aMode, const m_immutable_t& im, m_mutable_t& mu);
	void m_set_hero_input(const m_immutable_t& im, const uint8_t someFlags, m_mutable_t& mu);

	void m_load(m_immutable_t& im);
	m_events_t m_update(const m_immutable_t& im, const uint32_t tick, m_mutable_t& mu);
	void m_do_switches(const m_immutable_t& im, m_mutable_t& mu, m_events_t& events);
	void m_back_to_checkpoint(m_mutable_t& mu, m_events_t& events);

	bool m_game_active(const m_mutable_t& m);

	uint32_t m_tile_type_at_offset(const uint32_t offset, const bool aSwitchesFlag, const m_immutable_t& im, const m_mutable_t& m);
	uint32_t m_tile_type_at_position(const c_vec2f_t& aPosition, const bool aSwitchesFlag, const m_immutable_t& im, const m_mutable_t& m);
	uint32_t m_tile_type(const uint16_t tile, const bool aSwitchesFlag, const m_immutable_t& im, const m_mutable_t& m);

	void m_persist(const m_immutable_t& im);

	bool m_logic_is(const uint32_t aValue, const uint32_t anIndex, const m_immutable_t& im);
	uint32_t m_logic_index_of(const uint32_t aValue, const m_immutable_t& im);

	uint16_t m_tile(const uint32_t offset, const m_immutable_t& im);
	uint16_t m_tile(const int32_t aX, const int32_t aY, const m_immutable_t& im);
	uint16_t m_tile(const c_vec2f_t& aPosition, const m_immutable_t& im);

	uint8_t m_visited(const int32_t aX, const int32_t aY, const m_mutable_t& mu);
	uint8_t m_visited(const c_vec2f_t& aPosition, const m_mutable_t& mu);

	void m_events_push(const c_vec2f_t& position, const uint32_t arg, const uint32_t type, m_events_t& events);

	bool m_mob_sees_hero(const m_immutable_t& im, const m_mutable_t& mu, const m_mob_t& mob);

	void m_character_restart(const c_vec2f_t& aPosition, m_mutable_t& mu);
	bool m_character_is_solid_left_more(const m_immutable_t& im, const m_mutable_t& mu);
	bool m_character_is_solid_right(const m_immutable_t& im, const m_mutable_t& mu);
	c_vec2f_t m_character_test_above(const m_mutable_t& mu);
	c_vec2f_t m_character_get_test_left(const bool aSecondFlag, const m_mutable_t& mu);
	c_vec2f_t m_character_get_test_right(const bool aSecondFlag, const m_mutable_t& mu);
	c_vec2f_t m_character_test_below(const bool aSecondFlag, const m_mutable_t& mu);
	bool m_character_being_abducted(const m_immutable_t& im, const m_mutable_t& mu);
	bool m_character_alive(const m_mutable_t& mu);
}
