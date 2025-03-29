#pragma once

#include "c_fixed_string.h"

struct p_context_t;

struct p_member_value_t
{
	c_path_t x;
	c_path_t y;
};

struct p_util_t
{
	explicit p_util_t(const char* aPath);

	const char* id(const char* in_member);
	const char* id(const char* in_member, const uint32_t in_index);

	const char* _path;
	c_path_t _buffer;
};

struct p_string_puller_t
{
	explicit p_string_puller_t(
		const char* in_kind, const uint32_t in_instance, const char* in_member, const char* in_default,
		p_context_t& out_context);

	c_path_t _result;
};
