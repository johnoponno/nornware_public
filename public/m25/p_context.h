#pragma once

#include "c_fixed_string.h"

struct p_instance_t;
struct p_kind_t;

//2024-07-08: pulls are NOT const since member names need to be encoded
struct p_context_t
{
	explicit p_context_t();
	~p_context_t();

	bool read(const char* in_file);

	void pull_string(
		const char* in_kind, const uint32_t in_instance, const char* in_member, const char* in_default, 
		c_path_t& out_result);
	void push_string(const char* in_kind, const uint32_t in_instance, const char* in_member, const char* in_value);

	int32_t pull_int32(const char* in_kind, const uint32_t in_instance, const char* in_member, const int32_t in_default);
	void push_int32(const char* in_kind, const uint32_t in_instance, const char* in_member, const int32_t in_value);

	uint32_t pull_uint32(const char* in_kind, const uint32_t in_instance, const char* in_member, const uint32_t in_default);
	void push_uint32(const char* in_kind, const uint32_t in_instance, const char* in_member, const uint32_t in_value);

	bool write(const char* in_file) const;

	std::map<uint32_t, p_kind_t*> _kinds;
	std::map<c_path_t, uint32_t> _kind_names;//NOTE: choice of which is key vs value favors fast encoding over decoding (map a string name to an index, happens every load...)
	std::map<c_path_t, uint32_t> _member_names;//NOTE: choice of which is key vs value favors fast encoding over decoding (map a string name to an index, happens every load...)

private:

	explicit p_context_t(const p_context_t& in_other) = delete;
};
