#include "stdafx.h"
#include "fs_parse_tool.h"

//#include "../core/c_macros.h"

fs_parse_tool_t::fs_parse_tool_t(const char* in_it)
{
	::memset(&_last_read_line, 0, sizeof(_last_read_line));
	_it = in_it;
}

bool fs_parse_tool_t::end_of_file_eh() const
{
	return nullptr == _it;
}

bool fs_parse_tool_t::read_line(const char* in_eol, const bool in_strip_comments)
{
	//whitespace
	while (*_it == ' ' || *_it == '\t')
		++_it;

	//c++ comment
	if (in_strip_comments)
	{
		if (_it[0] == '/' && _it[1] == '/')
			_it += 2;
	}

	//endline
	if (_it[0] == '\r' && _it[1] == '\n')
		_it += 2;

	//find end of line
	const char* eol = ::strstr(_it, in_eol);
	if (eol)
	{
		::strncpy_s(_last_read_line, _it, eol - _it);
		_it = eol + ::strlen(in_eol);
		return true;
	}

	//eof
	_it = nullptr;
	return false;
}
