#include "stdafx.h"
#include "w32_d3d9_resource.h"

#include "w32_d3d9_state.h"

static w32_d3d9_resource_t* __first = nullptr;

::HRESULT CALLBACK w32_d3d9_resource_callback_on_create_device()
{
	w32_d3d9_resource_t* resource = __first;
	while (resource)
	{
		if (!resource->w32_d3d9_resource_on_create_device())
			return E_FAIL;
		resource = resource->_w32_d3d9_resource_next;
	}

	return S_OK;
}

::HRESULT CALLBACK w32_d3d9_resource_callback_on_reset_device()
{
	w32_d3d9_resource_t* resource = __first;
	while (resource)
	{
		if (!resource->w32_d3d9_resource_on_reset_device())
			return E_FAIL;
		resource = resource->_w32_d3d9_resource_next;
	}

	return S_OK;
}

void CALLBACK w32_d3d9_resource_callback_on_destroy_device()
{
	w32_d3d9_resource_t* resource = __first;
	while (resource)
	{
		resource->w32_d3d9_resource_on_destroy_device();
		resource = resource->_w32_d3d9_resource_next;
	}
}

void CALLBACK w32_d3d9_resource_callback_on_lost_device()
{
	w32_d3d9_resource_t* resource = __first;
	while (resource)
	{
		resource->w32_d3d9_resource_on_lost_device();
		resource = resource->_w32_d3d9_resource_next;
	}
}

bool w32_d3d9_resource_t::w32_d3d9_resource_on_create_device()
{
	return true;
}

void w32_d3d9_resource_t::w32_d3d9_resource_on_destroy_device()
{
}

void w32_d3d9_resource_t::w32_d3d9_resource_on_lost_device()
{
}

bool w32_d3d9_resource_t::w32_d3d9_resource_on_reset_device()
{
	return true;
}

w32_d3d9_resource_t::w32_d3d9_resource_t()
{
	if (__first)
	{
		w32_d3d9_resource_t* current = __first;
		while (current->_w32_d3d9_resource_next)
			current = current->_w32_d3d9_resource_next;
		assert(current);
		current->_w32_d3d9_resource_next = this;
	}
	else
	{
		__first = this;
	}

	_w32_d3d9_resource_next = nullptr;
}

w32_d3d9_resource_t::~w32_d3d9_resource_t()
{
	if (this == __first)
	{
		__first = _w32_d3d9_resource_next;
	}
	else
	{
		w32_d3d9_resource_t* previous = __first;
		w32_d3d9_resource_t* current = previous->_w32_d3d9_resource_next;

		while (current != this)
		{
			previous = current;
			current = current->_w32_d3d9_resource_next;
		}

		previous->_w32_d3d9_resource_next = _w32_d3d9_resource_next;
	}
}
