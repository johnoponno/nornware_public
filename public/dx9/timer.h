#pragma once

namespace dx9
{
	struct timer_values_t
	{
		double time;
		float elapsed;
	};

	struct timer_t
	{
		explicit timer_t();

		::LONGLONG qpf_ticks_per_second;
		::LONGLONG stop_time;
		::LONGLONG last_elapsed_time;
		::LONGLONG base_time;
		bool stopped;
	};

	void timer_reset(timer_t& timer);
	void timer_start(timer_t& timer);
	void timer_stop(timer_t& timer);
	timer_values_t timer_mutate_and_get_values(timer_t& timer);
}
