#include "stdafx.h"
#include "sound_stream.h"

#include "win32_dsound_stream_source.h"
#include "sound_engine.h"

namespace sound
{
	enum struct copy_to_buffer_result_t
	{
		ok,
		fail,
		end_of_sample
	};

	static bool __init(const engine_t& anEngine, stream_t& s)
	{
#define NUM_FRAMES 100
		::DSBUFFERDESC dsbd;

		//create DSBuffer to match
		::memset(&dsbd, 0, sizeof(dsbd));
		dsbd.dwSize = sizeof(::DSBUFFERDESC);
		dsbd.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME;// | DSBCAPS_LOCSOFTWARE;
		dsbd.dwBufferBytes = s.buffer_size = s.source->FRAME_SIZE * NUM_FRAMES;
		dsbd.lpwfxFormat = (::LPWAVEFORMATEX)&s.source->output_format;

		//try to create buffer
		if (FAILED(anEngine.directsound->CreateSoundBuffer(&dsbd, &s.buffer, nullptr)))
			return false;

		return true;
	}

	static copy_to_buffer_result_t __copy_to_buffer(const uint32_t aDstOffset, uint32_t& aCopyLength, stream_t& s)
	{
		::HRESULT error;
		uint8_t* dst;
		uint8_t* dst2;
		::DWORD dstLength;
		::DWORD dstLength2;

		//lock buffer
		error = s.buffer->Lock(aDstOffset,			//write offset
			aCopyLength,			//size of mem to lock
			(void**)&dst,			//pointer to first block of locked mem
			&dstLength,				//length of first block
			(void**)&dst2,			//pointer to second block of locked mem
			&dstLength2,			//length of second block
			0L);					//flags
		if (error == DS_OK)
		{
			copy_to_buffer_result_t result;

			assert(!dst2 && !dstLength2);

			//copy chunk 1 from cache
			if (dstLength)
			{
				//if looping, load wrapped
				if (s.looping_eh)
				{
					if (s.source->win32_dsound_stream_source_load_wrapped_segment(dst, dstLength))
					{
						//set copy length & return value
						aCopyLength = dstLength;
						result = copy_to_buffer_result_t::ok;
					}
					else
					{
						aCopyLength = 0;
						result = copy_to_buffer_result_t::fail;
					}
				}
				//else load nonwrapped
				else
				{
					if (s.source->win32_dsound_stream_source_load_segment(dst, dstLength, aCopyLength))
					{
						//check if reached EOF
						if (aCopyLength < dstLength)
						{
							//clear rest of segment
							::memset(dst + aCopyLength, 0, dstLength - aCopyLength);
							result = copy_to_buffer_result_t::end_of_sample;
						}
						else
							result = copy_to_buffer_result_t::ok;
					}
					else
					{
						aCopyLength = 0;
						result = copy_to_buffer_result_t::fail;
					}
				}
			}
			else
			{
				//set copy length & return value
				aCopyLength = 0;
				result = copy_to_buffer_result_t::fail;
			}

			//unlock
			if (FAILED(s.buffer->Unlock(dst, dstLength, dst2, dstLength2)))
			{
				aCopyLength = 0;
				return copy_to_buffer_result_t::fail;
			}

			return result;
		}

		aCopyLength = 0;
		return copy_to_buffer_result_t::fail;
	}

	static bool __play(const bool aLoopFlag, const float aVolume, const float aStartPosition, stream_t& s)
	{
		s.total_position = 0;

		s.looping_eh = aLoopFlag;

		//pre-cache
		//if(!_precacheBuffer(aStartPosition))
		//	return false;
		{
			uint32_t copyLength;

			//rewind stream
			if (!s.source->win32_dsound_stream_source_rewind(aStartPosition))
				return false;

			//figure out where we actually started in bytes (based on where we rewinded to)
			s.total_position = s.source->win32_dsound_stream_source_position_bytes();

			//fill entire sound buffer
			copyLength = s.buffer_size;
			switch (__copy_to_buffer(0, copyLength, s))
			{
			case copy_to_buffer_result_t::ok:
				break;

			case copy_to_buffer_result_t::end_of_sample:
				s.stop_pending_eh = true;
				s.end_of_sample_position = copyLength;
				break;

			default:
				return false;
			}
		}

		//set volume
		if (s.buffer->SetVolume(win32_dsound_linear_to_directx_volume(aVolume)) != DS_OK)
			return false;

		//reset buffer
		if (s.buffer->SetCurrentPosition(0) != DS_OK)
			return false;

		//play buffer
		s.last_play_position = 0;	//active update might mess with this
		if (s.buffer->Play(0, 0, DSBPLAY_LOOPING) == DS_OK)
		{
			s.playing_eh = true;
			return true;
		}

		return false;
	}

