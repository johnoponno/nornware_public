#include "stdafx.h"
#include "p_context.h"

#define SANITY 0

#include "../microlib/fs.h"
#include "c_std.h"
#include "c_vector.h"

struct fs_parse_tool_t
{
	explicit fs_parse_tool_t(const char* in_it)
	{
		::memset(&_last_read_line, 0, sizeof(_last_read_line));
		_it = in_it;
	}

	bool read_line(const char* in_eol, const bool in_strip_comments = true)
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

	bool end_of_file_eh() const
	{
		return nullptr == _it;
	}

	char _last_read_line[1 << 16];
	const char* _it;

private:

	explicit fs_parse_tool_t(const fs_parse_tool_t& other) = delete;
};

struct c_file_writer_t
{
	explicit c_file_writer_t()
	{
		_handle = nullptr;
	}

	~c_file_writer_t()
	{
		close();
	}

	bool open_text(const char* in_filename)
	{
		if (_open(in_filename, "wt"))
		{
			assert(_handle);
			return true;
		}

		return false;
	}

	void close()
	{
		if (_handle)
		{
			::fclose(_handle);
			_handle = nullptr;
		}
	}

	::FILE* _handle;

private:

	bool _open(const char* in_filename, const char* in_mode)
	{
		close();

		if (0 == ::fopen_s(&_handle, in_filename, in_mode))
		{
			assert(_handle);
			return true;
		}

		return false;
	}
};

struct p_instance_t
{
	std::map<uint32_t, c_path_t> members;
};

struct p_kind_t
{
	explicit p_kind_t::p_kind_t()
	{
	}

	~p_kind_t()
	{
		c_std_map_delete_all_and_clear(instances);
	}

	std::map<uint32_t, p_instance_t*> instances;

private:

	explicit p_kind_t(const p_kind_t& in_other) = delete;
};

