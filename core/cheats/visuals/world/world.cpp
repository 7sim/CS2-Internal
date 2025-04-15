#include "world.h"
#include "..\..\..\sdk\cache\cache_entity.h"
#include "..\..\backtrack\record.h"
#include "..\preview\preview.h"
#include "..\..\..\sdk\addresses\entity.h"
#include <imgui_settings.h>
#include "..\..\..\sdk\addresses\entity.h"
#include "..\proccess.h"
#include "effects.h"
#include "..\..\..\utils\hook\hooks.h"
#include <imgui_internal.h>
#include <imgui.h>
using namespace visuals::preview;
static std::unordered_map<int, InfernoData> g_fireCache;




enum e_weapon_item : std::uint32_t
{
	weapon_deagle = 1,
	weapon_dual_berettas = 2,
	weapon_five_seven = 3,
	weapon_glock = 4,
	weapon_ak47 = 7,
	weapon_aug = 8,
	weapon_awp = 9,
	weapon_famas = 10,
	weapon_g3sg1 = 11,
	weapon_galil = 13,
	weapon_m249 = 14,
	weapon_m4a4 = 16,
	weapon_mac10 = 17,
	weapon_p90 = 19,
	weapon_ump = 24,
	weapon_mp5sd = 23,
	weapon_xm1024 = 25,
	weapon_bizon = 26,
	weapon_mag7 = 27,
	weapon_negev = 28,
	weapon_sawedoff = 29,
	weapon_tec9 = 30,
	weapon_taser = 31,
	weapon_hkp2000 = 32,
	weapon_mp7 = 33,
	weapon_mp9 = 34,
	weapon_nova = 35,
	weapon_p250 = 36,
	weapon_scar20 = 38,
	weapon_sg553 = 39,
	weapon_ssg08 = 40,
	weapon_flashbang = 43,
	weapon_hegrenade = 44,
	weapon_smoke = 45,
	weapon_molotov = 46,
	weapon_decoy = 47,
	weapon_incdendiary = 48,
	weapon_healthshot = 57,
	weapon_m4a1_s = 60,
	weapon_usp_s = 61,
	weapon_cz75 = 63,
	weapon_revolver = 64,
	weapon_Bayonet = 500,
	weapon_KnifeCss = 503,
	weapon_KnifeFlip = 505,
	weapon_KnifeGut = 506,
	weapon_KnifeKarambit = 507,
	weapon_KnifeM9Bayonet = 508,
	weapon_KnifeTactical = 509,
	weapon_KnifeFalchion = 512,
	weapon_KnifeSurvivalBowie = 514,
	weapon_KnifeButterfly = 515,
	weapon_KnifePush = 516,
	weapon_KnifeCord = 517,
	weapon_KnifeCanis = 518,
	weapon_KnifeUrsus = 519,
	weapon_KnifeGypsyJackknife = 520,
	weapon_KnifeOutdoor = 521,
	weapon_KnifeStiletto = 522,
	weapon_KnifeWidowmaker = 523,
	weapon_KnifeSkeleton = 525
};
std::unordered_map<int, const char8_t*> m_weapon_icons = {
	{ weapon_deagle, u8"\uE001" },
	{ weapon_dual_berettas, u8"\uE002" },
	{ weapon_five_seven, u8"\uE003" },
	{ weapon_glock, u8"\uE004" },
	{ weapon_ak47, u8"\uE007" },
	{ weapon_aug, u8"\uE008" },
	{ weapon_awp, u8"\uE009" },
	{ weapon_famas, u8"\uE00A" },
	{ weapon_g3sg1, u8"\uE00B" },
	{ weapon_galil, u8"\uE00D" },
	{ weapon_m249, u8"\uE00E" },
	{ weapon_m4a4, u8"\uE010" },
	{ weapon_mac10, u8"\uE011" },
	{ weapon_p90, u8"\uE013" },
	{ weapon_ump, u8"\uE018" }, // Corrected from weapon_ump45 to weapon_ump
	{ weapon_mp5sd, u8"\uE017" },
	{ weapon_xm1024, u8"\uE019" }, // Corrected from weapon_xm1014 to weapon_xm1024
	{ weapon_bizon, u8"\uE01A" },
	{ weapon_mag7, u8"\uE01B" },
	{ weapon_negev, u8"\uE01C" },
	{ weapon_sawedoff, u8"\uE01D" },
	{ weapon_tec9, u8"\uE01E" },
	{ weapon_taser, u8"\uE01F" },
	{ weapon_hkp2000, u8"\uE020" },
	{ weapon_mp7, u8"\uE021" },
	{ weapon_mp9, u8"\uE022" },
	{ weapon_nova, u8"\uE023" },
	{ weapon_p250, u8"\uE024" },
	{ weapon_scar20, u8"\uE026" },
	{ weapon_sg553, u8"\uE027" }, // Corrected from weapon_sg556 to weapon_sg553
	{ weapon_ssg08, u8"\uE028" },
	{ weapon_flashbang, u8"\uE02B" },
	{ weapon_hegrenade, u8"\uE02C" },
	{ WEAPON_C4_EXPLOSIVE, u8"\uE031"},
	{ weapon_smoke, u8"\uE02D" },
	{ weapon_molotov, u8"\uE02E" },
	{ weapon_decoy, u8"\uE02F" },
	{ weapon_incdendiary, u8"\uE030" },
	{ weapon_healthshot, u8"\uE039" },
	{ weapon_m4a1_s, u8"\uE03C" },
	{ weapon_usp_s, u8"\uE03D" },
	{ weapon_cz75, u8"\uE03F" },
	{ weapon_revolver, u8"\uE040" },
	{ weapon_Bayonet, u8"\uE1F4" },
	{ weapon_KnifeCss, u8"\uE1F7" },
	{ weapon_KnifeFlip, u8"\uE1F9" },
	{ weapon_KnifeGut, u8"\uE1FA" },
	{ weapon_KnifeKarambit, u8"\uE1FB" },
	{ weapon_KnifeM9Bayonet, u8"\uE1FC" },
	{ weapon_KnifeTactical, u8"\uE1FD" },
	{ weapon_KnifeFalchion, u8"\uE200" },
	{ weapon_KnifeSurvivalBowie, u8"\uE202" },
	{ weapon_KnifeButterfly, u8"\uE203" },
	{ weapon_KnifePush, u8"\uE204" },
	{ weapon_KnifeCord, u8"\uE205" },
	{ weapon_KnifeCanis, u8"\uE206" },
	{ weapon_KnifeUrsus, u8"\uE207" },
	{ weapon_KnifeGypsyJackknife, u8"\uE208" },
	{ weapon_KnifeOutdoor, u8"\uE209" },
	{ weapon_KnifeStiletto, u8"\uE20A" },
	{ weapon_KnifeWidowmaker, u8"\uE20B" },
	{ weapon_KnifeSkeleton, u8"\uE20C" }
};
const char* convertToUpper(const char* str) {
	static char buffer[256]; // Adjust the size as needed
	int i = 0;

	// Convert each character to uppercase
	while (str[i] != '\0' && i < sizeof(buffer) - 1) {
		buffer[i] = toupper(str[i]);
		i++;
	}
	buffer[i] = '\0'; // Null-terminate the string

	return buffer;
}


