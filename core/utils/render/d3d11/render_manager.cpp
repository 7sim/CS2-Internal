// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "render_manager.h"

#include "../../../math/math.h"
void calc_arc_endpoints(render_position position, int start_angle, int end_angle, int radius, std::vector<render_position>& endpoints) {

	if (end_angle > 360.f)
		end_angle -= 180.f;
	// Convert angles to radians
	double s_rad = start_angle * 3.14f / 180.0;
	double e_rad = end_angle * 3.14f / 180.0;

	// Calculate the length of the chord
	double chord_length = 2 * radius * sin((e_rad - s_rad) / 2);

	// Calculate the midpoint of the chord
	double mx = position.x + radius * cos(s_rad);
	double my = position.y + radius * sin(s_rad);
	double mxd = mx + chord_length * cos((e_rad - s_rad) / 2);
	double myd = my + chord_length * sin((e_rad - s_rad) / 2);

	// Calculate the number of points along the arc
	int num_points = 14;

	// Calculate the angle increment
	double angle_inc = (e_rad - s_rad) / num_points;

	// Calculate the points along the arc
	for (int i = 0; i <= num_points; ++i) {
		double angle = s_rad + angle_inc * i;
		double x = mx + radius * cos(angle);
		double y = my + radius * sin(angle);

		endpoints.emplace_back(render_position(x, y));
	}
}

void c_render_manager::arc_line(render_position pos, float start, float degress, float radius, render_color color, float thikness)
{
	if (m_reset)
	{
		m_buffer = m_manager->get_buffer(m_current_index_buffer);
		set_reset_buffer(false);
	}

	pos *= g_render->get_dpi();
	radius *= g_render->get_dpi();

	std::vector<render_position> points;
	calc_arc_endpoints(pos, start, start + degress, radius, points);

	m_buffer->poly_line(points.data(), points.size(), color.override_alpha(get_override_alpha()), thikness, true);
}

void c_render_manager::arc_fill(render_position pos, float start, float degress, float radius, render_color color)
{
	if (m_reset)
	{
		m_buffer = m_manager->get_buffer(m_current_index_buffer);
		set_reset_buffer(false);
	}

	pos *= g_render->get_dpi();
	radius *= g_render->get_dpi();

	std::vector<render_position> points;
	calc_arc_endpoints(pos, start, start + degress, radius, points);

	m_buffer->poly_fill(points.data(), points.size(), color.override_alpha(get_override_alpha()));
	m_buffer->poly_line(points.data(), points.size(), color.override_alpha(get_override_alpha()), 1.f, true);
	m_buffer->push_clip_rect(render_position(pos.x, pos.y - radius * 2.f), render_position(points.front().x, pos.y + radius * 2.f));
	m_buffer->rectangle_filled(render_position(points.front().x, pos.y), render_position(points.front().x + 1, pos.y + 1), color.override_alpha(get_override_alpha()));
	m_buffer->pop_clip_rect();
}

void c_render_manager::push_override_alpha(float value)
{
	m_override_alpha.push(value);
}

void c_render_manager::pop_override_alpha()
{
	m_override_alpha.pop();
}

void c_render_manager::push_override_buffer(int index)
{
	m_render_foreground = true;
	m_current_index_buffer = index;
	m_buffer = m_manager->get_buffer(m_current_index_buffer);
}

void c_render_manager::pop_override_buffer(int value)
{
	m_current_index_buffer = value;
	m_render_foreground = false;
	m_buffer = m_manager->get_buffer(m_current_index_buffer);
}

