#include "stdafx.h"
#include "p_context.h"

#define SANITY 0

#include "../microlib/fs.h"
#include "c_file_writer.h"
#include "c_std.h"
#include "c_vector.h"
#include "fs_parse_tool.h"
#include "p_util.h"

//p_names_t
//p_names_t
//p_names_t
//p_names_t

uint32_t p_names_t::encode(const char* in_name)
{
	const c_path_t NAME(in_name);

	//already encoded?
	const uint32_t CODE = decode(NAME);
	if (UINT32_MAX != CODE)
		return CODE;

	//add
	auto i = _names.insert({ NAME, _names.size() });
	assert(i.second);
	if (i.second)
		return i.first->second;

	return UINT32_MAX;
}

uint32_t p_names_t::decode(const c_path_t& in_name) const
{
	auto i = _names.find(in_name);
	if (_names.cend() != i)
		return i->second;

	return UINT32_MAX;
}

void p_names_t::write(
	const char* in_prefix,
	c_file_writer_t& out) const
{
	//DO NOT write in key order, as the key is the name, that will result in alphanumeric order instead of "submission" order which is what we want
	if (_names.size())
	{
		//in code order...
		std::map<uint32_t, c_path_t> sub;

		{
			auto i = _names.cbegin();
			while (_names.cend() != i)
			{
				sub.insert({ i->second, i->first });
				++i;
			}
		}

		{
			auto i = sub.cbegin();
			while (i != sub.cend())
			{
				::fprintf(out._handle, "%s\t%s\n", in_prefix, i->second.buffer);
				++i;
			}
		}
	}
}

//p_instance_t
//p_instance_t
//p_instance_t
//p_instance_t

bool p_instance_t::pull_string(
	const uint32_t in_member_key,
	c_path_t& out_value) const
{
	const c_path_t* MEMBER = member_read(in_member_key);
	if (MEMBER)
		out_value = *MEMBER;
	return nullptr != MEMBER;
}

const c_path_t* p_instance_t::member_read(const uint32_t in_member_key) const
{
	auto find = _members.find(in_member_key);
	if (_members.cend() == find)
		return nullptr;
	return &find->second;
}

//p_kind_t
//p_kind_t
//p_kind_t
//p_kind_t

p_kind_t::p_kind_t()
{
}

p_kind_t::~p_kind_t()
{
	c_std_map_delete_all_and_clear(_instances);
}

void p_kind_t::get_all_keys(std::set<uint32_t>& out) const
{
	for (const auto& ITR_INSTANCE : _instances)
		out.insert(ITR_INSTANCE.first);
}

bool p_kind_t::pull_string(
	const uint32_t in_instance_key, const uint32_t in_member_key,
	c_path_t& out_value) const
{
	auto find = _instances.find(in_instance_key);
	if (_instances.cend() == find)
		return false;
	return find->second->pull_string(in_member_key, out_value);
}

void p_kind_t::push_string(const uint32_t in_instance_key, const uint32_t in_member_key, const char* in_value)
{
	member_write(in_instance_key, in_member_key) = in_value;
}

int32_t p_kind_t::pull_int32(const uint32_t in_instance_key, const uint32_t in_member_key, const int32_t in_default) const
{
	const c_path_t* MEMBER = _member_read(in_instance_key, in_member_key);
	if (!MEMBER)
		return in_default;

	return ::atoi(MEMBER->buffer);
}

void p_kind_t::push_int32(const uint32_t in_instance_key, const uint32_t in_member_key, const int32_t in_value)
{
	member_write(in_instance_key, in_member_key).format("%d", in_value);
}

uint32_t p_kind_t::pull_uint32(const uint32_t in_instance_key, const uint32_t in_member_key, const uint32_t in_default) const
{
	const c_path_t* MEMBER = _member_read(in_instance_key, in_member_key);
	if (!MEMBER)
		return in_default;

	return ::strtoul(MEMBER->buffer, nullptr, 10);
}

void p_kind_t::push_uint32(const uint32_t in_instance_key, const uint32_t in_member_key, const uint32_t in_value)
{
	member_write(in_instance_key, in_member_key).format("%u", in_value);
}

