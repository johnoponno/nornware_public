#include "stdafx.h"
#include "p_util.h"

#include "p_context.h"

p_util_t::p_util_t(const char* in_path)
	:_path(in_path)
{
}

const char* p_util_t::id(const char* in_member)
{
	return _buffer.format("%s/%s", _path, in_member);
}

const char* p_util_t::id(const char* in_member, const uint32_t in_index)
{
	return _buffer.format("%s/%s[%u]", _path, in_member, in_index);
}

p_string_puller_t::p_string_puller_t(
	const char* in_kind, const uint32_t in_instance, const char* in_member, const char* in_default, 
	p_context_t& out_context)
{
	out_context.pull_string(in_kind, in_instance, in_member, in_default, _result);
}
