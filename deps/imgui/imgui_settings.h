#include "imgui.h"

namespace font
{
	inline ImFont* inter_medium_widget = nullptr;
	inline ImFont* inter_medium_small = nullptr;
	inline ImFont* inter_medium_big = nullptr;
	inline ImFont* inter_bold = nullptr;
	inline ImFont* icomoon = nullptr;
	inline ImFont* icomoon_default = nullptr;
}


namespace c
{

	inline ImVec4 accent = ImColor(0, 255, 0, 255);
	inline ImVec4 black = ImColor(23, 23, 26, 255);

	// Status colors
	inline ImVec4 green = ImColor(0, 255, 0, 255);
	inline ImVec4 red = ImColor(255, 0, 0, 255);

	namespace bg
	{
		inline ImVec4 filling = ImColor(23, 23, 26, 255);
		inline ImVec4 stroke = ImColor(40, 40, 44, 255);
		inline ImVec4 topbar = ImColor(30, 30, 33, 255);
		inline ImVec4 container = ImColor(28, 28, 31, 255);
		inline ImVec2 size = ImVec2(962, 537);
		//inline ImVec2 size = ImVec2(962, 537);
		inline float rounding = 4.0f;
	}

	namespace child
	{
		inline ImVec4 filling = ImColor(28, 28, 31, 255);
		inline ImVec4 stroke = ImColor(40, 40, 44, 255);
		inline ImVec4 top = ImColor(30, 30, 33, 255);
		inline ImVec4 child_text = ImColor(255, 255, 255, 255);

		inline float rounding = 4.0f;
	}

	namespace element
	{
		namespace info_bar
		{
			inline ImVec4 filling = ImColor(30, 30, 33, 255);
			inline ImVec4 stroke = ImColor(40, 40, 44, 255);

			inline ImVec4 box = ImColor(45, 45, 50, 255);
			inline ImVec4 box_outline = ImColor(50, 50, 55, 255);
		}


		inline ImVec4 page_active = ImColor(50, 50, 55, 255);
		inline ImVec4 selectable = ImColor(40, 40, 45, 255);

		namespace popup_elements
		{
			inline ImVec4 filling = ImColor(30, 30, 33, 255);
			inline ImVec4 cog = ImColor(64, 69, 104, 255);
		}

		namespace notify
		{

			inline ImVec4 red_status = ImColor(255, 48, 47, 255);
			inline ImVec4 blue_status = ImColor(0, 160, 255, 255);
			inline ImVec4 green_status = ImColor(0, 255, 0, 255);
			inline ImVec4 yellow_status = ImColor(255, 255, 0, 255);
		}

		namespace combo
		{
			inline ImVec4 filling = ImColor(45, 45, 50, 255);
		}


		inline ImVec4 filling = ImColor(39, 36, 33, 255);
		inline ImVec4 circle_mark = ImColor(150, 150, 150, 230);
		inline ImVec4 text_active = ImColor(255, 255, 255, 255);
		inline ImVec4 text_hov = ImColor(150, 150, 150, 255);
		inline ImVec4 text = ImColor(120, 120, 120, 255);
		inline ImVec4 selector = ImColor(50, 50, 55, 255);

		inline float rounding = 4.0f;
	}
}