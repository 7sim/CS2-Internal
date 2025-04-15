#include "render.h"
#include "ui\menu.h"

#include "../../defs.h"
#include "fonts/icon.h"
#include "../utils.h"
#include <imgui_internal.h>
#include "..\..\..\core\cheats\skins\skins.h"

#include "..\..\..\core\cheats\visuals\proccess.h"
#include "animations/easing.h"
#include <imgui_settings.h>
#include "fonts/modern_icon.hpp"
// Declare a mutex to ensure thread safety during initialization
std::mutex init_mutex;
bool inited = false;

void parse_items() {
	// Lock to ensure thread safety during initialization
	std::lock_guard<std::mutex> lock(init_mutex);

	if (!inited) {
		Items::skins->parse();
		inited = true;
	}
}
#include "d3d11/images/images.h"
#include "..\..\utils\hook\hooks.h"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
static bool set = false;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
bool render::OnWndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	if (!m_initialized)
		return false;

	Input::OnWndProc(hwnd, umsg, wparam, lparam);


	if (Input::IsKeyReleased(VK_INSERT)) {
		menu::m_opened = !menu::m_opened;
		const auto oIsRelativeMouseMode = hooks::m_relative_mouse.GetOriginal();
		oIsRelativeMouseMode(sdk::m_input_system, menu::m_opened ? false : menu::m_wind_active);
	}
	ImGui_ImplWin32_WndProcHandler(hwnd, umsg, wparam, lparam);
	// disable input if GUI is open.
	return    menu::m_opened;
}


ImDrawList* render::GetDrawList( )
{
	// get draw list
	ImDrawList* draw_list = ImGui::GetBackgroundDrawList( );

	// sanity
	if ( draw_list == nullptr )
	{
		// log error
		CS2_LOG( xorstr_( "draw_list was null while calling render::DrawString" ) );

		// return out from the function
		return nullptr;
	}

	// return draw list
	return draw_list;
}


bool render::WorldToScreen(const Vector_t& in, ImVec2& out)
{
	if (!ImGui::GetCurrentContext())
		return false;

	auto z = g::m_view_matrix[3][0] * in.x + g::m_view_matrix[3][1] * in.y + g::m_view_matrix[3][2] * in.z + g::m_view_matrix[3][3];
	if (z < 0.001f) return false;

	out = ImGui::GetIO().DisplaySize * 0.5f;
	out.x *= 1.0f + (g::m_view_matrix[0][0] * in.x + g::m_view_matrix[0][1] * in.y + g::m_view_matrix[0][2] * in.z + g::m_view_matrix[0][3]) / z;
	out.y *= 1.0f - (g::m_view_matrix[1][0] * in.x + g::m_view_matrix[1][1] * in.y + g::m_view_matrix[1][2] * in.z + g::m_view_matrix[1][3]) / z;

	// Prevents rounded corners.
	out = ImFloor(out);

	return true;
}


void render::AddDrawListRect(ImDrawList* pDrawList, const ImVec2& vecMin, const ImVec2& vecMax, const Color_t& colRect, const unsigned int uFlags, const Color_t& colOutline, const float flRounding, const ImDrawFlags roundingCorners, float flThickness, const float flOutlineThickness)
{
	if (pDrawList == nullptr)
		pDrawList = GetDrawList();

	const ImU32 colRectPacked = colRect.GetU32();
	const ImU32 colOutlinePacked = colOutline.GetU32();

	if (uFlags & DRAW_RECT_FILLED)
		pDrawList->AddRectFilled(vecMin, vecMax, colRectPacked, flRounding, roundingCorners);
	else
	{
		pDrawList->AddRect(vecMin, vecMax, colRectPacked, flRounding, roundingCorners, flThickness);
		flThickness *= 0.5f;
	}

	const float flHalfOutlineThickness = flOutlineThickness * 0.5f;
	const ImVec2 vecThicknessOffset = { flThickness + flHalfOutlineThickness, flThickness + flHalfOutlineThickness };

	if (uFlags & DRAW_RECT_BORDER)
		pDrawList->AddRect(vecMin + vecThicknessOffset, vecMax - vecThicknessOffset, colOutlinePacked, flRounding, roundingCorners, flOutlineThickness);

	if (uFlags & DRAW_RECT_OUTLINE)
		pDrawList->AddRect(vecMin - vecThicknessOffset, vecMax + vecThicknessOffset, colOutlinePacked, flRounding, roundingCorners, flOutlineThickness);
}

