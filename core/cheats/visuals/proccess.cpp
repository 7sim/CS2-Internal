#include "proccess.h"
#include "..\..\sdk\cache\cache_entity.h"
#include "..\backtrack\record.h"
#include "player/player.h"
#include "world/world.h"
#include "grenades/grenade.h"
#include "../../utils/hook/hooks.h"
#include "../legitbot/legitbot.h"
#include "../ragebot/ragebot.h"
#include "..\..\cheats\visuals\world\hitsound.h"
std::mutex m_proccessing_mutex;

std::vector< c_cached_weapons > m_weapons;
std::vector< c_cached_molotov > m_molotovs;
std::vector<object_t> m_entity_object;
std::vector<item_object_t> m_items_object;
std::vector<projectile_object_t> m_projectile_object;
std::vector<c4_object_t> m_c4_object;
std::vector<player_object_t> m_player_object; 

cachec_local m_local_player_data;

int tick_prev = 0;

void velocity_indicator(CCSPlayerPawn * player) {
	if (!sdk::m_global_vars || !sdk::m_engine->is_valid() || !cfg_get(bool, g_cfg.velocity_indicator))
		return;

	static int last_speed = 0, last_jump_speed = 0;
	static float last_vel_update = 0, jump_speed_duration = 0;
	int current_speed = 0;
	static bool last_onground = false;
	bool current_onground = false;

	if (player  && m_local_player_data.alive) {
		int screenWidth = ImGui::GetIO().DisplaySize.x;
		int screenHeight = ImGui::GetIO().DisplaySize.y;

		 last_speed = 0, last_jump_speed = 0;
		 last_vel_update = 0, jump_speed_duration = 0;
		 current_speed = m_local_player_data.velocity.Length2D() + 0.5;
		 last_onground = false;
		 current_onground = m_local_player_data.flags & FL_ONGROUND;

		if (last_onground && !current_onground)
			last_jump_speed = current_speed;

		Color_t c = current_speed >= 150 ? Color_t(30, 255, 109, 255) : current_speed < 50 ? Color_t(255, 119, 119, 255) :
			Color_t(255, 199, 89, 255)  ;

		std::string str = std::to_string(current_speed);
		if (!current_onground) {
			jump_speed_duration = sdk::m_global_vars->m_curtime + 1.5f;
		}

		

		ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenWidth / 2 - 15, screenHeight / 2 + 250), c.GetU32(),str.c_str());
		if (tick_prev + 5 < sdk::m_global_vars->m_tickcount) {
			last_speed = current_speed;
			tick_prev = sdk::m_global_vars->m_tickcount;
		}
		if (sdk::m_global_vars->m_tickcount <= 320)
		{
			tick_prev = 0;
		}
		last_onground = current_onground;
	}

	if (!m_local_player_data.alive) {
		tick_prev = 0;
		last_speed = 0;
		last_jump_speed = 0;
		last_vel_update = 0;
		jump_speed_duration = 0;
		current_speed = 0;
		last_onground = false;
		current_onground = false;
	}
}

struct Hitmarker {
	int expireTime; 
	int startTime;
	int dmg;
};
std::vector<Hitmarker> hitmarkers;  // Vector to store active hitmarkers

void visuals::c_visuals::add_hitmarker(int dmg) {
	// Create a new hitmarker
	Hitmarker hitmarker;
	hitmarker.expireTime = sdk::m_global_vars->m_tickcount + 20;  // 0.5 seconds lifespan
	hitmarker.dmg = dmg;  // Get hitmarker size from config
	hitmarker.startTime = sdk::m_global_vars->m_tickcount;
	// Add the hitmarker to the vector
	hitmarkers.push_back(hitmarker);
 }

void render_hitmark() {
	if (hitmarkers.empty())
		return;

	static int width = ImGui::GetIO().DisplaySize.x;
	static int height = ImGui::GetIO().DisplaySize.y;
	int lineSize = cfg_get(int, g_cfg.hitmarker_size);
	Color_t color = cfg_get(ColorPickerVar_t, g_cfg.hitmarker_col).colValue;

	if (cfg_get(bool, g_cfg.hitmarker)) {
		// Iterate through all active hitmarkers
		for (size_t i = 0; i < hitmarkers.size(); ++i) {
			Hitmarker& hitmarker = hitmarkers[i];

			if (sdk::m_global_vars->m_tickcount <= hitmarker.expireTime) {
				int complete = (sdk::m_global_vars->m_tickcount - hitmarker.startTime) / hitmarker.expireTime;
				int x = width / 2,
					y = height / 2,
					alpha = (20 - complete) * 240;

				constexpr int line{ 6 };

				ImGui::GetBackgroundDrawList()->AddLine(
					ImVec2(x - line, y - line),
					ImVec2(x - (line / 4), y - (line / 4)),
					color.GetU32()
				);
				ImGui::GetBackgroundDrawList()->AddLine(
					ImVec2(x - line, y + line),
					ImVec2(x - (line / 4), y + (line / 4)),
					color.GetU32()
				);
				ImGui::GetBackgroundDrawList()->AddLine(
					ImVec2(x + line, y + line),
					ImVec2(x + (line / 4), y + (line / 4)),
					color.GetU32()
				);
				ImGui::GetBackgroundDrawList()->AddLine(
					ImVec2(x + line, y - line),
					ImVec2(x + (line / 4), y - (line / 4)),
					color.GetU32()
				);
			}
			else {
				hitmarkers.erase(hitmarkers.begin() + i);
				i--;
			}

		}
	}

}

