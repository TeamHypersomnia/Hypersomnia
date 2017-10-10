#include "augs/templates/string_templates.h"
#include "game/components/interpolation_component.h"
#include "game/components/flags_component.h"
#include "game/components/fixtures_component.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmos.h"

#include "game/messages/health_event.h"
#include "game/messages/item_picked_up_message.h"
#include "game/messages/interpolation_correction_request.h"
#include "game/messages/will_soon_be_deleted.h"

#include "view/audiovisual_state/audiovisual_state.h"
#include "view/audiovisual_state/systems/interpolation_system.h"

void audiovisual_state::reserve_caches_for_entities(const std::size_t n) {
	systems.for_each([n](auto& sys) {
		sys.reserve_caches_for_entities(n);
	});
}

void audiovisual_state::advance(const audiovisual_advance_input input) {
	auto scope = measure_scope(profiler.advance);

	const auto& cosm = input.viewed_character.get_cosmos();
	const auto dt = input.delta;

	reserve_caches_for_entities(cosm.get_entity_pool().capacity());

	auto& thunders = get<thunder_system>();
	auto& exploding_rings = get<exploding_ring_system>();
	auto& flying_numbers = get<flying_number_indicator_system>();
	auto& highlights = get<pure_color_highlight_system>();
	auto& interp = get<interpolation_system>();
	auto& particles = get<particles_simulation_system>();

	const auto viewed_character = input.viewed_character;

	thunders.advance(cosm, input.particle_effects, dt, particles);
	exploding_rings.advance(cosm, input.particle_effects, dt, particles);
	flying_numbers.advance(dt);
	highlights.advance(dt);

	{
		auto scope = measure_scope(profiler.interpolation);
		interp.integrate_interpolated_transforms(input.interpolation, cosm, dt, cosm.get_fixed_delta());
	}
	
	camera.tick(
		input.screen_size,
		interp,
		dt,
		input.camera,
		viewed_character
	);

	{
		{
			auto scope = measure_scope(profiler.camera_visibility_query);
			
			all_visible.reacquire({ cosm, get_viewing_camera() });

			profiler.visible_entities.measure(all_visible.all.size());
		}

		auto scope = measure_scope(profiler.particle_logic);

		particles.advance_visible_streams_and_all_particles(
			camera.smoothed_camera,
			cosm,
			input.particle_effects,
			dt,
			interp
		);
	}

	get<light_system>().advance_attenuation_variations(cosm, dt);

	{
		auto scope = measure_scope(profiler.wandering_pixels);

		get<wandering_pixels_system>().advance_for(
			all_visible,
			cosm,
			dt
		);
	}

	world_hover_highlighter.cycle_duration_ms = 400;
	world_hover_highlighter.update(dt.in_milliseconds());

	auto& sounds = get<sound_system>();

	if (viewed_character.alive()) {
		auto scope = measure_scope(profiler.sound_logic);

		auto listener_cone = camera.smoothed_camera;
		listener_cone.transform = viewed_character.get_viewing_transform(interp);
		
		sounds.track_new_sound_existences_near_camera(
			input.audio_volume,
			input.sounds,
			listener_cone,
			viewed_character,
			interp
		);
	}

	sounds.fade_sources(sound_fading_timer.extract_delta());
}

void audiovisual_state::spread_past_infection(const const_logic_step step) {
	const auto& cosm = step.cosm;

	const auto& events = step.transient.messages.get_queue<messages::collision_message>();

	for (const auto& it : events) {
		const const_entity_handle subject_owner_body = cosm[it.subject].get_owner_body();
		const const_entity_handle collider_owner_body = cosm[it.collider].get_owner_body();

		auto& past_system = get<past_infection_system>();

		if (past_system.is_infected(subject_owner_body) && !collider_owner_body.get_flag(entity_flag::IS_IMMUNE_TO_PAST)) {
			past_system.infect(collider_owner_body);
		}
	}
}

