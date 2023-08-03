#pragma once
#include "augs/misc/machine_entropy.h"
#include "augs/gui/formatted_string.h"
#include "augs/string/format_enum.h"
#include "augs/templates/enum_introspect.h"

#include "view/necessary_resources.h"

#include "application/gui/main_menu_button_type.h"
#include "application/gui/menu/menu_context.h"

using main_menu_context = menu_context<false, main_menu_button_type>;
using const_main_menu_context = menu_context<true, main_menu_button_type>;
using viewing_main_menu_context = viewing_menu_context<main_menu_button_type>;

struct main_menu_gui {
	bool show = true;

	main_menu_context::tree_type tree;
	main_menu_context::root_type root;
	main_menu_context::rect_world_type world;

	main_menu_gui() {
		for (auto& m : root.buttons) {
			m.hover_highlight_maximum_distance = 10.f;
			m.hover_highlight_duration_ms = 300.f;
		}
	}

	auto create_context(
		const vec2i screen_size,
		const augs::event::state input_state,
		const menu_context_dependencies deps
	) {
		return main_menu_context {
			{ world, tree, screen_size, input_state },
			root,
			deps
		};
	}

	template <class B>
	auto control(
		const main_menu_context context,
		const augs::event::change change,
		B button_callback
	) {
		if (change.was_pressed(augs::event::keys::key::D)) {
			button_callback(main_menu_button_type::DOWNLOAD_MAPS);
			return true;
		}

		if (change.was_pressed(augs::event::keys::key::E)) {
			button_callback(main_menu_button_type::EDITOR);
			return true;
		}

		if (change.was_pressed(augs::event::keys::key::L)) {
			button_callback(main_menu_button_type::SHOOTING_RANGE);
			return true;
		}

		if (change.was_pressed(augs::event::keys::key::T)) {
			button_callback(main_menu_button_type::TUTORIAL);
			return true;
		}

		if (change.was_pressed(augs::event::keys::key::S)) {
			button_callback(main_menu_button_type::SETTINGS);
			return true;
		}

		if (change.was_pressed(augs::event::keys::key::C)) {
			button_callback(main_menu_button_type::PLAY_ON_THE_OFFICIAL_SERVER);
			return true;
		}

		if (change.was_pressed(augs::event::keys::key::U)) {
			button_callback(main_menu_button_type::CONNECT_TO_SERVER);
			return true;
		}

		if (change.was_pressed(augs::event::keys::key::B)) {
			button_callback(main_menu_button_type::BROWSE_SERVERS);
			return true;
		}

		if (change.was_pressed(augs::event::keys::key::H)) {
			button_callback(main_menu_button_type::HOST_SERVER);
			return true;
		}

		if (change.was_pressed(augs::event::keys::key::Q)) {
			button_callback(main_menu_button_type::QUIT);
			return true;
		}

		const auto gui_entropies = 
			world.consume_raw_input_and_generate_gui_events(
				context, 
				change
			)
		;

		world.respond_to_events(context, gui_entropies);

		augs::for_each_enum_except_bounds([&](const main_menu_button_type t) {
			if (root.buttons[t].click_callback_required) {
				root.buttons[t].click_callback_required = false;

				button_callback(t);
			}
		});

		if (!gui_entropies.empty()) {
			return true;
		}

		return false;
	}

	void advance(
		const main_menu_context context,
		const augs::delta vdt
	) {
		const auto& gui_font = context.get_gui_font();

		root.set_menu_buttons_colors(cyan);
		root.buttons[std::size_t(main_menu_button_type::DOWNLOAD_MAPS)].colorize = green;
		root.set_menu_buttons_sizes(context.get_necessary_images(), gui_font, { 1000, 1000 });

		for (std::size_t i = 0; i < root.buttons.size(); ++i) {
			const auto e = static_cast<main_menu_button_type>(i);
			root.buttons[i].set_complete_caption(format_enum(e));
		}

		root.buttons[0].is_discord = true;

		world.advance_elements(context, vdt);
		world.rebuild_layouts(context);
		world.build_tree_data_into(context);
	}

	auto draw(const viewing_main_menu_context context) const {
		if (!show) {
			return assets::necessary_image_id::INVALID;
		}

		const auto output = context.get_output();
		const auto screen_size = context.get_screen_size();

		const bool draw_overlay = false;

		if (draw_overlay) {
			output.color_overlay(screen_size, rgba{ 0, 0, 0, 140 });
		}

		root.draw_background_behind_buttons(context);
		world.draw(context);

		auto determined_cursor = assets::necessary_image_id::GUI_CURSOR;

		if (context.alive(world.rect_hovered)) {
			determined_cursor = assets::necessary_image_id::GUI_CURSOR_HOVER;
		}

		return determined_cursor;
	}
};