float p_kind_t::pull_float(const uint32_t in_instance_key, const uint32_t in_member_key, const float in_default) const
{
	const c_path_t* MEMBER = _member_read(in_instance_key, in_member_key);
	if (!MEMBER)
		return in_default;

	return (float)::atof(MEMBER->buffer);
}

void p_kind_t::push_float(const uint32_t in_instance_key, const uint32_t in_member_key, const float in_value)
{
	member_write(in_instance_key, in_member_key).format("%f", in_value);
}

c_vec2i_t p_kind_t::pull_vec2i(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec2i_t& in_default) const
{
	const c_path_t* MEMBER = _member_read(in_instance_key, in_member_key);
	if (!MEMBER)
		return in_default;

	c_vec2i_t result;
	{
		char* end = nullptr;
		result.x = ::strtol(MEMBER->buffer, &end, 10);
		result.y = ::strtol(end, &end, 10);
	}

	return result;
}

void p_kind_t::push_vec2i(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec2i_t& in_value)
{
	member_write(in_instance_key, in_member_key).format("%d %d", in_value.x, in_value.y);
}

c_vec2ui_t p_kind_t::pull_vec2ui(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec2ui_t& in_default) const
{
	const c_path_t* MEMBER = _member_read(in_instance_key, in_member_key);
	if (!MEMBER)
		return in_default;

	c_vec2ui_t result;
	{
		char* end = nullptr;
		result.x = ::strtoul(MEMBER->buffer, &end, 10);
		result.y = ::strtoul(end, &end, 10);
	}

	return result;
}

void p_kind_t::push_vec2ui(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec2ui_t& value)
{
	member_write(in_instance_key, in_member_key).format("%u %u", value.x, value.y);
}

c_vec2f_t p_kind_t::pull_vec2f(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec2f_t& in_default) const
{
	const c_path_t* MEMBER = _member_read(in_instance_key, in_member_key);
	if (!MEMBER)
		return in_default;

	c_vec2f_t result;
	{
		char* end = nullptr;
		result.x = (float)::strtod(MEMBER->buffer, &end);
		result.y = (float)::strtod(end, &end);
	}

	return result;
}

void p_kind_t::push_vec2f(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec2f_t& in_value)
{
	member_write(in_instance_key, in_member_key).format("%f %f", in_value.x, in_value.y);
}

c_vec3i_t p_kind_t::pull_vec3i(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec3i_t& in_default) const
{
	const c_path_t* MEMBER = _member_read(in_instance_key, in_member_key);
	if (!MEMBER)
		return in_default;

	c_vec3i_t result;
	{
		char* end = nullptr;
		result.x = ::strtol(MEMBER->buffer, &end, 10);
		result.y = ::strtol(end, &end, 10);
		result.z = ::strtol(end, &end, 10);
	}

	return result;
}

void p_kind_t::push_vec3i(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec3i_t& in_value)
{
	member_write(in_instance_key, in_member_key).format("%d %d %d", in_value.x, in_value.y, in_value.z);
}

c_vec3f_t p_kind_t::pull_vec3f(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec3f_t& in_default) const
{
	const c_path_t* MEMBER = _member_read(in_instance_key, in_member_key);
	if (!MEMBER)
		return in_default;

	c_vec3f_t result;
	{
		char* end = nullptr;
		result.x = (float)::strtod(MEMBER->buffer, &end);
		result.y = (float)::strtod(end, &end);
		result.z = (float)::strtod(end, &end);
	}

	return result;
}

void p_kind_t::push_vec3f(const uint32_t in_instance_key, const uint32_t in_member_key, const c_vec3f_t& in_value)
{
	member_write(in_instance_key, in_member_key).format("%f %f %f", in_value.x, in_value.y, in_value.z);
}

c_path_t& p_kind_t::member_write(const uint32_t in_instance_key, const uint32_t in_member_key)
{
	auto find = _instances.find(in_instance_key);
	if (_instances.cend() != find)
		return find->second->_members[in_member_key];

	p_instance_t* instance = new p_instance_t;
	assert(instance);
	_instances.insert({ in_instance_key, instance });
	return instance->_members[in_member_key];
}

const c_path_t* p_kind_t::_member_read(const uint32_t in_instance_key, const uint32_t in_member_key) const
{
	auto find = _instances.find(in_instance_key);
	if (_instances.cend() == find)
		return nullptr;
	return find->second->member_read(in_member_key);
}

