#include "radar.hpp"
#include "..\..\..\math\elements\vector.h"
#include <string>
#include "..\..\..\utils\hook\hooks.h"
#include "..\..\..\sdk\cache\cache_entity.h"
#include "..\..\backtrack\record.h"
#include "..\preview\preview.h"
#include <imgui_settings.h>
#include "..\..\..\utils\render\render.h"
#include "../../../utils/hook/hooks.h"
#include <fstream>
#include <iostream>
#include <D3DX11tex.h>

#include <wrl/client.h>
#include <VTexParser.h>
#include "..\..\..\helpers\tfm\tinyformat.h"

ID3D11ShaderResourceView* texture_map = nullptr;

Vector_t radar::c_radar::world_to_map(const Vector_t& vWorldPosition)
{
	const int iMapCenter = 512;

	// World2Map
	Vector_t vMapPosition;

	vMapPosition.x = (vWorldPosition.x - map_origin.x) / map_scale;
	vMapPosition.y = (vWorldPosition.y - map_origin.y) / -map_scale;
	vMapPosition.z = 0;

	// Map2Panel
	Vector_t vOffset;
	vOffset.x = vMapPosition.x - iMapCenter;
	vOffset.y = vMapPosition.y - iMapCenter;
	vOffset.z = 0;

	// (m_fZoom * m_fFullZoom) / OVERVIEW_MAP_SIZE
	float fScale = 1.f / 1024;
	vOffset.x *= fScale;
	vOffset.y *= fScale;

	Vector_t vFinalPos;
	vFinalPos.x = (ImGui::GetWindowSize().x * 0.5f) + (ImGui::GetWindowSize().x * vOffset.x);
	vFinalPos.y = (ImGui::GetWindowSize().x * 0.5f) + (ImGui::GetWindowSize().x * vOffset.y);
	vFinalPos.z = 0;

	vMapPosition.x = vMapPosition.x / (1024.f / ImGui::GetWindowSize().x);
	vMapPosition.y = vMapPosition.y / (1024.f / ImGui::GetWindowSize().y);

	vMapPosition.x += ImGui::GetWindowPos().x;
	vMapPosition.y += ImGui::GetWindowPos().y;

	return vMapPosition;
}