void world::c_esp::run() {
	if (!sdk::m_engine->is_valid())
		return;

	this->sky_box();

	if (m_entity_object.empty())
		return;

	g_fireCache.clear(); // Clear outdated cache

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

		if (!entity->IsInferno())
			continue;

		auto inferno = sdk::m_game->pGameEntitySystem->Get<C_Inferno>(entity->handle());
		if (!inferno)
			continue;

		// Cache only burning positions
		g_fireCache[entity->handle().index()] = InfernoData(
			inferno->m_fireCount(),      // Fire count
			inferno->m_bFireIsBurning(), // Pointer to bool[64]
			inferno->m_firePositions()   // Pointer to Vector_t[64]
		);
	}
}

struct C4Data {
	bool planted;
	int bomb_site;
	std::time_t plant_time;
	int seccond_time;
	int defuse_countdown;
	CBaseHandle defuser;
	Vector_t position;
};
static std::vector<C4Data> g_c4;
void world::c_c4::cache() {
	if (!sdk::m_engine->is_valid())
		return;
	if (!cfg_get(bool, g_cfg.long_jump))
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

		Vector_t m_origin = Vector_t();
		object_type_t m_type = object_none;

		/* perhaps i coudl use this logic instead for all entities and sort them based on class name ??? prob better*/
		if (entity->IsPlantedC4()) {

			if (!sdk::m_game_rules->m_bBombPlanted())
				continue;

			auto planted_c4 = sdk::m_game->pGameEntitySystem->Get<C_PlantedC4>(entity->handle());
			if (!planted_c4)
				continue;

			C4Data data;

			if (g_c4.empty()) {
				data.planted = true;
				data.bomb_site = planted_c4->m_nBombSite();
				data.position = entity->m_pGameSceneNode()->m_vecAbsOrigin();

			}
			else {		/* c4 exists keep updating time etc */
				g_c4.back().planted = true;
				g_c4.back().bomb_site = planted_c4->m_nBombSite();
				g_c4.back().position = entity->m_pGameSceneNode()->m_vecAbsOrigin();
			}

			if (g_c4.empty())
				g_c4.push_back(data);
		}
	}
	
}
const char* convertToUpperr2(const char* str) {
	static char buffer[256]; // Adjust the size as needed
	int i = 0;

	// Convert each character to uppercase
	while (str[i] != '\0' && i < sizeof(buffer) - 1) {
		buffer[i] = toupper(str[i]);
		i++;
	}
	buffer[i] = '\0'; // Null-terminate the string

	return buffer;
}

