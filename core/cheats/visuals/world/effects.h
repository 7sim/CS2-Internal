#pragma once

#include <memory>
#include "..\..\..\utils\render\render.h"
#include "..\..\..\utils\utils.h"
#include "..\..\..\sdk\sdk.h"
#include "..\..\..\math\elements\qangle.h"
#include "..\..\..\math\elements\vector.h"
#include "..\..\..\math\elements\transform.h"
#include "..\..\..\utils\configs\config.h"
#include "..\proccess.h"
#include "..\..\..\sdk\interfaces\particles.h"

class tracer_info
{
public:
	unsigned int effect_index = -1;
	Vector_t* positions = nullptr;
	float* times = nullptr;
	CStrongHandle<CParticleSnapshot> handle_snapshot_particle{};
	CParticleData particle_data_;
};
namespace effects
{
	class c_beams {
	public:
		unsigned int index_effect = -1;
		float last_create_time = 0.f;
		CStrongHandle<CParticleSnapshot> handle_snapshot_particle{};
		CParticleData particle_data_{};

		std::vector<Vector_t> positions{};
		std::vector<float> times{};

		std::vector<tracer_info> bullets{};
		std::vector<tracer_info> beams{};

		void add_glow_line(Vector_t start, Vector_t end, Color_t clr, float width);

		void clear_circle();
		void clear_line();

		void add_glow_circle(Vector_t center, float radius, Color_t clr, float width, int num_particles);
		void add(Vector_t start, Vector_t end, Color_t clr_, float width);
	};

	const auto beam = std::make_unique<c_beams>();

	class c_hitmarker {
	public:
		struct data
		{
			data(Vector_t position, float time) : position(position), is_world_screen{}, world_position{}, time(time) {}
			Vector_t position;
			bool is_world_screen;
			ImVec2 world_position;
			float time;
		};

		std::vector<data> m_data;
		std::mutex mutex;

		void run();
		void update();
		void add(Vector_t position);
	};

	const auto hitmarker = std::make_unique<c_hitmarker>();
}