const char* GunIcon(const std::string& weapon)
{
	std::map<std::string, const char*> gunIcons = {
		{"nova", "T"},
		{"ak47", "A"},
		{"awp", "C"},
		{"m4a4", "M"},
		{"deagle", "F"},
		{"mp9", "R"},
		{"ump45", "b"},
		{"glock", "g"},
	};

	gunIcons[("knife_ct")] = "]";
	gunIcons[("knife_t")] = "[";
	gunIcons[("knife")] = "[";
	gunIcons[("deagle")] = "A";
	gunIcons[("elite")] = "B";
	gunIcons[("fiveseven")] = "C";
	gunIcons[("five-seven")] = "C";
	gunIcons[("glock")] = "D";
	gunIcons[("hkp2000")] = "E";
	gunIcons[("p2000")] = "E";
	gunIcons[("p250")] = "F";
	gunIcons[("usp_silencer")] = "G";
	gunIcons[("tec9")] = "H";
	gunIcons[("cz75a")] = "I";
	gunIcons[("revolver")] = "J";
	gunIcons[("mac10")] = "K";
	gunIcons[("mac-10")] = "K";
	gunIcons[("ump45")] = "L";
	gunIcons[("bizon")] = "M";
	gunIcons[("mp7")] = "N";
	gunIcons[("mp9")] = "O";
	gunIcons[("mp5sd")] = "x";
	gunIcons[("p90")] = "P";
	gunIcons[("galilar")] = "Q";
	gunIcons[("galil")] = "Q";
	gunIcons[("famas")] = "R";
	gunIcons[("m4a4")] = "S";
	gunIcons[("m4a1_silencer")] = "T";
	gunIcons[("m4a1")] = "T";
	gunIcons[("aug")] = "U";
	gunIcons[("sg556")] = "V";
	gunIcons[("ak47")] = "W";
	gunIcons[("g3sg1")] = "X";
	gunIcons[("scar20")] = "Y";
	gunIcons[("awp")] = "Z";
	gunIcons[("ssg08")] = "a";
	gunIcons[("ssg-08")] = "a";
	gunIcons[("xm1014")] = "b";
	gunIcons[("sawedoff")] = "c";
	gunIcons[("mag7")] = "d";
	gunIcons[("nova")] = "e";
	gunIcons[("negev")] = "f";
	gunIcons[("m249")] = "g";
	gunIcons[("taser")] = "h";
	gunIcons[("flashbang")] = "i";
	gunIcons[("hegrenade")] = "j";
	gunIcons[("smokegrenade")] = "k";
	gunIcons[("molotov")] = "l";
	gunIcons[("decoy")] = "m";
	gunIcons[("incgrenade")] = "n";
	gunIcons[("c4")] = "o";
	gunIcons[("defuse kit")] = "r";

	auto it = gunIcons.find(weapon);
	if (it != gunIcons.end()) {
		return it->second;
	}

	return "";
}
void render_fov_cs2_circle(int fovDegrees)
{
	if (!cfg_get(bool, g_cfg.renderize_rage_fov))
		return;


	float legit_fov = cfg_get(int, g_cfg.rage_fov);
	legit_fov -= 20;
	if (cfg_get(bool, g_cfg.renderize_rage_fov) && legit_fov > 0.f) {
		//draw circle
		const auto [width, height] = ImGui::GetIO().DisplaySize;
		float diagonal = sqrt(pow(width, 2) + pow(height, 2));
		float r = (legit_fov * (diagonal / 145.f)) * 0.85f;
		auto clr = cfg_get(ColorPickerVar_t, g_cfg.renderize_fov_rage_clr);
		ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(width / 2, height / 2), r, clr.colValue.GetU32(), 64);
	}
}

float BezierBlend4(float t)
{
	return t * t * (3.0f - 2.0f * t);
}

