#pragma once

#include <memory>
#include <array>

#include "..\..\..\utils\render\render.h"

#include "..\..\..\utils\utils.h"
#include "..\..\..\sdk\sdk.h"
#include "..\..\..\math\elements\qangle.h"
#include "..\..\..\math\elements\vector.h"
#include "..\..\..\math\elements\transform.h"
#include "..\..\..\utils\configs\config.h"
#include "..\proccess.h"

#include <iostream>
#include <string>
#include <map>

#include <imgui.h>
#include <imgui_internal.h>
class CTextureDx11 {
public:

	ID3D11ShaderResourceView* m_pTextureSRV0()
	{
		return *(ID3D11ShaderResourceView**)(std::uintptr_t(this) + 0x10);
	}	
	ID3D11ShaderResourceView* m_pTextureSRV1()
	{
		return *(ID3D11ShaderResourceView**)(std::uintptr_t(this) + 0x18);
	}
};

class CSource2UITexture {
public:
	class Data {
	public:

		CTextureDx11* m_pDx11Texture()
		{
			return *(CTextureDx11**)(std::uintptr_t(this) + 0x0);
		}
	};
	Data* m_pData()
	{
		return *(Data**)(std::uintptr_t(this) + 0x28);
	}
};

class CImageData {
public:

	CSource2UITexture* m_pUITexture()
	{
		return *(CSource2UITexture**)(std::uintptr_t(this) + 0x30);
	}
};

class CImageProxySource {
public:
	CImageData* m_pImageData()
	{
		return *(CImageData**)(std::uintptr_t(this) + 0x10);
	}
};

class CImagePanel {
public:
	CImageProxySource* m_pImageProxy()
	{
		return *(CImageProxySource**)(std::uintptr_t(this) + 0xd0);
	}
	inline ID3D11ShaderResourceView* GetNativeTexture() {
		auto image_proxy = m_pImageProxy();
		if (!image_proxy)
			return nullptr;

		auto image_data = image_proxy->m_pImageData();
		if (!image_data)
			return nullptr;

		auto ui_texture = image_data->m_pUITexture();
		if (!ui_texture)
			return nullptr;

		auto ui_texture_data = ui_texture->m_pData();
		if (!ui_texture_data)
			return nullptr;

		auto dx11_texture = ui_texture_data->m_pDx11Texture();
		if (!dx11_texture)
			return nullptr;

		return dx11_texture->m_pTextureSRV0();
	}
};
class CCSGO_MapOverview {
public:
	// 0x2f0 // "file://{images}/overheadmaps/%s_radar.%s" ref to smallest function
	// [Ptr - 0x28]
	CImagePanel* image()
	{
		return *(CImagePanel**)(std::uintptr_t(this) + 0x2D8);
	}
	float map_position_x()
	{
		return *(float*)(std::uintptr_t(this) + 0x1B8);
	}
	float map_position_y()
	{
		return *(float*)(std::uintptr_t(this) + 0x1BC);
	}
	float map_scale()
	{
		return *(float*)(std::uintptr_t(this) + 0x1CC);
	}
	float map_heigh()
	{
		return *(float*)(std::uintptr_t(this) + 0x1D8);
	}
};
namespace radar
{
	class c_radar {
	public:
		struct c_map_image_data {
			int width;
			int weight;
			unsigned char* data;
		} image_data;
		bool loaded;

		Vector_t map_origin;
		double map_scale;
		Vector_t bomb_site_A;
		Vector_t bomb_site_B;

		int texture_index;
		bool draw_plant_A;
		bool draw_plant_B;

		void cache();
		bool render_map();
		bool on_map_load();
		void render();
		Vector_t world_to_map(const Vector_t& vWorldPosition);
	};

	const auto instance = std::make_unique<c_radar>();
}
