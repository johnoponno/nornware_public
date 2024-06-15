#include "stdafx.h"
#include "minyin.h"

bool minyin_t::key_is_down(const int32_t in_key) const
{
	if (in_key >= 0 && in_key < _countof(_keys))
		return _keys[in_key].down_current;
	return false;
}

bool minyin_t::key_downflank(const int32_t in_key) const
{
	if (in_key >= 0 && in_key < _countof(_keys))
		return _keys[in_key].down_current && !_keys[in_key].down_last;
	return false;
}
