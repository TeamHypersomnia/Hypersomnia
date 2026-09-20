#include <cstddef>
#include "augs/templates/get_by_dynamic_id.h"
#include "augs/gui/text/printer.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"

#include "game/components/sentience_component.h"

#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_handle.h"

#include "view/viewables/image_in_atlas.h"
#include "view/viewables/images_in_atlas_map.h"

#include "view/game_gui/elements/drag_and_drop_target_drop_item.h"
#include "view/game_gui/elements/character_gui.h"
#include "view/game_gui/elements/character_gui.h"
#include "view/game_gui/elements/game_gui_root.h"

#include "view/game_gui/game_gui_system.h"
#include "view/rendering_scripts/minimap_layout.h"
#include "3rdparty/imgui/imgui.h"
#include "augs/drawing/drawing.hpp"

using namespace augs::gui::text;

static constexpr std::size_t num_sentience_meters = num_types_in_list_v<decltype(components::sentience::meters)>;

template <class F, class G>
decltype(auto) visit_by_vertical_index(
	const components::sentience& sentience,
	const cosmos& cosm,
	const unsigned index, 
	F&& meter_callback,
	G&& perk_callback
) {
	if (index < num_sentience_meters) {
		meter_id id;
		id.set_index(index);

		return get_by_dynamic_id(sentience.meters, id, 
			[&cosm, &meter_callback](const auto& meter){
				return meter_callback(meter, get_meta_of(meter, cosm.get_common_significant().meters));
			}
		);
	}

	{
		perk_id id;
		id.set_index(index - num_sentience_meters);

		return get_by_dynamic_id(sentience.perks, id, 
			[&cosm, &perk_callback](const auto& perk){
				return perk_callback(perk, get_meta_of(perk, cosm.get_common_significant().perks));
			}
		);
	}
}

template <class F>
decltype(auto) visit_by_vertical_index(
	const components::sentience& sentience,
	const cosmos& cosm,
	const unsigned index, 
	F&& callback
) {
	return visit_by_vertical_index(
		sentience,
		cosm,
		index,
		std::forward<F>(callback),
		std::forward<F>(callback)
	);
}

bool value_bar::is_sentience_meter(const const_this_pointer this_id) {
	return this_id.get_location().vertical_index < num_sentience_meters;
}

std::string value_bar::get_description_for_hover(
	const const_game_gui_context context,
	const const_this_pointer self
) {
	const auto& cosm = context.get_cosmos();
	const auto& sentience = context.get_subject_entity().get<components::sentience>();

	return visit_by_vertical_index(
		sentience,
		cosm,
		self.get_location().vertical_index,
		
		[&](const auto& meter, const auto& meta){
			return typesafe_sprintf(meta.appearance.get_description(), meter.get_value(), meter.get_maximum_value());
		},

		[&](const auto& /* perk */, const auto& meta){
			return typesafe_sprintf(meta.appearance.get_description());
		}
	);
}

value_bar::value_bar() {
	unset_flag(augs::gui::flag::CLIP);
	set_flag(augs::gui::flag::ENABLE_DRAWING);
}

void value_bar::draw(
	const viewing_game_gui_context context, 
	const const_this_pointer this_id
) {
	const auto& cosm = context.get_cosmos();
	const auto& clk = cosm.get_clock();
	const auto& game_images = context.get_game_images();

	if (!this_id->get_flag(augs::gui::flag::ENABLE_DRAWING)) {
		return;
	}

	if (!context.dependencies.settings.draw_character_status) {
		return;
	}

	rgba icon_col = white;
	auto icon_tex = get_bar_icon(context, this_id);

	if (this_id->detector.is_hovered) {
		icon_col.a = 255;
	}
	else {
		icon_col.a = 200;
	}

	const auto& tree_entry = context.get_tree_entry(this_id);
	const auto absolute = tree_entry.get_absolute_rect();

	const auto& necessarys = context.get_necessary_images();
	const auto output = context.get_output();

	output.aabb_lt(game_images.at(icon_tex).diffuse, absolute.get_position());

	{
		const auto full_bar_rect_bordered = get_bar_rect_with_borders(context, this_id, absolute);

		auto bar_col = get_bar_col(context, this_id);
		bar_col.a = icon_col.a;

		const auto vertical_index = this_id.get_location().vertical_index;

		const auto& sentience = context.get_subject_entity().get<components::sentience>();

		const auto current_value_ratio = visit_by_vertical_index(
			sentience,
			cosm,
			vertical_index,

			[](const auto& meter, auto){
				return meter.get_ratio();
			},

			[clk](const auto& perk, auto){
				return perk.timing.get_ratio(clk);
			}
		);

		/* The same look scheme the tutorial's progress bars use. */

		auto appearance = hud_bar_appearance();
		appearance.color = bar_col;
		appearance.border_w = this_id->border.width;
		appearance.particle_tint = 0.12f;
		//appearance.splits = 3;
		//appearance.split_gap = 1;
		/*
			A plain number right of the bar, no brick underneath -
			the padding is just the gap between the bar's end and the text.
		*/

		appearance.label_align_right = true;
		appearance.label_padding = vec2i(5, 0);

		/* The value label - only meters have one, perks do not. */

		visit_by_vertical_index(sentience, cosm, vertical_index,
			[&](const auto& meter, auto) {
				appearance.label = typesafe_sprintf("%x", static_cast<int>(meter.get_value()));
				appearance.label_widest_text = typesafe_sprintf("%x", static_cast<int>(meter.get_maximum_value()));
			},

			[](auto...) { return; }
		);

		::draw_hud_bar(
			output,
			necessarys,
			appearance,
			full_bar_rect_bordered,
			current_value_ratio,
			cosm.get_total_seconds_passed(),
			context.dependencies.white_damage_highlight_secs,
			this_id->highlight,
			std::addressof(this_id->particles_state),
			std::addressof(context.get_gui_font())
		);
	}
}

