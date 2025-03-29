#pragma once

template <const uint32_t SIZE>
struct c_fixed_string_t
{
	explicit c_fixed_string_t()
	{
		buffer[0] = 0;
	}

	explicit c_fixed_string_t(const char* aString)
	{
		if (aString)
			::strcpy_s(buffer, aString);
		else
			buffer[0] = 0;
	}

	void clear()
	{
		buffer[0] = 0;
	}

	const char* operator = (const char* aString)
	{
		if (aString)
			::strcpy_s(buffer, aString);
		else
			buffer[0] = 0;
		return buffer;
	}

	const char* format(const char* aFormat, ...)
	{
		::va_list args;
		va_start(args, aFormat);
		::vsprintf_s(buffer, aFormat, args);
		va_end(args);
		return buffer;
	}

	bool operator == (const char* aString) const
	{
		if (aString)
			return 0 == ::strcmp(buffer, aString);
		return false;
	}

	bool operator != (const char* aString) const
	{
		return !operator == (aString);
	}

	bool operator == (const c_fixed_string_t& aString) const
	{
		return operator == (aString.buffer);
	}

	bool operator != (const c_fixed_string_t& aString) const
	{
		return operator != (aString.buffer);
	}

	bool operator < (const c_fixed_string_t& aString) const
	{
		return ::strcmp(buffer, aString.buffer) < 0;
	}

	char buffer[SIZE];
};

template <const uint32_t SIZE>
struct c_string_memory_t
{
	explicit c_string_memory_t()
	{
		wipe();
	}

	void wipe()
	{
		::memset(_buffer, 0, sizeof(_buffer));
		_cursor = _buffer;
	}

	const char* format(const char* in_format, ...)
	{
		const char* PRE_CURSOR = _cursor;

		{
			::va_list args;
			va_start(args, in_format);
			::vsprintf_s(_cursor, SIZE - (_cursor - _buffer), in_format, args);
			va_end(args);
		}

		while (*_cursor)
			++_cursor;
		++_cursor;

		return PRE_CURSOR;
	}

	char _buffer[SIZE];
	char* _cursor;
};

using c_path_t = c_fixed_string_t<256>;	//almost MAX_PATH (260)

#define C_STR(string, value) 	string.format("%s %s", #value, value)
#define C_HEX(string, value) 	string.format("%s %x", #value, value)
#define C_IP4(string, value) 	string.format("%s %u.%u.%u.%u", #value, ((value >> 0) & 0xff), ((value >> 8) & 0xff), ((value >> 16) & 0xff), ((value >> 24) & 0xff))
#define C_S32(string, value) 	string.format("%s %d", #value, value)
#define C_U32(string, value) 	string.format("%s %u", #value, value)
#define C_U64(string, value) 	string.format("%s %llu", #value, value)
#define C_F32(string, value)	string.format("%s %f", #value, value)
#define C_V2F(string, value) 	string.format("%s %f %f", #value, value.x, value.y)
#define C_XZF(string, value) 	string.format("%s %f %f", #value, value.x, value.z)
#define C_V2I(string, value) 	string.format("%s %d %d", #value, value.x, value.y)
#define C_XZI(string, value) 	string.format("%s %d %d", #value, value.x, value.z)
#define C_V3F(string, value) 	string.format("%s %f %f %f", #value, value.x, value.y, value.z)
#define C_V3I(string, value) 	string.format("%s %d %d %d", #value, value.x, value.y, value.z)
//#define C_V4F(string, value) 	string.format("%s %f %f %f %f", #value, value.x, value.y, value.z, value.w)