	static void __clear_buffer(const uint32_t aDstOffset, const uint32_t aClearLength, stream_t& s)
	{
		HRESULT error;
		uint8_t* dst;
		uint8_t* dst2;
		::DWORD dstLength;
		::DWORD dstLength2;

		//lock buffer
		error = s.buffer->Lock(
			aDstOffset,					//write offset
			aClearLength,			//size of mem to lock
			(void**)&dst,			//pointer to first block of locked mem
			&dstLength,				//length of first block
			(void**)&dst2,			//pointer to second block of locked mem
			&dstLength2,			//length of second block
			0L);					//flags
		if (error == DS_OK)
		{
			assert(!dst2 && !dstLength2);

			//copy chunk 1 from cache
			if (dstLength)
				::memset(dst, 0, dstLength);

			//unlock
			if (FAILED(s.buffer->Unlock(dst, dstLength, dst2, dstLength2)))
			{
				assert(0);
				return;
			}
		}
		else
		{
			assert(0);
		}
	}

	stream_t::stream_t(win32_dsound_stream_source_i* aSource)
	{
		source = aSource;
		buffer = nullptr;
		fade_start = 0.f;
		fade_duration = 0.f;
		fade_delta = 0.f;
		last_play_position = 0;
		buffer_size = 0;
		total_position = 0;
		end_of_sample_position = 0;
		fading_eh = 0;
		playing_eh = 0;
		looping_eh = 0;
		stop_pending_eh = 0;
	}

	stream_t::~stream_t()
	{
		if (buffer)
			buffer->Release();
		delete source;
	}

	stream_t* stream_create(
		const engine_t& anEngine,
		win32_dsound_stream_source_i* aSource)
	{
		stream_t* s = new stream_t(aSource);

		if (s && !__init(anEngine, *s))
		{
			delete s;
			s = nullptr;
		}

		return s;
	}

	bool stream_play(const bool aLoopFlag, const float aStartPosition, const float aMaxVolume, stream_t& s)
	{
		if (!s.playing_eh)
			return __play(aLoopFlag, aMaxVolume, aStartPosition, s);

		return false;
	}

	bool stream_fade_in_play(const bool aLoopFlag, const float aCurrentTime, const float aDuration, const float aStartPosition, stream_t& s)
	{
		if (!s.playing_eh)
		{
			s.fading_eh = true;
			s.fade_start = aCurrentTime;
			s.fade_duration = aDuration;
			s.fade_delta = 1;

			return __play(aLoopFlag, 0.f, aStartPosition, s);
		}

		return false;
	}

	bool stream_fade_out_stop(const float aCurrentTime, const float aDuration, stream_t& s)
	{
		if (s.playing_eh)
		{
			s.fading_eh = true;
			s.fade_start = aCurrentTime;
			s.fade_duration = aDuration;
			s.fade_delta = -1;

			return true;
		}

		return false;
	}