//public
//public
//public
//public

p_context_t::p_context_t()
{
}

p_context_t::~p_context_t()
{
	clear();
}

const p_kind_t* p_context_t::kind_read(const char* in_kind) const
{
	const uint32_t CODE = _kind_names.decode(c_path_t(in_kind));
	auto find = _kinds.find(CODE);
	if (_kinds.cend() == find)
		return nullptr;
	return find->second;
}

void p_context_t::clear()
{
	c_std_map_delete_all_and_clear(_kinds);
	_kind_names._names.clear();
	_member_names._names.clear();
}

p_kind_t* p_context_t::kind_write(const uint32_t in_kind_code)
{
	auto find = _kinds.find(in_kind_code);
	if (_kinds.cend() != find)
		return find->second;

	p_kind_t* kind = new p_kind_t;
	assert(kind);
	if (!kind)
		return nullptr;
	_kinds.insert({ in_kind_code, kind });
	return kind;
}

void p_context_t::get_all_keys(
	const char* in_kind,
	std::set<uint32_t>& out_keys) const
{
	out_keys.clear();
	const p_kind_t* KIND = kind_read(in_kind);
	if (KIND)
		KIND->get_all_keys(out_keys);

	//C_LOG("found %u instances of kind %s", out_keys.size(), in_kind);
}

//string
void p_context_t::pull_string(
	const char* in_kind, const uint32_t in_instance_key, const char* in_member, const char* in_default,
	c_path_t& out_result)
{
	const p_kind_t* KIND = kind_read(in_kind);
	if (KIND)
	{
		if (KIND->pull_string(in_instance_key, _member_names.encode(in_member), out_result))
			return;
	}

	out_result = in_default;
}

void p_context_t::push_string(const char* in_kind, const uint32_t in_instance_key, const char* in_member, const char* in_value)
{
	p_kind_t* kind = kind_write(_kind_names.encode(in_kind));
	assert(kind);
	kind->push_string(in_instance_key, _member_names.encode(in_member), in_value);
}

//int32_t
int32_t p_context_t::pull_int32(const char* in_kind, const uint32_t in_instance_key, const char* in_member, const int32_t in_default)
{
	const p_kind_t* KIND = kind_read(in_kind);
	if (!KIND)
		return in_default;

	return KIND->pull_int32(in_instance_key, _member_names.encode(in_member), in_default);
}

void p_context_t::push_int32(const char* in_kind, const uint32_t in_instance_key, const char* in_member, const int32_t in_value)
{
	p_kind_t* kind = kind_write(_kind_names.encode(in_kind));
	assert(kind);
	kind->push_int32(in_instance_key, _member_names.encode(in_member), in_value);
}

//uint32
uint32_t p_context_t::pull_uint32(const char* in_kind, const uint32_t in_instance_key, const char* in_member, const uint32_t in_default)
{
	const p_kind_t* KIND = kind_read(in_kind);
	if (!KIND)
		return in_default;

	return KIND->pull_uint32(in_instance_key, _member_names.encode(in_member), in_default);
}

void p_context_t::push_uint32(const char* in_kind, const uint32_t in_instance_key, const char* in_member, const uint32_t in_value)
{
	p_kind_t* kind = kind_write(_kind_names.encode(in_kind));
	assert(kind);
	kind->push_uint32(in_instance_key, _member_names.encode(in_member), in_value);
}

//float
float p_context_t::pull_float(const char* in_kind, const uint32_t in_instance_key, const char* in_member, const float in_default)
{
	const p_kind_t* KIND = kind_read(in_kind);
	if (!KIND)
		return in_default;

	return KIND->pull_float(in_instance_key, _member_names.encode(in_member), in_default);
}

void p_context_t::push_float(const char* in_kind, const uint32_t in_instance_key, const char* in_member, const float in_value)
{
	p_kind_t* kind = kind_write(_kind_names.encode(in_kind));
	assert(kind);
	kind->push_float(in_instance_key, _member_names.encode(in_member), in_value);
}

//vector2i
c_vec2i_t p_context_t::pull_vec2i(const char* in_kind, const uint32_t in_instance_key, const char* in_member, const c_vec2i_t& in_default)
{
	const p_kind_t* KIND = kind_read(in_kind);
	if (!KIND)
		return in_default;

	return KIND->pull_vec2i(in_instance_key, _member_names.encode(in_member), in_default);
}

