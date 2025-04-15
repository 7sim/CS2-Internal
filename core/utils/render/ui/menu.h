#pragma once
#include <d3d11.h>

namespace menu
{

	inline int nSelectedConfig = 0;
	inline int preview_tabs = 0;
	inline int m_misc_tab = 0;

	inline int page = 0;
	inline float tab_alpha = 0.f;
	inline float tab_add = 0.f;
	inline int active_tab = 0;

	bool load_texture_from_disk(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);

	void load_texture_from_memory(const unsigned char* image_data, int image_data_size, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);

	void Render( );
	inline char szConfigFile[256U] = {};

	inline bool m_opened = true;
	inline bool m_wind_active = false;

}
