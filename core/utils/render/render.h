#pragma once
#include "../../sdk/sdk.h"
#include "../../sdk/datatypes/color.h"

#pragma region draw_objects_enumerations

enum ERectRenderFlags : unsigned int
{
	DRAW_RECT_NONE = 0,
	DRAW_RECT_OUTLINE = (1 << 0),
	DRAW_RECT_BORDER = (1 << 1),
	DRAW_RECT_FILLED = (1 << 2)
};

enum ECircleRenderFlags : unsigned int
{
	DRAW_CIRCLE_NONE = 0,
	DRAW_CIRCLE_OUTLINE = (1 << 0),
	DRAW_CIRCLE_FILLED = (1 << 1)
};

enum ETriangleRenderFlags : unsigned int
{
	DRAW_TRIANGLE_NONE = 0,
	DRAW_TRIANGLE_OUTLINE = (1 << 0),
	DRAW_TRIANGLE_FILLED = (1 << 1)
};

enum EQuadRenderFlags : unsigned int
{
	DRAW_QUAD_NONE = 0,
	DRAW_QUAD_OUTLINE = (1 << 0),
	DRAW_QUAD_FILLED = (1 << 1)
};

enum EPolygonRenderFlags : unsigned int
{
	DRAW_POLYGON_NONE = 0,
	DRAW_POLYGON_OUTLINE = (1 << 0),
	DRAW_POLYGON_FILLED = (1 << 1)
};

enum ETextRenderFlags : unsigned int
{
	DRAW_TEXT_NONE = 0,
	DRAW_TEXT_DROPSHADOW = (1 << 0),
	DRAW_TEXT_OUTLINE = (1 << 1)
};

#pragma endregion

#pragma region draw_structures
struct BBox_t {
	BBox_t() { Invalidate(); }

	void Invalidate() {
		static constexpr auto MAX_FLOAT = std::numeric_limits<float>::max();

		m_Mins.x = m_Mins.y = MAX_FLOAT;
		m_Maxs.x = m_Maxs.y = -MAX_FLOAT;

		std::fill(std::begin(m_Vertices), std::end(m_Vertices), ImVec2{ -MAX_FLOAT, -MAX_FLOAT });
	}

	ImVec2 m_Mins, m_Maxs;
	ImVec2 m_Vertices[8];
};

// predefined custom user type
struct ColorPickerVar_t;

typedef double (*EasingFunction_t)(double);
struct AnimationHandler_t
{
	// default: ease::in/outsine
	AnimationHandler_t(EasingFunction_t fnIn = nullptr, EasingFunction_t fnOut = nullptr) :
		fnEaseIn(fnIn), fnEaseOut(fnOut), bSwitch(false), bLastSwitch(false), flElapsedTime(0.f), flValue(0.1f) {};
	~AnimationHandler_t() = default;

	// Has to be called every frame
	void Update(const float flDeltaTime, const float flDuration);

	// Get the current value multiplied by a scale
	float GetValue(float flScale = 1.0f)
	{
		return flValue * flScale;
	}

	const bool GetSwitch() const
	{
		return bSwitch;
	}

	// switch state
	void Switch()
	{
		bSwitch = !bSwitch;
	}

	void SetSwitch(const bool bState)
	{
		bSwitch = bState;
	}

private:
	// Set to true for ease-in animation, false for ease-out
	bool bSwitch = 0;
	bool bLastSwitch = bSwitch;

	float flElapsedTime = 0.f;

	// Current value of the animation
	float flValue = 0.1f;

	// Ease in and out functions Declaration
	EasingFunction_t fnEaseIn = nullptr;
	EasingFunction_t fnEaseOut = nullptr;
};

#pragma endregion

#include "d3d11/tex_dict_dx11.hpp"

namespace animations {
	inline AnimationHandler_t menu_background_anim;
}

namespace render
{

