#include "stdafx.h"
#include "m_immutable.h"

namespace mlm
{
	m_immutable_t::m_immutable_t()
	{
		for (uint32_t& ti : tile_type)
			ti = M_TYPE_SOLID;

		for (
			uint32_t i = 0;
			i < M_NUM_LOGIC;
			++i
			)
			logic_indices[i] = M_LOGIC_INFO[i].default;

		for (uint16_t& t : world_tiles)
			t = 0;
	}
}
