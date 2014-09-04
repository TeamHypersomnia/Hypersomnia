#include "stdafx.h"
#pragma once
#include "hypersomnia_gui.h"
#include "window_framework/window.h"

#include "game_framework/systems/render_system.h"


hypersomnia_gui::hypersomnia_gui(glwindow& gl, texture_baker::texture* blank_texture) : sys(gl.events), gl(gl) {
	graphics::gui::null_texture = blank_texture;

	//wchar_t* str = L" qvxQVXa¹bcædeêfghijkl³mnñoóprsœtuwyzŸ¿A¥BCÆDEÊFGHIJKL£MNÑOÓPRSŒTUWYZ¯0123456789.!@#$%^&*()_+-=[];'\\,./{}:\"|<>?";
	//
	//fontf[0].open(fin, "hypersomnia/data/Kubasta.ttf", 16, str);

	graphics::gui::null_texture->translate_uv(vec2<>(2, 2));
	graphics::gui::null_texture->scale_uv(0.000000001f, 0.000000001f);

	ltblue_theme();

	//main_window.middlescroll.mat = material(textures + 4, pixel_32(255, 255, 255, 180));
}

void hypersomnia_gui::update() {
	augs::graphics::fps.sumframes = delta_timer.extract<std::chrono::seconds>();
}

gui_group::gui_group(hypersomnia_gui& owner) : main_window(owner.sys) {
	main_window.middlescroll.speed_mult = 90.0f;
}

void gui_group::poll_events() {
	main_window.update_rectangles();
	main_window.poll_events();
}

void gui_group::draw_call(resources::renderable::draw_input& in) {
	main_window.default_update();

	in.output->triangles.insert(in.output->triangles.end(), main_window.quad_array.begin(), main_window.quad_array.end());
}