ltrb value_bar::get_bar_rect_with_borders(
	const const_game_gui_context context,
	const const_this_pointer this_id,
	const ltrb absolute
) {
	auto icon_rect = absolute;

	auto icon_tex = get_bar_icon(context, this_id);
	icon_rect.set_size(context.get_game_images().at(icon_tex).get_original_size());

	/*
		The bar stretches to the row's very right edge - the value label
		is drawn centered over the bar, so no caption space is reserved.
	*/

	auto value_bar_rect = icon_rect;
	value_bar_rect.set_position(icon_rect.get_position() + vec2(this_id->border.get_total_expansion() + icon_rect.get_size().x, 0));
	value_bar_rect.r = absolute.r;

	return value_bar_rect;
}

void value_bar::advance_elements(
	const game_gui_context,
	const this_pointer,
	const augs::delta
) {
	/* The shared HUD bar drawing advances its own particles at draw time. */
}

void value_bar::respond_to_events(
	const game_gui_context, 
	const this_pointer this_id, 
	const gui_entropy& entropies
) {
	for (const auto& e : entropies.get_events_for(this_id)) {
		this_id->detector.update_appearance(e);
	}
}

assets::image_id value_bar::get_bar_icon(
	const const_game_gui_context context, 
	const const_this_pointer this_id
) {
	const auto& cosm = context.get_cosmos();
	const auto& sentience = context.get_subject_entity().get<components::sentience>();

	return visit_by_vertical_index(
		sentience,
		cosm,
		this_id.get_location().vertical_index,
		[](const auto& /* perk_or_meter */, const auto& meta){
			return meta.appearance.get_icon();
		}
	);
}

rgba value_bar::get_bar_col(
	const const_game_gui_context context, 
	const const_this_pointer this_id
) {
	rgba result;

	if (const auto sentience = context.get_subject_entity().find<components::sentience>()) {
		const auto& cosm = context.get_cosmos();

		result = 
			visit_by_vertical_index(
				*sentience,
				cosm,
				this_id.get_location().vertical_index,
				[](auto, const auto& meta){
					return meta.appearance.get_bar_color();
				}
			)
		;
	}

	return result;
}

bool value_bar::is_enabled(
	const const_game_gui_context context, 
	const unsigned vertical_index
) {
	bool result = false;

	if (const auto sentience = context.get_subject_entity().find<components::sentience>()) {
		const auto& cosm = context.get_cosmos();
		const auto& clk = cosm.get_clock();

		result =
			visit_by_vertical_index(
				context.get_subject_entity().get<components::sentience>(),
				cosm,
				vertical_index,

				[](const auto& meter, auto){
					return meter.is_enabled();
				},

				[clk](const auto& perk, auto){
					return perk.timing.is_enabled(clk);
				}
			)
		;
	}

	return result;
}

void value_bar::rebuild_layouts(
	const game_gui_context context,
	const this_pointer this_id
) {
	const auto vertical_index = this_id.get_location().vertical_index;

	if (!is_enabled(context, vertical_index)) {
		this_id->unset_flag(augs::gui::flag::ENABLE_DRAWING);
		return;
	}
	else {
		this_id->set_flag(augs::gui::flag::ENABLE_DRAWING);
	}

	unsigned drawing_vertical_index = 0;

	for (unsigned i = 0; i < vertical_index; ++i) {
		if (is_enabled(context, i)) {
			++drawing_vertical_index;
		}
	}

	unsigned total_enabled = 0;

	for (unsigned i = 0; i < value_bar_count; ++i) {
		if (is_enabled(context, i)) {
			++total_enabled;
		}
	}

	const auto screen_size = context.get_screen_size();
	const auto icon_size = context.get_game_images().at(get_bar_icon(context, this_id)).get_original_size();
	const auto with_bar_size = vec2i(icon_size.x + 4 + 180, icon_size.y);

	/*
		The bars live at the right bottom, in top-down order (health first),
		with the same bottom padding as the hotbar. When the minimap
		occupies the same corner, the bars keep that padding above it.
	*/

	const auto bottom_pad = static_cast<int>(50 * ImGui::GetTextLineHeight() / 22.0f);

	const auto rb_minimap_shift = [&]() {
		const auto& minimap = context.dependencies.drawing.minimap;

		if (minimap.occupies_corner(hud_corner_type::RIGHT_BOTTOM)) {
			return minimap.get_gameplay_appearance().size + minimap_screen_margin_v + minimap.extra_bottom_margin + minimap.extra_hud_space;
		}

		return 0;
	}();

	const auto bars_bottom = screen_size.y - bottom_pad - rb_minimap_shift;

	/*
		Pushed apart a little so that the centered value labels' backdrops
		do not overlap the neighboring rows.
	*/
	const auto row_pitch = static_cast<int>(icon_size.y) + 8;

	const auto lt = vec2i(
		screen_size.x - minimap_screen_margin_v - with_bar_size.x,
		bars_bottom - static_cast<int>(total_enabled - drawing_vertical_index) * row_pitch
	);

	auto& rc = this_id->rc;
	rc.set_position(lt);
	rc.set_size(with_bar_size);
}