#include "stdafx.h"
#include "m_mutable.h"

#include "m_work.h"

namespace mlm
{
	m_mutable_t::m_mutable_t()
	{
		start_time = 0.f;
		exit_time = 0.f;

		m_character_restart({ 64.f, 64.f }, *this);

		for (uint8_t& v : world_visited)
			v = MLM_M_VISITED_NOT;

		checkpoint_offset = 0;
	}

}
