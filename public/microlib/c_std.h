#pragma once

template <typename map_t>
void c_std_map_delete_all_and_clear(map_t& aMap)
{
	if (aMap.size() > 0)
	{
		for (const auto& I : aMap)
			delete I.second;
		aMap.clear();
	}
}
