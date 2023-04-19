#pragma once

struct w32_timer_values_t
{
	double time;
	float elapsed;
};

struct w32_timer_t
{
	explicit w32_timer_t();

	::LONGLONG qpf_ticks_per_second;
	::LONGLONG stop_time;
	::LONGLONG last_elapsed_time;
	::LONGLONG base_time;
	bool stopped;
};

void w32_timer_reset(w32_timer_t& timer);
void w32_timer_start(w32_timer_t& timer);
void w32_timer_stop(w32_timer_t& timer);
w32_timer_values_t w32_timer_mutate_and_get_values(w32_timer_t& timer);
