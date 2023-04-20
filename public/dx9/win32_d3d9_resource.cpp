#include "stdafx.h"
#include "win32_d3d9_resource.h"

#include "win32_d3d9_state.h"

TRAVERSABLE(win32_d3d9_resource_t)

//callbacks
::HRESULT CALLBACK win32_d3d9_resource_callback_on_create_device()
{
	//FS_LOG("%u resources to callback", count);
	if (win32_d3d9_resource_t::count)
	{
		//fs::log::indent();
		win32_d3d9_resource_t* resource = win32_d3d9_resource_t::first;
		uint32_t index = 1;
		while (resource)
		{
			if (!resource->win32_d3d9_resource_on_create_device())
			{
				//FS_ERROR("failed for %s", resource->descriptor.buffer);
				return E_FAIL;
			}
#ifdef _DEBUG
			//FS_LOG("%u / %u: %s", index, count, resource->descriptor.buffer);
#endif
			resource = resource->next;
			++index;
		}
		//fs::log::undent();
	}

	return S_OK;
}

::HRESULT CALLBACK win32_d3d9_resource_callback_on_reset_device()
{
	//FS_LOG("%u resources to callback", count);
	if (win32_d3d9_resource_t::count)
	{
		//fs::log::indent();
		win32_d3d9_resource_t* resource = win32_d3d9_resource_t::first;
		uint32_t index = 1;
		while (resource)
		{
			if (!resource->win32_d3d9_resource_on_reset_device())
			{
				//FS_ERROR("failed for %s", resource->descriptor.buffer);
				return E_FAIL;
			}
#ifdef _DEBUG
			//FS_LOG("%u / %u: %s", index, count, resource->descriptor.buffer);
#endif
			resource = resource->next;
			++index;
		}
		//fs::log::undent();
	}

	return S_OK;
}

void CALLBACK win32_d3d9_resource_callback_on_destroy_device()
{
	//FS_LOG("%u resources to callback", count);
	if (win32_d3d9_resource_t::count)
	{
		//fs::log::indent();
		win32_d3d9_resource_t* resource = win32_d3d9_resource_t::first;
		uint32_t index = 1;
		while (resource)
		{
			resource->win32_d3d9_resource_on_destroy_device();
#ifdef _DEBUG
			//FS_LOG("%u / %u: %s", index, count, resource->descriptor.buffer);
#endif
			resource = resource->next;
			++index;
		}
		//fs::log::undent();
	}
}

void CALLBACK win32_d3d9_resource_callback_on_lost_device()
{
	//FS_LOG("%u resources to callback", count);
	if (win32_d3d9_resource_t::count)
	{
		//fs::log::indent();
		win32_d3d9_resource_t* resource = win32_d3d9_resource_t::first;
		uint32_t index = 1;
		while (resource)
		{
			resource->win32_d3d9_resource_on_lost_device();
#ifdef _DEBUG
			//FS_LOG("%u / %u: %s", index, count, resource->descriptor.buffer);
#endif
			resource = resource->next;
			++index;
		}
		//fs::log::undent();
	}
}

//default implementations
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
}
