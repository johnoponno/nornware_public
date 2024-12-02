#pragma once

struct win32_d3d9_resource_t
{
	virtual bool win32_d3d9_resource_on_create_device();
	virtual bool win32_d3d9_resource_on_reset_device();
	virtual void win32_d3d9_resource_on_destroy_device();
	virtual void win32_d3d9_resource_on_lost_device();

	virtual ~win32_d3d9_resource_t();

	win32_d3d9_resource_t* _win32_d3d9_resource_next;

protected:

	explicit win32_d3d9_resource_t();
};

::HRESULT CALLBACK win32_d3d9_resource_callback_on_create_device();
::HRESULT CALLBACK win32_d3d9_resource_callback_on_reset_device();
void CALLBACK win32_d3d9_resource_callback_on_destroy_device();
void CALLBACK win32_d3d9_resource_callback_on_lost_device();