std::string extractWeaponName2(const std::string& fullName) {
	// Remove the "weapon_" prefix
	const std::string prefix = "weapon_";
	std::string truncatedName = fullName;

	// Check if the prefix exists in the name
	if (truncatedName.find(prefix) == 0) {
		truncatedName = truncatedName.substr(prefix.length());
	}

	// Find the first underscore after the prefix removal
	size_t underscorePos = truncatedName.find('_');
	if (underscorePos != std::string::npos) {
		truncatedName = truncatedName.substr(0, underscorePos);
	}

	return truncatedName;
}
bool get_item_bounding_box(CTransform node, Vector_t min, Vector_t max, ImVec4& out)
{
	const Matrix3x4_t matTransform = node.quatOrientation.ToMatrix(node.vecPosition);
	out.x = out.y = std::numeric_limits<float>::max();
	out.z = out.w = -std::numeric_limits<float>::max();
	for (int i = 0; i < 8; ++i) {
		const Vector_t vecPoint{ i & 1 ? max.x : min.x, i & 2 ? max.y : min.y, i & 4 ? max.z : min.z };
		ImVec2 vecScreen;
		if (!math::WorldToScreen(vecPoint.Transform(matTransform), vecScreen))
			return false;

		out.x = math::Min(out.x, vecScreen.x); out.y = math::Min(out.y, vecScreen.y); out.z = math::Max(out.z, vecScreen.x); out.w = math::Max(out.w, vecScreen.y);
	}

	const float offset = 2.0f;
	out.x -= offset;  // Left
	out.y -= offset;  // Top
	out.z += offset;  // Right
	out.w += offset;  // Bottom

	return true;
} 

