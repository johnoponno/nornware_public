#pragma once

#include <vector>
#include <dsound.h>
#include "includes.h"

template <typename vector_t, typename type_t>
bool vector_contains(const vector_t& aVector, const type_t& aValue)
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
void vector_set_delete_all_and_clear(container_t& aContainer)
{
	if (aContainer.size() > 0)
	{
		for (const auto& I : aContainer)
			delete I;
		aContainer.clear();
	}
}
