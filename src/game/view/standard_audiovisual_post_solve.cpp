#include "viewing_session.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmos.h"
#include "game/messages/health_event.h"
#include "game/messages/item_picked_up_message.h"
#include "game/messages/interpolation_correction_request.h"
#include "augs/templates/string_templates.h"

void viewing_session::standard_audiovisual_post_solve(const const_logic_step step) {
	const auto& cosmos = step.cosm;
	reserve_caches_for_entities(cosmos.get_aggregate_pool().capacity());
	
	const auto& healths = step.transient.messages.get_queue<messages::health_event>();
	const auto& new_thunders = step.transient.messages.get_queue<thunder_input>();
	auto new_rings = step.transient.messages.get_queue<exploding_ring_input>();

	const auto& new_interpolation_corrections = step.transient.messages.get_queue<messages::interpolation_correction_request>();
	auto& interp = systems_audiovisual.get<interpolation_system>();

	for (const auto& c : new_interpolation_corrections) {
		const auto from = cosmos[c.set_previous_transform_from];

		if (from.alive()) {
			//LOG_NVPS(interp.get_cache_of(c.subject).interpolated_transform.pos, interp.get_cache_of(from).interpolated_transform.pos);

			interp.set_updated_interpolated_transform(
				cosmos[c.subject],
				interp.get_cache_of(from).interpolated_transform
			);

			//interp.get_cache_of(c.subject).interpolated_transform =
			//	interp.get_cache_of(from).interpolated_transform
			//;
		}
		else {
			interp.set_updated_interpolated_transform(
				cosmos[c.subject],
				c.set_previous_transform_value
			);

			//interp.get_cache_of(c.subject).interpolated_transform = 
			//	c.set_previous_transform_value
			//;
		}
	}

	auto& thunders = systems_audiovisual.get<thunder_system>();
	auto& exploding_rings = systems_audiovisual.get<exploding_ring_system>();
	auto& flying_numbers = systems_audiovisual.get<flying_number_indicator_system>();
	auto& highlights = systems_audiovisual.get<pure_color_highlight_system>();

	for (const auto& h : healths) {
		flying_number_indicator_system::number::input vn;

		vn.impact_velocity = h.impact_velocity;
		vn.maximum_duration_seconds = 0.7f;
		vn.value = h.effective_amount;

		rgba number_col;
		rgba highlight_col;

		if (h.target == messages::health_event::target_type::HEALTH) {
			if (h.effective_amount > 0) {
				number_col = red;
				highlight_col = white;

				const bool destroyed = h.special_result == messages::health_event::result_type::DEATH;

				if (destroyed) {
					vn.text.set_text(augs::gui::text::format(L"Death", augs::gui::text::style(assets::font_id::GUI_FONT, number_col)));
					vn.pos = cosmos[h.subject].get_logic_transform().pos;
					flying_numbers.add(vn);
				}

				const auto base_radius = destroyed ? 80.f : h.effective_amount * 1.5;
				{
					exploding_ring_input ring;

					ring.outer_radius_start_value = base_radius / 1.5;
					ring.outer_radius_end_value = base_radius / 3;

					ring.inner_radius_start_value = base_radius / 2.5;
					ring.inner_radius_end_value = base_radius / 3;

					ring.emit_particles_on_ring = false;

					ring.maximum_duration_seconds = 0.20f;

					ring.color = red;
					ring.center = h.point_of_impact;

					new_rings.push_back(ring);
				}

				{
					exploding_ring_input ring;

					ring.outer_radius_start_value = base_radius / 2;
					ring.outer_radius_end_value = base_radius;

					ring.inner_radius_start_value = 0.f;
					ring.inner_radius_end_value = base_radius;

					ring.emit_particles_on_ring = false;

					ring.maximum_duration_seconds = 0.20f;

					ring.color = red;
					ring.center = h.point_of_impact;

					new_rings.push_back(ring);
				}

				{
					thunder_input th;

					th.delay_between_branches_ms = std::make_pair(5.f, 17.f);
					th.max_branch_lifetime_ms = std::make_pair(30.f, 55.f);
					th.branch_length = std::make_pair(10.f, 60.f);

					th.max_all_spawned_branches = static_cast<unsigned>(h.effective_amount);
					++th.max_all_spawned_branches;
					th.max_branch_children = 3;

					th.first_branch_root = h.point_of_impact;
					th.first_branch_root.rotation = (-h.impact_velocity).degrees();
					th.branch_angle_spread = 60.f;

					th.color = highlight_col;

					thunders.add(th);
				}
			}
			else {
				number_col = green;
				highlight_col = green;
			}
		}
		else if (h.target == messages::health_event::target_type::PERSONAL_ELECTRICITY) {
			const bool destroyed = h.special_result == messages::health_event::result_type::PERSONAL_ELECTRICITY_DESTRUCTION;

			if (h.effective_amount > 0) {
				number_col = turquoise;
				highlight_col = turquoise;

				const auto base_radius = destroyed ? 80.f : h.effective_amount * 2.f;
				{
					exploding_ring_input ring;

					ring.outer_radius_start_value = base_radius / 1.5;
					ring.outer_radius_end_value = base_radius / 3;

					ring.inner_radius_start_value = base_radius / 2.5;
					ring.inner_radius_end_value = base_radius / 3;

					ring.emit_particles_on_ring = false;

					ring.maximum_duration_seconds = 0.20f;

					ring.color = cyan;
					ring.center = h.point_of_impact;

					new_rings.push_back(ring);
				}

				{
					exploding_ring_input ring;

					ring.outer_radius_start_value = base_radius / 2;
					ring.outer_radius_end_value = base_radius;

					ring.inner_radius_start_value = 0.f;
					ring.inner_radius_end_value = base_radius;

					ring.emit_particles_on_ring = false;

					ring.maximum_duration_seconds = 0.20f;

					ring.color = turquoise;
					ring.center = h.point_of_impact;

					new_rings.push_back(ring);
				}
			}
		}
		else if (h.target == messages::health_event::target_type::CONSCIOUSNESS) {
			const bool destroyed = h.special_result == messages::health_event::result_type::LOSS_OF_CONSCIOUSNESS;

			if (h.effective_amount > 0) {
				number_col = orange;
				highlight_col = orange;

				if (destroyed) {
					vn.text.set_text(augs::gui::text::format(L"Unconscious", augs::gui::text::style(assets::font_id::GUI_FONT, number_col)));
					vn.pos = cosmos[h.subject].get_logic_transform().pos;
					flying_numbers.add(vn);
				}

				const auto base_radius = destroyed ? 80.f : h.effective_amount * 2.f;
				{
					exploding_ring_input ring;

					ring.outer_radius_start_value = base_radius / 1.5;
					ring.outer_radius_end_value = base_radius / 3;

					ring.inner_radius_start_value = base_radius / 2.5;
					ring.inner_radius_end_value = base_radius / 3;

					ring.emit_particles_on_ring = false;

					ring.maximum_duration_seconds = 0.20f;

					ring.color = yellow;
					ring.center = h.point_of_impact;

					new_rings.push_back(ring);
				}

				{
					exploding_ring_input ring;

					ring.outer_radius_start_value = base_radius / 2;
					ring.outer_radius_end_value = base_radius;

					ring.inner_radius_start_value = 0.f;
					ring.inner_radius_end_value = base_radius;

					ring.emit_particles_on_ring = false;

					ring.maximum_duration_seconds = 0.20f;

					ring.color = orange;
					ring.center = h.point_of_impact;

					new_rings.push_back(ring);
				}
			}
		}
		else {
			continue;
		}

		vn.text.set_text(
			augs::gui::text::format(
				to_wstring(
					std::abs(int(vn.value) == 0 ? 1 : int(vn.value))
				),
				augs::gui::text::style(assets::font_id::GUI_FONT, number_col)
			)
		);

		vn.pos = h.point_of_impact;
		
		flying_numbers.add(vn);

		pure_color_highlight_system::highlight::input new_highlight;

		new_highlight.target = h.subject;
		new_highlight.starting_alpha_ratio = 1.f;// std::min(1.f, h.ratio_effective_to_maximum * 5);

		new_highlight.maximum_duration_seconds = 0.10f;
		new_highlight.color = highlight_col;

		highlights.add(new_highlight);
	}

	for (const auto& t : new_thunders) {
		thunders.add(t);
	}

	exploding_rings.acquire_new_rings(new_rings);

	auto& gui = systems_audiovisual.get<gui_element_system>();

	gui.reposition_picked_up_and_transferred_items(step);

	gui.erase_caches_for_dead_entities(cosmos);
	systems_audiovisual.get<sound_system>().erase_caches_for_dead_entities(cosmos);
	systems_audiovisual.get<particles_simulation_system>().erase_caches_for_dead_entities(cosmos);
	systems_audiovisual.get<wandering_pixels_system>().erase_caches_for_dead_entities(cosmos);

	for (const auto& pickup : step.transient.messages.get_queue<messages::item_picked_up_message>()) {
		gui.get_character_gui(pickup.subject).assign_item_to_first_free_hotbar_button(
			cosmos[pickup.subject],
			cosmos[pickup.item]
		);
	}
}