/* render dropped items */
void world::c_items::run() {
	if (!cfg_get(bool, g_cfg.weapon_esp))
		return;

	auto local_controller = CCSPlayerController::Get();
	if (!local_controller)
		return;

	auto local = hooks::m_get_local_player(0);
	if (!local)
		return;

	if (m_items_object.empty())
		return;

	/* loop thru cached entities and run esp on each them */
	/* im not sure about rendering delay on ms etc thats something that gotta be benchmarked with this "caching" method */
	for (const auto& item_object : m_items_object)
	{
		if (!item_object.handle.valid() || item_object.state != 0)
			continue;

		visuals::preview::Context_t context;
		ImVec4 bbox;
		if (!get_item_bounding_box(item_object.node, item_object.mins, item_object.maxs, bbox))
			continue;		
		
		if (const auto& frameOverlayConfig = cfg_get(FrameOverlayVar_t, g_cfg.weapon_bbox); frameOverlayConfig.bEnable) {

			context.AddBoxComponent(ImGui::GetBackgroundDrawList(), bbox, 1, frameOverlayConfig.flThickness, frameOverlayConfig.flRounding, frameOverlayConfig.colPrimary, frameOverlayConfig.colOutline, 1.f);
		}

		if (const auto& name_config = cfg_get(TextOverlayVar_t, g_cfg.weapon_name); name_config.bEnable) {
			// Extract the weapon name
			std::string cleanedWeaponName = extractWeaponName2(item_object.name);
			const char* upperCaseWeaponName = convertToUpperr2(cleanedWeaponName.c_str());

				context.AddComponent(new CTextComponent(true, false, SIDE_BOTTOM, DIR_BOTTOM, render::m_esp_weapons, upperCaseWeaponName, g_cfg.weapon_name, 1.f));
		
	
		}
	
		/* ammo esp */
		if (const auto& ammo_config = cfg_get(BarOverlayVar_t, g_cfg.weapon_ammo_bar); ammo_config.bEnable) {
			if (item_object.max_ammo > 0) {
				const float factor = static_cast<float>(item_object.ammo) / item_object.max_ammo;
				context.AddComponent(new CBarComponent(true, SIDE_BOTTOM, bbox, item_object.max_ammo, factor, g_cfg.weapon_ammo_bar, 1.f));
			}
		}

		context.Render(ImGui::GetBackgroundDrawList(), bbox);
	}
}

void world::c_esp::molotov() {
	if (!cfg_get(bool, g_cfg.molotov_radius))
		return;

	if (g_fireCache.empty())
		return;

	static const auto flameCircumference = [] {
		std::array<Vector_t, 72> points;
		for (std::size_t i = 0; i < points.size(); ++i) {
			constexpr auto flameRadius = 60.0f; // radius of the molotov flame
			points[i] = Vector_t{ flameRadius * std::cos(M_DEG2RAD(i * (360.0f / points.size()))),
								  flameRadius * std::sin(M_DEG2RAD(i * (360.0f / points.size()))),
								  0.0f };
		}
		return points;
	}();

	for (auto& molotov : g_fireCache) {
		if (molotov.second.points.empty())
			continue;

		for (const auto& pos : molotov.second.points) {
			// Check if the center point is visible
			ImVec2 screenCenter;
			if (!math::WorldToScreen(pos, screenCenter))
				continue; // Skip if center point is out of view

			std::array<ImVec2, flameCircumference.size()> screenPoints;
			std::size_t count = 0;

			// Transform the flameCircumference points to screen space
			for (const auto& point : flameCircumference) {
				if (math::WorldToScreen(pos + point, screenPoints[count]))
					++count;
			}

			// Skip rendering if no points are visible
			if (count < 3) // At least 3 points are needed to form a polygon
				continue;

			// Find the point with the minimum Y coordinate (and X coordinate as a tie-breaker)
			std::swap(screenPoints[0], *std::min_element(screenPoints.begin(), screenPoints.begin() + count, [](const auto& a, const auto& b) {
				return a.y < b.y || (a.y == b.y && a.x < b.x);
				}));

			// Sort points based on orientation relative to the reference point (screenPoints[0])
			const auto orientation = [](const ImVec2& a, const ImVec2& b, const ImVec2& c) {
				return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
			};

			std::sort(screenPoints.begin() + 1, screenPoints.begin() + count, [&](const auto& a, const auto& b) {
				return orientation(screenPoints[0], a, b) > 0.0f;
				});

			// Render the convex polygon
			ImGui::GetForegroundDrawList()->AddConvexPolyFilled(screenPoints.data(), count, cfg_get(ColorPickerVar_t, g_cfg.molotov_radius_clr).colValue.GetU32());
		}
	}
}
const char* GunIcon2(const std::string& weapon)
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
	gunIcons[("defuse")] = "r";

	auto it = gunIcons.find(weapon);
	if (it != gunIcons.end()) {
		return it->second;
	}

	return "";
}
// projectiles
struct c_detonation {
	int time_in_ticks = 0;
	int nade_detonation = 0;
};