void draw_scope()
{
	if (!cfg_get(bool, g_cfg.scope_line) || !sdk::m_global_vars)
		return;

	static auto progress = 0.f;
	
	if (g::scoped)
		progress = std::clamp(progress + sdk::m_global_vars->m_frame_time * 4.f, 0.f, 1.f);
	else
		progress = std::clamp(progress - sdk::m_global_vars->m_frame_time * 4.f, 0.f, 1.f);

	auto drawList = ImGui::GetBackgroundDrawList(); // Access the current window's draw list

	if (progress > 0.f)
	{
		const auto size =ImGui::GetIO().DisplaySize;
		const auto screen_center = ImVec2(ImGui::GetIO().DisplaySize.x / 2.f, ImGui::GetIO().DisplaySize.y / 2.f);

		int offset = size.y / 120;
		int length = (size.y / 2) * cfg_get(float, g_cfg.scope_line_size) * 0.01f - offset;

		length = std::min(length, int(size.y / 2) - offset - 1);

		const int cross_x = screen_center.x;
		const int cross_y = screen_center.y;

		auto selected = cfg_get(ColorPickerVar_t, g_cfg.scope_line_color);
		auto col = ImGui::VGetColorU32(selected.colValue.GetVec4(), progress);  

		auto col2 = ImGui::VGetColorU32(selected.colValue.GetVec4(), 0.f);

		drawList->AddLine(ImVec2(cross_x + offset + length * BezierBlend4(1.f - progress), cross_y),
			ImVec2(cross_x + offset + length, cross_y), col);
		drawList->AddLine(ImVec2(cross_x - offset - length * BezierBlend4(1.f - progress), cross_y),
			ImVec2(cross_x - offset - length, cross_y), col);

		drawList->AddLine(ImVec2(cross_x, cross_y + offset + length * BezierBlend4(1.f - progress)),
			ImVec2(cross_x, cross_y + offset + length), col);
		drawList->AddLine(ImVec2(cross_x, cross_y - offset - length * BezierBlend4(1.f - progress)),
			ImVec2(cross_x, cross_y - offset - length), col);
	}
}

void draw_land_circle(const Vector_t& pos, float radius)
{
	// Check if the feature is enabled
	if (!cfg_get(bool, g_cfg.grenade_prediction_radius))
		return;

	ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

	// Outline the circle
	ImU32 outline_color = IM_COL32(255, 255, 255, 255); // White outline
	Vector_t circle2_point = pos;
	circle2_point.x += pos.x;
	circle2_point.y += pos.y;

	ImVec2 screen2_pos;


	const int num_segments = 36;  // Number of segments to approximate the circle
	std::vector<ImVec2> screen_points;

	// Calculate circle points in 3D space and project to 2D
	for (int i = 0; i < num_segments; i++)
	{
		float theta = 2.0f * math::_PI * float(i) / float(num_segments);  // Angle for this segment
		float x = radius * cosf(theta);  // X coordinate on the circle
		float y = radius * sinf(theta);  // Y coordinate on the circle

		// Offset the circle to the grenade landing position in 3D space
		Vector_t circle_point = pos;
		circle_point.x += x;
		circle_point.y += y;

		ImVec2 screen_pos;
		if (math::WorldToScreen(circle_point, screen_pos))
		{
			screen_points.push_back(screen_pos);
		}
	}

	// Check if there are enough points to draw a circle
	if (screen_points.size() < 3) // At least 3 points are needed to draw a circle
		return;

	float thickness = 2.0f;

	// Draw lines connecting the points to form a circle
	for (size_t i = 0; i < screen_points.size(); i++)
	{
		ImVec2 start = screen_points[i];
		ImVec2 end = screen_points[(i + 1) % screen_points.size()];  // Wrap to the first point
		draw_list->AddLine(start, end, outline_color, thickness);

	}
}
void run_grenade_pred(bool alive, bool has_grenade)
{
	if (!cfg_get(bool, g_cfg.grenade_prediction))
		return;

	if (g::lines.empty())
		return;

	auto local = hooks::m_get_local_player(0);
	if (!local || !alive)
		return;

	if (!has_grenade)
		return;

	if (!g::attack && !g::attack2)
		return;

	if (g::lines.size() <= 0 || g::lines_3d.empty())
		return;

	std::deque<ImVec2> grenade_prediction_lines;
	std::deque<Vector_t> w2s_grenade_prediction_lines;

	grenade_prediction_lines.insert(grenade_prediction_lines.end(), g::lines.begin(), g::lines.end());
	w2s_grenade_prediction_lines.insert(w2s_grenade_prediction_lines.end(), g::lines_3d.begin(), g::lines_3d.end());

	if (grenade_prediction_lines.size() <= 0 || grenade_prediction_lines.empty() || w2s_grenade_prediction_lines.size() <= 0 || w2s_grenade_prediction_lines.empty())
		return;

	auto prev = grenade_prediction_lines.front();

	for (const auto& it : grenade_prediction_lines)
	{
		ImGui::GetBackgroundDrawList()->AddLine(prev, it, cfg_get(ColorPickerVar_t, g_cfg.grenade_prediction_col).colValue.GetU32(), 2.0f);

		prev = it;
	}

	if (!w2s_grenade_prediction_lines.empty() && w2s_grenade_prediction_lines.size() > 0) {
		Vector_t landing_position = w2s_grenade_prediction_lines.back();
		// Create a glowing circle effect at the bullet impact position
		float radius = 50.0f; // Set the desired radius for the circle
		int num_particles = 30; // Set the number of particles to create a smooth circle
		auto width = 1.5f;

		//	effects::beam->clear_circle();

		//	effects::beam->add_glow_circle(landing_position, radius, cfg_get(ColorPickerVar_t, g_cfg.grenade_prediction_radius_col).colValue, width, num_particles);
		draw_land_circle(landing_position, 25);
	}

	if (!grenade_prediction_lines.empty())
		grenade_prediction_lines.clear();
}
void visuals::c_visuals::proccess()
{

	if (!sdk::m_engine->is_valid() || !this->processing_directx())
		return;

	auto local_controller = CCSPlayerController::Get();
	if (!local_controller)
		return;

	auto local = hooks::m_get_local_player(0);
	if (!local || !sdk::m_global_vars)
		return;

	render_fov_cs2_circle(cfg_get(int, g_cfg.rage_fov));
	draw_scope();


	run_grenade_pred(m_local_player_data.alive, m_local_player_data.has_grenade);
	hitsound::hitmarker->run();

	world::c4->run();	
	legitbot::data->render_fov_circle();
	legitbot::data->render_multi_points();

	ragebot::data->render_multi_points();
	world::items->run();
	world::esp->molotov();
	//grenades::prediction->run(m_local_player_data.alive, m_local_player_data.has_grenade);
	world::esp->grenade_projectile();

	velocity_indicator(local);
	
	player::esp->run();

}


