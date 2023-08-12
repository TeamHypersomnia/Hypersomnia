#include "augs/misc/randomization.h"
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

	const auto total_spacing = this_id->border.get_total_expansion();

	{
		const auto full_bar_rect_bordered = get_bar_rect_with_borders(context, this_id, absolute);
		const auto value_bar_rect = get_value_bar_rect(context, this_id, absolute);

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

		auto current_value_bar_rect = value_bar_rect;

		const auto bar_width = std::max(0, static_cast<int>(current_value_bar_rect.w() * current_value_ratio));

		current_value_bar_rect.w(static_cast<float>(bar_width));

		output.aabb(current_value_bar_rect, bar_col);
		output.border(value_bar_rect, bar_col, this_id->border);

		/* Draw the value label if it is a meter, draw nothing if perk */

		visit_by_vertical_index(sentience, cosm, vertical_index,
			[&](const auto& meter, auto) {
				const auto value = meter.get_value();

				print_stroked(
					output,
					vec2{ full_bar_rect_bordered.r + total_spacing * 2, full_bar_rect_bordered.t - total_spacing },
					{ typesafe_sprintf("%x", static_cast<int>(value)),{ context.get_gui_font(), white } }
				);
			},

			[](auto...) { return; }
		);

		if (/* should_draw_particles */ bar_width >= 1) {
			for (const auto& p : this_id->particles) {
				const auto particle_col = bar_col + rgba(30, 30, 30, 0);
			
				output.aabb_lt_clipped(
					necessarys.at(p.image_id),
					value_bar_rect.get_position() - vec2(6, 6) + p.relative_pos,
					current_value_bar_rect,
					particle_col
				);
			}
		}
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

	const auto max_value_caption_size = get_text_bbox({ "99999", context.get_gui_font() });

	auto value_bar_rect = icon_rect;
	value_bar_rect.set_position(icon_rect.get_position() + vec2(this_id->border.get_total_expansion() + icon_rect.get_size().x, 0));
	value_bar_rect.r = absolute.r - max_value_caption_size.x;

	return value_bar_rect;
}

ltrb value_bar::get_value_bar_rect(
	const const_game_gui_context context,
	const const_this_pointer this_id,
	const ltrb absolute
) {
	const auto border_expansion = this_id->border.get_total_expansion();
	return get_bar_rect_with_borders(context, this_id, absolute).expand_from_center({ static_cast<float>(-border_expansion), static_cast<float>(-border_expansion) });
}

void value_bar::advance_elements(
	const game_gui_context context,
	const this_pointer this_id,
	const augs::delta dt
) {
	this_id->seconds_accumulated += dt.in_seconds();

	if (this_id->particles.size() > 0) {
		randomization rng{ static_cast<rng_seed_type>(this_id.get_location().vertical_index + context.get_cosmos().get_total_seconds_passed() * 1000) };

		const auto value_bar_size = get_value_bar_rect(context, this_id, this_id->rc).get_size();

		while (this_id->seconds_accumulated > 0.f) {
			for (auto& p : this_id->particles) {
				const auto action = rng.randval(0, 8);

				if (action == 0) {
					const auto y_dir = rng.randval(0, 1);

					if (y_dir == 0) {
						++p.relative_pos.y;
					}
					else {
						--p.relative_pos.y;
					}
				}
				
				++p.relative_pos.x;

				p.relative_pos.y = std::max(0, p.relative_pos.y);
				p.relative_pos.x = std::max(0, p.relative_pos.x);

				p.relative_pos.x %= static_cast<int>(value_bar_size.x + 12);
				p.relative_pos.y %= static_cast<int>(value_bar_size.y + 12);
			}

			this_id->seconds_accumulated -= 1.f / 15;
		}
	}
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

	const auto screen_size = context.get_screen_size();
	const auto icon_size = context.get_game_images().at(get_bar_icon(context, this_id)).get_original_size();
	const auto with_bar_size = vec2i(icon_size.x + 4 + 180, icon_size.y);

	const auto lt = vec2i(screen_size.x - 220, 20 + drawing_vertical_index * (icon_size.y + 4));

	auto& rc = this_id->rc;
	rc.set_position(lt);
	rc.set_size(with_bar_size);

	thread_local randomization rng;

	if (this_id->particles.empty()) {
		const auto value_bar_size = get_value_bar_rect(context, this_id, rc).get_size();

		constexpr auto num_particles_to_spawn = 40u;

		for (size_t i = 0; i < num_particles_to_spawn; ++i) {
			const auto mats = std::array<assets::necessary_image_id, 3> {
				assets::necessary_image_id::WANDERING_CROSS,
				assets::necessary_image_id::BLINK_1,
				static_cast<assets::necessary_image_id>(static_cast<int>(assets::necessary_image_id::BLINK_1) + 2),
			};

			effect_particle new_part;
			new_part.relative_pos = rng.randval(vec2(0, 0), value_bar_size);
			new_part.image_id = rng.choose_from(mats);

			this_id->particles.push_back(new_part);
		}
	}
}