void audiovisual_state::standard_post_solve(const const_logic_step step) {
	auto scope = measure_scope(profiler.post_solve);

	const auto& cosmos = step.cosm;
	reserve_caches_for_entities(cosmos.get_entity_pool().capacity());

	const auto& healths = step.transient.messages.get_queue<messages::health_event>();
	const auto& new_thunders = step.transient.messages.get_queue<thunder_input>();
	auto new_rings = step.transient.messages.get_queue<exploding_ring_input>();

	const auto& new_interpolation_corrections = step.transient.messages.get_queue<messages::interpolation_correction_request>();
	auto& interp = get<interpolation_system>();

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

	auto& thunders = get<thunder_system>();
	auto& exploding_rings = get<exploding_ring_system>();
	auto& flying_numbers = get<flying_number_indicator_system>();
	auto& highlights = get<pure_color_highlight_system>();

	for (const auto& h : healths) {
		flying_number_indicator_system::number::input vn;

		vn.impact_velocity = h.impact_velocity;
		vn.maximum_duration_seconds = 0.7f;

		rgba number_col;
		rgba highlight_col;

		if (h.target == messages::health_event::target_type::HEALTH) {
			if (h.effective_amount > 0) {
				number_col = red;
				highlight_col = white;

				const bool destroyed = h.special_result == messages::health_event::result_type::DEATH;

				if (destroyed) {
					vn.text = L"Death";
					vn.color = number_col;
					vn.pos = cosmos[h.subject].get_logic_transform().pos;

					flying_numbers.add(vn);
				}

				const auto base_radius = destroyed ? 80.f : h.effective_amount * 1.5f;
				{
					exploding_ring_input ring;

					ring.outer_radius_start_value = base_radius / 1.5f;
					ring.outer_radius_end_value = base_radius / 3.f;

					ring.inner_radius_start_value = base_radius / 2.5f;
					ring.inner_radius_end_value = base_radius / 3.f;

					ring.emit_particles_on_ring = false;

					ring.maximum_duration_seconds = 0.20f;

					ring.color = red;
					ring.center = h.point_of_impact;

					new_rings.push_back(ring);
				}

				{
					exploding_ring_input ring;

					ring.outer_radius_start_value = base_radius / 2.f;
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

					ring.outer_radius_start_value = base_radius / 1.5f;
					ring.outer_radius_end_value = base_radius / 3.f;

					ring.inner_radius_start_value = base_radius / 2.5f;
					ring.inner_radius_end_value = base_radius / 3.f;

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
					vn.text = L"Unconscious";
					vn.color = number_col;
					vn.pos = cosmos[h.subject].get_logic_transform().pos;

					flying_numbers.add(vn);
				}

				const auto base_radius = destroyed ? 80.f : h.effective_amount * 2.f;
				{
					exploding_ring_input ring;

					ring.outer_radius_start_value = base_radius / 1.5f;
					ring.outer_radius_end_value = base_radius / 3.f;

					ring.inner_radius_start_value = base_radius / 2.5f;
					ring.inner_radius_end_value = base_radius / 3.f;

					ring.emit_particles_on_ring = false;

					ring.maximum_duration_seconds = 0.20f;

					ring.color = yellow;
					ring.center = h.point_of_impact;

					new_rings.push_back(ring);
				}

				{
					exploding_ring_input ring;

					ring.outer_radius_start_value = base_radius / 2.f;
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

		const auto number_value = static_cast<int>(h.effective_amount);

		vn.text = to_wstring(std::abs(number_value ? number_value : 1));
		vn.color = number_col;
		
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

}

void audiovisual_state::standard_post_cleanup(const const_logic_step step) {
	auto scope = measure_scope(profiler.post_cleanup);

	if (const bool any_deletion_occured =
		step.transient.messages.get_queue<messages::will_soon_be_deleted>().size() > 0
	) {
		clear_dead_entities(step.cosm);
	}
}

void audiovisual_state::clear_dead_entities(const cosmos& cosmos) {
	all_visible.clear_dead(cosmos);

	get<sound_system>().clear_dead_entities(cosmos);
	get<particles_simulation_system>().clear_dead_entities(cosmos);
	get<wandering_pixels_system>().clear_dead_entities(cosmos);
}