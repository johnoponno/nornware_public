#include "stdafx.h"
#include "w32_timer.h"

static const ::LARGE_INTEGER __get_adjusted_current_time(const ::LONGLONG stop_time)
{
	::LARGE_INTEGER qwTime;
	if (stop_time != 0)
		qwTime.QuadPart = stop_time;
	else
		::QueryPerformanceCounter(&qwTime);
	return qwTime;
}

//public
//public
//public
//public
void w32_timer_reset(w32_timer_t& timer)
{
	const ::LARGE_INTEGER qwTime = __get_adjusted_current_time(timer.stop_time);

	timer.base_time = qwTime.QuadPart;
	timer.last_elapsed_time = qwTime.QuadPart;
	timer.stop_time = 0;
	timer.stopped = FALSE;
}

void w32_timer_start(w32_timer_t& timer)
{
	// get the current time
	::LARGE_INTEGER qwTime;
	::QueryPerformanceCounter(&qwTime);

	if (timer.stopped)
		timer.base_time += qwTime.QuadPart - timer.stop_time;
	timer.stop_time = 0;
	timer.last_elapsed_time = qwTime.QuadPart;
	timer.stopped = FALSE;
}

void w32_timer_stop(w32_timer_t& timer)
{
	if (!timer.stopped)
	{
		::LARGE_INTEGER qwTime;
		::QueryPerformanceCounter(&qwTime);
		timer.stop_time = qwTime.QuadPart;
		timer.last_elapsed_time = qwTime.QuadPart;
		timer.stopped = TRUE;
	}
}

w32_timer_values_t w32_timer_mutate_and_get_values(w32_timer_t& timer)
{
	const ::LARGE_INTEGER qwTime = __get_adjusted_current_time(timer.stop_time);

	float fElapsedTime = (float)((double)(qwTime.QuadPart - timer.last_elapsed_time) / (double)timer.qpf_ticks_per_second);
	timer.last_elapsed_time = qwTime.QuadPart;

	// Clamp the timer to non-negative values to ensure the timer is accurate.
	// fElapsedTime can be outside this range if processor goes into a 
	// power save mode or we somehow get shuffled to another processor.  
	// However, the main thread should call SetThreadAffinityMask to ensure that 
	// we don't get shuffled to another processor.  Other worker threads should NOT call 
	// SetThreadAffinityMask, but use a shared copy of the timer data gathered from 
	// the main thread.
	if (fElapsedTime < 0.f)
		fElapsedTime = 0.f;

	w32_timer_values_t result;
	result.time = (qwTime.QuadPart - timer.base_time) / (double)timer.qpf_ticks_per_second;
	result.elapsed = fElapsedTime;
	return result;
}

w32_timer_t::w32_timer_t()
{
	stopped = true;
	qpf_ticks_per_second = 0;

	stop_time = 0;
	last_elapsed_time = 0;
	base_time = 0;

	// Use QueryPerformanceFrequency to get the frequency of the counter
	LARGE_INTEGER qwTicksPerSec;
	::QueryPerformanceFrequency(&qwTicksPerSec);
	qpf_ticks_per_second = qwTicksPerSec.QuadPart;
}