c_detonation detonation_time(int nade_idx) {
	int next_think_tick = 0;
	int nade_detonate_time;
	switch (nade_idx)
	{
	case WEAPON_SMOKE_GRENADE:
		nade_detonate_time = 1.5f;
		next_think_tick = TIME_TO_TICKS(1.5f);
		break;
	case WEAPON_FLASHBANG:
	case WEAPON_FRAG_GRENADE:
		nade_detonate_time = 1.5f;
		next_think_tick = TIME_TO_TICKS(0.02f);

		break;
	case WEAPON_MOLOTOV:
	case WEAPON_INCENDIARY_GRENADE:
		nade_detonate_time = 2;
		next_think_tick = TIME_TO_TICKS(0.02f);
		break;
	}

	return c_detonation{ next_think_tick, nade_detonate_time };
}

__forceinline QAngle_t calc_ang(const Vector_t& src, const Vector_t& dst)
{
	QAngle_t vAngle;
	Vector_t delta((src.x - dst.x), (src.y - dst.y), (src.z - dst.z));
	double hyp = sqrt(delta.x * delta.x + delta.y * delta.y);

	vAngle.x = float(atanf(float(delta.z / hyp)) * 57.295779513082f);
	vAngle.y = float(atanf(float(delta.y / delta.x)) * 57.295779513082f);
	vAngle.z = 0.0f;

	if (delta.x >= 0.0)
		vAngle.y += 180.0f;

	return vAngle;
}

void detect_out_of_view_grenade(Vector_t grenade, int type) // Function to detect out-of-view grenades
{
	
}


void world::c_esp::grenade_projectile() {

}
bool m_has_been_planted = false;
std::time_t m_plant_time;


void world::c_c4::clear() {
	if (!g_c4.empty())
		g_c4.clear();
}
uint64_t current_ms() {
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}


std::pair<int, int> get_bomb_calculations_by_map(const std::string& map) {
	int map_hash = hash_32_fnv1a_const(map.c_str());
	if (map_hash == hash_32_fnv1a_const("de_dust2")) {
		return { 500, 1750 };
	}
	else if (map_hash == hash_32_fnv1a_const("de_ancient")) {
		return { 650, 2275 };
	}
	else if (map_hash == hash_32_fnv1a_const("de_anubis")) {
		return { 450, 1575 };
	}
	else if (map_hash == hash_32_fnv1a_const("de_inferno")) {
		return { 620, 2170 };
	}
	else if (map_hash == hash_32_fnv1a_const("de_mirage")) {
		return { 650, 2275 };
	}
	else if (map_hash == hash_32_fnv1a_const("de_nuke")) {
		return { 650, 2275 };
	}
	else if (map_hash == hash_32_fnv1a_const("de_overpass")) {
		return { 650, 2275 };
	}
	else if (map_hash == hash_32_fnv1a_const("de_vertigo")) {
		return { 500, 1750 };
	}
	else {
		return { 650, 2275 };
	}
}
float armor_modifier(float damage, int armor) {
	if (armor > 0) {
		const float armor_ratio = 0.5f;
		const float armor_bonus = 0.5f;
		float armor_ratio_multiply = damage * armor_ratio;
		float actual = (damage - armor_ratio_multiply) * armor_bonus;

		if (actual > static_cast<float>(armor)) {
			actual = static_cast<float>(armor) * (1.f / armor_bonus);
			armor_ratio_multiply = damage - actual;
		}

		damage = armor_ratio_multiply;
	}
	return damage;
}

