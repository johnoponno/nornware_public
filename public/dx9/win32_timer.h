#pragma once

struct win32_timer_values_t
{
	double time;
	float elapsed;
};

struct win32_timer_t
{
	void reset();
	void start();
	void stop();
	win32_timer_values_t mutate_and_get_values();

	explicit win32_timer_t();

	::LONGLONG _qpf_ticks_per_second;
	::LONGLONG _stop_time;
	::LONGLONG _last_elapsed_time;
	::LONGLONG _base_time;
	bool _stopped;
};