int c_render_manager::create_texture(std::vector<render_texture_raw> tex_raws)
{
	int weight = 0;
	int height = 0;
	int n = 0;

	if (tex_raws.empty())
		return -1;
	L_PRINT(LOG_WARNING) << "NOT SUPPOSED TO BE HERE CREATETEXTURE STB";
	//stbi_set_flip_vertically_on_load(0);
	//stbi_set_flip_vertically_on_load_thread(0);
	auto image_data = tex_raws.data();//stbi_load_from_memory(tex_raws.data(), tex_raws.size(), &weight, &height, &n, 4);

	if (!image_data)
		return -1;

	const auto tex = g_render->get_manager()->create_texture(weight, height);
	g_render->get_manager()->set_texture_rgba(tex, reinterpret_cast<const uint8_t*>(image_data), weight, height);

	auto& tex_ = g_render->texture.emplace_back();

	tex_.tex_id = tex;
	tex_.width = weight;
	tex_.height = height;

	//stbi_image_free(image_data);

	return texture.size() - 1;
}

int c_render_manager::create_texture_rgba(std::vector<render_texture_raw> tex_raws, int weight, int height)
{
	const auto tex = g_render->get_manager()->create_texture(weight, height);
	g_render->get_manager()->set_texture_rgba(tex, reinterpret_cast<const uint8_t*>(tex_raws.data()), weight, height);

	auto& tex_ = texture.emplace_back();

	tex_.tex_id = tex;
	tex_.width = weight;
	tex_.height = height;
	return texture.size() - 1;
}

int c_render_manager::create_texture_imgui_rgba(std::vector<render_texture_raw> tex_raws, int weight, int height)
{
	 ID3D11ShaderResourceView* tex = g_render->get_manager()->get_texture(weight, height);
   g_render->get_manager()->set_texture_rgba(tex, reinterpret_cast<const uint8_t*>(tex_raws.data()), weight, height);

	auto& tex_ = texture.emplace_back();
	L_PRINT(LOG_INFO) << "size of tex: " << sizeof(tex);
	tex_.tex_id = tex;
	tex_.width = weight;
	tex_.height = height;
	return texture.size() - 1;
} 

void c_render_manager::add_text(const std::string str, render_position position, render_color color, render_font* font, float font_size)
{
	if (m_reset)
	{
		m_buffer = m_manager->get_buffer(m_current_index_buffer);
		set_reset_buffer(false);
	}

	m_buffer->text(font, font_size * m_dpi, str.c_str(), position * m_dpi, color.override_alpha(get_override_alpha()));
}

void arc_path(render_position pos, float start, float degress, float radius, render_color color, std::vector<render_position>& path)
{
	calc_arc_endpoints(pos, start, start + degress, radius, path);
}

void rounded(render_position position, render_size size, int radius, render_color Color, int type = 0)
{
	std::vector<render_position> path;

	if (type & render_rounding_flags_::ROUND_RECT_TL)
	{
		arc_path(position + render_position(radius * 2.f, radius), 180, 90, radius, render_color::white(), path);
	}
	else
	{
		path.emplace_back(position + render_position(0, radius));
		path.emplace_back(position + render_position(0, 0));
		path.emplace_back(position + render_position(radius * 2.f, 0));
	}

	if (type & render_rounding_flags_::ROUND_RECT_TR)
	{
		arc_path(render_position(position.x + size.x - radius, position.y + radius * 2.f), 270, 90, radius, render_color::white(), path);
	}
	else
	{
		path.emplace_back(render_position(position.x + size.x - radius * 2.f, position.y));
		path.emplace_back(render_position(position.x + size.x, position.y));
		path.emplace_back(render_position(position.x + size.x, position.y + radius));
	}

	if (type & render_rounding_flags_::ROUND_RECT_BR)
	{
		arc_path(render_position(position.x + size.x - radius * 2.f, position.y + size.y - radius), 0, 90, radius, render_color::white(), path);
	}
	else
	{
		path.emplace_back(render_position(position.x + size.x, position.y + size.y - radius));
		path.emplace_back(render_position(position.x + size.x, position.y + size.y));
		path.emplace_back(render_position(position.x + size.x - radius * 2.f, position.y + size.y));
	}

	if (type & render_rounding_flags_::ROUND_RECT_BL)
	{
		arc_path(render_position(position.x + radius, position.y + size.y - (radius * 2.f)), 90, 90, radius, render_color::white(), path);
	}
	else
	{
		path.emplace_back(render_position(position.x + radius * 2.f, position.y + size.y));
		path.emplace_back(render_position(position.x, position.y + size.y));
		path.emplace_back(render_position(position.x, position.y + size.y - radius));
	}

	path.emplace_back(position + render_position(0, radius));

	g_render->get_buffer()->poly_fill(path.data(), path.size(), Color);
	g_render->get_buffer()->poly_line(path.data(), path.size(), Color, 1.f, true);

}