void p_context_t::push_vec2i(const char* in_kind, const uint32_t in_instance_key, const char* in_member, const c_vec2i_t& in_value)
{
	p_kind_t* kind = kind_write(_kind_names.encode(in_kind));
	assert(kind);
	kind->push_vec2i(in_instance_key, _member_names.encode(in_member), in_value);
}

//vector2ui
c_vec2ui_t p_context_t::pull_vec2ui(const char* in_kind, const uint32_t in_instance_key, const char* in_member, const c_vec2ui_t& in_default)
{
	const p_kind_t* KIND = kind_read(in_kind);
	if (!KIND)
		return in_default;

	return KIND->pull_vec2ui(in_instance_key, _member_names.encode(in_member), in_default);
}

void p_context_t::push_vec2ui(const char* in_kind, const uint32_t in_instance_key, const char* in_member, const c_vec2ui_t& in_value)
{
	p_kind_t* kind = kind_write(_kind_names.encode(in_kind));
	assert(kind);
	kind->push_vec2ui(in_instance_key, _member_names.encode(in_member), in_value);
}

//vector2f
c_vec2f_t p_context_t::pull_vec2f(const char* in_kind, const uint32_t in_instance, const char* in_member, const c_vec2f_t& in_default)
{
	const p_kind_t* KIND = kind_read(in_kind);
	if (!KIND)
		return in_default;

	return KIND->pull_vec2f(in_instance, _member_names.encode(in_member), in_default);
}

void p_context_t::push_vec2f(const char* in_kind, const uint32_t in_instance, const char* in_member, const c_vec2f_t& in_value)
{
	p_kind_t* kind = kind_write(_kind_names.encode(in_kind));
	assert(kind);
	kind->push_vec2f(in_instance, _member_names.encode(in_member), in_value);
}

//vector3i
c_vec3i_t p_context_t::pull_vec3i(const char* in_kind, const uint32_t in_instance, const char* in_member, const c_vec3i_t& in_default)
{
	const p_kind_t* KIND = kind_read(in_kind);
	if (!KIND)
		return in_default;

	return KIND->pull_vec3i(in_instance, _member_names.encode(in_member), in_default);
}

void p_context_t::push_vec3i(const char* in_kind, const uint32_t in_instance, const char* in_member, const c_vec3i_t& in_value)
{
	p_kind_t* kind = kind_write(_kind_names.encode(in_kind));
	assert(kind);
	kind->push_vec3i(in_instance, _member_names.encode(in_member), in_value);
}

//vector3f
c_vec3f_t p_context_t::pull_vec3f(const char* in_kind, const uint32_t in_instance, const char* in_member, const c_vec3f_t& in_default)
{
	const p_kind_t* KIND = kind_read(in_kind);
	if (!KIND)
		return in_default;

	return KIND->pull_vec3f(in_instance, _member_names.encode(in_member), in_default);
}

void p_context_t::push_vec3f(const char* in_kind, const uint32_t in_instance, const char* in_member, const c_vec3f_t& in_value)
{
	p_kind_t* kind = kind_write(_kind_names.encode(in_kind));
	assert(kind);
	kind->push_vec3f(in_instance, _member_names.encode(in_member), in_value);
}

