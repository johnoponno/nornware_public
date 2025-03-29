#pragma once

struct fs_parse_tool_t
{
	explicit fs_parse_tool_t(const char* in_it);

	bool read_line(const char* in_eol, const bool in_strip_comments = true);
	bool end_of_file_eh() const;

	char _last_read_line[1 << 16];
	const char* _it;

private:

	explicit fs_parse_tool_t(const fs_parse_tool_t& other) = delete;
};