	inline bool m_initialized = false;
	inline ImFont* m_esp_name = nullptr;
	inline ImFont* m_esp_weapons = nullptr;
	inline ImFont* m_esp_flags = nullptr;
	inline ImFont* m_default_font = nullptr;
	inline ImFont* weapon_icons = nullptr;
	inline ImFont* m_indicator_font = nullptr;
	inline ID3D11ShaderResourceView* m_cs2_player_model_texture = nullptr;
	inline ID3D11ShaderResourceView* m_plus = nullptr;
	inline ID3D11ShaderResourceView* m_shunk = nullptr;

	inline int model_w;


	inline int model_h;
	bool Init( HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* device_context );
	void Destroy( );

	void CreateRenderTarget( );
	void DestroyRenderTarget( );

	void NewFrame( );
	void Render( );

	bool OnWndProc( HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam );

	ImDrawList* GetDrawList( );
	bool WorldToScreen(const Vector_t& in, ImVec2& out);
	Vector_t GetScreenSize();
	float CalculateDPI(const int nScaleTarget = 0);
	/* @section: bindings */
	void AddDrawListRect(ImDrawList* pDrawList, const ImVec2& vecMin, const ImVec2& vecMax, const Color_t& colRect, const unsigned int uFlags = DRAW_RECT_NONE, const Color_t& colOutline = Color_t(0, 0, 0, 255), const float flRounding = 0.f, const ImDrawFlags roundingCorners = ImDrawFlags_RoundCornersAll, float flThickness = 1.0f, const float flOutlineThickness = 1.0f);
	void AddDrawListRectMultiColor(ImDrawList* pDrawList, const ImVec2& vecMin, const ImVec2& vecMax, const Color_t& colUpperLeft, const Color_t& colUpperRight, const Color_t& colBottomRight, const Color_t& colBottomLeft);
	void AddDrawListCircle(ImDrawList* pDrawList, const ImVec2& vecCenter, const float flRadius, const Color_t& colCircle, const int nSegments, const unsigned int uFlags = DRAW_CIRCLE_NONE, const Color_t& colOutline = Color_t(0, 0, 0, 255), const float flThickness = 1.0f, const float flOutlineThickness = 1.0f);
	void AddDrawListArc(ImDrawList* pDrawList, const ImVec2& vecPosition, const float flRadius, const float flMinimumAngle, const float flMaximumAngle, const Color_t& colArc = Color_t(255, 255, 255, 255), const float flThickness = 1.0f);
	void AddDrawListLine(ImDrawList* pDrawList, const ImVec2& vecFirst, const ImVec2& vecSecond, const Color_t& colLine, const float flThickness = 1.0f);
	void AddDrawListTriangle(ImDrawList* pDrawList, const ImVec2& vecFirst, const ImVec2& vecSecond, const ImVec2& vecThird, const Color_t& colTriangle, const unsigned int uFlags = DRAW_TRIANGLE_NONE, const Color_t& colOutline = Color_t(0, 0, 0, 255), const float flThickness = 0.f);
	void AddDrawListQuad(ImDrawList* pDrawList, const ImVec2& vecFirst, const ImVec2& vecSecond, const ImVec2& vecThird, const ImVec2& vecFourth, const Color_t& colQuad, const unsigned int uFlags = DRAW_QUAD_NONE, const Color_t& colOutline = Color_t(0, 0, 0, 255), const float flThickness = 0.f);
	void AddDrawListPolygon(ImDrawList* pDrawList, const ImVec2* vecPoints, const int nPointsCount, const Color_t& colPolygon, unsigned int uFlags = DRAW_POLYGON_NONE, const Color_t& colOutline = Color_t(0, 0, 0, 255), const bool bClosed = true, const float flThickness = 1.0f);
	void AddDrawListText(ImDrawList* pDrawList, const ImFont* pFont, const ImVec2& vecPosition, const char* szText, const Color_t& colText, const unsigned int uFlags = DRAW_TEXT_NONE, const Color_t& colOutline = Color_t(0, 0, 0, 255), const float flThickness = 1.0f);
	void AddDrawListShadowRect(ImDrawList* pDrawList, const ImVec2& vecMin, const ImVec2& vecMax, const Color_t& colShadow, float flThickness = 32.f, float flRounding = 0.0f, ImDrawFlags roundingCorners = ImDrawFlags_RoundCornersAll);
	void update_style(ImGuiStyle* pStyle);
}
