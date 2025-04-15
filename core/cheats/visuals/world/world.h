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
#include <bitset>
struct InfernoData {
	std::vector<Vector_t> points; // Stores only burning fire positions

	InfernoData() = default;

	InfernoData(int fire_count, const bool* is_burning, const Vector_t* fire_positions) noexcept {
		points.reserve(fire_count); // Reserve space for active fires only

		for (int i = 0; i < fire_count; ++i) {
			if (is_burning[i]) {
				points.emplace_back(fire_positions[i]); // Store only burning fires
			}
		}
	}
};

	
namespace world
{
	struct FirePolygon {
		std::vector<ImVec2> screenPoints;
		ImColor color;
	};
	class c_nightmode {
		mutable bool m_set_def_world_color{ }, m_need_update{ }, m_force_update{ };
		float m_nightmode_scale{ }, m_old_nightmode_scale{ };
	public:

		void update();
		void on_game();
		void on_end_game();

	};
	const auto nightmode = std::make_unique<c_nightmode>();

	class c_esp {
	public:
		void run();
		void motion_blur();
		void sky_box();
		void molotov();
		void grenade_projectile();

	};
	const auto esp = std::make_unique<c_esp>();

	class c_items {
	public:
		void run();
	};
	const auto items = std::make_unique<c_items>();

	class c_grenade {
	public:
		void run();
	};
	const auto grenade = std::make_unique<c_grenade>();


	class c_c4 {
	public:
		void cache();

		void clear();
		int c4_site(bool planted);
		Vector_t c4_position();

		void run();
	};
	const auto c4 = std::make_unique<c_c4>();
}
