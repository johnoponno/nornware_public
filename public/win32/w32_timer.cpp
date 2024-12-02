#include "stdafx.h"
#include "w32_timer.h"

static const ::LARGE_INTEGER __get_adjusted_current_time(const ::LONGLONG stop_time)
{
	::LARGE_INTEGER time;
	if (stop_time != 0)
		time.QuadPart = stop_time;
	else
		::QueryPerformanceCounter(&time);
	return time;
}

//public
//public
//public
//public

void w32_timer_t::reset()
{
	const ::LARGE_INTEGER TIME = __get_adjusted_current_time(_stop_time);

	_base_time = TIME.QuadPart;
	_last_elapsed_time = TIME.QuadPart;
	_stop_time = 0;
	_stopped = FALSE;
}

void w32_timer_t::start()
{
	// get the current time
	::LARGE_INTEGER time;
	::QueryPerformanceCounter(&time);

	if (_stopped)
		_base_time += time.QuadPart - _stop_time;
	_stop_time = 0;
	_last_elapsed_time = time.QuadPart;
	_stopped = FALSE;
}

void w32_timer_t::stop()
{
	if (!_stopped)
	{
		::LARGE_INTEGER time;
		::QueryPerformanceCounter(&time);
		_stop_time = time.QuadPart;
		_last_elapsed_time = time.QuadPart;
		_stopped = TRUE;
	}
}

w32_timer_values_t w32_timer_t::mutate_and_get_values()
{
	const ::LARGE_INTEGER TIME = __get_adjusted_current_time(_stop_time);

	float elapsed_time = (float)((double)(TIME.QuadPart - _last_elapsed_time) / (double)_qpf_ticks_per_second);
	_last_elapsed_time = TIME.QuadPart;

	// Clamp the timer to non-negative values to ensure the timer is accurate.
	// elapsed_time can be outside this range if processor goes into a 
	// power save mode or we somehow get shuffled to another processor.  
	// However, the main thread should call SetThreadAffinityMask to ensure that 
	// we don't get shuffled to another processor.  Other worker threads should NOT call 
	// SetThreadAffinityMask, but use a shared copy of the timer data gathered from 
	// the main thread.
	if (elapsed_time < 0.f)
		elapsed_time = 0.f;

	w32_timer_values_t result;
	result.time = (TIME.QuadPart - _base_time) / (double)_qpf_ticks_per_second;
	result.elapsed = elapsed_time;
	return result;
}

w32_timer_t::w32_timer_t()
{
	_stopped = true;
	_qpf_ticks_per_second = 0;

	_stop_time = 0;
	_last_elapsed_time = 0;
	_base_time = 0;

	// Use QueryPerformanceFrequency to get the frequency of the counter
	LARGE_INTEGER ticks_per_second;
	::QueryPerformanceFrequency(&ticks_per_second);
	_qpf_ticks_per_second = ticks_per_second.QuadPart;
}
