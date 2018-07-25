#include "rendering_scripts.h"
#include "game/cosmos/cosmos.h"
#include "game/inferred_caches/tree_of_npo_cache.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "game/components/sentience_component.h"
#include "game/components/container_component.h"
#include "augs/string/string_templates.h"
#include "game/detail/describers.h"
#include "augs/graphics/vertex.h"
#include "augs/gui/text/printer.h"
#include "augs/image/font.h"
#include "augs/math/math.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "game/detail/visible_entities.h"

using namespace augs::gui::text;

augs::vertex_triangle_buffer draw_circular_bars_and_get_textual_info(const draw_circular_bars_input in) {
	const auto& visible_entities = in.all;
	const auto& cosmos = in.cosm;
	const auto& interp = in.interpolation;
	const auto output = in.output;
	auto& specials = in.specials;

	const auto& watched_character = cosmos[in.viewed_character_id];

	const auto timestamp_ms = static_cast<unsigned>(in.global_time_seconds * 1000);
	augs::vertex_triangle_buffer circular_bars_information;

	visible_entities.for_each<render_layer::SENTIENCES>(cosmos, [&](const auto v) {
		if (const auto* const sentience = v.template find<components::sentience>();
			sentience && sentience->is_conscious()
		) {
			const auto& health = sentience->template get<health_meter_instance>();
			const auto hr = health.get_ratio();

			const auto pulse_duration = static_cast<int>(1250 - 1000 * (1 - hr));
			const float time_pulse_ratio = (timestamp_ms % pulse_duration) / float(pulse_duration);

			const auto health_col = sentience->calc_health_color(time_pulse_ratio);

			const auto transform = v.get_viewing_transform(interp);

			output.aabb_centered(in.circular_bar_tex, (transform.pos), health_col);

			const auto watched_character_transform = watched_character.get_viewing_transform(interp);
			float starting_health_angle = 0.f;
			float ending_health_angle = 0.f;

			if (v == watched_character) {
				starting_health_angle = watched_character_transform.rotation + 135;
				ending_health_angle = starting_health_angle + health.get_ratio() * 90.f;
			}
			else {
				starting_health_angle = (v.get_viewing_transform(interp).pos - watched_character_transform.pos).degrees() - 45;
				ending_health_angle = starting_health_angle + health.get_ratio() * 90.f;
			}

			const auto push_angles = [&](
				const float lower_outside, 
				const float upper_outside, 
				const float lower_inside, 
				const float upper_inside
			) {
				augs::special s;

				s.v1.set(augs::normalize_degrees(lower_outside), augs::normalize_degrees(upper_outside + 0.001f)) /= 180;
				s.v2.set(augs::normalize_degrees(lower_inside), augs::normalize_degrees(upper_inside + 0.001f)) /= 180;

				specials.push_back(s);
				specials.push_back(s);
				specials.push_back(s);
				
				specials.push_back(s);
				specials.push_back(s);
				specials.push_back(s);
			};

			push_angles(starting_health_angle, starting_health_angle + 90, starting_health_angle, ending_health_angle);

			struct circle_info {
				float angle;
				std::string text;
				rgba color;
			};

			std::vector<circle_info> textual_infos;

			if (v == watched_character) {
				const auto examine_item_slot = [&](
					const const_inventory_slot_handle id,
					const float lower_outside,
					const float max_angular_length,
					const bool ccw
				) {
					if (id.alive() && id.has_items()) {
						const auto item = cosmos[id.get_items_inside()[0]];

						const auto ammo_info = get_ammunition_information(item);

						if (ammo_info.total_ammunition_space_available > 0) {
							const auto ammo_ratio = 1 - (ammo_info.total_lsa / ammo_info.total_ammunition_space_available);

							auto ammo_color = augs::interp(white, red_violet, (1 - ammo_ratio)* (1 - ammo_ratio));
							ammo_color.a = 200;

							output.aabb_centered(in.circular_bar_tex, (transform.pos), ammo_color);

							circle_info new_info;

							auto upper_outside = lower_outside + max_angular_length;

							auto empty_amount = (1 - ammo_ratio) * max_angular_length;

							if (!ccw) {
								push_angles(lower_outside, upper_outside, lower_outside, lower_outside + ammo_ratio * max_angular_length);
								new_info.angle = upper_outside - empty_amount / 2;
							}
							else {
								push_angles(lower_outside, upper_outside, upper_outside - ammo_ratio * max_angular_length, upper_outside);
								new_info.angle = lower_outside + empty_amount / 2;
							}

							new_info.text = std::to_string(ammo_info.total_charges);
							new_info.color = ammo_color;

							textual_infos.push_back(new_info);
						}
					}
				};

				examine_item_slot(v.get_secondary_hand(), starting_health_angle + 90.f + 22.5f, 45.f, false);
				examine_item_slot(v.get_primary_hand(), starting_health_angle - 22.5f - 45.f, 45.f, true);
			}

			const int radius = in.circular_bar_tex.get_original_size().x / 2;
			const auto empty_health_amount = static_cast<int>((1 - health.get_ratio()) * 90);

			textual_infos.push_back({
				starting_health_angle + 90 - empty_health_amount / 2,
				std::to_string(int(health.value) == 0 ? 1 : int(health.value)),
				health_col
			});

			textual_infos.push_back({
				starting_health_angle,
				get_bbcoded_entity_name(v),
				health_col
			});

			for (const auto& info : textual_infos) {
				if (info.text.empty()) {
					continue;
				}

				//const auto circle_displacement_length = health_points.get_bbox().bigger_side() + radius;
				const auto text = formatted_string { info.text,{ in.gui_font, info.color } };
				const auto bbox = get_text_bbox(text);
				
				const vec2i screen_space_circle_center = transform.pos;
				const auto text_pos = screen_space_circle_center + position_rectangle_around_a_circle(radius + 6.f, bbox, info.angle) - bbox / 2;
				//health_points.pos = screen_space_circle_center + vec2::from_degrees(in.angle).set_length(circle_displacement_length);

				augs::gui::text::print_stroked(
					augs::drawer { circular_bars_information },
					text_pos,
					text
				);
			}
		}
	});

	return circular_bars_information;
}