void radar::c_radar::cache() {

	// Locate map by our local player
	auto local_player = hooks::m_get_local_player(0);

	if (local_player)
	{
		if (sdk::m_game_rules->m_bBombPlanted()) {
			for (auto& c4_object : m_c4_object )
			{		
				if (!c4_object.handle.valid())
					continue;

				Vector_t bomb_pos;

				bomb_pos = c4_object.abs_origin;
				bomb_pos = world_to_map(bomb_pos);

				ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(bomb_pos.x - 2, bomb_pos.y - 2), ImVec2(bomb_pos.x + 1, bomb_pos.y + 2), ImGui::GetColorU32(ImVec4(225 / 255.f, 165 / 255.f, 25 / 255.f, 1.f)));	
			}
		}

		if (m_local_player_data.alive)
		{
			Vector_t world_pos = m_local_player_data.abs_origin;
			Vector_t map_pos = world_to_map(world_pos);

			ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(map_pos.x, map_pos.y), 5, ImColor(30, 255, 30), 25);
		}

		for (const auto& player_object : visuals::visuals->m_get_cached_players())
		{
			// if the handle is invalid, skip this entity
			if (!player_object.handle.valid())
				continue;

			auto team_num = player_object.team_num;
			bool bIsEnemy = player_object.type == player_enemy;

			Vector_t EntityPos;

			EntityPos = player_object.abs_origin;
			EntityPos = world_to_map(EntityPos);

			ImColor clr = bIsEnemy ? ImColor(255, 50, 50) : (team_num == 3 ? ImColor(95, 185, 245) : ImColor(245, 205, 65));
			ImColor clr_dormant = bIsEnemy ? ImColor(255, 50, 50, (int)(255 * 255.0f)) : (team_num == 2 ? ImColor(95, 185, 245) : ImColor(245, 205, 65));

			ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(EntityPos.x, EntityPos.y), 5, clr, 30);

			//	if (g_cfg.radar.render_health)
			{
				float hp = math::clamp(player_object.health, 0, 100) / 100;
				if (hp <= 0)
					continue;

				float height = math::clamp(player_object.health / 100 * 5, 2, 5);

				ImColor clr = ImColor(int((1 - hp) * 255), int(hp * 255), 0, 255);
				ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(EntityPos.x - 4, EntityPos.y - height), ImVec2(EntityPos.x - 2, EntityPos.y + height), clr);
			}			
		}
	}
}
	
	bool radar::c_radar::render_map()
	{
		// check if texture is loaded and ingame bools
		bool bTrueRadar =  loaded && sdk::m_engine->in_game() && sdk::m_engine->connected();
		if (!loaded || !cfg_get(bool, g_cfg.fast_ladder) || !texture_map || g::update_radar)
			return false;

		static auto open = true;
		static auto set_once = false;
		if (!set_once)
		{
			ImGui::SetNextWindowSize({ 256.f, 256.f }, ImGuiCond_Once);
			set_once = true;
		}

		ImGui::Begin("##radar", &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar);

		if (ImGui::GetWindowSize().x != ImGui::GetWindowSize().y)
			ImGui::SetWindowSize(ImGui::GetWindowSize().x > ImGui::GetWindowSize().y ? ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().x) : ImVec2(ImGui::GetWindowSize().y, ImGui::GetWindowSize().y));


		// Vars
		int x = ImGui::GetWindowPos().x;
		int y = ImGui::GetWindowPos().y;
		int size = ImGui::GetWindowSize().x;
		int center_size = (size / 2);

		// Limit radar bounds
		ImGui::GetWindowDrawList()->PushClipRect(ImVec2(x, y), ImVec2(x + size, y + size), false);

		// Scale map
		float new_size = size;
		float size_diff = new_size - size;
		size = new_size;
		x -= size_diff / 2;
		y -= size_diff / 2;

		// Prepare imgui shit
		ImVec2 uv_a(0, 0), uv_c(1, 1), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);
		ImVec2 a(x, y), c(x + size, y + size), b(c.x, a.y), d(a.x, c.y);


		// Render background first
		ImGui::GetWindowDrawList()->AddShadowRect(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y), Color_t(35,35,35,255).GetU32(), 32.f, ImVec2(0,0), 0);

		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y), ImColor(0, 0, 0));

		// Render
		ImGui::GetWindowDrawList()->PushTextureID(texture_map);
		ImGui::GetWindowDrawList()->PrimReserve(6, 4);
		ImGui::GetWindowDrawList()->PrimQuadUV(a, b, c, d, uv_a, uv_b, uv_c, uv_d, 0xFFFFFFFF);
		ImGui::GetWindowDrawList()->PopTextureID();

		// Remove limit
		ImGui::GetWindowDrawList()->PopClipRect();

		ImGui::GetWindowDrawList()->AddRect(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y), ImColor(0, 0, 0));

		// Render the texture with AddImage
		ImGui::GetWindowDrawList()->AddImage(
			(void*)texture_map,               // The texture ID
			ImVec2(x, y),                     // Top-left position of the image
			ImVec2(x + size, y + size),       // Bottom-right position of the image
			uv_a,                             // Top-left UV
			uv_c,                             // Bottom-right UV
			ImColor(255, 255, 255, 255)       // Color (white means no tint)
		);

		if (bTrueRadar)
			cache();

		ImGui::End();
		return true;
	}

	void create_texture_vpk(int image_width, int image_height,
		unsigned char* image_data,
		ID3D11ShaderResourceView** out_srv) {
		// Create texture
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = image_width;
		desc.Height = image_height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;

		ID3D11Texture2D* pTexture = NULL;
		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = image_data;
		subResource.SysMemPitch = desc.Width * 4;
		subResource.SysMemSlicePitch = 0;
		sdk::m_device->CreateTexture2D(&desc, &subResource, &pTexture);

		// Create texture view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		sdk::m_device->CreateShaderResourceView(pTexture, &srvDesc,
			out_srv);
		pTexture->Release();
	}

	auto get_txt_buffer(const std::string_view path, int32_t& file_size) {
		// Verifica se já encontrou o offset anteriormente
		debug(hooks::m_get_vpk_buffer != nullptr);

		return std::unique_ptr<uint8_t, void(*)(uint8_t*)>(
			hooks::m_get_vpk_buffer(path.data(), &file_size),
			[](uint8_t* buf) {
				sdk::m_mem_alloc->Free(buf);
			});
	}
	bool radar::c_radar::on_map_load()
	{
		if (g::update_radar || !sdk::m_engine->is_valid())
			return false;

		auto local_player = hooks::m_get_local_player(0);
		if (!local_player)
			return false;

		_log(LOG_INFO) << "[radar] on map load called updating texture data";

		draw_plant_A = false;
		draw_plant_B = false;
		loaded = false;

		using fn = uintptr_t * (CS_FASTCALL*)(const char*);
		static auto find_csgo_map_overview_hud_element = reinterpret_cast<fn>(
			utils::FindPattern(xorstr_("client.dll"), xorstr_("40 55 48 83 EC 20 48 83"))
			);

		if (!find_csgo_map_overview_hud_element)
			return false;
		_log(LOG_INFO) << "[radar] on map load find_csgo_map_overview_hud_element";

		uintptr_t* raw_radar_hud_ptr = find_csgo_map_overview_hud_element(xorstr_("CCSGO_MapOverview"));
		if (!raw_radar_hud_ptr)
			return false;
		_log(LOG_INFO) << "[radar] on map load raw_radar_hud_ptr";

		CCSGO_MapOverview* radar_hud = reinterpret_cast<CCSGO_MapOverview*>(
			reinterpret_cast<uintptr_t>(raw_radar_hud_ptr) - 0x28
			);

		if (!radar_hud)
			return false;
		_log(LOG_INFO) << "[radar] on map load radar_hud";

		auto image = radar_hud->image();
		if (!image)
			return false;
		_log(LOG_INFO) << "[radar] on map load image";

		if (texture_map)
			texture_map = nullptr;
		_log(LOG_INFO) << "[radar] on map load reset old image";

		texture_map = image->GetNativeTexture();
		if (!texture_map)
			return false;
		_log(LOG_INFO) << "[radar] on map load texture_map";

		map_origin.x = radar_hud->map_position_x();
		map_origin.y = radar_hud->map_position_y();
		map_scale = radar_hud->map_scale();

		loaded = true;
		g::update_radar = true;

		_log(LOG_INFO) << "[radar] on map load loaded";

		return true;
	}
	void radar::c_radar::render()
	{
		if (!texture_map)
			return;

		if (!loaded)
			return;

		// menu bool 
	
		render_map();
	}
	
	