void weapon_cache_system::weapon_cache::startup()  {
	if (!sdk::m_engine->connected())
		return;

	int highest_index = sdk::m_game->pGameEntitySystem->GetHighestEntityIndex();
	for (int i = 1; i <= highest_index; i++) {
		auto entity = sdk::m_game->pGameEntitySystem->Get(i);
		if (!entity)
			continue;

		if (!entity->IsWeapon())
			continue;

		auto weapon = (CCSWeaponBase*)entity;
		if (!weapon) continue;

		auto handle = entity->handle();
		// ( this is just a ghetto fix of caching sys in case people inject mid game )
		weapon_cache_system::g_weapons_cache->write_entity([&](auto& items) {
			items.emplace_back(weapon_cache_system::cached_entity{ handle, entity, nullptr, 0 });
		});
	}
}
void visuals::c_visuals::cache_weapons() {
	auto local_pawn = hooks::m_get_local_player(0);
	if (!local_pawn) {
		update_items({});
		return;
	}

	std::vector<item_object_t> new_items;
	weapon_cache_system::g_weapons_cache->read_entity([&](const std::vector<weapon_cache_system::cached_entity>& cached_items) {
		new_items.reserve(cached_items.size());

		for (const auto& entity : cached_items) {

			if (entity.entity && !entity.handle.valid()) {
				const_cast<weapon_cache_system::cached_entity&>(entity).update_entity(entity.entity);
			}

			if (!entity.valid())
				continue;

			if (!entity.entity)
				continue;

			auto weapon = (CCSWeaponBase*)entity.entity;
			if (!weapon)
				continue;

			/* store data here */
			auto data = weapon->GetWeaponData();
			if (!data)
				continue;

			auto attribute_container = &weapon->m_AttributeManager();
			if (!attribute_container)
				continue;

			auto item_view = &attribute_container->m_Item();
			if (!item_view)
				continue;

			auto item_definition = item_view->GetStaticData();
			if (!item_definition)
				continue;

			auto name = item_definition->GetSimpleWeaponName();

			int state = (int)weapon->m_iState();

			_log(LOG_INFO) << " weapon: " << name << " state:" << state;

			ImVec4 box;

			int ammo = weapon->clip1();
			int max_clip = data->m_iMaxClip1();
			int weapon_index = item_definition->m_nDefIndex;
			object_type_t m_type = object_weapons;
			Vector_t mins, maxs;
			CTransform node;

			mins = weapon->m_iState() == weapon_state::WEAPON_NOT_CARRIED ? entity.entity->m_pCollision()->m_vecMins() : Vector_t();
			maxs = weapon->m_iState() == weapon_state::WEAPON_NOT_CARRIED ? entity.entity->m_pCollision()->m_vecMaxs() : Vector_t();
			node = weapon->m_iState() == weapon_state::WEAPON_NOT_CARRIED ? entity.entity->m_pGameSceneNode()->m_nodeToWorld() : CTransform();

			item_object_t item_obj(entity.entity,
				name,
				ammo,
				max_clip, 
				entity.handle,
				box,
				weapon_index,
				node,
				mins,
				maxs,
				state
			);

	
			new_items.push_back(std::move(item_obj));
		}
		});

	update_items(std::move(new_items));
}

