#include "stdafx.h"
#include "d3d_resource.h"

#include "state.h"

TRAVERSABLE(dx9::d3d_resource_t)

namespace dx9
{
	//callbacks
	::HRESULT CALLBACK d3d_resource_t::callback_on_create_device()
	{
		//FS_LOG("%u resources to callback", count);
		if (count)
		{
			//fs::log::indent();
			d3d_resource_t* resource = first;
			uint32_t index = 1;
			while (resource)
			{
				if (!resource->on_create_device())
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

	::HRESULT CALLBACK d3d_resource_t::callback_on_reset_device()
	{
		//FS_LOG("%u resources to callback", count);
		if (count)
		{
			//fs::log::indent();
			d3d_resource_t* resource = first;
			uint32_t index = 1;
			while (resource)
			{
				if (!resource->on_reset_device())
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

	void CALLBACK d3d_resource_t::callback_on_destroy_device()
	{
		//FS_LOG("%u resources to callback", count);
		if (count)
		{
			//fs::log::indent();
			d3d_resource_t* resource = first;
			uint32_t index = 1;
			while (resource)
			{
				resource->on_destroy_device();
#ifdef _DEBUG
				//FS_LOG("%u / %u: %s", index, count, resource->descriptor.buffer);
#endif
				resource = resource->next;
				++index;
			}
			//fs::log::undent();
		}
	}

	void CALLBACK d3d_resource_t::callback_on_lost_device()
	{
		//FS_LOG("%u resources to callback", count);
		if (count)
		{
			//fs::log::indent();
			d3d_resource_t* resource = first;
			uint32_t index = 1;
			while (resource)
			{
				resource->on_lost_device();
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
	bool d3d_resource_t::on_create_device()
	{
		return true;
	}

	void d3d_resource_t::on_destroy_device()
	{
	}

	void d3d_resource_t::on_lost_device()
	{
	}

	bool d3d_resource_t::on_reset_device()
	{
		return true;
	}

	d3d_resource_t::d3d_resource_t()
	{
	}
}