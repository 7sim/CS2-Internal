#pragma once

#include <d3d11.h>
#include <memory>
#include <stack>

#include "draw_manager.hpp"
#include "d3d11_manager.hpp"
#include "fonts/inter.h"
#include "images/images.h"

//forward class
class c_render_texture;

//using
using render_buffer = dx11::draw_buffer;
using render_manager = std::unique_ptr<dx11::d3d11_manager>;
using render_position = dx11::position;
using render_position_3d = math::vec3f;
using render_size = dx11::position;
using render_color = dx11::color;
using render_font = dx11::font;
using render_index = size_t;
using render_device = ID3D11Device;
using render_device_context = ID3D11DeviceContext;
using render_reset_buffer = bool;
using render_flags = int;
using render_fonts = std::vector<dx11::font*>;
using render_font_index = size_t;
using render_rounding = float;
using render_rounding_flags = size_t;
using render_rounding_flags_ = ROUND_RECT_FLAG;
using render_texture = dx11::tex_id;
using render_texture_width = float;
using render_texture_height = float;
using render_textures = std::vector<c_render_texture>;
using render_texture_index = size_t;
using render_texture_raw = std::uint8_t;
using render_override_alpha = std::stack<float>;
using render_radius = float;
using render_foreground_force = bool;
using render_dpi = float;

class c_render_texture
{
public:
	render_texture tex_id;
	render_texture_width width;
	render_texture_height height;
};

class render_color_multi
{
public:
	render_color_multi(const render_color color_left_up, const render_color color_left_down, const render_color color_right_up, const render_color color_right_down) :
		left_up_{ color_left_up }, left_down_{ color_left_down }, right_up_{ color_right_up }, right_down_{ color_right_down } {};

	render_color left_up_;
	render_color left_down_;
	render_color right_up_;
	render_color right_down_;
};

class render_timer {
	double m_current_time = 0;
	double m_frequency = 0;
	double m_previous_time = 0;
	float m_delta = 0;
public:
	float get_delta() const { return m_delta; }

	void tick() {
		LARGE_INTEGER counter{}, frequency{};
		QueryPerformanceCounter(&counter);
		QueryPerformanceFrequency(&frequency);
		m_current_time = counter.QuadPart;
		m_frequency = frequency.QuadPart;
		if (m_previous_time)
			m_delta = (m_current_time - m_previous_time) / m_frequency;
		m_previous_time = m_current_time;
	}
};

class c_render_manager
{
public:
	enum render_flags_ : int
	{
		none,
		blur,
		clip
	};
public:
	c_render_manager() = default;
	c_render_manager(render_device* device, render_device_context* context, render_size screen_size, render_index max_layers) : m_screen_size{ screen_size }, m_max_layers{ max_layers }
	{
		create_manager(device, context);
		m_manager->update_screen_size(screen_size);

		for (auto i = 1; i < m_max_layers; i++)
			m_manager->register_buffer(i);

		m_current_index_buffer = 0;

		if (m_reset)
		{
			m_buffer = m_manager->get_buffer(m_current_index_buffer);
			set_reset_buffer(false);
		}
	}
private:
	void create_manager(ID3D11Device* device, ID3D11DeviceContext* context)
	{
		m_manager = std::make_unique<dx11::d3d11_manager>(device, context);
	}
public:
	dx11::d3d11_manager* get_manager()
	{
		return m_manager.get();
	}

	float get_dpi() {
		return m_dpi;
	}

	render_buffer* get_buffer()
	{
		if (m_reset || m_render_foreground)
		{
			m_buffer = m_manager->get_buffer(m_current_index_buffer);
			set_reset_buffer(false);
		}

		return  m_buffer;
	}

	render_size get_screen_size() const
	{
		return m_screen_size;
	}

	void update_screen_size(const render_size new_size)
	{
		m_screen_size = new_size;
		m_manager->update_screen_size(m_screen_size);
	}

	render_index get_render_index() const
	{
		return m_current_index_buffer;
	}

	void begin()
	{
		set_reset_buffer(true);
	}

	void end()
	{
		for (int i = 0; i < m_max_layers - 1; i++)
			m_manager->swap_buffers(i);
	}

	void push_font(dx11::font* font)
	{
		fonts.push_back(font);
	}

	render_font* get_font(const render_font_index m_index_font)
	{
		if (m_index_font >= texture.size())
			return nullptr;

		return fonts[m_index_font];
	}

	c_render_texture get_texture(render_texture_index m_index_tex)
	{
		if (m_index_tex >= texture.size())
			return {};

		return texture[m_index_tex];
	}

	float get_delta_render() const
	{
		return m_render_timer.get_delta();
	}

private:
	void set_reset_buffer(render_reset_buffer value)
	{
		m_reset = value;
	}
public:
	int create_texture(std::vector<render_texture_raw> tex_raws);
	int create_texture_imgui_rgba(std::vector<render_texture_raw> tex_raws, int weight, int height);
	int create_texture_rgba(std::vector<render_texture_raw> tex_raws, int weight, int height);
	void add_text(const std::string str, render_position position, render_color color, render_font* font, float font_size = 14);
	void rect_filled(render_position position, render_size size, render_color color, render_flags flags = 0, render_rounding rounding = 0.f, render_rounding_flags rounding_flags = 15);
	void rect_filled_multi(render_position position, render_size size, render_color_multi color, render_rounding rounding = 0.f, render_rounding_flags rounding_flags = 15);
	void rect(render_position position, render_size size, render_color color);
	void add_texture(render_position position, render_size size, render_texture_index m_index_tex, render_color color = render_color::white());
	void add_texture_external(render_position position, render_size size, ID3D11ShaderResourceView* resource_image);
	void add_texture_circle(render_position position, render_radius radius, render_texture_index m_index_tex, bool outline = false, render_color outline_color = render_color::white());
	void circle_filled(render_position center, render_radius radius, render_color color);
	void circle_3d(const render_position_3d& origin, float radius, render_color color);
	bool circle_filled_3d(const render_position_3d& origin, float radius, render_color color);
	void line(render_position pos, render_position end, render_color color);
	void line(render_position pos, render_position end, render_color color, render_color color_end);
	void arc_line(render_position pos, float start, float degress, float radius, render_color color, float thikness = 1.f);
	void arc_fill(render_position pos, float start, float degress, float radius, render_color color);
	void set_limit(render_position pos, render_size size);
	void pop_limit();
public:
	void push_override_alpha(float value);
	void pop_override_alpha();
	void push_dpi_scale(float value) {
		m_dpi = value;
	}
	void pop_dpi_scale() {
		m_dpi = 1.f;
	}
	float get_override_alpha()
	{
		float override_alpha = 1.f;

		if (!m_override_alpha.empty())
		{
			for (auto value : m_override_alpha._Get_container())
			{
				override_alpha *= value;
			}
		}

		return override_alpha;
	}
	void push_override_buffer(int value);
	void pop_override_buffer(int value = 0);
	bool has_override_buffer()
	{
		return m_current_index_buffer >= 2;
	}
private:
	render_index m_current_index_buffer = -1;
	render_index m_max_layers = -1;
	render_buffer* m_buffer = nullptr;
	render_manager m_manager = nullptr;
	render_size m_screen_size = render_size();
	render_reset_buffer m_reset = true;
public:
	render_fonts fonts;
private:
	render_textures texture;
	render_override_alpha m_override_alpha;
	render_foreground_force m_render_foreground = false;
	render_dpi m_dpi;
public:
	render_timer m_render_timer;
public:
	std::mutex m_mutex;
};

inline c_render_manager* g_render = nullptr;