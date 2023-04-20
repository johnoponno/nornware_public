#include "stdafx.h"
#include "win32_timer.h"

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
void win32_timer_t::reset()
{
	const ::LARGE_INTEGER qwTime = __get_adjusted_current_time(_stop_time);

	_base_time = qwTime.QuadPart;
	_last_elapsed_time = qwTime.QuadPart;
	_stop_time = 0;
	_stopped = FALSE;
}

void win32_timer_t::start()
{
	// get the current time
	::LARGE_INTEGER qwTime;
	::QueryPerformanceCounter(&qwTime);

	if (_stopped)
		_base_time += qwTime.QuadPart - _stop_time;
	_stop_time = 0;
	_last_elapsed_time = qwTime.QuadPart;
	_stopped = FALSE;
}

void win32_timer_t::stop()
{
	if (!_stopped)
	{
		::LARGE_INTEGER qwTime;
		::QueryPerformanceCounter(&qwTime);
		_stop_time = qwTime.QuadPart;
		_last_elapsed_time = qwTime.QuadPart;
		_stopped = TRUE;
	}
}

win32_timer_values_t win32_timer_t::mutate_and_get_values()
{
	const ::LARGE_INTEGER qwTime = __get_adjusted_current_time(_stop_time);

	float fElapsedTime = (float)((double)(qwTime.QuadPart - _last_elapsed_time) / (double)_qpf_ticks_per_second);
	_last_elapsed_time = qwTime.QuadPart;

	// Clamp the timer to non-negative values to ensure the timer is accurate.
	// fElapsedTime can be outside this range if processor goes into a 
	// power save mode or we somehow get shuffled to another processor.  
	// However, the main thread should call SetThreadAffinityMask to ensure that 
	// we don't get shuffled to another processor.  Other worker threads should NOT call 
	// SetThreadAffinityMask, but use a shared copy of the timer data gathered from 
	// the main thread.
	if (fElapsedTime < 0.f)
		fElapsedTime = 0.f;

	win32_timer_values_t result;
	result.time = (qwTime.QuadPart - _base_time) / (double)_qpf_ticks_per_second;
	result.elapsed = fElapsedTime;
	return result;
}

win32_timer_t::win32_timer_t()
{
	_stopped = true;
	_qpf_ticks_per_second = 0;

	_stop_time = 0;
	_last_elapsed_time = 0;
	_base_time = 0;

	// Use QueryPerformanceFrequency to get the frequency of the counter
	LARGE_INTEGER qwTicksPerSec;
	::QueryPerformanceFrequency(&qwTicksPerSec);
	_qpf_ticks_per_second = qwTicksPerSec.QuadPart;
}