void render::AddDrawListRectMultiColor(ImDrawList* pDrawList, const ImVec2& vecMin, const ImVec2& vecMax, const Color_t& colUpperLeft, const Color_t& colUpperRight, const Color_t& colBottomRight, const Color_t& colBottomLeft)
{
	if (pDrawList == nullptr)
		pDrawList = GetDrawList();

	pDrawList->AddRectFilledMultiColor(vecMin, vecMax, colUpperLeft.GetU32(), colUpperRight.GetU32(), colBottomRight.GetU32(), colBottomLeft.GetU32());
}

void render::AddDrawListCircle(ImDrawList* pDrawList, const ImVec2& vecCenter, const float flRadius, const Color_t& colCircle, const int nSegments, const unsigned int uFlags, const Color_t& colOutline, float flThickness, const float flOutlineThickness)
{
	if (pDrawList == nullptr)
		pDrawList = GetDrawList();

	const ImU32 colCirclePacked = colCircle.GetU32();

	if (uFlags & DRAW_CIRCLE_FILLED)
	{
		pDrawList->AddCircleFilled(vecCenter, flRadius, colCirclePacked, nSegments);
		flThickness = 0.0f;
	}
	else
		pDrawList->AddCircle(vecCenter, flRadius, colCirclePacked, nSegments, flThickness);

	if (uFlags & DRAW_CIRCLE_OUTLINE)
		pDrawList->AddCircle(vecCenter, flRadius + flOutlineThickness, colOutline.GetU32(), nSegments, flThickness + flOutlineThickness);
}

void render::AddDrawListArc(ImDrawList* pDrawList, const ImVec2& vecPosition, const float flRadius, const float flMinimumAngle, const float flMaximumAngle, const Color_t& colArc, const float flThickness)
{
	if (pDrawList == nullptr)
		pDrawList = GetDrawList();

	pDrawList->PathArcTo(vecPosition, flRadius, M_DEG2RAD(flMinimumAngle), M_DEG2RAD(flMaximumAngle), 32);
	pDrawList->PathStroke(colArc.GetU32(), false, flThickness);
}

void render::AddDrawListLine(ImDrawList* pDrawList, const ImVec2& vecFirst, const ImVec2& vecSecond, const Color_t& colLine, const float flThickness)
{
	if (pDrawList == nullptr)
		pDrawList = GetDrawList();

	pDrawList->AddLine(vecFirst, vecSecond, colLine.GetU32(), flThickness);
}

void render::AddDrawListTriangle(ImDrawList* pDrawList, const ImVec2& vecFirst, const ImVec2& vecSecond, const ImVec2& vecThird, const Color_t& colTriangle, const unsigned int uFlags, const Color_t& colOutline, const float flThickness)
{
	if (pDrawList == nullptr)
		pDrawList = GetDrawList();

	const ImU32 colTrianglePacked = colTriangle.GetU32();

	if (uFlags & DRAW_TRIANGLE_FILLED)
		pDrawList->AddTriangleFilled(vecFirst, vecSecond, vecThird, colTrianglePacked);
	else
		pDrawList->AddTriangle(vecFirst, vecSecond, vecThird, colTrianglePacked, flThickness);

	if (uFlags & DRAW_TRIANGLE_OUTLINE)
		pDrawList->AddTriangle(vecFirst, vecSecond, vecThird, colOutline.GetU32(), flThickness + 1.0f);
}

void render::AddDrawListQuad(ImDrawList* pDrawList, const ImVec2& vecFirst, const ImVec2& vecSecond, const ImVec2& vecThird, const ImVec2& vecFourth, const Color_t& colQuad, const unsigned int uFlags, const Color_t& colOutline, const float flThickness)
{
	if (pDrawList == nullptr)
		pDrawList = GetDrawList();

	const ImU32 colQuadPacked = colQuad.GetU32();

	if (uFlags & DRAW_QUAD_FILLED)
		pDrawList->AddQuadFilled(vecFirst, vecSecond, vecThird, vecFourth, colQuadPacked);
	else
		pDrawList->AddQuad(vecFirst, vecSecond, vecThird, vecFourth, colQuadPacked, flThickness);

	if (uFlags & DRAW_QUAD_OUTLINE)
		pDrawList->AddQuad(vecFirst, vecSecond, vecThird, vecFourth, colOutline.GetU32(), flThickness + 1.0f);
}

