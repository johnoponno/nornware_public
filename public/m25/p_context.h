#pragma once

#include "c_fixed_string.h"

struct c_file_writer_t;
struct c_vec2i_t;
struct c_vec2ui_t;
struct c_vec2f_t;
struct c_vec3i_t;
struct c_vec3f_t;

struct p_member_value_t;

struct p_names_t
{
	uint32_t encode(const char* in_name);
	uint32_t decode(const c_path_t& in_name) const;
	void write(const char* in_prefix, c_file_writer_t& out) const;

	//NOTE: choice of which is key vs value favors fast encoding over decoding (map a string name to an index, happens every load...)
	std::map<c_path_t, uint32_t> _names;
};

struct p_instance_t
{
	bool pull_string(const uint32_t in_member_key, c_path_t& out_value) const;
	const c_path_t* member_read(const uint32_t in_member_key) const;

	std::map<uint32_t, c_path_t> _members;
};

struct p_kind_t
{
	explicit p_kind_t();
	~p_kind_t();

	void push_vec3f(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec3f_t& in_value);
	void push_vec3i(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec3i_t& in_value);
	void push_vec2f(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec2f_t& in_value);
	void push_vec2ui(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec2ui_t& value);
	void push_vec2i(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec2i_t& in_value);
	void push_float(const uint32_t in_instance_key, const uint32_t in_member_key, const float in_value);
	void push_uint32(const uint32_t in_instance_key, const uint32_t in_member_key, const uint32_t in_value);
	void push_int32(const uint32_t in_instance_key, const uint32_t in_member_key, const int32_t in_value);
	void push_string(const uint32_t in_instance_key, const uint32_t in_member_key, const char* in_value);
	c_path_t& member_write(const uint32_t in_instance_key, const uint32_t in_member_key);

	c_vec3f_t pull_vec3f(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec3f_t& in_default) const;
	c_vec3i_t pull_vec3i(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec3i_t& in_default) const;
	c_vec2f_t pull_vec2f(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec2f_t& in_default) const;
	c_vec2ui_t pull_vec2ui(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec2ui_t& in_default) const;
	c_vec2i_t pull_vec2i(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec2i_t& in_default) const;
	float pull_float(const uint32_t in_instance_key, const uint32_t in_member_key, const float in_default) const;
	uint32_t pull_uint32(const uint32_t in_instance_key, const uint32_t in_member_key, const uint32_t in_default) const;
	int32_t pull_int32(const uint32_t in_instance_key, const uint32_t in_member_key, const int32_t in_default) const;
	bool pull_string(const uint32_t in_instance_key, const uint32_t in_member_key, c_path_t& out_value) const;
	void get_all_keys(std::set<uint32_t>& out) const;

	std::map<uint32_t, p_instance_t*> _instances;

private:

	explicit p_kind_t(const p_kind_t& in_other) = delete;
	const c_path_t* _member_read(const uint32_t in_instance_key, const uint32_t in_member_key) const;
};

//2024-07-08: pulls are NOT const since member names need to be encoded
struct p_context_t
{
	explicit p_context_t();
	~p_context_t();

	void clear();
	bool read(const char* in_file);
	void pull_string(const char* in_kind, const uint32_t in_instance, const char* in_member, const char* in_default, c_path_t& out_result);
	void push_string(const char* in_kind, const uint32_t in_instance, const char* in_member, const char* in_value);
	int32_t pull_int32(const char* in_kind, const uint32_t in_instance, const char* in_member, const int32_t in_default);
	void push_int32(const char* in_kind, const uint32_t in_instance, const char* in_member, const int32_t in_value);
	uint32_t pull_uint32(const char* in_kind, const uint32_t in_instance, const char* in_member, const uint32_t in_default);
	void push_uint32(const char* in_kind, const uint32_t in_instance, const char* in_member, const uint32_t in_value);
	float pull_float(const char* in_kind, const uint32_t in_instance, const char* in_member, const float in_default);
	void push_float(const char* in_kind, const uint32_t in_instance, const char* in_member, const float in_value);
	c_vec2i_t pull_vec2i(const char* in_kind, const uint32_t in_instance, const char* in_member, const c_vec2i_t& in_default);
	void push_vec2i(const char* in_kind, const uint32_t in_instance, const char* in_member, const c_vec2i_t& in_value);
	c_vec2ui_t pull_vec2ui(const char* in_kind, const uint32_t in_instance, const char* in_member, const c_vec2ui_t& in_default);
	void push_vec2ui(const char* in_kind, const uint32_t in_instance, const char* in_member, const c_vec2ui_t& in_value);
	c_vec2f_t pull_vec2f(const char* in_kind, const uint32_t in_instance, const char* in_member, const c_vec2f_t& in_default);
	void push_vec2f(const char* in_kind, const uint32_t in_instance, const char* in_member, const c_vec2f_t& in_value);
	c_vec3i_t pull_vec3i(const char* in_kind, const uint32_t in_instance, const char* in_member, const c_vec3i_t& in_default);
	void push_vec3i(const char* in_kind, const uint32_t in_instance, const char* in_member, const c_vec3i_t& in_value);
	c_vec3f_t pull_vec3f(const char* in_kind, const uint32_t in_instance, const char* in_member, const c_vec3f_t& in_default);
	void push_vec3f(const char* in_kind, const uint32_t in_instance, const char* in_member, const c_vec3f_t& in_value);
	p_kind_t* kind_write(const uint32_t in_kind_code);

	void get_all_keys(const char* in_kind, std::set<uint32_t>& out_keys) const;
	void get_member_values_by_substring(const char* in_substring, std::vector<p_member_value_t>& out_member_values) const;
	bool write(const char* in_file) const;
	const p_kind_t* kind_read(const char* in_kind) const;

	std::map<uint32_t, p_kind_t*> _kinds;
	p_names_t _kind_names;
	p_names_t _member_names;

private:

	explicit p_context_t(const p_context_t& in_other) = delete;
};