void player_cache_system::entity_cache::startup()  {
	if (!sdk::m_engine->connected())
		return;

	auto local_player = hooks::m_get_local_player(0);
	if (!local_player)
		return;

	int max_ents = sdk::m_game->pGameEntitySystem->GetHighestEntityIndex();

	for (int i = 1; i <= max_ents; i++)
	{
		auto entity = sdk::m_game->pGameEntitySystem->Get(i);
		if (!entity)
			continue;

		if (!entity->handle().valid())
			continue;

		SchemaClassInfoData_t* _class = nullptr;
		entity->dump_class_info(&_class);
		if (!_class)
			continue;

		const uint32_t hash = hash_32_fnv1a_const(_class->szName);
		
		/* perhaps i coudl use this logic instead for all entities and sort them based on class name ??? prob better*/
		if (hash == hash_32_fnv1a_const(xorstr_("CCSPlayerController")))
		{
			_log(LOG_INFO) << "found ent";

			// cache local player only here 
			CCSPlayerController* p_controller = reinterpret_cast<CCSPlayerController*>(entity);

			if (!p_controller)
				continue;

			auto handle = p_controller->handle().index();

			// valid handle
			if (!p_controller->PawnHandle().valid())
				continue;

			auto player = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(p_controller->PawnHandle().index());
			if (!player)
				continue;
			_log(LOG_INFO) << "added ent:" << entity->handle().index();
			// ( this is just a ghetto fix of caching sys in case people inject mid game )
			player_cache_system::g_entity_cache->write_players([&](auto& players) {
				players.emplace_back(player_cache_system::cached_entity{ entity->handle(), p_controller, player });
				});
		}
	}
}
void visuals::c_visuals::cache_players() {
	auto local_pawn = hooks::m_get_local_player(0);
	if (!local_pawn) {
		update_players({});
		return;
	}

	std::vector<player_object_t> new_players;
	player_cache_system::g_entity_cache->read_players([&](const std::vector<player_cache_system::cached_entity>& cached_players) {
		new_players.reserve(cached_players.size());

		for (const auto& entity : cached_players) {

			if (entity.controller && !entity.pawn) {
				const_cast<player_cache_system::cached_entity&>(entity).update_pawn();
			}

			if (!entity.valid())
				continue;

			if (!entity.pawn)
				continue;

			if (entity.pawn == local_pawn || entity.pawn->m_iHealth() <= 0)
				continue;

			if (entity.pawn == local_pawn)
				continue;

			if (entity.pawn->m_iHealth() <= 0)
				continue;

			auto player = entity.pawn;
			if (!player)
				continue;

			/* initialize all vars first */
			ImVec4 box;
			const char* name = "bot";
			const char* weapon_name = "bot";
			player_object_type_t m_player_type = player_none;
			Vector_t m_origin, eye_pos, abs_origin, mins, maxs;
			int ammo, max_ammo, armour, health, weapon_idx = 0;

			if (!player->IsAlive())
				continue;

			auto is_team_mate = player->m_iTeamNum() == local_pawn->m_iTeamNum();

			if (!g::deathmatch && is_team_mate) continue;

			auto scene = player->m_pGameSceneNode();
			if (!scene) continue;

			auto cur_weapon = player->GetActiveWeapon();
			if (!cur_weapon)  continue;

			auto cur_weapon_data = cur_weapon->GetWeaponData();
			if (!cur_weapon_data) continue;

			auto skeleton = scene->m_pSkeleton();
			if (!skeleton) continue;

			auto model_state = &skeleton->GetModel();
			if (!model_state) continue;

			stronghandle<CModel> model = model_state->m_hModel();
			if (!model) continue;

			auto model_skeleton = &model->m_modelSkeleton();
			if (!model_skeleton) continue;

			auto bone_cache = model_state->hitbox_data();
			if (!bone_cache) continue;

			/* cache type of player (enemy/mate) */
			m_player_type = (player->m_iTeamNum() != local_pawn->m_iTeamNum()) ? player_enemy : player_teammate;

			/* store player data */
			health = player->m_iHealth();
			armour = player->GetArmorValue();
			name = entity.controller->GetPlayerName();

			/* store position data */
			m_origin = scene->m_vecAbsOrigin();
			eye_pos = player->GetEyePosition();
			abs_origin = scene->m_vecAbsOrigin();
			mins = player->m_pCollision()->m_vecMins();
			maxs = player->m_pCollision()->m_vecMaxs();

			/* store weapon data */
			ammo = cur_weapon->clip1();
			max_ammo = cur_weapon_data->m_iMaxClip1();

			auto attribute_container = &cur_weapon->m_AttributeManager();
			if (attribute_container) {
				auto item_view = &attribute_container->m_Item();
				if (item_view) {
					auto item_definition = item_view->GetStaticData();
					if (item_definition) {
						auto name = item_definition->GetSimpleWeaponName();
						weapon_name = name;
						weapon_idx = item_definition->m_nDefIndex;
					}
				}
			}

			player->get_bounding_box(box, false);
			/*
			player_object_t player_obj(
				(CBaseEntity*)entity.controller,               // a1: CBaseEntity* a1
				player,               // plyr: CCSPlayerPawn* plyr
				entity.handle, // a2: CBaseHandle a2 (assuming you have access to entity.controller)
				box,                  // a3: ImVec4 a3 (you should define the bounding box beforehand)
				m_player_type,        // a4: player_object_type_t a4
				ammo,                 // a5: int a5
				max_ammo,             // a6: int a6
				health,               // a7: int a7
				weapon_idx,           // a8: int a8
				armour,               // a9: int a9
				name,                 // a10: const char* a10
				weapon_name,          // a11: const char* a11
				bone_cache,           // a12: bone_data* a12
				eye_pos,              // a13: Vector_t a13
				abs_origin,           // a14: Vector_t a14
				mins,                 // a15: Vector_t a15
				maxs,                 // a16: Vector_t a16
				scene->m_nodeToWorld(), // node_transf: CTransform node_transf
				player->m_iTeamNum()  // a17: int a17 (team number)
			);
					
			new_players.push_back(std::move(player_obj));*/
		}
	});

	update_players(std::move(new_players));
}

