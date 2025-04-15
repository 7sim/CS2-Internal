#pragma once

#pragma once

#include <memory>
#include <array>

#include "..\..\utils\render\render.h"

#include "..\..\utils\utils.h"
#include "..\..\sdk\sdk.h"
#include "..\..\math\elements\qangle.h"
#include "..\..\math\elements\vector.h"
#include "..\..\math\elements\transform.h"
#include "..\..\sdk\handle\handle.hpp"

#include "..\..\utils\configs\config.h"
#include <deque>
#include <vector>
#include "..\..\sdk\interfaces\network.h"

namespace animations
{
	class c_animations {
	public:
		void set_playback_rate(CBaseAnimGraph* base_anim_graph);
		void on_sequence_update(CBaseAnimGraph* base_anim_graph);
		float get_finished_cycle_rate(CBaseAnimGraph* a1);
		void on_frame_stage();
	};

	const auto data = std::make_unique<c_animations>();
}