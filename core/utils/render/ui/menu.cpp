#include "menu.h"
#include "..\render.h"

#include "../../../deps/xorstr/xorstr.h"

#include "menu.h"
#include "../../../deps/imgui/imgui_internal.h"
#include "..\..\..\helpers\crt\crt.h"
#include "../../configs/config.h"
#include <imgui_edited.hpp>
#include "../../../cheats/visuals/preview/preview.h"
#include "../../../cheats/skins/skins.h"

#include "../../../cheats/skins/inventory.h"
#include "../../../cheats/skins/inventory_manager.h"
#include <stdio.h>
#include <stdint.h>
#include "..\..\..\utils\render\d3d11\images\shunk.h"
#include <d3d11.h>
#include <stdlib.h>
#include <assert.h>
#include "..\..\..\helpers\tfm\tinyformat.h"
#include <format>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include "..\..\..\helpers\vpk\VTexParser.h"
#include <examples/example_win32_directx11/font.h>
#include <examples/example_win32_directx11/textures.h>

#include <comdef.h>





using namespace visuals::preview;
static gui_skins* selected_item = nullptr;
static gui_skins* selected_weapon = nullptr;
static c_paintKits* selected_skin = nullptr;
#define CS_CONFIGURATION_FILE_EXTENSION ".bin"
#include "..\..\sdk\addresses\entity.h"

#include "..\blur\pshader.hpp"
using namespace ImGui;


ID3D11ShaderResourceView* previeww = nullptr;
ID3D11ShaderResourceView* ragebott = nullptr;
ID3D11ShaderResourceView* legitbott = nullptr;
ID3D11ShaderResourceView* visualss = nullptr;
ID3D11ShaderResourceView* miscc = nullptr;
ID3D11ShaderResourceView* worldd = nullptr;
ID3D11ShaderResourceView* settingss = nullptr;


std::string description_test = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.";

void init_thread() {
	if (selected_weapon->paintkits.empty() || !selected_weapon)
		return;
	for (auto& skin : selected_weapon->paintkits) {

		if (!skin.image) {
			auto vtex = VTexParser::Load(skin.path_image.c_str());

			if (vtex.data.empty())
				continue;

			Items::skins->create_texture(vtex.w, vtex.h, reinterpret_cast<unsigned char*>(vtex.data.data()), &skin.image);
		}
	}



}


#include <intrin.h>
#include <string>
#include <iostream>
#include <sstream>


struct Notification {
	int id;
	std::string icon, message, information;
	std::chrono::steady_clock::time_point startTime;
	std::chrono::steady_clock::time_point endTime;
	ImU32 color;
};

class NotificationSystem {
public:
	NotificationSystem() : notificationIdCounter(0) {}

	void AddNotification(const std::string& icon, const std::string& information, const std::string& message, int durationMs, ImU32 color) {
		auto now = std::chrono::steady_clock::now();
		auto endTime = now + std::chrono::milliseconds(durationMs);
		notifications.push_back({ notificationIdCounter++, icon, information, message, now, endTime, color });
	}

	void DrawNotifications() {
		auto now = std::chrono::steady_clock::now();

		std::sort(notifications.begin(), notifications.end(),
			[now](const Notification& a, const Notification& b) -> bool {
				float durationA = std::chrono::duration_cast<std::chrono::milliseconds>(a.endTime - a.startTime).count();
				float elapsedA = std::chrono::duration_cast<std::chrono::milliseconds>(now - a.startTime).count();
				float percentageA = (durationA - elapsedA) / durationA;

				float durationB = std::chrono::duration_cast<std::chrono::milliseconds>(b.endTime - b.startTime).count();
				float elapsedB = std::chrono::duration_cast<std::chrono::milliseconds>(now - b.startTime).count();
				float percentageB = (durationB - elapsedB) / durationB;

				return percentageA < percentageB;
			}
		);

		int currentNotificationPosition = 0;

		for (auto& notification : notifications) {
			if (now < notification.endTime) {
				float duration = std::chrono::duration_cast<std::chrono::milliseconds>(notification.endTime - notification.startTime).count();
				float elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - notification.startTime).count();
				float percentage = (duration - elapsed) / duration * 100.0f;

				ShowNotification(currentNotificationPosition, notification.icon, notification.information, notification.message, percentage, notification.color);
				currentNotificationPosition++;
			}
		}

		notifications.erase(std::remove_if(notifications.begin(), notifications.end(),
			[now](const Notification& notification) { return now >= notification.endTime; }),
			notifications.end());
	}

private:
	std::vector<Notification> notifications;
	int notificationIdCounter;

	void ShowNotification(int position, const std::string& icon, const std::string& information, const std::string& message, float percentage, ImU32 color) {

		float duePercentage = 100.0f - percentage;
		float alpha = percentage > 10.0f ? 1.0f : percentage / 10.0f;

		PushFont(font::inter_medium_widget);
		SetNextWindowPos(ImVec2(GetIO().DisplaySize.x - (CalcTextSize(message.c_str()).x + CalcTextSize(information.c_str()).x + 70), 15 + (position * 55)), ImGuiCond_Always);
		SetNextWindowSize(ImVec2(CalcTextSize(message.c_str()).x + CalcTextSize(information.c_str()).x + 60, 40));
		PopFont();

		PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		PushStyleVar(ImGuiStyleVar_WindowRounding, 6.f);
		PushStyleColor(ImGuiCol_WindowBg, GetColorU32(c::element::info_bar::filling));

		ImGui::Begin(("##NOTIFY" + std::to_string(position)).c_str(), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
		{
			ImVec2 pos = ImGui::GetWindowPos();

			PushStyleVar(ImGuiStyleVar_Alpha, 0.1f);
			GetForegroundDrawList()->AddRectFilled(pos + ImVec2(0, 0), pos + ImVec2(GetContentRegionMax().x * (duePercentage / 100.0f), GetContentRegionMax().y), ImGui::GetColorU32(color), c::element::rounding - 2);
			PopStyleVar();

			edited::RenderTextColor(font::icomoon, pos + ImVec2(0, 0), pos + ImVec2(40, 40), GetColorU32(color), icon.c_str(), ImVec2(0.52f, 0.52f));
			edited::RenderTextColor(font::inter_medium_widget, pos + ImVec2(40, 0), pos + ImVec2(GetContentRegionMax().x, 40), GetColorU32(color), message.c_str(), ImVec2(0.0f, 0.5f));
			edited::RenderTextColor(font::inter_medium_widget, pos + ImVec2(CalcTextSize(message.c_str()).x + 45, 0), pos + ImVec2(GetContentRegionMax().x, 40), GetColorU32(c::element::text_active), information.c_str(), ImVec2(0.0f, 0.5f));

		}
		End();

		PopStyleColor(1);
		PopStyleVar(2);
	}
};


NotificationSystem notificationSystem;

