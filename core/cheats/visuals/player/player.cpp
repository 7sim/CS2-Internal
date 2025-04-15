#include "player.h"
#include "..\..\..\sdk\cache\cache_entity.h"
#include "..\..\backtrack\record.h"
#include "..\preview\preview.h"
#include "..\..\..\utils\render\render.h"
#include <imgui_settings.h>
#include "../../../utils/hook/hooks.h"
#include "../../../utils/hook/hooks.h"
#include "..\proccess.h"
using namespace visuals::preview;
const char* convertToUpperr(const char* str) {
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

std::string extractWeaponName(const std::string& fullName) {
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

ImVec2 RotateVertex(const Vector_t& p, const ImVec2& v, float angle) {
	// convert theta angle to sine and cosine representations.
	float c = std::cos(M_DEG2RAD(angle));
	float s = std::sin(M_DEG2RAD(angle));

	return {
		p.x + (v.x - p.x) * c - (v.y - p.y) * s,
		p.y + (v.x - p.x) * s + (v.y - p.y) * c
	};
}
void offscreen(CCSPlayerPawn* player) {

	if (!player->m_pGameSceneNode())
		return;


	Vector_t view_origin, target_pos, delta;
	ImVec2 screen_pos;
	Vector_t offscreen_pos;
	float leeway_x, leeway_y, radius, offscreen_rotation;
	bool is_on_screen;
	std::array<ImVec2, 3> verts;

	// todo - dex; move this?
	auto get_offscreen_data = [](const Vector_t& delta, float radius, Vector_t& out_offscreen_pos, float& out_rotation) {
		Vector_t view_angles(g::angle);
		Vector_t fwd, right, up(0.f, 0.f, 1.f);
		float front, side, yaw_rad, sa, ca;

		// get viewport angles forward directional vector.
		math::angle_vectors(view_angles, fwd);

		// convert viewangles forward directional vector to a unit vector.
		fwd.z = 0.f;
		fwd.normalize();

		// calculate front / side positions.
		right = up.CrossProduct(fwd);
		front = delta.Dot(fwd);
		side = delta.Dot(right);

		// setup offscreen position.
		out_offscreen_pos.x = radius * -side;
		out_offscreen_pos.y = radius * -front;

		// get the rotation (yaw, 0 - 360).
		out_rotation = M_RAD2DEG(std::atan2(out_offscreen_pos.x, out_offscreen_pos.y) + math::_PI);

		// get needed sine / cosine values.
		yaw_rad = M_DEG2RAD(out_rotation);
		sa = std::sin(yaw_rad);
		ca = std::cos(yaw_rad);

		// rotate offscreen position around.
		out_offscreen_pos.x = (ImGui::GetIO().DisplaySize.x / 2.f) + (radius * sa);
		out_offscreen_pos.y = (ImGui::GetIO().DisplaySize.y / 2.f) - (radius * ca);
	};

	// get the player's center screen position.
	target_pos = player->GetEyePosition();
	is_on_screen = math::WorldToScreen(target_pos, screen_pos);

	// give some extra room for screen position to be off screen.
	leeway_x = ImGui::GetIO().DisplaySize.x / 18.f;
	leeway_y = ImGui::GetIO().DisplaySize.y / 18.f;

	// origin is not on the screen at all, get offscreen position data and start rendering.
	if (!is_on_screen
		|| screen_pos.x < -leeway_x
		|| screen_pos.x >(ImGui::GetIO().DisplaySize.x + leeway_x)
		|| screen_pos.y < -leeway_y
		|| screen_pos.y >(ImGui::GetIO().DisplaySize.y + leeway_y)) {

		// get viewport origin.
		view_origin = g::m_local_player_pawn->m_pGameSceneNode()->m_vecAbsOrigin();

		// get direction to target.
		delta = (target_pos - view_origin).normalized();

		// note - dex; this is the 'YRES' macro from the source sdk.
		radius = 200.f * (ImGui::GetIO().DisplaySize.y / 480.f);

		// get the data we need for rendering.
		get_offscreen_data(delta, radius, offscreen_pos, offscreen_rotation);

		// bring rotation back into range... before rotating verts, sine and cosine needs this value inverted.
		// note - dex; reference: 
		// https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/src_main/game/client/tf/tf_hud_damageindicator.cpp#L182
		offscreen_rotation = -offscreen_rotation;

		// setup vertices for the triangle.
		verts[0] = { offscreen_pos.x, offscreen_pos.y };        // 0,  0
		verts[1] = { offscreen_pos.x - 12.f, offscreen_pos.y + 24.f }; // -1, 1
		verts[2] = { offscreen_pos.x + 12.f, offscreen_pos.y + 24.f }; // 1,  1

		// rotate all vertices to point towards our target.
		verts[0] = RotateVertex(offscreen_pos, verts[0], offscreen_rotation);
		verts[1] = RotateVertex(offscreen_pos, verts[1], offscreen_rotation);
		verts[2] = RotateVertex(offscreen_pos, verts[2], offscreen_rotation);

		ImGui::GetBackgroundDrawList()->AddConvexPolyFilled(verts.data(), verts.size(), ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));
	}
}


void skeleton(std::vector< cached_bone_data> bone_array) {

	if (!cfg_get(bool, g_cfg.bSkeleton))
		return;

	if (bone_array.empty())
		return;

	const Color_t skeleton_color{ cfg_get(ColorPickerVar_t, g_cfg.colSkeleton).colValue }; // Light purple with transparency
	int alpha = cfg_get(ColorPickerVar_t, g_cfg.colSkeleton).colValue.a > 50 ? 50 : 0;
	const Color_t outline_color{ 0, 0, 0, alpha }; // Dark outline for better visibility
	float thickness = 1.f;
	float outline_thickness = 1.5f;

	// Helper to draw lines with outlines
	auto draw_bone = [&](int from, int to) {
		ImVec2 screen_start, screen_end;
		if (math::WorldToScreen(bone_array[from].pos, screen_start) &&
			math::WorldToScreen(bone_array[to].pos, screen_end)) {
			ImGui::GetBackgroundDrawList()->AddLine(screen_start, screen_end, outline_color.GetU32(), outline_thickness);
			ImGui::GetBackgroundDrawList()->AddLine(screen_start, screen_end, skeleton_color.GetU32(), thickness);
		}
	};

	// Head and Spine
	draw_bone(6, 5); // Head to Neck
	draw_bone(5, 4); // Neck to Spine_3
	draw_bone(4, 3); // Spine_3 to Spine_2
	draw_bone(3, 2); // Spine_2 to Spine_1
	draw_bone(2, 0); // Spine_1 to Pelvis

	// Left Leg
	draw_bone(0, 22); // Pelvis to Left Upper Leg
	draw_bone(22, 23); // Left Upper Leg to Left Lower Leg
	draw_bone(23, 24); // Left Lower Leg to Left Ankle

	// Right Leg
	draw_bone(0, 25); // Pelvis to Right Upper Leg
	draw_bone(25, 26); // Right Upper Leg to Right Lower Leg
	draw_bone(26, 27); // Right Lower Leg to Right Ankle

	// Left Arm
	draw_bone(4, 8);  // Spine_3 to Left Upper Arm
	draw_bone(8, 9);  // Left Upper Arm to Left Lower Arm
	draw_bone(9, 10); // Left Lower Arm to Left Hand

	// Right Arm
	draw_bone(4, 13); // Spine_3 to Right Upper Arm
	draw_bone(13, 14); // Right Upper Arm to Right Lower Arm
	draw_bone(14, 15); // Right Lower Arm to Right Hand
}


std::mutex m_mutex;



ImVec2 RotateVertex(const ImVec2& center, const ImVec2& vertex, float angle) {
	// Convert the angle to radians
	float rad = M_DEG2RAD(angle);

	// Get sine and cosine of the angle
	float sin_angle = std::sin(rad);
	float cos_angle = std::cos(rad);

	// Translate point to origin (centered at 0,0)
	ImVec2 translated;
	translated.x = vertex.x - center.x;
	translated.y = vertex.y - center.y;

	// Rotate the point around the origin (0,0)
	ImVec2 rotated;
	rotated.x = translated.x * cos_angle - translated.y * sin_angle;
	rotated.y = translated.x * sin_angle + translated.y * cos_angle;

	// Translate the point back to the original center
	ImVec2 result;
	result.x = rotated.x + center.x;
	result.y = rotated.y + center.y;

	return result;
}

void create_arrow(CCSPlayerPawn* player) {
	if (!cfg_get(bool, g_cfg.off_screen))
		return;
	Vector_t view_origin, target_pos, delta;
	ImVec2 screen_pos, offscreen_pos;
	float leeway_x, leeway_y, radius, offscreen_rotation;
	bool is_on_screen;

	// Color of the offscreen arrow
	ImU32 arrow_color = IM_COL32(255, 0, 0, 255); // Arrow color (red with transparency)
	ImU32 border_color = IM_COL32(0, 0, 0, 255);    // Border color (black)

	// todo - dex; move this?
	static auto get_offscreen_data = [](const Vector_t& delta, float radius, ImVec2& out_offscreen_pos, float& out_rotation) {
		QAngle_t view_angles(g::command__number_angle);
		Vector_t fwd, right, up(0.f, 0.f, 1.f);
		float front, side, yaw_rad, sa, ca;

		// Get the viewport forward directional vector
		math::AngleVectors(view_angles, &fwd);
		fwd.z = 0.f;
		fwd.normalize();

		// Calculate front and side positions
		right = up.CrossProduct(fwd);
		front = delta.Dot(fwd);
		side = delta.Dot(right);

		// Set up the offscreen position
		out_offscreen_pos.x = radius * -side;
		out_offscreen_pos.y = radius * -front;

		// Get the rotation (yaw, 0 - 360 degrees)
		out_rotation = M_RAD2DEG(std::atan2(out_offscreen_pos.x, out_offscreen_pos.y) + math::_PI);

		// Convert yaw to radians for sine/cosine calculations
		yaw_rad = M_DEG2RAD(-out_rotation);
		sa = std::sin(yaw_rad);
		ca = std::cos(yaw_rad);

		// Adjust offscreen position based on screen size and center
		out_offscreen_pos.x = (ImGui::GetIO().DisplaySize.x / 2.f) + (radius * sa);
		out_offscreen_pos.y = (ImGui::GetIO().DisplaySize.y / 2.f) - (radius * ca);
	};

	// Get the player's center screen position
	target_pos = player->m_pGameSceneNode()->m_vecAbsOrigin();
	is_on_screen = render::WorldToScreen(target_pos, screen_pos);

	// Leeway for offscreen detection
	leeway_x = ImGui::GetIO().DisplaySize.x / 18.f;
	leeway_y = ImGui::GetIO().DisplaySize.y / 18.f;

	// Check if the target is offscreen
	if (!is_on_screen
		|| screen_pos.x < -leeway_x
		|| screen_pos.x >(ImGui::GetIO().DisplaySize.x + leeway_x)
		|| screen_pos.y < -leeway_y
		|| screen_pos.y >(ImGui::GetIO().DisplaySize.y + leeway_y)) {

		// Get the viewport origin
		view_origin = g::view_origin;

		// Get direction to the target
		delta = (target_pos - view_origin).normalized();

		// Radius scaling based on screen size
		radius = 200.f * (ImGui::GetIO().DisplaySize.y / 480.f);

		// Get offscreen position and rotation data
		get_offscreen_data(delta, radius, offscreen_pos, offscreen_rotation);

		// Bring rotation back into range for vertex rotation
		offscreen_rotation = -offscreen_rotation;

		// Array to hold the vertices (ImVec2) for the triangle (arrow)
		ImVec2 verts[3];

		// Define the triangle vertices based on the offscreen position
		verts[0] = ImVec2(offscreen_pos.x, offscreen_pos.y);            // Tip of the arrow
		verts[1] = ImVec2(offscreen_pos.x - 12.f, offscreen_pos.y + 24.f); // Bottom-left of the arrow
		verts[2] = ImVec2(offscreen_pos.x + 12.f, offscreen_pos.y + 24.f); // Bottom-right of the arrow

		// Rotate the vertices to point towards the target
		verts[0] = RotateVertex(offscreen_pos, verts[0], offscreen_rotation);
		verts[1] = RotateVertex(offscreen_pos, verts[1], offscreen_rotation);
		verts[2] = RotateVertex(offscreen_pos, verts[2], offscreen_rotation);

		// Get the background draw list for rendering
		ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

		// Draw the filled arrow (triangle)
		draw_list->AddConvexPolyFilled(verts, 3, arrow_color);

		// Optionally, draw a border around the triangle
		draw_list->AddPolyline(verts, 3, border_color, ImDrawFlags_Closed, 1.0f);
	}
}

#include "..\..\ragebot\ragebot.h"

__forceinline QAngle_t CalccAngle(const Vector_t& src, const Vector_t& dst)
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
inline float Length(const ImVec2& vec) {
	return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}
void add_arc(ImVec2 position, float radius, float min_angle, float max_angle, Color_t col, float thickness)
{
	ImGui::GetBackgroundDrawList()->PathArcTo(position, radius, M_DEG2RAD(min_angle), M_DEG2RAD(max_angle), 32);
	ImGui::GetBackgroundDrawList()->PathStroke(col.GetU32(), false, thickness);
}

void detect_out_of_view_entity(Vector_t eye_pos, Vector_t pos, Color_t clr) // Default distance set to 250
{
	auto local = hooks::m_get_local_player(0);
	ImVec2 w2s;

	// Check if the target is in view
	if (!local || !cfg_get(bool, g_cfg.out_of_view))
		return;
	if (!local->IsAlive())
		return;

	if (math::WorldToScreen(eye_pos, w2s))
	{	// if the target's position is inside the screen bounds
		if (w2s.x >= 0 && w2s.x <= ImGui::GetIO().DisplaySize.x &&
			w2s.y >= 0 && w2s.y <= ImGui::GetIO().DisplaySize.y) {
			return;
		}
	}

	

	// Get the center of the screen
	ImVec2 screenCenter(ImGui::GetIO().DisplaySize.x / 2.0f, ImGui::GetIO().DisplaySize.y / 2.0f);

	float width = cfg_get(int, g_cfg.out_of_view_size);
	QAngle_t viewangles = g::command__number_angle;

	// Calculate the angle to the target
	auto angle = viewangles.y - CalccAngle(local->GetEyePosition(), pos).y - 90;

	// Get screen size
	int iScreenSizeX = ImGui::GetIO().DisplaySize.x;
	int iScreenSizeY = ImGui::GetIO().DisplaySize.y;
	float distance = cfg_get(int, g_cfg.out_of_view_distance) + 50;
	// Use the custom distance for the arrow
	add_arc(screenCenter, distance, angle - width, angle + width, cfg_get(ColorPickerVar_t, g_cfg.out_of_view_clr).colValue, 4.f);
	add_arc(screenCenter, distance - 6, angle - width, angle + width, cfg_get(ColorPickerVar_t, g_cfg.out_of_view_clr).colValue, 1.5f);
}



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
std::unordered_map<int, const char8_t*> m_weapon_icons_cs2 = {
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
	{ weapon_smoke, u8"\uE02D" },
	{ WEAPON_C4_EXPLOSIVE, u8"\uE031"},
	{ weapon_molotov, u8"\uE02E" },
	{ weapon_decoy, u8"\uE02F" },
	{ weapon_incdendiary, u8"\uE030" },
	{ weapon_healthshot, u8"\uE039" },
	{ weapon_m4a1_s, u8"\uE03C" },
	{ weapon_usp_s, u8"\uE03D" },
	{ weapon_cz75, u8"\uE03F" },
	{ weapon_revolver, u8"\uE040" },
	{ WEAPON_KNIFE0, u8"\uE1F7" },
	{ WEAPON_KNIFE1, u8"\uE1F7" },
	{ WEAPON_KNIFE2, u8"\uE1F7" },
	{ WEAPON_KNIFE3, u8"\uE1F7" },
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


bool get_bounding_box(CTransform node, Vector_t min, Vector_t max, ImVec4& out)
{
	const Matrix3x4_t matTransform = node.quatOrientation.ToMatrix(node.vecPosition);
	out.x = out.y = std::numeric_limits<float>::max();
	out.z = out.w = -std::numeric_limits<float>::max();
	for (int i = 0; i < 8; ++i) {
		const Vector_t vecPoint{ i & 1 ? max.x : min.x, i & 2 ? max.y : min.y, i & 4 ? max.z : min.z };
		ImVec2 vecScreen;
		if (!math::WorldToScreen(vecPoint.Transform(matTransform), vecScreen))
			return false;

		ImVec4 box;

		out.x = math::Min(out.x, vecScreen.x);
		out.y = math::Min(out.y, vecScreen.y);
		out.z = math::Max(out.z, vecScreen.x);
		out.w = math::Max(out.w, vecScreen.y);
	}

	return true;
} 


void player::c_esp::run()
{
	/* stupid sanity checks */
	if (!cfg_get(bool, g_cfg.bVisualOverlay))
		return;

	if (!sdk::m_engine->is_valid())
		return;

	auto local_controller = CCSPlayerController::Get();
	if (!local_controller)
		return;

	auto local = hooks::m_get_local_player(0);
	if (!local)
		return;

	if (m_player_object.empty())
		return;

	/* loop thru cached entities and run esp on each them */
	/* im not sure about rendering delay on ms etc thats something that gotta be benchmarked with this "caching" method */
	for (const auto& player_object : visuals::visuals->m_get_cached_players())
	{
		int handle = player_object.handle.index();

		// if the handle is invalid, skip this entity
		if (!player_object.handle.valid() || player_object.health <= 0 || player_object.handle.index() == INVALID_EHANDLE_INDEX)
			continue;
		

		if (player_object.type != player_enemy) 
			continue;
		

		/* out of view arrow  */
		{
			auto out_of_view_color = cfg_get(ColorPickerVar_t, g_cfg.out_of_view_clr).colValue;
			detect_out_of_view_entity(player_object.eye_pos, player_object.abs_origin, out_of_view_color);
		}

		/* get fresh frame bounding box */
		ImVec4 box;
		if (!get_bounding_box(player_object.node, player_object.mins, player_object.maxs, box)) 
			continue;
	

		/* skeleton / bone stuff */
		if (!player_object.bone_cache.empty())
			skeleton(player_object.bone_cache);
		
		visuals::preview::Context_t context;

		// run drawing elements
		{
			/* bounding box esp */
			if (const auto& frameOverlayConfig = cfg_get(FrameOverlayVar_t, g_cfg.overlayBox); frameOverlayConfig.bEnable) {

				context.AddBoxComponent(ImGui::GetBackgroundDrawList(), box, 1, frameOverlayConfig.flThickness, frameOverlayConfig.flRounding, frameOverlayConfig.colPrimary, frameOverlayConfig.colOutline, 1.f);
			}
			/* health bar esp */
			if (const auto& health_config = cfg_get(BarOverlayVar_t, g_cfg.overlayHealthBar); health_config.bEnable) {

				auto hp = player_object.health / 100.f;

				context.AddComponent(new CBarComponent(false, SIDE_LEFT, box, 100, hp, g_cfg.overlayHealthBar, 1.f));
			}

			/* name esp */
			if (const auto& name_config = cfg_get(TextOverlayVar_t, g_cfg.overlayName); name_config.bEnable) {
				const char* szName = ("bot");

				szName = player_object.name;

				context.AddComponent(new CTextComponent(true, false, SIDE_TOP, DIR_TOP, render::m_esp_name, szName, g_cfg.overlayName, 1.f));
			}

			if (const auto& weaponOverlayConfig = cfg_get(TextOverlayVar_t, g_cfg.Weaponesp); weaponOverlayConfig.bEnable) {

				auto name = player_object.weapon_name;
				auto def_index = player_object.weapon_idx;

				std::string cleanedWeaponName = extractWeaponName(name);
				const char* upperCaseWeaponName = convertToUpperr(cleanedWeaponName.c_str());

			
				 if (cfg_get(int, g_cfg.text_type) == 0 && name) {
					context.AddComponent(new CTextComponent(true, false, SIDE_BOTTOM, DIR_BOTTOM, render::m_esp_weapons, upperCaseWeaponName, g_cfg.Weaponesp, 1.f));
				}
			}

			if (const auto& ammo_config = cfg_get(BarOverlayVar_t, g_cfg.AmmoBar); ammo_config.bEnable) {

				if (player_object.max_ammo > 0) {
					const float factor = static_cast<float>(player_object.ammo) / player_object.max_ammo;
					// here we use the SIDE_BOTTOM which overrided max_ammo as the var used for factor limitation
					context.AddComponent(new CBarComponent(true, SIDE_BOTTOM, box, player_object.max_ammo, factor, g_cfg.AmmoBar, 1.f));
				}
			}

			/* Start Player Flags*/
			{
				if (cfg_get(unsigned int, g_cfg.pEspFlags) & FLAGS_ARMOR) {
					if (player_object.armour > 0) {
						if (const auto& hk_cfg = cfg_get(TextOverlayVar_t, g_cfg.HKFlag); hk_cfg.bEnable) {
							const char* szName = ("K");

							context.AddComponent(new CTextComponent(true, false, SIDE_RIGHT, DIR_RIGHT, render::m_esp_flags, szName, g_cfg.HKFlag, 1.f));
						}
					}
				}

			}
		}

		context.Render(ImGui::GetForegroundDrawList(), box);	
	}

}