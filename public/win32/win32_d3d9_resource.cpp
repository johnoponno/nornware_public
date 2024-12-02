#include "stdafx.h"
#include "win32_d3d9_resource.h"

#include "win32_d3d9_state.h"

static win32_d3d9_resource_t* __first = nullptr;

::HRESULT CALLBACK win32_d3d9_resource_callback_on_create_device()
{
	win32_d3d9_resource_t* resource = __first;
	while (resource)
	{
		if (!resource->win32_d3d9_resource_on_create_device())
			return E_FAIL;
		resource = resource->_win32_d3d9_resource_next;
	}

	return S_OK;
}

::HRESULT CALLBACK win32_d3d9_resource_callback_on_reset_device()
{
	win32_d3d9_resource_t* resource = __first;
	while (resource)
	{
		if (!resource->win32_d3d9_resource_on_reset_device())
			return E_FAIL;
		resource = resource->_win32_d3d9_resource_next;
	}

	return S_OK;
}

void CALLBACK win32_d3d9_resource_callback_on_destroy_device()
{
	win32_d3d9_resource_t* resource = __first;
	while (resource)
	{
		resource->win32_d3d9_resource_on_destroy_device();
		resource = resource->_win32_d3d9_resource_next;
	}
}

void CALLBACK win32_d3d9_resource_callback_on_lost_device()
{
	win32_d3d9_resource_t* resource = __first;
	while (resource)
	{
		resource->win32_d3d9_resource_on_lost_device();
		resource = resource->_win32_d3d9_resource_next;
	}
}

bool win32_d3d9_resource_t::win32_d3d9_resource_on_create_device()
{
	return true;
}

void win32_d3d9_resource_t::win32_d3d9_resource_on_destroy_device()
{
}

void win32_d3d9_resource_t::win32_d3d9_resource_on_lost_device()
{
}

bool win32_d3d9_resource_t::win32_d3d9_resource_on_reset_device()
{
	return true;
}

win32_d3d9_resource_t::win32_d3d9_resource_t()
{
	if (__first)
	{
		win32_d3d9_resource_t* current = __first;
		while (current->_win32_d3d9_resource_next)
			current = current->_win32_d3d9_resource_next;
		assert(current);
		current->_win32_d3d9_resource_next = this;
	}
	else
	{
		__first = this;
	}

	_win32_d3d9_resource_next = nullptr;
}

win32_d3d9_resource_t::~win32_d3d9_resource_t()
{
	if (this == __first)
	{
		__first = _win32_d3d9_resource_next;
	}
	else
	{
		win32_d3d9_resource_t* previous = __first;
		win32_d3d9_resource_t* current = previous->_win32_d3d9_resource_next;

		while (current != this)
		{
			previous = current;
			current = current->_win32_d3d9_resource_next;
		}

		previous->_win32_d3d9_resource_next = _win32_d3d9_resource_next;
	}
}
