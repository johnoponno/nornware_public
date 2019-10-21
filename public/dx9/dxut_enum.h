#pragma once

namespace dx9
{
	struct enum_depth_stencil_multisample_conflict_t;
	struct enum_device_settings_combo_t;

	//--------------------------------------------------------------------------------------
	// A struct describing a Direct3D device that contains a 
	//       unique supported device type 
	//--------------------------------------------------------------------------------------
	struct enum_device_info_t
	{
		~enum_device_info_t();

		::D3DDEVTYPE device_type;
		::D3DCAPS9 caps;
		std::vector<enum_device_settings_combo_t*> device_settings_combo_list;		// List of CD3DEnumDeviceSettingsCombo* with a unique set of AdapterFormat, BackBufferFormat, and Windowed
	};

	//--------------------------------------------------------------------------------------
	// A struct describing an adapter which contains a unique adapter ordinal 
	// that is installed on the system
	//--------------------------------------------------------------------------------------
	struct enum_adapter_info_t
	{
		~enum_adapter_info_t();

		::UINT adapter_ordinal;
		::D3DADAPTER_IDENTIFIER9 adapter_identifier;
		char description[256];
		std::vector<::D3DDISPLAYMODE> display_mode_list; // Array of supported D3DDISPLAYMODEs
		std::vector<enum_device_info_t*> device_info_list; // Array of CD3DEnumDeviceInfo* with unique supported DeviceTypes
	};

	//--------------------------------------------------------------------------------------
	// A struct describing device settings that contains a unique combination of 
	// adapter format, back buffer format, and windowed that is compatible with a 
	// particular Direct3D device and the app.
	//--------------------------------------------------------------------------------------
	struct enum_device_settings_combo_t
	{
		::UINT adapter_ordinal;
		::D3DDEVTYPE device_type;
		::D3DFORMAT adapter_format;
		::D3DFORMAT backbuffer_format;
		::BOOL windowed;

		std::vector<::D3DFORMAT> depth_stencil_formats; // List of D3DFORMATs
		std::vector<::D3DMULTISAMPLE_TYPE> multisample_types; // List of D3DMULTISAMPLE_TYPEs
		std::vector<::DWORD> multisample_qualities; // List of number of quality levels for each multisample type
		std::vector<::UINT> present_intervals; // List of D3DPRESENT flags
		std::vector<enum_depth_stencil_multisample_conflict_t> dsms_conflicts; // List of CD3DEnumDSMSConflict

		enum_adapter_info_t* adapter_info;
		enum_device_info_t* device_info;
	};

	//--------------------------------------------------------------------------------------
	// Enumerates available Direct3D adapters, devices, modes, etc.
	// Use DXUTGetEnumeration() to access global instance
	//--------------------------------------------------------------------------------------
	struct enumeration_t
	{
		static enumeration_t* instance();
		~enumeration_t();

		std::vector<enum_adapter_info_t*> adapter_info_list;
		std::vector<UINT> present_interval_list;
		std::vector<D3DMULTISAMPLE_TYPE> multisample_type_list;
		std::vector<D3DFORMAT> depth_stencil_possible_list;
		UINT multisample_quality_max;
		UINT min_width;
		UINT max_width;
		UINT min_height;
		UINT max_height;
		UINT refresh_min;
		UINT refresh_max;
		bool require_post_pixel_shaders_blending;
		bool software_vp;
		bool hardware_vp;
		bool pure_hardware_vp;
		bool mixed_vp;

	private:

		// Use DXUTGetEnumeration() to access global instance
		explicit enumeration_t();
	};

	extern HRESULT enumerate(enumeration_t& e);
	extern const enum_device_settings_combo_t* get_device_settings_combo(const UINT AdapterOrdinal, const D3DDEVTYPE DeviceType, const D3DFORMAT AdapterFormat, const D3DFORMAT BackBufferFormat, const BOOL Windowed, const enumeration_t& e);
	extern const enum_adapter_info_t* get_adapter_info(const UINT AdapterOrdinal, const enumeration_t& e);
}