void render::AddDrawListPolygon(ImDrawList* pDrawList, const ImVec2* vecPoints, const int nPointsCount, const Color_t& colPolygon, unsigned int uFlags, const Color_t& colOutline, const bool bClosed, const float flThickness)
{
	if (pDrawList == nullptr)
		pDrawList = GetDrawList();

	const ImU32 colPolygonPacked = colPolygon.GetU32();

	if (uFlags & DRAW_POLYGON_FILLED)
		pDrawList->AddConvexPolyFilled(vecPoints, nPointsCount, colPolygonPacked);
	else
		pDrawList->AddPolyline(vecPoints, nPointsCount, colPolygonPacked, bClosed, flThickness);

	if (uFlags & DRAW_POLYGON_OUTLINE)
		pDrawList->AddPolyline(vecPoints, nPointsCount, colOutline.GetU32(), bClosed, flThickness + 1.0f);
}

void render::AddDrawListText(ImDrawList* pDrawList, const ImFont* pFont, const ImVec2& vecPosition, const char* szText, const Color_t& colText, const unsigned int uFlags, const Color_t& colOutline, const float flThickness)
{
	if (pDrawList == nullptr)
		pDrawList = GetDrawList();

	// set font texture
	pDrawList->PushTextureID(pFont->ContainerAtlas->TexID);

	const ImU32 colOutlinePacked = colOutline.GetU32();

	if (uFlags & DRAW_TEXT_DROPSHADOW)
		pDrawList->AddText(pFont, pFont->FontSize, vecPosition + ImVec2(flThickness, flThickness), colOutlinePacked, szText);
	else if (uFlags & DRAW_TEXT_OUTLINE)
	{
		pDrawList->AddText(pFont, pFont->FontSize, vecPosition + ImVec2(flThickness, -flThickness), colOutlinePacked, szText);
		pDrawList->AddText(pFont, pFont->FontSize, vecPosition + ImVec2(-flThickness, flThickness), colOutlinePacked, szText);
	}

	pDrawList->AddText(pFont, pFont->FontSize, vecPosition, colText.GetU32(), szText);
	pDrawList->PopTextureID();
}

void render::AddDrawListShadowRect(ImDrawList* pDrawList, const ImVec2& vecMin, const ImVec2& vecMax, const Color_t& colShadow, float flThickness, float flRounding, ImDrawFlags roundingCorners)
{
	if (pDrawList == nullptr)
		pDrawList = GetDrawList();

	pDrawList->AddShadowRect(vecMin, vecMax, colShadow.GetU32(), flThickness, ImVec2(0, 0), roundingCorners, flRounding);
}

#pragma endregion

#pragma region draw_structures

void AnimationHandler_t::Update(const float flDeltaTime, const float flDuration)
{
	if (fnEaseIn == nullptr)
		fnEaseIn = &EASING::InSine;

	if (fnEaseOut == nullptr)
		fnEaseOut = &EASING::OutSine;

	// Reset the elapsed time if the bool switches
	if (bSwitch != bLastSwitch)
		flElapsedTime = 0;

	flElapsedTime = math::Max(0.0f, math::Min(flElapsedTime, flDuration));
	float flTime = flElapsedTime / flDuration;

	// Determine the initial and target value based on the current state
	float flInitialValue = bSwitch ? 0.1f : flValue;
	float flTargetValue = bSwitch ? 1.0f : 0.1f; /*(1.0f is max value)*/

	// Select the appropriate easing function based on the current state
	EasingFunction_t fnCurrentEase = bSwitch ? fnEaseIn : fnEaseOut;

	// Apply the appropriate easing function based on fade-in or fade-out (with lerping, which is basically what's the math were doing)
	flValue = (flInitialValue + (flTargetValue - flInitialValue)) * (float)fnCurrentEase(flTime);
	flValue = math::Clamp(flValue, 0.1f, 1.0f);

	flElapsedTime += flDeltaTime;
	bLastSwitch = bSwitch;
}

