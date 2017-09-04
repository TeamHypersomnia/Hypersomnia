#pragma once
#include "augs/misc/machine_entropy.h"
#include "augs/gui/formatted_string.h"

#include "view/necessary_resources.h"

#include "application/gui/main_menu_button_type.h"
#include "application/gui/menu/menu_context.h"

using main_menu_context = menu_context<false, main_menu_button_type>;
using const_main_menu_context = menu_context<true, main_menu_button_type>;
using viewing_main_menu_context = viewing_menu_context<main_menu_button_type>;

struct main_menu_gui {
	bool show = false;

	main_menu_context::root_type root;
	main_menu_context::rect_world_type world;

	struct input {
		augs::local_entropy& local;
		const augs::drawer_with_default output;
		const vec2i screen_size;
		const augs::gui::text::style text_style;
		const necessary_images_in_atlas& necessary_images;
		const necessary_sound_buffers& sounds;
		const augs::delta vdt;
		const augs::audio_volume_settings& audio_volume;
	};

	template <class B>
	auto perform(
		const input in,
		B button_callback
	) {
		if (!show) {
			return assets::necessary_image_id::INVALID;
		}

		const auto& gui_font = in.text_style.get_font();

		in.output.color_overlay(in.screen_size, rgba{ 0, 0, 0, 140 });

		auto tree = main_menu_context::tree_type();

		auto context = main_menu_context(
			{ world, tree, in.screen_size },
			root,
			in.necessary_images,
			gui_font,
			in.sounds,
			in.audio_volume
		);

		root.set_menu_buttons_sizes(in.necessary_images, gui_font, { 1000, 1000 });

		world.build_tree_data_into(context);

		const auto gui_entropies = augs::consume_inputs_with_imgui_precedence(context, in.local);

		world.respond_to_events(context, gui_entropies);
		world.advance_elements(context, in.vdt);

		root.set_menu_buttons_colors(cyan);
		world.rebuild_layouts(context);

		const auto viewing_context = viewing_main_menu_context(const_main_menu_context(context), in.output);

		root.draw_background_behind_buttons(viewing_context);
		world.draw(viewing_context);

		augs::for_each_enum_except_bounds<main_menu_button_type>([&](const main_menu_button_type t) {
			if (root.buttons[t].click_callback_required) {
				root.buttons[t].click_callback_required = false;

				button_callback(t);
			}
		});

		auto determined_cursor = assets::necessary_image_id::GUI_CURSOR;

		if (context.alive(world.rect_hovered)) {
			determined_cursor = assets::necessary_image_id::GUI_CURSOR_HOVER;
		}

		return determined_cursor;
	}
};
