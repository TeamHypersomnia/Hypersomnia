#pragma once
#include "augs/misc/machine_entropy.h"
#include "augs/gui/formatted_string.h"
#include "augs/templates/enum_introspect.h"
#include "augs/string/format_enum.h"

#include "view/necessary_resources.h"

#include "application/gui/ingame_menu_button_type.h"
#include "application/gui/menu/menu_context.h"

using ingame_menu_context = menu_context<false, ingame_menu_button_type>;
using const_ingame_menu_context = menu_context<true, ingame_menu_button_type>;
using viewing_ingame_menu_context = viewing_menu_context<ingame_menu_button_type>;

struct ingame_menu_gui {
	bool show = false;

	ingame_menu_context::tree_type tree;
	ingame_menu_context::root_type root;
	ingame_menu_context::rect_world_type world;

	auto create_context(
		const vec2i screen_size,
		const augs::event::state input_state,
		const menu_context_dependencies deps
	) {
		return ingame_menu_context {
			{ world, tree, screen_size, input_state },
			root,
			deps
		};
	}

	template <class B>
	auto control(
		const ingame_menu_context context,
		const augs::event::change change,
		B button_callback
	) {
		/*
			It's useful during development but can really easily cause me to exit the game mid-match
		*/

#if !IS_PRODUCTION_BUILD
		if (change.was_pressed(augs::event::keys::key::Q)) {
			button_callback(ingame_menu_button_type::QUIT_TO_MENU);
			return true;
		}
#endif

		if (change.was_pressed(augs::event::keys::key::R)) {
			button_callback(ingame_menu_button_type::RESUME);
			return true;
		}

		if (change.was_pressed(augs::event::keys::key::S)) {
			button_callback(ingame_menu_button_type::SETTINGS);
			return true;
		}

		if (change.was_pressed(augs::event::keys::key::B)) {
			button_callback(ingame_menu_button_type::BROWSE_SERVERS);
			return true;
		}

		if (change.was_pressed(augs::event::keys::key::C)) {
			button_callback(ingame_menu_button_type::SERVER_DETAILS);
			return true;
		}

		const auto gui_entropies = 
			world.consume_raw_input_and_generate_gui_events(
				context, 
				change
			)
		;

		world.respond_to_events(context, gui_entropies);

		augs::for_each_enum_except_bounds([&](const ingame_menu_button_type t) {
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
		const ingame_menu_context context,
		const augs::delta vdt
	) {
		const auto& gui_font = context.get_gui_font();

		root.set_menu_buttons_colors(cyan);
		root.set_menu_buttons_sizes(context.get_necessary_images(), gui_font, { 1000, 1000 });

		for (std::size_t i = 0; i < root.buttons.size(); ++i) {
			const auto e = static_cast<ingame_menu_button_type>(i);
			root.buttons[i].set_complete_caption(format_enum(e));

			if (e == ingame_menu_button_type::QUIT_TO_MENU) {
				if (context.deps.will_quit_to_editor) {
					root.buttons[i].set_complete_caption("Quit to Editor");
				}
				else if (context.deps.will_quit_to_projects) {
					root.buttons[i].set_complete_caption("Quit to Projects");
				}
			}

			if (e == ingame_menu_button_type::SERVER_DETAILS) {
				if (context.deps.is_tutorial) {
					root.buttons[i].set_complete_caption("Restart from last checkpoint");
				}
			}
		}

		world.advance_elements(context, vdt);
		world.rebuild_layouts(context);
		world.build_tree_data_into(context);
	}

	auto draw(const viewing_ingame_menu_context context) const {
		if (!show) {
			return assets::necessary_image_id::INVALID;
		}

		const auto output = context.get_output();
		const auto screen_size = context.get_screen_size();

		output.color_overlay(screen_size, rgba{ 0, 0, 0, 140 });

		root.draw_background_behind_buttons(context);
		world.draw(context);

		auto determined_cursor = assets::necessary_image_id::GUI_CURSOR;

		if (context.alive(world.rect_hovered)) {
			determined_cursor = assets::necessary_image_id::GUI_CURSOR_HOVER;
		}

		return determined_cursor;
	}
};
