#pragma once

struct w32_d3d9_resource_t
{
	virtual bool w32_d3d9_resource_on_create_device();
	virtual bool w32_d3d9_resource_on_reset_device();
	virtual void w32_d3d9_resource_on_destroy_device();
	virtual void w32_d3d9_resource_on_lost_device();

	virtual ~w32_d3d9_resource_t();

	w32_d3d9_resource_t* _w32_d3d9_resource_next;

protected:

	explicit w32_d3d9_resource_t();
};

::HRESULT CALLBACK w32_d3d9_resource_callback_on_create_device();
::HRESULT CALLBACK w32_d3d9_resource_callback_on_reset_device();
void CALLBACK w32_d3d9_resource_callback_on_destroy_device();
void CALLBACK w32_d3d9_resource_callback_on_lost_device();
