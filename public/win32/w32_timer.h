#pragma once

struct w32_timer_values_t
{
	double time;
	float elapsed;
};

struct w32_timer_t
{
	void reset();
	void start();
	void stop();
	w32_timer_values_t mutate_and_get_values();

	explicit w32_timer_t();

	::LONGLONG _qpf_ticks_per_second;
	::LONGLONG _stop_time;
	::LONGLONG _last_elapsed_time;
	::LONGLONG _base_time;
	bool _stopped;
};
