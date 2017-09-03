#pragma once
#include "augs/misc/machine_entropy.h"
#include "augs/gui/formatted_string.h"

#include "game/hardcoded_content/requisite_collections.h"

#include "application/gui/ingame_menu_button_type.h"
#include "application/gui/menu/menu_context.h"

using ingame_menu_context = menu_context<false, ingame_menu_button_type>;
using const_ingame_menu_context = menu_context<true, ingame_menu_button_type>;
using viewing_ingame_menu_context = viewing_menu_context<ingame_menu_button_type>;

struct ingame_menu_gui {
	bool show = false;

	ingame_menu_context::root_type root;
	ingame_menu_context::rect_world_type world;

	struct input {
		augs::local_entropy& local;
		const augs::drawer_with_default output;
		const vec2i screen_size;
		const augs::gui::text::style text_style;
		const requisite_images_in_atlas& requisite_images;
		const sound_buffer_collection& sounds;
		const augs::delta vdt;
		const augs::audio_volume_settings& audio_volume;
	};

	template <class B>
	auto perform(
		const input in,
		B button_callback
	) {
		ensure(show);

		for (std::size_t i = 0; i < root.buttons.size(); ++i) {
			const auto e = static_cast<ingame_menu_button_type>(i);
			root.buttons[i].set_complete_caption(to_wstring(format_enum(e)));
		}

		const auto& gui_font = in.text_style.get_font();

		in.output.color_overlay(in.screen_size, rgba{ 0, 0, 0, 140 });

		auto tree = ingame_menu_context::tree_type();
		auto context = ingame_menu_context(
			{ world, tree, in.screen_size },
			root,
			in.requisite_images,
			gui_font,
			in.sounds,
			in.audio_volume
		);

		root.set_menu_buttons_sizes(in.requisite_images, gui_font, { 1000, 1000 });

		world.build_tree_data_into(context);

		const auto gui_entropies = augs::consume_inputs_with_imgui_precedence(context, in.local);

		world.respond_to_events(context, gui_entropies);
		world.advance_elements(context, in.vdt);

		root.set_menu_buttons_colors(cyan);
		world.rebuild_layouts(context);

		const auto viewing_context = viewing_ingame_menu_context(const_ingame_menu_context(context), in.output);

		root.draw_background_behind_buttons(viewing_context);
		world.draw(viewing_context);

		augs::for_each_enum_except_bounds<ingame_menu_button_type>([&](const ingame_menu_button_type t) {
			if (root.buttons[t].click_callback_required) {
				root.buttons[t].click_callback_required = false;

				button_callback(t);
			}
		});

		/* Some callback might have hidden the menu */
		const bool wont_receive_events_anymore = !show;

		if (wont_receive_events_anymore) {
			/* We must manually unhover "Resume" button */
			ingame_menu_context::rect_world_type::gui_entropy gui_entropies;

			world.unhover_and_undrag(context, gui_entropies);
			world.respond_to_events(context, gui_entropies);
		}
		
		auto determined_cursor = assets::requisite_image_id::GUI_CURSOR;

		if (context.alive(world.rect_hovered)) {
			determined_cursor = assets::requisite_image_id::GUI_CURSOR_HOVER;
		}

		return determined_cursor;
	}
};