void c_render_manager::line(render_position pos, render_position end, render_color color)
{
	pos *= g_render->get_dpi();
	end *= g_render->get_dpi();
	g_render->get_buffer()->line(pos, end, color.override_alpha(get_override_alpha()), 1.f);
}

void c_render_manager::line(render_position pos, render_position end, render_color color, render_color color_end)
{
	pos *= g_render->get_dpi();
	end *= g_render->get_dpi();
	g_render->get_buffer()->line(pos, end, color.override_alpha(get_override_alpha()), color_end.override_alpha(get_override_alpha()), 1.f, true);
}


void c_render_manager::circle_3d(const render_position_3d& origin, float radius, render_color color)
{

	float step = (float)3.14f * 2.0f / 50.f;

	for (float a = 0; a < (3.14f * 2.0f); a += step)
	{
		Vector_t start(radius * cosf(a) + origin.x, radius * sinf(a) + origin.y, origin.z);
		Vector_t end(radius * cosf(a + step) + origin.x, radius * sinf(a + step) + origin.y, origin.z);

		ImVec2 start2d, end2d;
		if (!math::WorldToScreen(start, start2d) || !math::WorldToScreen(end, end2d))
			continue;

		line(render_position(start2d.x, start2d.y), render_position(end2d.x, end2d.y), color);
	}
}

bool c_render_manager::circle_filled_3d(const render_position_3d& origin, float radius, render_color color)
{

	std::vector<render_position> vert{};

	float step = (float)3.14f * 2.0f / 50.f;

	for (float a = 0; a < (3.14f * 2.0f); a += step)
	{
		Vector_t start(radius * cosf(a) + origin.x, radius * sinf(a) + origin.y, origin.z);
		Vector_t end(radius * cosf(a + step) + origin.x, radius * sinf(a + step) + origin.y, origin.z);

		ImVec2 start2d, end2d;
		if (!math::WorldToScreen(start, start2d) || !math::WorldToScreen(end, end2d))
			continue;

		vert.emplace_back(render_position(start2d.x, start2d.y));
		vert.emplace_back(render_position(end2d.x, end2d.y));
	}

	if (vert.size() > 3)
		g_render->get_buffer()->poly_fill(vert.data(), vert.size(), color);
		
	return true;
}

void c_render_manager::rect_filled(render_position position, render_size size, render_color color, render_flags flags, render_rounding rounding, render_rounding_flags rounding_flags)
{
	if (m_reset)
	{
		m_buffer = m_manager->get_buffer(m_current_index_buffer);
		set_reset_buffer(false);
	}

	if ((flags & render_flags_::blur) && get_override_alpha() > 0.99f)
		m_buffer->set_blur(50, 1);

	if((flags & render_flags_::clip))
	m_buffer->push_clip_rect({ position * m_dpi, position * m_dpi + (size * m_dpi) });

	if (rounding < 1.f)
		m_buffer->rectangle_filled(position * m_dpi, position * m_dpi + (size * m_dpi), color.override_alpha(get_override_alpha()));
	else
		rounded(position * m_dpi, size * m_dpi, rounding, color.override_alpha(get_override_alpha()), rounding_flags);

	if ((flags & render_flags_::clip))
		m_buffer->pop_clip_rect();

	if (flags & render_flags_::blur)
		m_buffer->set_blur(0);
}

