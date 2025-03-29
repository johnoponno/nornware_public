#pragma once

#include "m_immutable.h"
#include "m_mutable.h"
#include "vc_assets.h"
#include "vc_fatpack.h"
#include "d_dev.h"

struct micron_t;

namespace mlm
{
	struct game_t
	{
		bool init(micron_t& out_micron);
		bool tick(micron_t& out_micron);
		void shutdown();

		m_immutable_t _im;
		m_mutable_t _mu;
		vc_assets_t _assets;
		vc_fatpack_t _fatpack;
		d_dev_t _dev;
	};
}