std::string extract_map_name(const std::string & input) {
	// Find the last occurrence of '/' and '.vpk'
	size_t startPos = input.rfind('/') + 1; // +1 to move past the '/'
	size_t endPos = input.rfind(".vpk");

	// Check if both positions are valid
	if (startPos != std::string::npos && endPos != std::string::npos && startPos < endPos) {
		return input.substr(startPos, endPos - startPos);
	}

	// Return an empty string if the format is not as expected
	return "";
}
int calculate_bomb_damage(Vector_t player, Vector_t bomb, int armor) {

	const std::pair<int, int> bomb_calculations = get_bomb_calculations_by_map(extract_map_name(std::string(sdk::m_global_vars->m_current_map_name)));
	const int bomb_damage = bomb_calculations.first;
	const int bomb_radius = bomb_calculations.second;

	const double c = bomb_radius / 3;
	const float damage = bomb_damage * std::exp(-std::pow(sqrt(pow(player.x - bomb.x, 2) +
		pow(player.y - bomb.y, 2) +
		pow(player.z - bomb.z, 2)), 2) / (2 * std::pow(c, 2)));
	const float damage_armor = armor_modifier(damage, armor);

	return static_cast<int>(std::floor(damage_armor));
}

// Helper function for drawing a circular progress bar
void DrawCircularProgressBar(float x, float y, float radius, float thickness, float progress, ImU32 color) {
	const int num_segments = 64;
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	const float start_angle = -math::_PI / 2; // Start at the top of the circle
	const float end_angle = start_angle + progress * 2 * math::_PI;

	for (int i = 0; i < num_segments; ++i) {
		const float t = (float)i / (num_segments - 1);
		const float angle = start_angle + t * (end_angle - start_angle);

		float x1 = x + std::cos(angle) * (radius - thickness / 2.0f);
		float y1 = y + std::sin(angle) * (radius - thickness / 2.0f);
		float x2 = x + std::cos(angle) * (radius + thickness / 2.0f);
		float y2 = y + std::sin(angle) * (radius + thickness / 2.0f);

		draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), color, thickness);
	}
}

#include <imgui_internal.h>
void bomb_ui(bool planted, const char* site, int damage, float remaining_time) {
	int nWindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar;

	
}


#include "..\..\..\utils\render\ui\menu.h"

void world::c_c4::run() {
	

}
static Color_t old_sky_color = Color_t(255, 255, 255);
static float old_sky_bright = 1.f;

