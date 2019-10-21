#pragma once

#include "Traversable.h"

namespace dx9
{
	struct d3d_resource_t : public traversable_t<d3d_resource_t>
	{
		static ::HRESULT CALLBACK callback_on_create_device();
		static ::HRESULT CALLBACK callback_on_reset_device();
		static void CALLBACK callback_on_destroy_device();
		static void CALLBACK callback_on_lost_device();

		virtual bool on_create_device();
		virtual bool on_reset_device();
		virtual void on_destroy_device();
		virtual void on_lost_device();

	protected:

		explicit d3d_resource_t();
	};
}