bool p_context_t::read(const char* in_file)
{
#if SANITY
	c_deprecated_string_t dbgLine, a, b, v;
#endif

	clear();

	const c_blob_t CONTENTS = fs_file_contents_null_terminated(in_file);
	if (CONTENTS.data)
	{
		uint32_t first_token_parsed = 0;
		fs_parse_tool_t tool((char*)CONTENTS.data);
		char* tok;
		char* ctx = nullptr;
		int32_t c = -1;
		uint32_t i;
		uint32_t m;

		while (!tool.end_of_file_eh())
		{
			if (tool.read_line("\r\n"))
			{
#if SANITY
				dbgLine = line;
				a = core::stringutil::leftPart(dbgLine, '\t');
#endif
				tok = ::strtok_s(tool._last_read_line, "\t", &ctx);

				//first token must be "kind" for this to be a proper text context
				if (0 == first_token_parsed)
				{
					if (0 != ::strcmp("kind", tok))
					{
						delete[] CONTENTS.data;
						return false;
					}
					first_token_parsed = 1;
				}
#if SANITY
				assert(a == tok);
#endif
				//not in struct
				if (c == -1)
				{
					if (!::strcmp(tok, "kind"))
					{
#if SANITY
						b = core::stringutil::rightPart(dbgLine, '\t');
#endif
						tok = ::strtok_s(nullptr, "\t", &ctx);
#if SANITY
						assert(b == tok);
#endif
						_kind_names.encode(tok);
				}
					else if (!::strcmp(tok, "member"))
					{
#if SANITY
						b = core::stringutil::rightPart(dbgLine, '\t');
#endif
						tok = ::strtok_s(nullptr, "\t", &ctx);
#if SANITY
						assert(b == tok);
#endif
						_member_names.encode(tok);
				}
					else
					{
						//begin struct
						c = ::atoi(tok);
#if SANITY
						assert(::atoi(a) == c);
#endif
					}
		}
				//in struct
				else
				{
					//instances / members
					if (::strstr(ctx, "\t"))
					{
#if SANITY
						assert(dbgLine.find('\t') > -1);
#endif
						//instance
						i = ::atoi(tok);
#if SANITY
						assert(::atoi(a) == i);
						b = core::stringutil::rightPart(dbgLine, '\t');
						assert(b == ctx);
#endif
						//all members
						while (::strlen(ctx))
						{
#if SANITY
							assert(b.length());
#endif
							tok = ::strtok_s(nullptr, "\t", &ctx);
							assert(tok);
							m = ::atoi(tok);
#if SANITY
							assert(::atoi(core::stringutil::leftPart(b, '\t')) == m);
							b = core::stringutil::rightPart(b, '\t');
#endif
							//handle empty values
							if (*ctx == '\t')
								tok = nullptr;
							else
								tok = ::strtok_s(nullptr, "\t", &ctx);
							if (!tok)
								tok = "";
#if SANITY
							v = core::stringutil::leftPart(b, '\t');
							b = core::stringutil::rightPart(b, '\t');
							assert(v == tok);
#endif

							p_kind_t* kind = kind_write(c);
							assert(kind);
							kind->member_write(i, m) = tok;
				}
	}
					//next struct
					else
					{
						//next struct
						c = ::atoi(tok);
#if SANITY
						assert(::atoi(a) == c);
#endif
					}
				}
			}
		}

		delete[] CONTENTS.data;

		return true;
	}

	return false;
}

bool p_context_t::write(const char* in_file) const
{
	c_file_writer_t handle;

	//write even if there are no objects, as there might have been before and now we want to delete them...
	if (handle.open_text(in_file))
	{
		//classnames
		_kind_names.write("kind", handle);

		//membernames
		_member_names.write("member", handle);

		//data
		for (const auto& ITR_KIND : _kinds)
		{
			::fprintf(handle._handle, "%d\n", ITR_KIND.first);
			{
				for (const auto& ITR_INSTANCE : ITR_KIND.second->_instances)
				{
					::fprintf(handle._handle, "%u", ITR_INSTANCE.first);
					for (const auto& ITR_MEMBER : ITR_INSTANCE.second->_members)
						::fprintf(handle._handle, "\t%u\t%s", ITR_MEMBER.first, ITR_MEMBER.second.buffer);
					::fprintf(handle._handle, "\n");
				}
			}
		}

		handle.close();

		return true;
	}

	return false;
}

void p_context_t::get_member_values_by_substring(
	const char* in_substring,
	std::vector<p_member_value_t>& out_member_values) const
{
	out_member_values.clear();

	for (const auto& ITR_KIND : _kinds)
	{
		//try to pull all asset members from all classes (as we don't know which ones include them)
		for (const auto& ITR_INSTANCE : ITR_KIND.second->_instances)
		{
			//try to pull from every instance, as we don't know which ones inclue the members in question
			p_member_value_t member_value;
			for (const auto& MEMBER_NAME : _member_names._names)
			{
				if (::strstr(MEMBER_NAME.first.buffer, in_substring))
				{
					if (ITR_INSTANCE.second->pull_string(MEMBER_NAME.second, member_value.y) && ::strlen(member_value.y.buffer))
					{
						member_value.x = MEMBER_NAME.first;
						out_member_values.push_back(member_value);
					}
				}
			}
		}
	}
}