static bool should_reset_sky_color = false;
static bool reseted_sky_color = false;
void world::c_esp::sky_box() {
	if (g::m_unloading) {
		for (int i = 0; i < sdk::m_game->pGameEntitySystem->GetHighestEntityIndex(); ++i) {
			auto entity = sdk::m_game->pGameEntitySystem->Get(i);

			if (!entity)
				continue;

			SchemaClassInfoData_t* class_info = nullptr;
			entity->dump_class_info(&class_info);

			if (!class_info)
				continue;

			if (c_run_time::StringCompare(class_info->szName, xorstr_("C_EnvSky")) != 0)
				continue;

			entity->sky_m_bEnabled() = true;
			entity->sky_m_vTintColor() = old_sky_color;
			entity->sky_m_flBrightnessScale() = old_sky_bright;
			utils::CallVFunc<void, 7>(entity, 1);
		}
		return;
	}

	if (cfg_get(bool, g_cfg.modulate_sky)) {
		for (int i = 0; i < sdk::m_game->pGameEntitySystem->GetHighestEntityIndex(); ++i) {
			auto entity = sdk::m_game->pGameEntitySystem->Get(i);

			if (!entity)
				continue;

			SchemaClassInfoData_t* class_info = nullptr;
			entity->dump_class_info(&class_info);

			if (!class_info)
				continue;

			if (c_run_time::StringCompare(class_info->szName, xorstr_("C_EnvSky")) != 0)
				continue;

			if (!should_reset_sky_color) {
				old_sky_color = entity->sky_m_vTintColor();
				old_sky_bright = entity->sky_m_flBrightnessScale();
				should_reset_sky_color = true;
			}

			Color_t sky_color = cfg_get(ColorPickerVar_t, g_cfg.sky_col).colValue;
			auto brightness = cfg_get(float, g_cfg.sky_brightness);

			entity->sky_m_bEnabled() = true;
			entity->sky_m_vTintColor() = sky_color;
			entity->sky_m_flBrightnessScale() = brightness;
			utils::CallVFunc<void, 7>(entity, 1);
		}
	}
	// if we grabbed old color succesfully
	// if we aint did this yet
	// if we just disabled skybox color changer
	// then do this ( set color default ):

	else if ( should_reset_sky_color && !cfg_get(bool, g_cfg.modulate_sky)) {

		for (int i = 0; i < sdk::m_game->pGameEntitySystem->GetHighestEntityIndex(); ++i) {
			auto entity = sdk::m_game->pGameEntitySystem->Get(i);

			if (!entity)
				continue;

			SchemaClassInfoData_t* class_info = nullptr;
			entity->dump_class_info(&class_info);

			if (!class_info)
				continue;

			if (c_run_time::StringCompare(class_info->szName, xorstr_("C_EnvSky")) != 0)
				continue;

			entity->sky_m_bEnabled() = true;
			entity->sky_m_vTintColor() = old_sky_color;
			entity->sky_m_flBrightnessScale() = old_sky_bright;
			utils::CallVFunc<void, 7>(entity, 1);
		}

		should_reset_sky_color = false;
	}
}
static bool changed = false;
void world::c_esp::motion_blur() {

	
}
/*
void world::c_nightmode::update() {
	if (ENGINE2::m_engine->is_in_game() && m_force_update)
		m_need_update = true;

	if (!m_need_update)
		return;

	cs2::material_array resource{ };
	sdk::m_resource_utils->DeleteResource enumerate_material(&resource);

	const auto color{ std::clamp< float >(1.f - m_nightmode_scale, 0.f, 1.f) };

	for (int i{ }; i < resource.m_count; i++) {
		const auto material{ *resource.m_resource[i] };

		if (!material)
			continue;

		if (std::strstr(material->get_name(), "weapon")
			|| (std::strncmp(material->get_name(), _("materials/"), 10) != 0
				&& std::strncmp(material->get_name(), _("models/"), 7) != 0)) {
			continue;
		}

		const auto vec_value{ utils::vec4_t{ color, color, color, 0 } };

		material->set_vec_param(_("g_vColorTint"), vec_value);
		material->update_param();
	}

	m_force_update = false;
}
void world::c_nightmode::on_game() {
	m_nightmode_scale = g_settings->m_misc.m_night_mode;

	static bool old_in_game = true;

	// proper method of comparing two floats by @mhalaider
	m_need_update =
		(static_cast<int>(m_nightmode_scale * 1000.f) != static_cast<int>(m_old_nightmode_scale * 1000.f)
			|| !old_in_game)
		&& ENGINE2::m_engine->is_in_game();

	update_world_color();

	old_in_game = ENGINE2::m_engine->is_in_game();

	m_need_update = false;
	m_old_nightmode_scale = m_nightmode_scale;
}
void  world::c_nightmode::on_end_game() {
	m_need_update = true;
	update_world_color();

	m_need_update = false;
}*/