	bool stream_update(const float aCurrentTime, const float aMaxVolume, stream_t& s)
	{
		//if not playing, just return
		if (!s.playing_eh)
			return true;

		//UPDATE FADE (volume)
		{
			float volume = aMaxVolume;

			if (s.playing_eh && s.fading_eh)
			{
				float fadeParam;

				//fade in
				if (s.fade_delta == 1)
					fadeParam = (aCurrentTime - s.fade_start) / s.fade_duration;//calc parametric time
				//fade out
				else if (s.fade_delta == -1)
					fadeParam = 1.f - (aCurrentTime - s.fade_start) / s.fade_duration;//calc parametric time
				else
					fadeParam = -1.f;

				//if not faded past min/max
				if (fadeParam <= 1.f && fadeParam >= 0.f)
				{
					//calc volume
					volume = fadeParam * aMaxVolume;
				}
				//else min/max volume and end of fade
				else
				{
					if (s.fade_delta == 1)
					{
						volume = aMaxVolume;
					}
					else if (s.fade_delta == -1)
					{
						volume = 0.f;

						//should stop
						stream_stop_playback(s);
						return false;
					}
					else
					{
						volume = aMaxVolume;
					}

					s.fading_eh = false;
				}
			}

			//set volume
			if (FAILED(s.buffer->SetVolume(win32_dsound_linear_to_directx_volume(volume))))
				return false;
		}

		//CHECK IF WE SHOULD UPDATE THE BUFFER BYTES
		{
			//get current play position
			::DWORD play_position;
			if (FAILED(s.buffer->GetCurrentPosition(&play_position, nullptr)))
				return false;

			const uint32_t MIDBUFFER_OFFSET = s.buffer_size / 2;

			//if pending stop, just check if play position has passed end of sample position
			if (s.stop_pending_eh)
			{
				const bool STOP =
					(play_position >= s.end_of_sample_position && s.last_play_position < s.end_of_sample_position) || // last, end, cur
					(play_position < s.last_play_position && s.last_play_position <= s.end_of_sample_position) || // cur, last, end
					(play_position >= s.end_of_sample_position && s.last_play_position > play_position); // end, cur, last

				if (play_position < s.end_of_sample_position && s.end_of_sample_position < MIDBUFFER_OFFSET && s.last_play_position >= MIDBUFFER_OFFSET)
					__clear_buffer(MIDBUFFER_OFFSET, MIDBUFFER_OFFSET, s);
				else if (play_position < s.end_of_sample_position && play_position >= MIDBUFFER_OFFSET && s.last_play_position < MIDBUFFER_OFFSET)
					__clear_buffer(0, MIDBUFFER_OFFSET, s);

				if (STOP)
				{
					//stop directsound playback
					stream_stop_playback(s);
					return false;
				}
			}
			//check if need to copy in more data
			else if (((play_position >= MIDBUFFER_OFFSET) && (s.last_play_position < MIDBUFFER_OFFSET)) || (play_position < s.last_play_position))
			{
				uint32_t dst_offset;

				//decided which half to refresh
				if (play_position >= MIDBUFFER_OFFSET)
					dst_offset = 0;
				else
					dst_offset = MIDBUFFER_OFFSET;

				//copy to buffer
				uint32_t copy_length = MIDBUFFER_OFFSET;
				switch (__copy_to_buffer(dst_offset, copy_length, s))
				{
				case copy_to_buffer_result_t::fail:
					return false;
					break;

				case copy_to_buffer_result_t::end_of_sample:
					//store where sample ends
					s.stop_pending_eh = true;
					s.end_of_sample_position = dst_offset + copy_length;
					break;
				}
			}

			//have we wrapped the buffer?
			if (play_position < s.last_play_position)
				s.total_position += s.buffer_size;

			//store last play position
			s.last_play_position = play_position;
		}

		return true;
	}

	void stream_stop_playback(stream_t& s)
	{
		s.buffer->Stop();

		s.playing_eh = false;
		s.looping_eh = false;
		s.stop_pending_eh = false;

		s.last_play_position = 0;
		s.end_of_sample_position = 0;

		s.fading_eh = false;
		s.fade_start = 0.f;
		s.fade_duration = 0.f;
		s.fade_delta = 0.f;
	}

	float stream_position_seconds(const stream_t& s)
	{
		uint32_t p;
		uint32_t t;

		stream_position_bytes(p, t, s);
		return p / (float)WIN32_DSOUND_STEREO_WAV_FORMAT.average_bytes_per_second;
	}

	void stream_position_bytes(uint32_t& aPosition, uint32_t& aTotal, const stream_t& s)
	{
		::DWORD p;
		if (SUCCEEDED(s.buffer->GetCurrentPosition(&p, nullptr)))
		{
			aTotal = s.source->win32_dsound_stream_source_bytes_total();
			aPosition = (s.total_position + p) % aTotal;
		}
		else
		{
			aPosition = aTotal = 0;
		}
	}
}
