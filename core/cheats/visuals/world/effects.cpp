#include "effects.h"
#include "..\..\..\sdk\interfaces\particles.h"


void effects::c_hitmarker::add(Vector_t position) {
	mutex.lock();
	m_data.push_back(data(position, sdk::m_global_vars->m_curtime + 4.f));
	mutex.unlock();
}

void effects::c_hitmarker::update() {
	mutex.lock();

	for (int i = 0; i < m_data.size(); i++) {
		m_data[i].is_world_screen = math::WorldToScreen(m_data[i].position, m_data[i].world_position);
	}

	mutex.unlock();
}
void effects::c_hitmarker::run() {

	mutex.lock();

	for (int i = 0; i < m_data.size(); i++) {

		if ( sdk::m_global_vars->m_curtime >= m_data[i].time)
			m_data[i].time = 0.f;

		if (!m_data[i].is_world_screen)
			continue;

		constexpr float gap = 2.f;
		constexpr float size = 4.f;

		auto color = &cfg_get(ColorPickerVar_t, g_cfg.hitmarker_col).colValue;

		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(m_data[i].world_position.x - gap, m_data[i].world_position.y - gap), ImVec2(m_data[i].world_position.x - gap - size, m_data[i].world_position.y - gap - size), ImGui::VGetColorU32(color->GetVec4()), 1.f);
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(m_data[i].world_position.x - gap, m_data[i].world_position.y + gap), ImVec2(m_data[i].world_position.x - gap - size, m_data[i].world_position.y + gap + size), ImGui::VGetColorU32(color->GetVec4()), 1.f);
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(m_data[i].world_position.x + gap, m_data[i].world_position.y - gap), ImVec2(m_data[i].world_position.x + gap + size, m_data[i].world_position.y - gap - size), ImGui::VGetColorU32(color->GetVec4()), 1.f);
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(m_data[i].world_position.x + gap, m_data[i].world_position.y + gap), ImVec2(m_data[i].world_position.x + gap + size, m_data[i].world_position.y + gap + size), ImGui::VGetColorU32(color->GetVec4()), 1.f);
	}

	mutex.unlock();
}

void effects::c_beams::add(Vector_t start, Vector_t end, Color_t clr_2, float width)
{
	auto& bullet = bullets.emplace_back();

	auto particles_system = sdk::m_particle_game;
	if (!particles_system)
		return; 
	
	// just init the struct which will be overriden
	particle_effect particle_effect_{};
	particle_effect_.name = "particles/entity/spectator_utility_trail.vpcf";
	particle_effect_.pad[0] = 8;
	
	// cache particle options
	sdk::m_particle_game->CacheParticleEffect(&bullet.effect_index, particle_effect_.name);
	auto color = clr_2;
	// set particle clr
	particle_color clr = { float(color.r), float(color.g), float(color.b)};

	// create particle
	sdk::m_particle_game->CreateParticleEffect(bullet.effect_index, 16, &clr, 0);

	// init data struct
	bullet.particle_data_ = {};

	// get bullet angle direction
	auto dir = (end - start);
	auto stage_2 = start + (dir * 0.3f);
	auto stage_3 = start + (dir * 0.5f);
	Vector_t positions_[] = { start, stage_2, stage_3, end };

	for (int i{}; i < sizeof(positions_) / sizeof(Vector_t); i++) {

		particle_information particle_info{};
		particle_info.time = 4.f;
		particle_info.width = width;
		particle_info.unk2 = 1.f;
		sdk::m_particle_game->CreateParticleEffect(bullet.effect_index, 16, &clr, 0);

		bullet.positions = new Vector_t[i + 1];
		bullet.times = new float[i + 1];

		for (int j{}; j < i + 1; j++) {
			bullet.positions[j] = positions_[j];
			bullet.times[j] = g::interval * float(j);
		}

		bullet.particle_data_.m_vecPosition= bullet.positions;
		bullet.particle_data_.m_flTimes2 = bullet.times;

		sdk::m_particle_system->CreateSnapshot(&bullet.handle_snapshot_particle);


		sdk::m_particle_game->UnknownFunction(bullet.effect_index, 0, &bullet.handle_snapshot_particle);
		bullet.handle_snapshot_particle->Draw(static_cast<int>(i + 1U), &bullet.particle_data_);


		delete[] bullet.positions;
		delete[] bullet.times;
	}
}
#include <cmath> 

void effects::c_beams::clear_circle() {
	if (!beams.empty())
		beams.clear();
}


void effects::c_beams::add_glow_circle(Vector_t center, float radius, Color_t clr2, float width, int num_segments) {
	// Clear previous effects if necessary

	auto& circle_info = beams.emplace_back();

	auto particles_system = sdk::m_particle_game;
	if (!particles_system)
		return;

	// just init the struct which will be overriden
	particle_effect particle_effect_{};
	particle_effect_.name = "particles/entity/spectator_utility_trail.vpcf";
	particle_effect_.pad[0] = 8;

	// get particle idx from name
	sdk::m_particle_game->CacheParticleEffect(&circle_info.effect_index, particle_effect_.name);
	auto color = clr2;
	// set particle clr
	particle_color clr = { float(color.r), float(color.g), float(color.b) };

	// update particle from idx idk what 16 stands for tho
	sdk::m_particle_game->CreateParticleEffect(circle_info.effect_index, 16, &clr, 0);

	// init data struct
	circle_info.particle_data_ = {};
	
	// Calculate positions for the circle
	circle_info.positions = new Vector_t[num_segments];
	circle_info.times = new float[num_segments];
	Vector_t* positions = new Vector_t[num_segments];

	for (int i = 0; i < num_segments; ++i) {
		float angle = (2.0f * math::_PI * i) / num_segments; // Angle for each particle
		positions[i].x = center.x + radius * cos(angle); // X coordinate
		positions[i].y = center.y + radius * sin(angle); // Y coordinate
		positions[i].z = center.z; // Keep Z constant for a flat circle

		circle_info.positions[i] = positions[i];
		circle_info.times[i] = 0.5f; // Set a very short lifespan for each particle
	}

	for (int i{}; i < sizeof(num_segments) / sizeof(Vector_t); i++) {

		particle_information particle_info{};
		particle_info.time = 0.5f;
		particle_info.width = width;
		particle_info.unk2 = 1.f;
		/* get each segment/particle and apply data for them */
		sdk::m_particle_game->CreateParticleEffect(circle_info.effect_index, 16, &clr, 0);

		circle_info.positions = new Vector_t[i + 1];
		circle_info.times = new float[i + 1];

		for (int j{}; j < i + 1; j++) {
			circle_info.positions[j] = positions[j];
			circle_info.times[j] = 0.5f;
		}

		circle_info.particle_data_.m_vecPosition = circle_info.positions;
		circle_info.particle_data_.m_flTimes2 = circle_info.times;

		sdk::m_particle_system->CreateSnapshot(&circle_info.handle_snapshot_particle);


		sdk::m_particle_game->UnknownFunction(circle_info.effect_index, 0, &circle_info.handle_snapshot_particle);
		circle_info.handle_snapshot_particle->Draw(static_cast<int>(i + 1U), &circle_info.particle_data_);


		delete[] circle_info.positions;
		delete[] circle_info.times;
	}
}
