#pragma once

template <typename vector_t, typename type_t>
bool c_std_vector_contains(const vector_t& aVector, const type_t& aValue)
{
	const type_t* P = aVector.data();
	const type_t* END = P + aVector.size();

	while (P < END)
	{
		if (aValue == *P)
			return true;
		++P;
	}

	return false;
}

template <typename container_t>
void c_std_vector_set_deque_delete_all_and_clear(container_t& aContainer)
{
	if (aContainer.size() > 0)
	{
		for (const auto& I : aContainer)
			delete I;
		aContainer.clear();
	}
}

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