bool render::Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* device_context)
{
	// check if we loaded already once
	if (m_initialized)
		return true;



	// create imgui context
	ImGui::CreateContext();

	// setup platform
	if (!ImGui_ImplWin32_Init(hwnd))
		return false;

	// setup renderer bindings
	if (!ImGui_ImplDX11_Init(device, device_context))
		return false;
	//post_request_with_hwid(1);
	// get imgui style
	ImGuiStyle& style = ImGui::GetStyle();

	style.AntiAliasedLines = true;
	style.AntiAliasedFill = true;
	style.AntiAliasedLinesUseTex = true;

	// setup shadow settings
	style.WindowShadowSize = 25.0f;
	// Main colors inspired by Midnight's blue theme
	ImColor midnightBlue = ImColor(0, 160, 255, 255);  // Main blue color
	ImColor darkBlue = ImColor(0, 120, 255, 255);      // Darker blue for active states
	ImColor mutedBlue = ImColor(0, 90, 200, 255);      // Muted blue for subtle elements

	// Text colors
	style.Colors[ImGuiCol_Text] = ImColor(255, 255, 255, 255);               // White text
	style.Colors[ImGuiCol_TextDisabled] = ImColor(150, 150, 150, 255);       // Disabled gray text

	// Background colors
	style.Colors[ImGuiCol_WindowBg] = ImColor(23, 23, 26, 255);              // Dark window background
	style.Colors[ImGuiCol_ChildBg] = ImColor(28, 28, 31, 255);               // Slightly lighter background
	style.Colors[ImGuiCol_PopupBg] = ImColor(30, 30, 33, 255);               // Popup background

	// Borders and separators
	style.Colors[ImGuiCol_Border] = ImColor(40, 40, 44, 255);                // Border color
	style.Colors[ImGuiCol_Separator] = ImColor(60, 60, 65, 255);             // Separator color
	style.Colors[ImGuiCol_SeparatorHovered] = midnightBlue;                  // Hovered separator
	style.Colors[ImGuiCol_SeparatorActive] = midnightBlue;                       // Active separator



	// Title and header
	style.Colors[ImGuiCol_TitleBg] = ImColor(30, 30, 33, 255);               // Title bar background
	style.Colors[ImGuiCol_TitleBgActive] = ImColor(35, 35, 40, 255);         // Active title bar background
	style.Colors[ImGuiCol_Header] = ImColor(30, 30, 33, 255);                // Header background
	style.Colors[ImGuiCol_HeaderHovered] = ImColor(40, 40, 45, 255);         // Hovered header
	style.Colors[ImGuiCol_HeaderActive] = ImColor(50, 50, 55, 255);          // Active header

	// Sliders and checkboxes (now in Midnight blue theme)
	//style.Colors[ImGuiCol_CheckMark] = midnightBlue;                         // Checkmark color
	style.Colors[ImGuiCol_SliderGrab] = darkBlue;                            // Slider grab handle
	style.Colors[ImGuiCol_SliderGrabActive] = midnightBlue;                  // Active slider grab

	// Tabs and buttons
	style.Colors[ImGuiCol_Tab] = ImColor(35, 35, 40, 255);                   // Tab background
	style.Colors[ImGuiCol_TabHovered] = midnightBlue;                            // Hovered tab
	style.Colors[ImGuiCol_TabActive] = midnightBlue;                         // Active tab
	style.Colors[ImGuiCol_Button] = ImColor(40, 40, 45, 255);                // Button background
	style.Colors[ImGuiCol_ButtonHovered] = midnightBlue;                         // Hovered button
	style.Colors[ImGuiCol_ButtonActive] = midnightBlue;                      // Active button

	// Checkbox background when disabled or inactive


	// Other window elements
	style.WindowRounding = 4.0f;
	style.WindowBorderSize = 1.0f;
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

	style.ChildRounding = 4.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 4.0f;
	style.PopupBorderSize = 1.0f;
	style.FrameRounding = 4.0f;
	style.FrameBorderSize = 1.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabRounding = 4.0f;
	style.TabRounding = 4.0f;
	style.TabBorderSize = 1.0f;




	// get imgui io
	ImGuiIO& io = ImGui::GetIO();

	// tell imgui to keep the default font for GUI
	io.Fonts->AddFontDefault();

	// create font config
	ImFontConfig font_config;
	font_config.PixelSnapH = false;
	font_config.FontDataOwnedByAtlas = false;
	font_config.OversampleH = 5;
	font_config.OversampleV = 5;
	font_config.RasterizerMultiply = 1.2f;

	static const ImWchar ranges[] = {

		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
		0x2DE0, 0x2DFF, // Cyrillic Extended-A
		0xA640, 0xA69F, // Cyrillic Extended-B
		0xE000, 0xE226, // icons
		0,
	};

	font_config.GlyphRanges = ranges;


	ImFontConfig verdana_cfg;
	ImFontConfig name_cfg;
	ImFontConfig seg_cfg;
	ImFontConfig tahoma_cfg;
	ImFontConfig flags_cfg;

	verdana_cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_LightHinting;
	tahoma_cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_Bitmap;
	seg_cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_NoHinting;
	name_cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint;
	flags_cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting;


	// add default font (normal)
	m_default_font = io.Fonts->AddFontFromFileTTF(xorstr_("C:\\Windows\\Fonts\\Tahoma.ttf"), 14.f, &verdana_cfg, io.Fonts->GetGlyphRangesCyrillic());
	//menu::load_texture_from_disk("C:\\Program Files\\cs2.png", &render::m_cs2_player_model_texture, &render::model_w, &render::model_h);
	// load Images
	//menu::load_texture_from_disk("C:\\Program Files\\cs2.png", &render::m_cs2_player_model_texture, &render::model_w, &render::model_h);
	render::weapon_icons = io.Fonts->AddFontFromFileTTF(("C:\\Windows\\Fonts\\segoeui.ttf"), 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

	m_esp_name = io.Fonts->AddFontFromFileTTF(xorstr_("C:\\Windows\\Fonts\\Verdana.ttf"), 10.f, &name_cfg, io.Fonts->GetGlyphRangesDefault());
	m_esp_weapons = io.Fonts->AddFontFromFileTTF(xorstr_("C:\\Windows\\Fonts\\Verdana.ttf"), 9.f, &name_cfg, io.Fonts->GetGlyphRangesDefault());
	m_esp_flags = io.Fonts->AddFontFromFileTTF(xorstr_("C:\\Windows\\Fonts\\Verdana.ttf"), 9.f, &flags_cfg, io.Fonts->GetGlyphRangesDefault());

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	/*
	ImFontConfig cfg;
	cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor | ImGuiFreeTypeBuilderFlags_Bitmap;
	font::inter_medium_widget = io.Fonts->AddFontFromFileTTF(xorstr_("C:\\Windows\\Fonts\\Tahoma.ttf"), 14.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());
	font::inter_medium_small = io.Fonts->AddFontFromFileTTF(xorstr_("C:\\Windows\\Fonts\\Tahoma.ttf"), 14.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());
	font::inter_medium_big = io.Fonts->AddFontFromFileTTF(xorstr_("C:\\Windows\\Fonts\\Tahoma.ttf"), 14.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());
	font::inter_bold = io.Fonts->AddFontFromFileTTF(xorstr_("C:\\Windows\\Fonts\\Tahoma.ttf"), 14.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());
	font::icomoon = io.Fonts->AddFontFromFileTTF(xorstr_("C:\\Windows\\Fonts\\Tahoma.ttf"), 14.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());
	font::icomoon_default = io.Fonts->AddFontFromFileTTF(xorstr_("C:\\Windows\\Fonts\\Tahoma.ttf"), 14.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());
	*/
	//C::Setup(L"sirius.bin");
	//C::CreateFile(L"config0.cfg");
	ImFontConfig cfg;
	cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor | ImGuiFreeTypeBuilderFlags_Bitmap;

	font::inter_medium_widget = io.Fonts->AddFontFromMemoryTTF(inter_medium, sizeof(inter_medium), 14.5f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	font::inter_medium_small = io.Fonts->AddFontFromMemoryTTF(inter_medium, sizeof(inter_medium), 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	font::inter_medium_big = io.Fonts->AddFontFromMemoryTTF(inter_bold, sizeof(inter_bold), 25.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	font::inter_bold = io.Fonts->AddFontFromMemoryTTF(inter_bold, sizeof(inter_bold), 17.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	font::icomoon = io.Fonts->AddFontFromMemoryTTF(icomoon, sizeof(icomoon), 17.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	font::icomoon_default = io.Fonts->AddFontFromMemoryTTF(icomoon, sizeof(icomoon), 20.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

	D3DX11_IMAGE_LOAD_INFO info; ID3DX11ThreadPump* pump{ nullptr };
	if (previeww == nullptr) D3DX11CreateShaderResourceViewFromMemory(sdk::m_device, preview, sizeof(preview), &info, pump, &previeww, 0);

	if (ragebott == nullptr) D3DX11CreateShaderResourceViewFromMemory(sdk::m_device, ragebot, sizeof(ragebot), &info, pump, &ragebott, 0);
	if (legitbott == nullptr) D3DX11CreateShaderResourceViewFromMemory(sdk::m_device, legitbot, sizeof(legitbot), &info, pump, &legitbott, 0);

	if (visualss == nullptr) D3DX11CreateShaderResourceViewFromMemory(sdk::m_device, cssss, sizeof(cssss), &info, pump, &visualss, 0);
	if (miscc == nullptr) D3DX11CreateShaderResourceViewFromMemory(sdk::m_device, misc, sizeof(misc), &info, pump, &miscc, 0);
	if (worldd == nullptr) D3DX11CreateShaderResourceViewFromMemory(sdk::m_device, world, sizeof(world), &info, pump, &worldd, 0);
	if (settingss == nullptr) D3DX11CreateShaderResourceViewFromMemory(sdk::m_device, settings, sizeof(settings), &info, pump, &settingss, 0);
	// add indicator font (bold)
	font_config.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_Bold;
	m_indicator_font = io.Fonts->AddFontFromFileTTF(xorstr_("C:\\Windows\\Fonts\\Tahoma.ttf"), 14.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());

	// build fonts
	m_initialized = io.Fonts->Build();

	// return status
	return m_initialized;
}
void render::Destroy()
{
	// check if we initialized
	if (!m_initialized)
		return;

	// destroy imgui
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// mark as uninitialized
	m_initialized = false;
}

void render::CreateRenderTarget()
{
	// attempt to get the device from the swap chain
	if (FAILED(sdk::m_swap_chain->m_swap_chain->GetDevice(__uuidof(ID3D11Device), (void**)&sdk::m_device)))
	{
		// log that we failed to get device from swap chain
		CS2_LOG(xorstr_("failed to get device from swapchain while creating render target\n"));
	}

	else
	{
		// successfully got the device, so get the device context
		sdk::m_device->GetImmediateContext(&sdk::m_device_context);
	}

	// lambda function to get the correct DXGI format
	static const auto get_correct_dxgi_format = [](DXGI_FORMAT current_frame)
	{
		switch (current_frame)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		return current_frame;
	};

	DXGI_SWAP_CHAIN_DESC sd{ };

	// retrieve the description of the swap chain
	sdk::m_swap_chain->m_swap_chain->GetDesc(&sd);

	ID3D11Texture2D* back_buffer = nullptr;

	// attempt to get the back buffer from the swap chain
	if (SUCCEEDED(sdk::m_swap_chain->m_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer))))
	{
		if (back_buffer)
		{
			D3D11_RENDER_TARGET_VIEW_DESC desc{ };

			// set gxgi format
			desc.Format = static_cast<DXGI_FORMAT>(get_correct_dxgi_format(sd.BufferDesc.Format));
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

			// attempt to create a render target view from the back buffer
			if (FAILED(sdk::m_device->CreateRenderTargetView(back_buffer, &desc, &sdk::m_render_target_view)))
			{
				// try with multisampled texture if failed
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
				if (FAILED(sdk::m_device->CreateRenderTargetView(back_buffer, &desc, &sdk::m_render_target_view)))
				{
					// if still failed, create render target view with default parameters
					if (FAILED(sdk::m_device->CreateRenderTargetView(back_buffer, NULL, &sdk::m_render_target_view)))
					{
						// at this point kill yourself
						CS2_LOG(xorstr_("failed to get render target view after multiple attempts.\n"));
					}
				}
			}

			// release back buffer
			back_buffer->Release();

			// invalidate back buffer
			back_buffer = nullptr;
		}
	}
}

void render::DestroyRenderTarget()
{
	// check if render view is valid
	if (sdk::m_render_target_view != nullptr)
	{
		// release render view
		sdk::m_render_target_view->Release();

		// invalidate render view
		sdk::m_render_target_view = nullptr;
	}
}

void render::NewFrame()
{
	// start a new frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	// create the fram
	ImGui::NewFrame();
	// update mouse input from CCSGOClient
	// @note: is there an better method?
	//sdk::m_csgo_input->m_release_mouse = menu::m_opened;
}

void render::Render()
{
	// end frame and add to queue
	ImGui::EndFrame();

	// render the queued frame
	ImGui::Render();

	// render the queued draw data
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
namespace esp_preview
{
	bool money = true;
	bool nickname = true;
	bool weapon = true;
	bool zoom = true;

	bool c4 = true;
	bool HP_line = true;
	bool hit = true;
	bool box = true;
	bool bomb = true;

	float box_color[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };
	float nick_color[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };
	float money_color[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };
	float zoom_color[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };
	float c4_color[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };
	float bomb_color[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };
	float hp_color[4] = { 255 / 255.f, 0 / 255.f, 0 / 255.f, 1.f };
	float hp_line_color[4] = { 255 / 255.f, 0 / 255.f, 0 / 255.f, 1.f };
	float weapon_color[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };
	float hit_color[4] = { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f };

	int hp = 85;

	bool active_esp_preview = true;
}


namespace var
{
	static float tab_alpha = 0.f; /* */ static float tab_add; /* */ static int active_tab = 0;

	bool interactive = false;
	float tab_right = 0, tab_left = 0;

	bool check[40], value[40];

	float col0[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float col1[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float col2[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float col3[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
	float col4[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
	float col5[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
	float col6[4] = { 1.0f, 0.5f, 0.0f, 1.0f };
	float col7[4] = { 0.5f, 0.0f, 1.0f, 1.0f };
	float col8[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float col9[4] = { 0.5f, 1.0f, 0.0f, 1.0f };
	float col10[4] = { 1.0f, 0.0f, 0.5f, 1.0f };
	float col11[4] = { 0.0f, 1.0f, 0.5f, 1.0f };
	float col12[4] = { 0.5f, 0.0f, 1.0f, 1.0f };
	float col13[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float col14[4] = { 1.0f, 0.5f, 0.5f, 1.0f };
	float col15[4] = { 0.5f, 1.0f, 0.5f, 1.0f };

	int slider_value[40], key[40], m[40], select[40];

	int selected[40];
	const char* items[4] = { "Selected", "Selected", "Selected", "Selected" };

	const char* pageNames[] = { "Ragebot", "LegitBot", "Visuals", "Misc", "World", "Skins" };

	const ImVec2 buttonSize(125, 35);
	int page = 0;

	char search[120] = { "" };
}





enum inventory_page : int {
	home_page = 0,
	item_list = 1, // weapons gloves knifes stickers etc
	weapon = 2, // 
	agent = 3,
	knife = 4,
	glove = 5,
	sticker = 6,
	paints = 7,
	paint_config = 8,
	gloves_paint = 9,
};
enum tab_type2 : int {
	legit_tab = 0,
	//ragebot_tab = 1,
	players_tab = 1,
	visuals_tab = 2,
	misc_tab = 3,
	skins_tab = 4,
	configs_tab = 5,
};
enum wep_type : int {
	PISTOL = 1,
	HEAVY_PISTOL = 2,
	ASSULT = 3,
	SNIPERS = 4,
	SCOUT = 5,
	AWP = 6,
};



int skin_page;
static int last_page;

static constexpr const char* arrEspFlags[] = {
	"Armor",
	"KIT"
};
static constexpr const char* arrHitbox[] = {

	"head",
	"chest",
	"stomach",
	"legs",
	"feet"

};
static constexpr const char* arrLegit[] = {

	"Smoke",
	"Flashed"
};
static constexpr const char* arr_game_removals[] = {

	"View bob",
	"Flash"

};

class c_specs
{
public:
	std::string name;
	std::uint64_t steam_id;
	bool m_show;
	int texture_id;
};

static 	std::vector<c_specs> specs_;

void RenderSpectators() {
	if (!cfg_get(bool, g_cfg.spectator_list))
		return;

	int nWindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar;
	int nWindowSizeAppend = 0;

	ImGui::Begin(xorstr_("Spectators"), NULL, nWindowFlags);
	{
		ImGui::SetWindowSize(ImVec2(145, 26 + nWindowSizeAppend));

		auto draw = ImGui::GetWindowDrawList();
		auto pos = ImGui::GetWindowPos();
		auto size = ImGui::GetWindowSize();

		// Draw shadow outline
		ImColor shadowColor(0, 0, 0, 80); // Darker color with lower opacity
		draw->AddRect(pos - ImVec2(1, 1), pos + size + ImVec2(1, 1), shadowColor, 7, ImDrawFlags_RoundCornersAll);

		draw->AddRectFilled(pos, pos + size, ImColor(45, 45, 45, 100), 7, ImDrawFlags_RoundCornersAll);
		draw->AddRectFilled(pos, pos + ImVec2(225, 22), ImColor(45, 45, 45, 100), 7, ImDrawFlags_RoundCornersAll);
		draw->AddRectFilledMultiColor(pos + ImVec2(0, 23), pos + ImVec2(225, 22), ImGui::GetColorU32(c::accent), ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0), ImGui::GetColorU32(c::accent));


		ImVec2 text_pos = pos + ImVec2(8, 3);

		draw->AddText(text_pos, ImColor(240, 240, 240, 222), xorstr_("Spectators"));

		std::vector<c_specs> specs_temp{};
		specs_.clear();
		/* connected ? */
		if (sdk::m_engine->is_valid() && sdk::m_game->pGameEntitySystem) {

			auto lcoal_controller = CCSPlayerController::Get();
			if (lcoal_controller) {

				auto local_player = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(lcoal_controller->PawnHandle());
				if (local_player) {
					if (local_player->IsAlive())
					{
						auto handle_local = local_player->handle();

						for (int i = 1; i < 32; i++)
						{
							auto entity = sdk::m_game->pGameEntitySystem->Get(i);
							if (!entity)
								continue;

							auto controller = sdk::m_game->pGameEntitySystem->Get<CCSPlayerController>(entity->handle());

							if (!controller)
								continue;

							CCSPlayerPawn* pawn = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(controller->PawnHandle());

							if (!pawn)
								continue;

							if (pawn->IsAlive()) {
								// Remove the spectator from the specs_ vector if they are alive
								specs_.erase(std::remove_if(specs_.begin(), specs_.end(), [&](const c_specs& spec) {
									return spec.steam_id == controller->m_steamID();
									}), specs_.end());
								continue;
							}


							if (pawn == local_player)
								continue;

							auto obs_pawn = sdk::m_game->pGameEntitySystem->Get<C_CSObserverPawn>(controller->m_hObserverPawn().index());
							if (!obs_pawn)
								continue;

							SchemaClassInfoData_t* class_info;
							obs_pawn->dump_class_info(&class_info);

							if (c_run_time::StringCompare(class_info->szName, xorstr_("C_CSObserverPawn")) != 0)
								continue;

							if (pawn->m_fFlags() & 0x100)
								continue;

							CPlayer_ObserverServices* obs_service = obs_pawn->m_pObserverServices();
							if (obs_service->m_hObserverTarget().index() != handle_local.index())
								continue;

							auto name = std::string(controller->GetPlayerName(), 16);
							specs_temp.push_back({ name, controller->m_steamID(), true, 0 });
						}


						for (auto it = specs_temp.begin(); it != specs_temp.end(); it++)
						{
							auto steam_id = it->steam_id;

							if (std::find_if(specs_.begin(), specs_.end(), [&](const auto& x) { return x.steam_id == steam_id; }) != specs_.end())
								continue;

							specs_.push_back(*it);
						}

						bool has_visible = specs_.size() > 0;

						ImGui::SetCursorPos({ 8, 26 });
						ImGui::BeginGroup();

						for (size_t idx = 0; idx < specs_.size(); ++idx)
						{

							const auto& spec = specs_[idx];
							ImGui::Text(spec.name.c_str());


							nWindowSizeAppend += 22;
							ImGui::SetWindowSize(ImVec2(145, 26 + nWindowSizeAppend));
						}

						ImGui::EndGroup();
					}
				}
			}
		}
	}
	ImGui::End();

	nWindowSizeAppend = 0;
}
void RenderKeybinds() {
	if (!cfg_get(bool, g_cfg.key_bind_list))
		return;
	m_key_binds.clear();
	int nWindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar;
	int nWindowSizeAppend = 0;

	// Setup keybinds for binding list etc.
	m_key_binds.push_back(cfg_get(KeyBind_t, g_cfg.legit_trigger_bot_key));
	cfg_get(KeyBind_t, g_cfg.legit_trigger_bot_key).szName = xorstr_("Triggerbot");


	m_key_binds.push_back(cfg_get(KeyBind_t, g_cfg.edge_bug_key));
	cfg_get(KeyBind_t, g_cfg.edge_bug_key).szName = xorstr_("Edge bug");

	m_key_binds.push_back(cfg_get(KeyBind_t, g_cfg.edge_jump_key));
	cfg_get(KeyBind_t, g_cfg.edge_jump_key).szName = xorstr_("Edge jump");

	m_key_binds.push_back(cfg_get(KeyBind_t, g_cfg.long_jump_key));
	cfg_get(KeyBind_t, g_cfg.long_jump_key).szName = xorstr_("Long jump");

	m_key_binds.push_back(cfg_get(KeyBind_t, g_cfg.mini_jump_key));
	cfg_get(KeyBind_t, g_cfg.mini_jump_key).szName = xorstr_("Mini jump");

	m_key_binds.push_back(cfg_get(KeyBind_t, g_cfg.ladder_jump_key));
	cfg_get(KeyBind_t, g_cfg.ladder_jump_key).szName = xorstr_("Ladder jump");

	ImGui::Begin(xorstr_("Binds"), NULL, nWindowFlags);
	{
		ImGui::SetWindowSize(ImVec2(145, 26 + nWindowSizeAppend));

		auto draw = ImGui::GetWindowDrawList();
		auto pos = ImGui::GetWindowPos();
		auto size = ImGui::GetWindowSize();

		// Draw shadow outline
		ImColor shadowColor(0, 0, 0, 80); // Darker color with lower opacity
		draw->AddRect(pos - ImVec2(1, 1), pos + size + ImVec2(1, 1), shadowColor, 7, ImDrawFlags_RoundCornersAll);

		draw->AddRectFilled(pos, pos + size, ImColor(45, 45, 45, 100), 4, ImDrawFlags_RoundCornersAll);
		draw->AddRectFilled(pos, pos + ImVec2(225, 22), ImColor(45, 45, 45, 100), 4, ImDrawFlags_RoundCornersAll);
		draw->AddRectFilledMultiColor(pos + ImVec2(0, 23), pos + ImVec2(225, 22), ImGui::GetColorU32(c::accent), ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0), ImGui::GetColorU32(c::accent));


		ImVec2 text_pos = pos + ImVec2(8, 3);
		draw->AddText(text_pos, ImColor(240, 240, 240, 222), xorstr_("Binds"));
		ImGui::SetCursorPos({ 8, 26 });
		ImGui::BeginGroup();

		for (size_t idx = 0; idx < m_key_binds.size(); ++idx)
		{

			if (!Input::GetBindState(m_key_binds[idx]))
				continue;

			if (m_key_binds[idx].szName != nullptr && strlen(m_key_binds[idx].szName) > 0)
			{
				ImGui::Text(m_key_binds[idx].szName);

				if (m_key_binds[idx].nMode == 0) {
					ImGui::SameLine(145 - ImGui::CalcTextSize("Aimbot").x);
					ImGui::Text("hold");
				}
				else if (m_key_binds[idx].nMode == 1) {
					ImGui::SameLine(145 - ImGui::CalcTextSize("Aimbot").x);
					ImGui::Text("toggle");
				}

				nWindowSizeAppend += 22;
				ImGui::SetWindowSize(ImVec2(145, 26 + nWindowSizeAppend));
			}
		}

		ImGui::EndGroup();
	}
	ImGui::End();

	nWindowSizeAppend = 0;
}

Color_t get_color_skin(int rarity)
{
	switch (rarity)
	{
	case 1:
		return Color_t(177, 196, 217, 100);
		break;
	case 2:
		return Color_t(95, 153, 217, 100);
		break;
	case 3:
		return Color_t(76, 106, 255, 100);
		break;
	case 4:
		return Color_t(137, 72, 255, 100);
		break;
	case 5:
		return Color_t(211, 45, 230, 100);
		break;
	case 6:
		return Color_t(235, 76, 76, 100);
		break;
	}

	return Color_t(155, 155, 155, 100);
}

ID3D11ShaderResourceView* get_image_x(std::string path_image) {

	auto vtex = VTexParser::Load(path_image.c_str());

	if (vtex.data.empty())
		return nullptr;

	ID3D11ShaderResourceView* image = nullptr;
	Items::skins->create_texture(vtex.w, vtex.h, reinterpret_cast<unsigned char*>(vtex.data.data()), &image);
	return image;

}
void menu::Render()
{
	// update menu accent
	c::accent = cfg_get(ColorPickerVar_t, g_cfg.accent_col).colValue.GetVec4();

	added_skins.reserve(7000);
	CEconItemSchema* pItemSchema = sdk::m_client->GetEconItemSystem()->GetEconItemSchema();

	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();

	notificationSystem.DrawNotifications();
	if (!m_opened)
		return;


	ImGui::SetNextWindowSize(c::bg::size);


	ImGui::GetStyle().WindowPadding = ImVec2(10, 10);
	ImGui::GetStyle().ItemSpacing = ImVec2(20, 20);
	ImGui::GetStyle().WindowBorderSize = 0;
	ImGui::GetStyle().ScrollbarSize = 2.f;
	auto current_chams_type = cfg_get(int, g_cfg.chams_type);
	const char* chams_type[4]{ ("Enemy"), ("Local"), ("Team"), ("Viewmodel") };

	ImGui::Begin("MENU", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
	{
		ImGuiStyle* style = &ImGui::GetStyle();

		const ImVec2& pos = GetWindowPos();
		const ImVec2& region = GetContentRegionMax();
		const ImVec2& spacing = style->ItemSpacing;

		ImGui::GetBackgroundDrawList()->AddRectFilledMultiColor(pos, pos + region, GetColorU32(c::bg::topbar, 1.f), GetColorU32(c::bg::topbar, 1.f), GetColorU32(c::black, 0.8f), GetColorU32(c::black, 0.8f), c::bg::rounding);

		ImGui::GetBackgroundDrawList()->AddRectFilled(pos + ImVec2(145, 60), pos + region, GetColorU32(c::bg::container), c::bg::rounding, ImDrawFlags_RoundCornersBottomRight);
		ImGui::GetBackgroundDrawList()->AddRectFilled(pos + ImVec2(145, 0), pos + ImVec2(region.x, 60), GetColorU32(c::bg::topbar), c::bg::rounding, ImDrawFlags_RoundCornersTopRight);
		ImGui::GetBackgroundDrawList()->AddLine(pos + ImVec2(0, 60), pos + ImVec2(region.x, 60), GetColorU32(c::bg::stroke), 1.f);

		ImGui::GetBackgroundDrawList()->AddCircle(pos + ImVec2(region.x - (60 / 2), (60 / 2)), 15, ImGui::GetColorU32(c::accent), 400.f, 2.f);

		//ImGui::GetBackgroundDrawList()->PushClipRect(pos + ImVec2(145, 60), pos + ImVec2(region), true);
		//ImGui::GetBackgroundDrawList()->AddShadowCircle(pos + ImVec2(145, 160), 100, ImGui::GetColorU32(c::accent, 0.15f), 400.f, ImVec2(0, 0), 0);
		//ImGui::GetBackgroundDrawList()->AddShadowCircle(pos + ImVec2(region.x, 160), 100, ImGui::GetColorU32(c::accent, 0.15f), 400.f, ImVec2(0, 0), 0);
		//ImGui::GetBackgroundDrawList()->AddShadowCircle(pos + ImVec2((145 + region.x) / 2, region.y), 100, ImGui::GetColorU32(c::accent, 0.15f), 400.f, ImVec2(0, 0), 0);
	   // ImGui::GetBackgroundDrawList()->PopClipRect();

		edited::RenderTextColor(font::inter_medium_big, pos, pos + ImVec2(145, 60), GetColorU32(c::element::text_active), "CS2", ImVec2(0.5, 0.5));

		ImGui::SetCursorPos(ImVec2(10, 70));
		BeginGroup();
		{
			ID3D11ShaderResourceView* pageShortcuts[6] = { ragebott, legitbott, visualss, miscc, worldd, settingss };
			for (int i = 0; i < 6; i++) if (edited::Pages(i == var::page, var::pageNames[i], pageShortcuts[i], var::buttonSize)) var::page = i;
		}
		EndGroup();

		//ImGui::SetCursorPos(ImVec2(region.x - (145 / 2), 60 - 35) / 2);

	   // edited::ICObutton(font::icomoon_default, "k", ImVec2(25, 35));

	  // / if (edited::BeginPopupW("Languages", "Setting up languages in the menu."))
//{

			//edited::EndPopupW();
	   // }

		//SameLine(0, 10);
		//ImGui::InputTextEx("q", "Search", var::search, IM_ARRAYSIZE(var::search), ImVec2(220, 35), NULL);
	   // SameLine(0, 10);
	   // edited::ICObutton(font::icomoon_default, "m", ImVec2(25, 35));

	  //  if (edited::BeginPopupW("preview", "Setting up the 'ESP PREVIEW' function."))
	   // {
	   //     edited::CheckboxWithPopup(false, "Activate the 'ESP Preview' window", "-", &esp_preview::active_esp_preview, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
//
	   //     edited::EndPopupW();
	  //  }

		var::tab_alpha = ImClamp(var::tab_alpha + (15.f * ImGui::GetIO().DeltaTime * (var::page == var::active_tab ? 1.f : -1.f)), 0.f, 1.f);
		if (var::tab_alpha == 0.f && var::tab_add == 0.f) var::active_tab = var::page;

		var::tab_right = ImLerp(var::tab_right, var::page == var::active_tab ? 0.f : c::bg::size.x, ImGui::GetIO().DeltaTime * 13.f);
		var::tab_left = ImLerp(var::tab_left, var::interactive ? c::bg::size.x : 0.f, ImGui::GetIO().DeltaTime * 13.f);

		if (var::tab_alpha == 0.f) var::interactive = true;
		if (var::interactive && var::tab_left >= (c::bg::size.x - 0.6f)) var::interactive = false;

		ImGui::SetCursorPos(!var::interactive ? ImVec2(145.f + var::tab_right, 60.f) : ImVec2(-c::bg::size.x + (145.f + var::tab_left), 60.f));

		PushClipRect(pos + ImVec2(145, 60), pos + ImVec2(c::bg::size.x, c::bg::size.y), true);
		ImGui::BeginChild("CONTAINER", ImVec2(region.x - 145, region.y - 60));
		{
			auto current_weapon = cfg_get(int, g_cfg.rage_weapon_selection);
			auto current_weapon_legit = cfg_get(int, g_cfg.legit_weapon_selection);

			
			if (var::active_tab == 0)
			{

			const char* weapons[7]{("Knife"), ("Pistols"), ("Heavy Pistols"),("Assult Rifles"), ("Auto"),("Scout"), ("Awp")};

		


				edited::BeginChild("General", ImVec2((region.x - (145 + spacing.x * 3)) / 2, 0));
				
				{
					edited::Combo(("Weapon"), "", &cfg_get(int, g_cfg.rage_weapon_selection), weapons, IM_ARRAYSIZE(weapons), 7);

					(edited::CheckboxWithPopup(true, "Enabled", "", &cfg_get(bool, g_cfg.rage_enable)));


					const char* targets_select[4]{ xorstr_("none"),xorstr_("Crosshair"), xorstr_("Damage"),xorstr_("Distance") };
					edited::Combo(xorstr_("Target selection"), xorstr_("Select target based on conditions"), &cfg_get_adaptive(int, 7, g_cfg.rage_target_select, current_weapon), targets_select, IM_ARRAYSIZE(targets_select), 3);
					const char* silent_mode[2]{ ("Always"), ("Prefer") };
					edited::Combo(("Silent mode"), ("Determine if should aim only with sub ticks angles ( avoid getting onshot )"), &cfg_get(int, g_cfg.rage_silent_mode), silent_mode, IM_ARRAYSIZE(silent_mode), 6);

					if (current_weapon > 3)
						edited::CheckboxWithPopup(true, "Auto Scope", "Automatically scope if a we found a target hitta...", &cfg_get_adaptive(bool, 7, g_cfg.rage_auto_scope, current_weapon));

	
					edited::MultiCombo("Hitboxes", &cfg_get_adaptive(unsigned int, 7, g_cfg.rage_hitboxes, current_weapon), arrHitbox, IM_ARRAYSIZE(arrHitbox));

					edited::CheckboxWithPopup(true, "Hitchance", "Allows you to hit players with more accuracy", &cfg_get_adaptive(bool, 7, g_cfg.rage_hitchance, current_weapon));
					if (cfg_get_adaptive(bool, 7, g_cfg.rage_hitchance, current_weapon))
						ImGui::SliderInt(("chance"), &cfg_get_adaptive(int, 7, g_cfg.rage_minimum_hitchance, current_weapon), 0, 100);

					edited::CheckboxWithPopup(true, "Penetration", "Allows you to hit players thru objects", &cfg_get_adaptive(bool, 7, g_cfg.rage_penetration, current_weapon));
					if (cfg_get_adaptive(bool, 7, g_cfg.rage_penetration, current_weapon))
						ImGui::SliderInt(("Damage"), &cfg_get_adaptive(int, 7, g_cfg.rage_minimum_damage, current_weapon), 0, 100);

				}
				ImGui::EndChild();


				ImGui::SameLine(0, 10);

				edited::BeginChild("Selection", ImVec2((region.x - (145 + spacing.x * 3)) / 2, 0));
					{edited::CheckboxWithPopup(true, "No spread", "Accuratly shoots without any spread effect", &cfg_get(bool, g_cfg.remove_weapon_accuracy_spread));

					edited::CheckboxWithPopup(true, "Auto Fire", "", &cfg_get(bool, g_cfg.rage_auto_fire));

					edited::CheckboxWithPopup(true, "Auto stop", "Stops local player in order to maintain best accuracy", &cfg_get_adaptive(bool, 7, g_cfg.rage_auto_stop, current_weapon));

					static bool a;
					edited::CheckboxWithPopup(true, "Prefer Lethal", "", &cfg_get(bool, a));


					const char* aa[2]{ xorstr_("none"),xorstr_("Down") };
					edited::Combo(xorstr_("Pitch"), xorstr_(""), &cfg_get(int, g_cfg.aa_pitch), aa, IM_ARRAYSIZE(aa), 2);

					const char* a_a[3]{ xorstr_("none"),xorstr_("Backwards"),xorstr_("Jitter") };
					edited::Combo(xorstr_("Yaw"), xorstr_(""), &cfg_get(int, g_cfg.aa_yaw), a_a, IM_ARRAYSIZE(a_a), 3);

				}
				ImGui::EndChild();

	
			}
			
			if (var::active_tab == 1)
			{

				const char* weapons[7]{ ("Knife"), ("Pistols"), ("Heavy Pistols"),("Assult Rifles"), ("Auto"),("Scout"), ("Awp") };

				//ImGui::SetCursorPos(ImVec2(655, 0));
				//ImGui::InputText("##Search", search_buf, sizeof(search_buf), 0, ImVec2(140, 25));


				edited::BeginChild("General", ImVec2((region.x - (145 + spacing.x * 3)) / 2, 0));
				{
					edited::Combo(("Weapon"), "", &cfg_get(int, g_cfg.legit_weapon_selection), weapons, IM_ARRAYSIZE(weapons), 7);
					switch (current_weapon_legit) {
					case 0:
						edited::CheckboxWithPopup(
							true,
							"Enabled",
							"",
							&cfg_get(bool, g_cfg.legit_enable),
							(int*)&cfg_get(KeyBind_t, g_cfg.legit_key0).uKey,
							(int*)&cfg_get(KeyBind_t, g_cfg.legit_key0).nMode,
							&cfg_get(KeyBind_t, g_cfg.legit_key0).bEnable,
							true
						);
						break;

					case 1:
						edited::CheckboxWithPopup(
							true,
							"Enabled",
							"",
							&cfg_get(bool, g_cfg.legit_enable),
							(int*)&cfg_get(KeyBind_t, g_cfg.legit_key1).uKey,
							(int*)&cfg_get(KeyBind_t, g_cfg.legit_key1).nMode,
							&cfg_get(KeyBind_t, g_cfg.legit_key1).bEnable,
							true
						);
						break;

					case 2:
						edited::CheckboxWithPopup(
							true,
							"Enabled",
							"",
							&cfg_get(bool, g_cfg.legit_enable),
							(int*)&cfg_get(KeyBind_t, g_cfg.legit_key2).uKey,
							(int*)&cfg_get(KeyBind_t, g_cfg.legit_key2).nMode,
							&cfg_get(KeyBind_t, g_cfg.legit_key2).bEnable,
							true
						);
						break;

					case 3:
						edited::CheckboxWithPopup(
							true,
							"Enabled",
							"",
							&cfg_get(bool, g_cfg.legit_enable),
							(int*)&cfg_get(KeyBind_t, g_cfg.legit_key3).uKey,
							(int*)&cfg_get(KeyBind_t, g_cfg.legit_key3).nMode,
							&cfg_get(KeyBind_t, g_cfg.legit_key3).bEnable,
							true
						);
						break;

					case 4:
						edited::CheckboxWithPopup(
							true,
							"Enabled",
							"",
							&cfg_get(bool, g_cfg.legit_enable),
							(int*)&cfg_get(KeyBind_t, g_cfg.legit_key4).uKey,
							(int*)&cfg_get(KeyBind_t, g_cfg.legit_key4).nMode,
							&cfg_get(KeyBind_t, g_cfg.legit_key4).bEnable,
							true
						);
						break;

					case 5:
						edited::CheckboxWithPopup(
							true,
							"Enabled",
							"",
							&cfg_get(bool, g_cfg.legit_enable),
							(int*)&cfg_get(KeyBind_t, g_cfg.legit_key5).uKey,
							(int*)&cfg_get(KeyBind_t, g_cfg.legit_key5).nMode,
							&cfg_get(KeyBind_t, g_cfg.legit_key5).bEnable,
							true
						);
						break;

					case 6:
						edited::CheckboxWithPopup(
							true,
							"Enabled",
							"",
							&cfg_get(bool, g_cfg.legit_enable),
							(int*)&cfg_get(KeyBind_t, g_cfg.legit_key6).uKey,
							(int*)&cfg_get(KeyBind_t, g_cfg.legit_key6).nMode,
							&cfg_get(KeyBind_t, g_cfg.legit_key6).bEnable,
							true
						);
						break;

					default:
						break;
					}
					//(edited::CheckboxWithPopup(false, "Silent", "", &cfg_get(bool, g_cfg.legit_silent)));

					edited::SliderFloat(xorstr_("FOV"), &cfg_get_adaptive(float, 7, g_cfg.legit_fov, current_weapon_legit), 0, 180);
					(edited::CheckboxWithPopup(true, "Visualize fov", "", &cfg_get(bool, g_cfg.renderize_fov), 0, 0, 0, 0, true, &cfg_get(ColorPickerVar_t, g_cfg.renderize_fov_clr).colValue, 0));
					edited::SliderInt(xorstr_("Target ms delay"), &cfg_get(int, g_cfg.legit_target_delay), 0, 750);

					edited::MultiCombo("Disable if", &cfg_get(unsigned int, g_cfg.legit_ignore), arrLegit, IM_ARRAYSIZE(arrLegit));

					const char* targets_select[4]{ xorstr_("none"),xorstr_("Crosshair"), xorstr_("Damage"),xorstr_("Distance") };
					edited::Combo(xorstr_("Target selection"), xorstr_("Select target based on conditions"), &cfg_get_adaptive(int, 7, g_cfg.legit_target_select, current_weapon_legit), targets_select, IM_ARRAYSIZE(targets_select), 3);
					edited::MultiCombo("Hitboxes", &cfg_get_adaptive(unsigned int, 7, g_cfg.legit_hitboxes, current_weapon_legit), arrHitbox, IM_ARRAYSIZE(arrHitbox));

				}
				edited::EndChild();

				ImGui::SameLine(0, 10);

				edited::BeginChild("Selection", ImVec2((region.x - (145 + spacing.x * 3)) / 2, 0));
				{
					(edited::CheckboxWithPopup(true, "Enable rcs", "", &cfg_get_adaptive(bool, 7, g_cfg.legit_rcs, current_weapon_legit)));

					edited::SliderInt(xorstr_("rcs after x shots"), &cfg_get(int, g_cfg.recoil_shots), 0, 10);
					edited::CheckboxWithPopup(true, "Smoothing", "", &cfg_get_adaptive(bool, 7, g_cfg.legit_smoothing, current_weapon_legit));

						edited::SliderFloat(xorstr_("Smooth"), &cfg_get_adaptive(float, 7, g_cfg.legit_smooth_x, current_weapon_legit), 0.f, 20.f);
						edited::SliderFloat(xorstr_("Smooth"), &cfg_get_adaptive(float, 7, g_cfg.legit_smooth_y, current_weapon_legit), 0.f, 20.f);

					(edited::CheckboxWithPopup(true, "Triggerbot", "", &cfg_get_adaptive(bool, 7, g_cfg.legit_triggerbot, current_weapon_legit), (int*)&cfg_get(KeyBind_t, g_cfg.legit_trigger_bot_key).uKey, (int*)&cfg_get(KeyBind_t, g_cfg.legit_trigger_bot_key).nMode, &cfg_get(KeyBind_t, g_cfg.legit_trigger_bot_key).bEnable, true, false, 0, 0));
					{				
						edited::MultiCombo("Trigger Hitboxes", &cfg_get_adaptive(unsigned int, 7, g_cfg.trigger_hitbox, current_weapon_legit), arrHitbox, IM_ARRAYSIZE(arrHitbox));

						(edited::CheckboxWithPopup(true, "Only on key", "", &cfg_get_adaptive(bool, 7, g_cfg.trigger_on_key, current_weapon_legit)));

						edited::SliderInt("Delay", &cfg_get_adaptive(int, 7, g_cfg.trigger_delay, current_weapon_legit), 0, 1000);
						edited::SliderInt("Minimum damage", &cfg_get_adaptive(int, 7, g_cfg.trigger_dmg, current_weapon_legit), 0, 100);
					}
				}
				edited::EndChild();

				// bottom group boxes

			}
			else if (var::active_tab == 2) {

				edited::BeginChild("Player", ImVec2((region.x - (145 + spacing.x * 3)) / 2, 0));
				{


					(edited::CheckboxWithPopup(true, "Enabled", "", &cfg_get(bool, g_cfg.bVisualOverlay), 0, 0, 0, 0, 0, 0, 0));

					(edited::CheckboxWithPopup(true, "Name", "", &cfg_get(TextOverlayVar_t, g_cfg.overlayName).bEnable, 0, 0, 0, 0, true, &cfg_get(TextOverlayVar_t, g_cfg.overlayName).colPrimary, 0));

					(edited::CheckboxWithPopup(true, "Skeleton", "", &cfg_get(TextOverlayVar_t, g_cfg.bSkeleton).bEnable, 0, 0, 0, 0, true, &cfg_get(TextOverlayVar_t, g_cfg.colSkeleton).colPrimary, 0));

					(edited::CheckboxWithPopup(true, "Bounding box", "", &cfg_get(FrameOverlayVar_t, g_cfg.overlayBox).bEnable, 0, 0, 0, 0, true, &cfg_get(FrameOverlayVar_t, g_cfg.overlayBox).colPrimary, 0));

					(edited::CheckboxWithPopup(true, "Health", "", &cfg_get(FrameOverlayVar_t, g_cfg.overlayHealthBar).bEnable, 0, 0, 0, 0, true, &cfg_get(FrameOverlayVar_t, g_cfg.overlayHealthBar).colPrimary, 0));

					(edited::CheckboxWithPopup(true, "Ammo bar", "", &cfg_get(BarOverlayVar_t, g_cfg.AmmoBar).bEnable, 0, 0, 0, 0, true, &cfg_get(BarOverlayVar_t, g_cfg.AmmoBar).colPrimary, 0));

					(edited::CheckboxWithPopup(true, "Weapon", "", &cfg_get(TextOverlayVar_t, g_cfg.Weaponesp).bEnable, 0, 0, 0, 0, true, &cfg_get(TextOverlayVar_t, g_cfg.Weaponesp).colPrimary, 0));

					(edited::CheckboxWithPopup(true, "Glow", "", &cfg_get(bool, g_cfg.bGlow), 0, 0, 0, 0, true, &cfg_get(ColorPickerVar_t, g_cfg.bGlowClr).colValue, 0));

					edited::MultiCombo(("Flags"), &cfg_get(unsigned int, g_cfg.pEspFlags), arrEspFlags, IM_ARRAYSIZE(arrEspFlags));

				}
				edited::EndChild();

				ImGui::SameLine(0, 10);

				edited::BeginChild("Chams", ImVec2((region.x - (145 + spacing.x * 3)) / 2, 0));
				{
					edited::Combo(("Player"), ("Select entity for chams modulation"), &cfg_get(int, g_cfg.chams_type), chams_type, IM_ARRAYSIZE(chams_type), 3);
					{
						if (cfg_get(int, g_cfg.chams_type) == 0) {

							(edited::CheckboxWithPopup(true, "Chams", "", &cfg_get(bool, g_cfg.bVisualChams), 0, 0, 0, 0, true, &cfg_get(ColorPickerVar_t, g_cfg.colVisualChams).colValue, 0));
							(edited::CheckboxWithPopup(true, "Invisible", "", &cfg_get(bool, g_cfg.bVisualChamsIgnoreZ), 0, 0, 0, 0, true, &cfg_get(ColorPickerVar_t, g_cfg.colVisualChamsIgnoreZ).colValue, 0));

							const char* chams[5]{ ("Flat"), ("Default"),("Illumin"), ("Glow"), ("Latex") };
							edited::Combo(("Models"), (""), &cfg_get(int, g_cfg.nVisualChamMaterial), chams, IM_ARRAYSIZE(chams), 5);
							edited::CheckboxWithPopup(false, "Radgoll", "", &cfg_get(bool, g_cfg.bVisualChamsRagDoll));

						}
						else 	if (cfg_get(int, g_cfg.chams_type) == e_chams_type::local) {
							(edited::CheckboxWithPopup(true, "Chams", "", &cfg_get(bool, g_cfg.bVisualChamsL), 0, 0, 0, 0, true, &cfg_get(ColorPickerVar_t, g_cfg.colVisualChamsL).colValue, 0));
							const char* chams[5]{ ("Flat"), ("Default"),("Illumin"), ("Glow"), ("Latex") };
							edited::Combo(("Models"), (""), &cfg_get(int, g_cfg.nVisualChamMaterialL), chams, IM_ARRAYSIZE(chams), 5);
						}
						else 	if (cfg_get(int, g_cfg.chams_type) == 2) {

							(edited::CheckboxWithPopup(true, "Chams", "", &cfg_get(bool, g_cfg.bVisualChamsT), 0, 0, 0, 0, true, &cfg_get(ColorPickerVar_t, g_cfg.colVisualChamsT).colValue, 0));
							(edited::CheckboxWithPopup(true, "Invisible", "", &cfg_get(bool, g_cfg.bVisualChamsIgnoreZT), 0, 0, 0, 0, true, &cfg_get(ColorPickerVar_t, g_cfg.colVisualChamsIgnoreZT).colValue, 0));
							const char* chams[5]{ ("Flat"), ("Default"),("Illumin"), ("Glow"), ("Latex") };
							edited::Combo(("Models"), (""), &cfg_get(int, g_cfg.nVisualChamMaterialT), chams, IM_ARRAYSIZE(chams), 5);
						}
						else 	if (cfg_get(int, g_cfg.chams_type) == e_chams_type::viewmodel) {
							(edited::CheckboxWithPopup(true, "Chams", "", &cfg_get(bool, g_cfg.nVisualChamMaterialV), 0, 0, 0, 0, true, &cfg_get(ColorPickerVar_t, g_cfg.colVisualChamsV).colValue, 0));
							const char* chams[5]{ ("Flat"), ("Default"),("Illumin"), ("Glow"), ("Latex") };
							edited::Combo(("Models"), (""), &cfg_get(int, g_cfg.nVisualChamMaterialV), chams, IM_ARRAYSIZE(chams), 5);
						}
						else 	if (cfg_get(int, g_cfg.chams_type) == e_chams_type::wep) {


							(edited::CheckboxWithPopup(true, "Chams", "", &cfg_get(bool, g_cfg.bVisualChamsW), 0, 0, 0, 0, true, &cfg_get(ColorPickerVar_t, g_cfg.colVisualChamsW).colValue, 0));
							const char* chams[5]{ ("Flat"), ("Default"),("Illumin"), ("Glow"), ("Latex") };
							edited::Combo(("Models"), (""), &cfg_get(int, g_cfg.nVisualChamMaterialW), chams, IM_ARRAYSIZE(chams), 5);


						}
					}
				}
				edited::EndChild();

			}
			else if (var::active_tab == 3) {

				edited::BeginChild("General", ImVec2((region.x - (145 + spacing.x * 3)) / 2, 0));
				{

					edited::CheckboxWithPopup(true, xorstr_("Spectator"), xorstr_("Shows who's spectating you"), &cfg_get(bool, g_cfg.spectator_list));

					edited::CheckboxWithPopup(true, xorstr_("Keybind"), xorstr_("Show currend enabled binds"), &cfg_get(bool, g_cfg.key_bind_list));

					(edited::CheckboxWithPopup(true, "Grenade Prediction", "", &cfg_get(bool, g_cfg.grenade_prediction), 0, 0, 0, 0, true, &cfg_get(ColorPickerVar_t, g_cfg.grenade_prediction_col).colValue, 0));
					(edited::CheckboxWithPopup(true, "Molotov", "", &cfg_get(bool, g_cfg.molotov_radius), 0, 0, 0, 0, true, &cfg_get(ColorPickerVar_t, g_cfg.molotov_radius_clr).colValue, 0));

					edited::CheckboxWithPopup(true, xorstr_("Hitsound"), xorstr_("Show currend enabled binds"), &cfg_get(bool, g_cfg.hit_sound));
					//(edited::CheckboxWithPopup(true, "Bullet beam", "", &cfg_get(bool, g_cfg.bullet_beam_local), 0, 0, 0, 0, true, &cfg_get(ColorPickerVar_t, g_cfg.bullet_beam_local_clr).colValue, 0));

					//if (cfg_get(bool, g_cfg.bullet_beam_local)) {
					//	edited::SliderFloat(xorstr_("Beam width"), &cfg_get(float, g_cfg.bullet_beam_width), 1.f, 5.f, xorstr_("%.1f"), ImGuiSliderFlags_AlwaysClamp);

					//}
					edited::CheckboxWithPopup(true, xorstr_("Unlock Inventory"), xorstr_("Unlock game Inventory"), &cfg_get(bool, g_cfg.unlock_inventory));
					(edited::CheckboxWithPopup(true, "ThirdPerson", "", &cfg_get(bool, g_cfg.thirdperson), (int*)&cfg_get(KeyBind_t, g_cfg.third_person_key).uKey, (int*)&cfg_get(KeyBind_t, g_cfg.third_person_key).nMode, &cfg_get(KeyBind_t, g_cfg.third_person_key).bEnable, true));
					edited::MultiCombo("Removals", &cfg_get(unsigned int, g_cfg.game_removals), arr_game_removals, IM_ARRAYSIZE(arr_game_removals));

					if (ImGui::Button("Save Config"))
					{
						C::SaveFile(0);
						C::SaveSkinToJson(0);
						ImU32 color = IM_COL32(255, 100, 150, 255);
						//notificationSystem.DrawNotifications();
						notificationSystem.AddNotification("d", "Successful", "Your settings have been saved successfully. ", 5000, color);
						// Add logic for saving a config
					}
					ImGui::SameLine();
					if (ImGui::Button("Load Config"))
					{

						C::LoadFile(0);
						C::LoadSkinFromJson(0);
						ImU32 color = IM_COL32(255, 100, 150, 255);
						//notificationSystem.DrawNotifications();
						notificationSystem.AddNotification("d", "Successful", "Your settings have been loaded successfully. ", 5000, color);
						// Add logic for loading a config
					}

				}
				edited::EndChild();

				ImGui::SameLine(0, 10);

				edited::BeginChild("Movement", ImVec2((region.x - (145 + spacing.x * 3)) / 2, 0));
				{




					edited::CheckboxWithPopup(true, xorstr_("Bunny hop"), xorstr_("Automatic jumps for you"), &cfg_get(bool, g_cfg.bunny_hop));
					edited::CheckboxWithPopup(true, xorstr_("Fast ladder"), xorstr_("Climbs ladder in a fast way"), &cfg_get(bool, g_cfg.fast_ladder));
					edited::CheckboxWithPopup(true, xorstr_("Automatic strafe"), xorstr_("Adjusts strafing"), &cfg_get(bool, g_cfg.auto_strafe));

					if (cfg_get(bool, g_cfg.auto_strafe)) {

						edited::CheckboxWithPopup(false, "Quick stop", "", &cfg_get(bool, g_cfg.movement_quick_stop));
						edited::CheckboxWithPopup(true, xorstr_("Movement strafe"), xorstr_("movement wasd Keys helper"), &cfg_get(bool, g_cfg.auto_strafe_wasd));
						edited::SliderFloat(xorstr_("Strafe smooth"), &cfg_get(float, g_cfg.auto_strafe_smooth), 0.f, 100.f);


					}
				}
				edited::EndChild();

			}
			else if (var::active_tab == 4) {

				edited::BeginChild("Weapon", ImVec2((region.x - (145 + spacing.x * 3)) / 2, 0));
				{

					(edited::CheckboxWithPopup(true, "Enabled", "", &cfg_get(bool, g_cfg.weapon_esp), 0, 0, 0, 0, 0, 0, 0));

					(edited::CheckboxWithPopup(true, "Name", "", &cfg_get(TextOverlayVar_t, g_cfg.weapon_name).bEnable, 0, 0, 0, 0, true, &cfg_get(TextOverlayVar_t, g_cfg.weapon_name).colPrimary, 0));

					(edited::CheckboxWithPopup(true, "Box", "", &cfg_get(FrameOverlayVar_t, g_cfg.weapon_bbox).bEnable, 0, 0, 0, 0, true, &cfg_get(FrameOverlayVar_t, g_cfg.weapon_bbox).colPrimary, 0));

					(edited::CheckboxWithPopup(true, "Ammo Bar", "", &cfg_get(FrameOverlayVar_t, g_cfg.weapon_ammo_bar).bEnable, 0, 0, 0, 0, true, &cfg_get(FrameOverlayVar_t, g_cfg.weapon_ammo_bar).colPrimary, 0));







				}
				edited::EndChild();

				ImGui::SameLine(0, 10);

				edited::BeginChild("World View", ImVec2((region.x - (145 + spacing.x * 3)) / 2, 0));
				{


					edited::CheckboxWithPopup(true, "Skybox", "", &cfg_get(bool, g_cfg.modulate_sky), 0, 0, 0, 0, true, &cfg_get(ColorPickerVar_t, g_cfg.sky_col).colValue, 0);
					if (cfg_get(bool, g_cfg.modulate_sky))
						edited::SliderFloat(xorstr_("Brightness"), &cfg_get(float, g_cfg.sky_brightness), 0.f, 1.f);


					edited::CheckboxWithPopup(false, xorstr_("Motion blur"), xorstr_(""), &cfg_get(bool, g_cfg.motion_blur));
					if (cfg_get(bool, g_cfg.motion_blur))
						edited::CheckboxWithPopup(false, xorstr_("Compensate exposure"), xorstr_(""), &cfg_get(bool, g_cfg.compensate_exposure));


					edited::CheckboxWithPopup(false, xorstr_("Camera fov"), xorstr_(""), &cfg_get(bool, g_cfg.camera_fov));
					if (cfg_get(bool, g_cfg.camera_fov))
						edited::SliderFloat(xorstr_("Camera fov"), &cfg_get(float, g_cfg.camera_fov_value), 0.f, 180.f);


					edited::CheckboxWithPopup(false, xorstr_("Custom viewmodel"), xorstr_(""), &cfg_get(bool, g_cfg.view_model));
					if (cfg_get(bool, g_cfg.view_model)) {
						edited::SliderFloat(xorstr_("Fov"), &cfg_get(float, g_cfg.view_model_fov), 0.f, 180.f);
						edited::SliderFloat(xorstr_("x"), &cfg_get(float, g_cfg.view_model_x), -180.f, 180.f);
						edited::SliderFloat(xorstr_("y"), &cfg_get(float, g_cfg.view_model_y), -180.f, 180.f);
						edited::SliderFloat(xorstr_("z"), &cfg_get(float, g_cfg.view_model_z), -180.f, 180.f);
					}
				}
				edited::EndChild();

			}
			else if (var::active_tab == 5) {
				bool a = false;
				bool b = false;

				int same_line_counter = 0;
				edited::BeginChild("Inventory Changer", ImVec2((region.x - (145 + spacing.x * 3)), 0), true, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar);
				{
					static ImGuiTextFilter filter;
					static char filter_text[128] = "";
					float scroll_speed = 5.0f; // Scroll speed (adjust as needed)
					float current_scroll_y = ImGui::GetScrollY(); // Get current scroll position
					float scroll_max_y = ImGui::GetScrollMaxY(); // Get maximum scrollable distance

					// Adjust scroll position based on mouse wheel movement
					if (scroll_max_y > 0) {
						current_scroll_y -= ImGui::GetIO().MouseWheel * scroll_speed;
						current_scroll_y = ImClamp(current_scroll_y, 0.0f, scroll_max_y); // Limit scrolling
						ImGui::SetScrollY(current_scroll_y);
					}


					{
						if (skin_page > 0) {
							if (edited::button(b, "back", ImVec2(80, 30))) {


								skin_page = home_page; //std::clamp(last_page == 1 ? home_page : last_page - 1, 0, 6);
							}
							ImGui::SameLine();
							if (edited::button(a, "update", ImVec2(80, 30))) {
								update = true;
								update_gloves = true;
								update_knife = true;
							}
							ImGui::SetCursorPos(ImVec2(233, 25));

							if (skin_page >= 1 && skin_page != inventory_page::paint_config) {
								filter.Draw("search", 400);
							}

						}
						if (skin_page == home_page) {
							last_page = home_page;

							ImGui::SetCursorPosX(20.f);
							if (edited::ButtonPlus("##Add_item_button", ImVec2(115, 140), 0, render::m_plus)) {
								skin_page = item_list;

							}

							// if we have already added/saved skins draw them
							if (!added_skins.empty()) {

								for (auto& Item : added_skins) {

									if (Item.image == nullptr) {
										Item.image = get_image_x(Item.path_image);
									}
									if (!Item.image)
										continue;

									if (edited::ImageSkinButton(Item.name, Item.image, ImVec2(130, 110), get_color_skin(Item.rarity).GetVec4())) {
										// add notify option to remove skin
									}

									if (same_line_counter < 3) { // continue push same-line
										ImGui::SameLine();
										same_line_counter++;
									}
									else { // we have maximum elements in 1 line
										same_line_counter = 0;
									}
								}

							}
						}

						if (skin_page == item_list) {
							last_page = item_list;
							ImGui::SetCursorPosY(85);

							for (auto& Item : items_list) {
								if (!filter.PassFilter(Item.name.c_str()))
									continue;

								if (!Item.texture_id) {
									continue;
								}
								if (edited::ImageSkinButton(Item.name.c_str(), Item.texture_id, ImVec2(130, 110), get_color_skin(Item.rarity).GetVec4())) {
									selected_item = &Item;


									skin_page = inventory_page::weapon;
								}

								if (same_line_counter < 4) { // continue push same-line
									ImGui::SameLine();
									same_line_counter++;
								}
								else { // we have maximum elements in 1 line
									same_line_counter = 0;
								}
							}
						}

						// awp deagle pistol ak etc
						if (skin_page == inventory_page::weapon) {
							last_page = inventory_page::weapon;
							ImGui::SetCursorPosY(85);

							if (selected_item) {

								switch (selected_item->type) {
								case type_item::type_weapons:

									// get all weapons default skins
									for (auto& Item : item_default) {
										if (!filter.PassFilter(Item.name.c_str()))
											continue;

										if (!Item.texture_id) {
											continue;
										}

										if (edited::ImageSkinButton(Item.name.c_str(), Item.texture_id, ImVec2(130, 110), get_color_skin(Item.rarity).GetVec4())) {
											selected_weapon = &Item;


											skin_page = inventory_page::paints;

										}
										if (same_line_counter < 4) { // continue push same-line
											ImGui::SameLine();
											same_line_counter++;
										}
										else { // we have maximum elements in 1 line
											same_line_counter = 0;
										}
									}

									break;

								case type_item::type_knife:


									// get all weapons default skins
									for (auto& Item : item_knife_default) {
										if (!filter.PassFilter(Item.name.c_str()))
											continue;

										if (!Item.texture_id) {
											continue;
										}

										if (edited::ImageSkinButton(Item.name.c_str(), Item.texture_id, ImVec2(130, 110), get_color_skin(Item.rarity).GetVec4())) {
											selected_weapon = &Item;


											skin_page = inventory_page::paints;

										}
										if (same_line_counter < 4) { // continue push same-line
											ImGui::SameLine();
											same_line_counter++;
										}
										else { // we have maximum elements in 1 line
											same_line_counter = 0;
										}
									}

									break;
								case type_item::type_glove:


									// get all weapons default skins
									for (auto& skins : item_glove_default) {
										if (!filter.PassFilter(skins.name.c_str()))
											continue;

										if (!skins.texture_id) {
											continue;
										}

										if (edited::ImageSkinButton(skins.name.c_str(), skins.texture_id, ImVec2(130, 110), get_color_skin(skins.rarity).GetVec4())) {
											selected_weapon = &skins;
											skin_page = inventory_page::paints;

										}
										if (same_line_counter < 4) { // continue push same-line
											ImGui::SameLine();
											same_line_counter++;
										}
										else { // we have maximum elements in 1 line
											same_line_counter = 0;
										}


									}
									break;
								}

							}

						}
						if (skin_page == inventory_page::paints) {
							last_page = inventory_page::paints;
							std::thread(init_thread).detach();

							ImGui::SetCursorPosY(85);

							if (selected_weapon && !selected_weapon->paintkits.empty()) {



								for (auto& skin : selected_weapon->paintkits) {

									if (!filter.PassFilter(skin.name))
										continue;

									if (!skin.image)
										continue;


									if (skin.image) {
										if (edited::ImageSkinButton(skin.name, skin.image, ImVec2(120, 110), get_color_skin(skin.rarity).GetVec4())) {
											selected_weapon->current_paint = &skin;


											skin_page = inventory_page::paint_config;

										}
										if (same_line_counter < 3) {
											ImGui::SameLine();
											same_line_counter++;
										}
										else {
											same_line_counter = 0;
										}
									}
								}



							}
						}

						if (skin_page == inventory_page::paint_config) {
							last_page = inventory_page::paint_config;
							ImGui::SetCursorPos(ImVec2(20, 88));

							if (selected_weapon) {

								if (selected_weapon->current_paint) {
									auto skin = selected_weapon->current_paint;
									if (!skin->image)
										return;

									static int wear = 0.f;
									static int seed = 1;
									static int stattrack = 61;
									static char skin_seed[32];
									static char skin_stattrack[32];
									static char skin_wear[32];
									edited::ImageSkin(skin->name, skin->image, ImVec2(280, 230), ImGui::VGetColorU32(ImColor(get_color_skin(skin->rarity).r, get_color_skin(skin->rarity).g, get_color_skin(skin->rarity).b, get_color_skin(skin->rarity).a), 1.f));
									ImGui::SameLine();
									if (edited::ButtonPlus("##equip_skin", ImVec2(88, 88), 0, settingss)) {

										auto pItem = CEconItem::CreateInstance();
										if (pItem) {
											auto pInventory = CCSPlayerInventory::GetInstance();
											if (pInventory) {
												auto highestIDs = pInventory->GetHighestIDs();

												pItem->m_ulID = highestIDs.first + 1;
												pItem->m_unInventory = highestIDs.second + 1;
												pItem->m_unAccountID = uint32_t(pInventory->GetOwner().m_id);
												pItem->m_unDefIndex = selected_weapon->item_def;

												pItem->m_nRarity = skin->rarity;

												pItem->SetPaintKit((float)skin->id);
												pItem->SetPaintSeed((float)seed);
												pItem->SetPaintWear((float)wear);
												if (stattrack >= 0) {
													pItem->SetStatTrak(stattrack);


												}

												// Assuming selected_weapon->added_stickers stores stickers with their positions (indices)
												if (!selected_weapon->added_stickers.empty()) {
													for (const auto& [index, sticker] : selected_weapon->added_stickers) {
														if (sticker.item_def <= 0)
														{
															continue;
														}
														// Ensure the index is within the valid range
														if (index < 5 && index >= 0) {
															pItem->set_sticker(index, sticker.item_def);
														}
													}
												}
												int team = 0;
												int slot = 0;

												for (int i = 0; i <= 3; i++)
												{
													CEconItemDefinition* item_view_loadout = nullptr;
													for (int j = 0; j <= 56; ++j) {
														C_EconItemView* item_view_ = pInventory->GetItemInLoadout(i, j);
														if (!item_view_)
															continue;

														auto econ_item_def = item_view_->GetStaticData();

														team = i;
														slot = j;
													}
												}



												EconItem_t econItem = {
													   pItem->m_ulID,
													   pItem->m_unInventory,
													   pItem->m_unAccountID,
													   pItem->m_unDefIndex,
													   pItem->m_nRarity,
													   skin->id,
													   (float)seed,
													   (float)wear,
													   stattrack,
													   0,
													   pItem->m_nQuality,
													   skin->path_image.c_str(),
													   true,
													   skin->name,
													   team,
													   slot
												};

												c_paintKits paintkits_ = *selected_weapon->current_paint;
												added_skins.emplace_back(paintkits_);

												added_item_list.emplace_back(econItem);


												if (pInventory->AddEconItem(pItem))
													Items::skins->add(pItem);

												wear = 0.f;
												seed = 1;
												stattrack = -1;
											}
										}
									}


								}
							}
						}
					}
				}
				ImGui::EndChild();


			}
			else if (var::active_tab == 5)
			{
				ImGui::SameLine(0, 10);

				edited::BeginChild("Configuration Manager", ImVec2((region.x - (145 + spacing.x * 3)) / 2, 0));
				{


					// Input field for naming a config
					static char config_name[64] = "";
					ImGui::InputText("Config Name", config_name, IM_ARRAYSIZE(config_name));

					// List available configs (dummy data here, replace with actual list)
					ImGui::Text("Available Configs:");
					for (int i = 0; i < 5; i++) // Replace 5 with the actual number of configs
					{
						edited::Selectable(("Config " + std::to_string(i)).c_str());
					}


					ImGui::SameLine();
					if (ImGui::Button("Delete Config"))
					{
						// Add logic for deleting a config
					}
				}
				edited::EndChild();
			}

		}
		ImGui::EndChild();
		PopClipRect();
	}
	ImGui::End();

}
void render::update_style(ImGuiStyle* pStyle)
{

}