void visuals::c_visuals::cache()
{
	if (!sdk::m_engine->is_valid())
		return;

	m_entity_object.clear();
	m_projectile_object.clear();
	m_c4_object.clear();
	m_items_object.clear();
	m_player_object.clear();


	auto local_controller = CCSPlayerController::Get();
	if (!local_controller)
		return;

	auto local_player = hooks::m_get_local_player(0);
	if (!local_player)
		return;
	/* start player caching etc */
	{

		/* make sure our vector holds max capacity */
		int max_ents = sdk::m_game->pGameEntitySystem->GetHighestEntityIndex();


		for (int i = 1; i <= max_ents; i++)
		{
			auto entity = sdk::m_game->pGameEntitySystem->Get(i);
			if (!entity)
				continue;

			if (!entity->handle().valid())
				continue;

			SchemaClassInfoData_t* _class = nullptr;
			entity->dump_class_info(&_class);
			if (!_class)
				continue;

			const uint32_t hash = hash_32_fnv1a_const(_class->szName);

			Vector_t m_origin = Vector_t();

			/* setup the type of each entity group */
			object_type_t m_type = object_none;
			player_object_type_t m_player_type = player_none;
			projectile_object_type_t m_projectile_type = projectile_none;

			/* perhaps i coudl use this logic instead for all entities and sort them based on class name ??? prob better*/
					/* perhaps i coudl use this logic instead for all entities and sort them based on class name ??? prob better*/
			if (hash == hash_32_fnv1a_const(xorstr_("CCSPlayerController")))
			{


				/* initialize all vars first */
				ImVec4 box;
				const char* name = "bot";
				const char* weapon_name = "bot";
				Vector_t eye_pos, abs_origin, mins, maxs;
				int ammo, max_ammo, armour, health, weapon_idx = 0;

				m_player_type = player_none;
				CCSPlayerController* p_controller = reinterpret_cast<CCSPlayerController*>(entity);

				/* sanity stuff */
				if (!p_controller) {

					continue;
				}

				if (!p_controller->PawnHandle().valid()) {
					if (p_controller == local_controller)
						m_local_player_data.reset();

					continue;
				}

				if (p_controller == local_controller) {
					auto player = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(p_controller->PawnHandle().index());
					if (!player) {
						m_local_player_data.reset();
						continue;
					}

					m_local_player_data.alive = player->IsAlive();
					if (player->IsAlive()) {
						m_local_player_data.eye = player->GetEyePosition();
						m_local_player_data.abs_origin = player->m_pGameSceneNode()->m_vecAbsOrigin();
						m_local_player_data.health = player->m_iHealth();
						m_local_player_data.armour = player->GetArmorValue();
						m_local_player_data.handle = player->handle().index();
						m_local_player_data.valid = player->IsAlive() && player->GetActiveWeapon();
						m_local_player_data.velocity = player->m_vecVelocity();
						m_local_player_data.flags = player->m_fFlags();

						auto weapon = player->GetActiveWeapon();
						if (weapon) {
							auto data = weapon->GetWeaponData();
							if (data) {
								m_local_player_data.has_grenade = data->m_WeaponType() == WEAPONTYPE_GRENADE;
							}
						}
					}
					else {
						m_local_player_data.eye = Vector_t();
						m_local_player_data.abs_origin = Vector_t();
						m_local_player_data.velocity = Vector_t();
						m_local_player_data.flags = 0;
						m_local_player_data.health = 0;
						m_local_player_data.armour = 0;
						m_local_player_data.handle = 0;
						m_local_player_data.valid = false;
						m_local_player_data.has_grenade = false;
					}
				}
				else {
					auto player = sdk::m_game->pGameEntitySystem->Get<CCSPlayerPawn>(p_controller->PawnHandle().index());
					if (!player) {
						continue;
					}

					if (!player->IsAlive())
						continue;

					auto scene = player->m_pGameSceneNode();
					if (!scene)
						continue;

					auto cur_weapon = player->GetActiveWeapon();
					if (!cur_weapon)
						continue;

					auto cur_weapon_data = cur_weapon->GetWeaponData();
					if (!cur_weapon_data)
						continue;

					auto skeleton = scene->m_pSkeleton();
					if (!skeleton)
						continue;

					auto model_state = &skeleton->GetModel();
					if (!model_state)
						continue;

					stronghandle<CModel> model = model_state->m_hModel();
					if (!model)
						continue;

					auto model_skeleton = &model->m_modelSkeleton();
					if (!model_skeleton)
						continue;

					auto bone_cache = model_state->hitbox_data();
					if (!bone_cache)
						continue;

					std::vector<cached_bone_data> _cached_bones;
					_cached_bones.resize(28);
					for (int i = 0; i <= 27; i++) {
						_cached_bones[i].pos = bone_cache[i].pos;
					}

					/* cache type of player (enemy/mate) */
					m_player_type = player->enemy(local_player) ? player_enemy : player_teammate;

					/* store player data */
					health = player->m_iHealth();
					armour = player->GetArmorValue();
					name = p_controller->GetPlayerName();

					/* store position data */
					m_origin = scene->m_vecAbsOrigin();
					eye_pos = player->GetEyePosition();
					abs_origin = scene->m_vecAbsOrigin();
					mins = player->m_pCollision()->m_vecMins();
					maxs = player->m_pCollision()->m_vecMaxs();

					/* store weapon data */
					ammo = cur_weapon->clip1();
					max_ammo = cur_weapon_data->m_iMaxClip1();

					auto attribute_container = &cur_weapon->m_AttributeManager();
					if (attribute_container) {
						auto item_view = &attribute_container->m_Item();
						if (item_view) {
							auto item_definition = item_view->GetStaticData();
							if (item_definition) {
								auto name = item_definition->GetSimpleWeaponName();
								weapon_name = name;
								weapon_idx = item_definition->m_nDefIndex;
							}
						}
					}

					player->get_bounding_box(box, false);

					m_player_object.emplace_back(entity, player, p_controller->handle(), box,
						m_player_type, ammo, max_ammo, health, weapon_idx, armour, name,
						weapon_name, _cached_bones, eye_pos, abs_origin, mins, maxs, scene->m_nodeToWorld(), player->m_iTeamNum());
				}
			}
			else if (hash == hash_32_fnv1a_const(xorstr_("C_SmokeGrenadeProjectile")) && local_player->IsAlive())
			{

				m_projectile_type = projectile_smoke;

				auto projectile = (C_BaseCSGrenade*)entity;
				if (!projectile)
					continue;

				if (!entity->m_hOwnerEntity().valid())
					continue;

				auto owner = sdk::m_game->pGameEntitySystem->Get(entity->m_hOwnerEntity());
				if (!owner)
					continue;

				auto smoke_genade = (C_SmokeGrenadeProjectile*)entity;
				if (!smoke_genade) {
					continue;
				}
				/* store data here */
				m_origin = Vector_t();

				ImVec4 projectile_box;
				projectile->get_bounding_box(projectile_box, true);
				auto screen_origin = entity->m_pGameSceneNode()->m_vecAbsOrigin();

				auto m_nSmokeEffectTickBegin = smoke_genade->m_nSmokeEffectTickBegin();
				
				const auto nSmokeLifetime = 19.5f;
				const auto spawn_time = TICKS_TO_TIME(m_nSmokeEffectTickBegin);
				const auto seconds_left = (spawn_time + nSmokeLifetime) - sdk::m_global_vars->m_curtime;
				const auto factor = ((spawn_time + nSmokeLifetime) - sdk::m_global_vars->m_curtime) / nSmokeLifetime;
				
				bool expired = seconds_left <= 0;
				if (m_nSmokeEffectTickBegin == 0)
					expired = false;
				/* team check */
				bool team_owner = (owner->get_entity_by_handle() != local_player->get_entity_by_handle() && owner->m_iTeamNum() == local_player->m_iTeamNum());

				m_projectile_object.emplace_back(entity, projectile->handle(), projectile_box, m_projectile_type, projectile, owner, entity->m_pGameSceneNode()->m_vecAbsOrigin(), expired, entity->m_pCollision()->m_vecMins(), entity->m_pCollision()->m_vecMaxs(), entity->m_pGameSceneNode()->m_nodeToWorld(), team_owner);
			}
			else if (hash == hash_32_fnv1a_const(xorstr_("C_FlashbangProjectile")) && local_player->IsAlive())
			{

				m_projectile_type = projectile_flash;

				auto projectile = (C_BaseCSGrenade*)entity;
				if (!projectile)
					continue;

				if (!entity->m_hOwnerEntity().valid())
					continue;

				auto owner = sdk::m_game->pGameEntitySystem->Get(entity->m_hOwnerEntity());
				if (!owner)
					continue;

				/* store data here */
				m_origin = Vector_t();

				ImVec4 projectile_box;
				projectile->get_bounding_box(projectile_box, true);
				bool team_owner = (owner->get_entity_by_handle() != local_player->get_entity_by_handle() && owner->m_iTeamNum() == local_player->m_iTeamNum());
				m_projectile_object.emplace_back(entity, projectile->handle(), projectile_box, m_projectile_type, projectile, owner, entity->m_pGameSceneNode()->m_vecAbsOrigin(), false, entity->m_pCollision()->m_vecMins(), entity->m_pCollision()->m_vecMaxs(), entity->m_pGameSceneNode()->m_nodeToWorld(), team_owner);
			}
			else if (hash == hash_32_fnv1a_const(xorstr_("C_HEGrenadeProjectile")) && local_player->IsAlive())
			{

				m_projectile_type = projectile_grenade;

				auto projectile = (C_BaseCSGrenade*)entity;
				if (!projectile)
					continue;

				if (!entity->m_hOwnerEntity().valid())
					continue;

				auto owner = sdk::m_game->pGameEntitySystem->Get(entity->m_hOwnerEntity());
				if (!owner)
					continue;

				/* store data here */
				m_origin = Vector_t();

				ImVec4 projectile_box;
				projectile->get_bounding_box(projectile_box, true);
				bool expired = projectile->m_nExplodeEffectTickBegin() == 0 ? false : true;

				bool team_owner = (owner->get_entity_by_handle() != local_player->get_entity_by_handle() && owner->m_iTeamNum() == local_player->m_iTeamNum());
				m_projectile_object.emplace_back(entity, projectile->handle(), projectile_box, m_projectile_type, projectile, owner, entity->m_pGameSceneNode()->m_vecAbsOrigin(), expired, entity->m_pCollision()->m_vecMins(), entity->m_pCollision()->m_vecMaxs(), entity->m_pGameSceneNode()->m_nodeToWorld(), team_owner);
			}
			else if (hash == hash_32_fnv1a_const(xorstr_("C_MolotovProjectile")) && local_player->IsAlive())
			{

				m_projectile_type = projectile_molotov;

				auto projectile = (C_BaseCSGrenade*)entity;
				if (!projectile)
					continue;

				if (!entity->m_hOwnerEntity().valid())
					continue;

				auto owner = sdk::m_game->pGameEntitySystem->Get(entity->m_hOwnerEntity());
				if (!owner)
					continue;

				/* store data here */
				m_origin = Vector_t();

				ImVec4 projectile_box;
				projectile->get_bounding_box(projectile_box, true);
				bool team_owner = (owner->get_entity_by_handle() != local_player->get_entity_by_handle() && owner->m_iTeamNum() == local_player->m_iTeamNum());
				m_projectile_object.emplace_back(entity, projectile->handle(), projectile_box, m_projectile_type, projectile, owner, entity->m_pGameSceneNode()->m_vecAbsOrigin(), false, entity->m_pCollision()->m_vecMins(), entity->m_pCollision()->m_vecMaxs(), entity->m_pGameSceneNode()->m_nodeToWorld(), team_owner);
			}
			else if (entity->IsWeapon() && local_player->IsAlive())
			{
				auto weapon = (CCSWeaponBase*)entity;
				if (!weapon || weapon->m_hOwnerEntity().valid() || !weapon->IsWeapon())
					continue;

				if (weapon->m_iState() != weapon_state::WEAPON_NOT_CARRIED)
					continue;

				/* store data here */
				auto data = weapon->GetWeaponData();
				if (!data)
					continue;

				auto attribute_container = &weapon->m_AttributeManager();
				if (!attribute_container)
					continue;

				auto item_view = &attribute_container->m_Item();
				if (!item_view)
					continue;

				auto item_definition = item_view->GetStaticData();
				if (!item_definition)
					continue;

				auto name = item_definition->GetSimpleWeaponName();

				ImVec4 box;

				int ammo = weapon->clip1();
				int max_clip = data->m_iMaxClip1();
				int weapon_index = item_definition->m_nDefIndex;
				m_type = object_weapons;

				m_items_object.emplace_back(entity, name, ammo, max_clip, entity->handle(), box, weapon_index, entity->m_pGameSceneNode()->m_nodeToWorld(), entity->m_pCollision()->m_vecMins(), entity->m_pCollision()->m_vecMaxs(), 0);

			//m_entity_object.emplace_back(entity,nullptr, entity->handle(), box, m_type);
			} /* store molotovs */
			else if (entity->IsInferno() && local_player->IsAlive())
			{

				auto inferno = (C_Inferno*)entity;
				if (!inferno)
					continue;

				m_type = object_molotovs;

				/* store data here */
				m_origin = Vector_t();
				ImVec4 box;

				if (entity->m_hOwnerEntity().valid()) {
					auto owner = sdk::m_game->pGameEntitySystem->Get(entity->m_hOwnerEntity());
					if (owner) {
				
						bool expired = false;
						int stopped_burning_count = 0;
						for (int i = 0; i < inferno->m_fireCount(); ++i) {
							if (!inferno->m_bFireIsBurning()[i]) {
								stopped_burning_count++;
								// at least 5 fires have stopped burning, mark as expired
								if (stopped_burning_count >= 5) {
									expired = true;
									break; 
								}
							}
						}

						ImVec4 projectile_box;
						m_projectile_type = projectile_molotov;
						bool team_owner = (owner->get_entity_by_handle() != local_player->get_entity_by_handle() && owner->m_iTeamNum() == local_player->m_iTeamNum());

						m_projectile_object.emplace_back(entity, entity->handle(), projectile_box, m_projectile_type, (C_BaseCSGrenade*)entity, owner, entity->m_pGameSceneNode()->m_vecAbsOrigin(), expired, entity->m_pCollision()->m_vecMins(), entity->m_pCollision()->m_vecMaxs(), entity->m_pGameSceneNode()->m_nodeToWorld(), team_owner);
					}
				}

				/* store data here */
				m_origin = Vector_t();

				m_entity_object.emplace_back(entity, nullptr, entity->handle(),box, m_type);
			}
			else if (entity->IsPlantedC4() && local_player->IsAlive()) {

				auto planted_c4 = (C_PlantedC4*)entity;
				if (!planted_c4)
					continue;

				m_type = object_planted_c4;
				/* store data here */
				ImVec4 box;

				m_origin = entity->m_pGameSceneNode()->m_vecAbsOrigin();
				m_c4_object.emplace_back(entity->handle(), m_origin);

				m_entity_object.emplace_back(entity, nullptr, entity->handle(), box, m_type);
			}
		}
	}

}