static void __write(
	const std::map<c_path_t, uint32_t>& in, const char* in_prefix,
	c_file_writer_t& out)
{
	//DO NOT write in key order, as the key is the name, that will result in alphanumeric order instead of "submission" order which is what we want
	if (in.size())
	{
		//in code order...
		std::map<uint32_t, c_path_t> sub;

		{
			auto i = in.cbegin();
			while (in.cend() != i)
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

static uint32_t __decode(const std::map<c_path_t, uint32_t>& in, const c_path_t& in_name)
{
	auto i = in.find(in_name);
	if (in.cend() != i)
		return i->second;

	return UINT32_MAX;
}

static uint32_t __encode(
	const char* in_name,
	std::map<c_path_t, uint32_t>& out)
{
	const c_path_t NAME(in_name);

	//already encoded?
	const uint32_t CODE = __decode(out, NAME);
	if (UINT32_MAX != CODE)
		return CODE;

	//add
	auto i = out.insert({ NAME, out.size() });
	assert(i.second);
	if (i.second)
		return i.first->second;

	return UINT32_MAX;
}

static const c_path_t* __member_read(const p_instance_t& in, const uint32_t in_member_key)
{
	auto find = in.members.find(in_member_key);
	if (in.members.cend() == find)
		return nullptr;
	return &find->second;
}

static bool __pull_string(
	const p_instance_t& in, const uint32_t in_member_key,
	c_path_t& out_value)
{
	const c_path_t* MEMBER = __member_read(in, in_member_key);
	if (MEMBER)
		out_value = *MEMBER;
	return nullptr != MEMBER;
}

static const c_path_t* __member_read(const p_kind_t& in, const uint32_t in_instance_key, const uint32_t in_member_key)
{
	auto find = in.instances.find(in_instance_key);
	if (in.instances.cend() == find)
		return nullptr;
	return __member_read(*find->second, in_member_key);
}

static bool __pull_string(
	const p_kind_t& in, const uint32_t in_instance_key, const uint32_t in_member_key,
	c_path_t& out_value)
{
	auto find = in.instances.find(in_instance_key);
	if (in.instances.cend() == find)
		return false;
	return __pull_string(*find->second, in_member_key, out_value);
}

static int32_t __pull_int32(const p_kind_t& in, const uint32_t in_instance_key, const uint32_t in_member_key, const int32_t in_default)
{
	const c_path_t* MEMBER = __member_read(in, in_instance_key, in_member_key);
	if (!MEMBER)
		return in_default;

	return ::atoi(MEMBER->buffer);
}

static c_path_t& __member_write(
	const uint32_t in_instance_key, const uint32_t in_member_key,
	p_kind_t& out)
{
	auto find = out.instances.find(in_instance_key);
	if (out.instances.cend() != find)
		return find->second->members[in_member_key];

	p_instance_t* instance = new p_instance_t;
	assert(instance);
	out.instances.insert({ in_instance_key, instance });
	return instance->members[in_member_key];
}

static void __push_string(
	const uint32_t in_instance_key, const uint32_t in_member_key, const char* in_value,
	p_kind_t& out)
{
	__member_write(in_instance_key, in_member_key, out) = in_value;
}

static void __push_int32(
	const uint32_t in_instance_key, const uint32_t in_member_key, const int32_t in_value,
	p_kind_t& out)
{
	__member_write(in_instance_key, in_member_key, out).format("%d", in_value);
}

static void __push_uint32(
	const uint32_t in_instance_key, const uint32_t in_member_key, const uint32_t in_value,
	p_kind_t& out)
{
	__member_write(in_instance_key, in_member_key, out).format("%u", in_value);
}

static p_kind_t* __kind_write(
	const uint32_t in_kind_code,
	p_context_t& out)
{
	auto find = out._kinds.find(in_kind_code);
	if (out._kinds.cend() != find)
		return find->second;

	p_kind_t* kind = new p_kind_t;
	assert(kind);
	if (!kind)
		return nullptr;
	out._kinds.insert({ in_kind_code, kind });
	return kind;
}

static void __clear(p_context_t& out)
{
	c_std_map_delete_all_and_clear(out._kinds);
	out._kind_names.clear();
	out._member_names.clear();
}

static const p_kind_t* __kind_read(const p_context_t& in, const char* in_kind)
{
	const uint32_t CODE = __decode(in._kind_names, c_path_t(in_kind));
	auto find = in._kinds.find(CODE);
	if (in._kinds.cend() == find)
		return nullptr;
	return find->second;
}

static uint32_t __pull_uint32(const p_kind_t& in, const uint32_t in_instance_key, const uint32_t in_member_key, const uint32_t in_default)
{
	const c_path_t* MEMBER = __member_read(in, in_instance_key, in_member_key);
	if (!MEMBER)
		return in_default;

	return ::strtoul(MEMBER->buffer, nullptr, 10);
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
	__clear(*this);
}

void p_context_t::pull_string(
	const char* in_kind, const uint32_t in_instance_key, const char* in_member, const char* in_default,
	c_path_t& out_result)
{
	const p_kind_t* KIND = __kind_read(*this, in_kind);
	if (KIND)
	{
		if (__pull_string(*KIND, in_instance_key, __encode(in_member, _member_names), out_result))
			return;
	}

	out_result = in_default;
}

void p_context_t::push_string(const char* in_kind, const uint32_t in_instance_key, const char* in_member, const char* in_value)
{
	p_kind_t* kind = __kind_write(__encode(in_kind, _kind_names), *this);
	assert(kind);
	__push_string(in_instance_key, __encode(in_member, _member_names), in_value, *kind);
}

int32_t p_context_t::pull_int32(const char* in_kind, const uint32_t in_instance_key, const char* in_member, const int32_t in_default)
{
	const p_kind_t* KIND = __kind_read(*this, in_kind);
	if (!KIND)
		return in_default;

	return __pull_int32(*KIND, in_instance_key, __encode(in_member, _member_names), in_default);
}

void p_context_t::push_int32(const char* in_kind, const uint32_t in_instance_key, const char* in_member, const int32_t in_value)
{
	p_kind_t* kind = __kind_write(__encode(in_kind, _kind_names), *this);
	assert(kind);
	__push_int32(in_instance_key, __encode(in_member, _member_names), in_value, *kind);
}

uint32_t p_context_t::pull_uint32(const char* in_kind, const uint32_t in_instance_key, const char* in_member, const uint32_t in_default)
{
	const p_kind_t* KIND = __kind_read(*this, in_kind);
	if (!KIND)
		return in_default;

	return __pull_uint32(*KIND, in_instance_key, __encode(in_member, _member_names), in_default);
}

void p_context_t::push_uint32(const char* in_kind, const uint32_t in_instance_key, const char* in_member, const uint32_t in_value)
{
	p_kind_t* kind = __kind_write(__encode(in_kind, _kind_names), *this);
	assert(kind);
	__push_uint32(in_instance_key, __encode(in_member, _member_names), in_value, *kind);
}

bool p_context_t::read(const char* in_file)
{
#if SANITY
	c_deprecated_string_t dbgLine, a, b, v;
#endif

	__clear(*this);

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
						__encode(tok, _kind_names);
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
						__encode(tok, _member_names);
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

							p_kind_t* kind = __kind_write(c, *this);
							assert(kind);
							__member_write(i, m, *kind) = tok;
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
		__write(_kind_names, "kind", handle);

		//membernames
		__write(_member_names, "member", handle);

		//data
		for (const auto& ITR_KIND : _kinds)
		{
			::fprintf(handle._handle, "%d\n", ITR_KIND.first);
			{
				for (const auto& ITR_INSTANCE : ITR_KIND.second->instances)
				{
					::fprintf(handle._handle, "%u", ITR_INSTANCE.first);
					for (const auto& ITR_MEMBER : ITR_INSTANCE.second->members)
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