void c_render_manager::rect_filled_multi(render_position position, render_size size, render_color_multi color, render_rounding rounding, render_rounding_flags rounding_flags)
{
	if (m_reset)
	{
		m_buffer = m_manager->get_buffer(m_current_index_buffer);
		set_reset_buffer(false);
	}

	if (rounding < 1.f)
		return m_buffer->rectangle_filled(position * m_dpi, position * m_dpi + size * m_dpi, color.left_up_.override_alpha(get_override_alpha()), color.right_up_.override_alpha(get_override_alpha()), color.left_down_.override_alpha(get_override_alpha()), color.right_down_.override_alpha(get_override_alpha()));

	dx11::rectangle_filled_rounded(m_buffer, position * m_dpi, position * m_dpi + size * m_dpi, rounding, color.left_up_.override_alpha(get_override_alpha()), color.right_up_.override_alpha(get_override_alpha()), color.left_down_.override_alpha(get_override_alpha()), color.right_down_.override_alpha(get_override_alpha()), rounding_flags);
}

void c_render_manager::rect(render_position position, render_size size, render_color color)
{
	if (m_reset)
	{
		m_buffer = m_manager->get_buffer(m_current_index_buffer);
		set_reset_buffer(false);
	}

	m_buffer->rectangle(position * m_dpi, position * m_dpi + size * m_dpi, 1.f, color.override_alpha(get_override_alpha()));
}

void c_render_manager::add_texture(render_position position, render_size size, render_texture_index m_index_tex, render_color color)
{
	if (m_reset)
	{
		m_buffer = m_manager->get_buffer(m_current_index_buffer);
		set_reset_buffer(false);
	}

	m_buffer->push_tex_id(get_texture(m_index_tex).tex_id);
	m_buffer->prim_reserve(6, 4);
	m_buffer->prim_rect_uv(position * m_dpi, position * m_dpi + render_size(std::min(size.x * m_dpi, get_texture(m_index_tex).width * m_dpi), std::min(size.y * m_dpi, get_texture(m_index_tex).height * m_dpi)), { 0, 0 }, { 1, 1 }, color.override_alpha(get_override_alpha()));
	m_buffer->pop_tex_id();
}

void c_render_manager::add_texture_external(render_position position, render_size size, ID3D11ShaderResourceView* resource_image)
{
	if (m_reset)
	{
		m_buffer = m_manager->get_buffer(m_current_index_buffer);
		set_reset_buffer(false);
	}

	m_buffer->push_tex_id(resource_image);
	m_buffer->prim_reserve(6, 4);
	m_buffer->prim_rect_uv(position, position + size * m_dpi, { 0, 0 }, { 1, 1 }, math::color_rgba::white().override_alpha(get_override_alpha()));
	m_buffer->pop_tex_id();
}

void c_render_manager::add_texture_circle(render_position position, render_radius radius, render_texture_index m_index_tex, bool outline, render_color outline_color)
{
	if (m_reset)
	{
		m_buffer = m_manager->get_buffer(m_current_index_buffer);
		set_reset_buffer(false);
	}

	if (outline)
		circle_filled(position + render_position(radius, radius), radius, outline_color);

	m_buffer->push_clip_rect(position * m_dpi, position * m_dpi + render_size(radius * 2, radius * 2) * m_dpi, true);
	add_texture(position, render_size(radius * 2, radius * 2), m_index_tex);
	m_buffer->pop_clip_rect();
}

void c_render_manager::circle_filled(render_position center, render_radius radius, render_color color)
{
	if (m_reset)
	{
		m_buffer = m_manager->get_buffer(m_current_index_buffer);
		set_reset_buffer(false);
	}

	g_render->get_buffer()->circle_filled(center * m_dpi, radius * m_dpi, color.override_alpha(get_override_alpha()));
}

void c_render_manager::set_limit(render_position pos, render_size size) {

	if (m_reset)
	{
		m_buffer = m_manager->get_buffer(m_current_index_buffer);
		set_reset_buffer(false);
	}

	pos *= m_dpi;
	size *= m_dpi;
	g_render->get_buffer()->push_clip_rect({ pos, pos + size });
}

void c_render_manager::pop_limit() {

	if (m_reset)
	{
		m_buffer = m_manager->get_buffer(m_current_index_buffer);
		set_reset_buffer(false);
	}

	g_render->get_buffer()->pop_clip_rect();
}