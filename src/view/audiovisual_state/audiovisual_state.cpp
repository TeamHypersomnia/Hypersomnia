#include "augs/string/string_templates.h"
#include "game/components/interpolation_component.h"
#include "game/components/flags_component.h"
#include "game/components/fixtures_component.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/detail/visible_entities.h"
#include "game/cosmos/cosmos.h"

#include "game/messages/health_event.h"
#include "game/messages/performed_transfer_message.h"
#include "game/messages/interpolation_correction_request.h"
#include "game/messages/thunder_effect.h"
#include "game/messages/exploding_ring_effect.h"

#include "view/audiovisual_state/audiovisual_state.h"
#include "view/audiovisual_state/systems/exploding_ring_system.hpp"
#include "view/audiovisual_state/systems/interpolation_system.h"

#include "view/character_camera.h"

void audiovisual_state::clear() {
	systems.for_each([](auto& sys) {
		sys.clear();
	});
}

void audiovisual_state::reserve_caches_for_entities(const std::size_t n) {
	systems.for_each([n](auto& sys) {
		using T = remove_cref<decltype(sys)>;

		if constexpr(can_reserve_caches_v<T>) {
			sys.reserve_caches_for_entities(n);
		}
	});
}

void audiovisual_state::advance(const audiovisual_advance_input input) {
	auto scope = measure_scope(performance.advance);

	const auto viewed_character = input.camera.viewed_character;
	const auto cone = input.camera.cone;
	const auto& cosm = viewed_character.get_cosmos();
	
	auto dt = input.frame_delta;
	dt *= input.speed_multiplier;
	
	const auto& anims = input.plain_animations;

	auto& rng = get_rng();

	auto& thunders = get<thunder_system>();
	auto& exploding_rings = get<exploding_ring_system>();
	auto& flying_numbers = get<flying_number_indicator_system>();
	auto& highlights = get<pure_color_highlight_system>();
	auto& interp = get<interpolation_system>();
	auto& particles = get<particles_simulation_system>();

	interp.id_to_integerize = viewed_character;

	thunders.advance(rng, cosm, input.particle_effects, dt, particles);
	exploding_rings.advance(rng, cosm.get_common_assets(), input.particle_effects, dt, particles);
	flying_numbers.advance(dt);
	highlights.advance(dt);

	{
		{
			auto scope = measure_scope(performance.integrate_particles);

			particles.integrate_all_particles(
				cosm,
				dt,
				anims,
				interp
			);

			performance.num_particles.measure(particles.count_all_particles());
		}

		{
			auto scope = measure_scope(performance.advance_particle_streams);

			particles.advance_visible_streams(
				rng,
				cone,
				cosm,
				input.particle_effects,
				anims,
				dt,
				interp
			);
		}
	}

	get<light_system>().advance_attenuation_variations(rng, cosm, dt);

	{
		auto scope = measure_scope(performance.wandering_pixels);

		get<wandering_pixels_system>().advance_for(
			rng,
			input.all_visible,
			cosm,
			dt
		);
	}

	world_hover_highlighter.cycle_duration_ms = 400;
	world_hover_highlighter.update(input.frame_delta);

	auto& sounds = get<sound_system>();

	if (viewed_character) {
		auto scope = measure_scope(performance.sound_logic);

		auto ear = input.camera;
		ear.cone.eye.transform = viewed_character.get_viewing_transform(interp);
		
		sounds.update_sound_properties(
			{
				input.audio_volume,
				input.sound_settings,
				input.sounds,
				interp,
				ear,
				dt,
				input.speed_multiplier,
				input.inv_tickrate
			}
		);
	}

	sounds.fade_sources(input.frame_delta);
}

void audiovisual_state::spread_past_infection(const const_logic_step step) {
	const auto& cosm = step.get_cosmos();

	const auto& events = step.get_queue<messages::collision_message>();

	for (const auto& it : events) {
		const const_entity_handle subject_owner_body = cosm[it.subject].get_owner_of_colliders();
		const const_entity_handle collider_owner_body = cosm[it.collider].get_owner_of_colliders();

		auto& past_system = get<past_infection_system>();

		if (past_system.is_infected(subject_owner_body) && !collider_owner_body.get_flag(entity_flag::IS_IMMUNE_TO_PAST)) {
			past_system.infect(collider_owner_body);
		}
	}
}

void audiovisual_state::standard_post_solve(
	const const_logic_step step, 
	const audiovisual_post_solve_input input
) {
	auto scope = measure_scope(performance.post_solve);

	const auto& cosm = step.get_cosmos();
	//reserve_caches_for_entities(cosm.get_solvable().get_entity_pool().capacity());

	auto& interp = get<interpolation_system>();

	const auto& settings = input.settings;

	if (settings.correct_interpolations) {
		const auto& new_interpolation_corrections = step.get_queue<messages::interpolation_correction_request>();

		for (const auto& c : new_interpolation_corrections) {
			const auto from = cosm[c.set_previous_transform_from];
			const auto subject = cosm[c.subject];

			if (subject.dead()) {
				continue;
			}

			const auto target_transform = 
				from 
				? interp.get_cache_of(from).interpolated_transform
				: c.set_previous_transform_value
			;

			interp.set_updated_interpolated_transform(
				subject,
				target_transform
			);
		}
	}

	if (settings.acquire_effect_messages) {
		auto& rng = get_rng();

		{
			const auto& new_thunders = step.get_queue<messages::thunder_effect>();
			auto& thunders = get<thunder_system>();

			for (const auto& t : new_thunders) {
				thunders.add(rng, t);
			}
		}
		
		{
			const auto& new_rings = step.get_queue<messages::exploding_ring_effect>();
			auto& exploding_rings = get<exploding_ring_system>();
			exploding_rings.acquire_new_rings(new_rings);
		}

		{
			auto& particles = get<particles_simulation_system>();
			particles.update_effects_from_messages(rng, step, input.particle_effects, interp);
		}

		{
			auto& sounds = get<sound_system>();

			auto ear = input.camera;

			if (const auto viewed_character = ear.viewed_character) {
				if (const auto transform = viewed_character.find_viewing_transform(interp)) {
					ear.cone.eye.transform = *transform;
				}
			}

			sounds.update_effects_from_messages(
				step, 
				{
					input.audio_volume,
					input.sound_settings,
					input.sounds, 
					interp, 
					ear,
					augs::delta::zero,
					1.0,
					0.0
				}
			);
		}
	}


	const auto& healths = step.get_queue<messages::health_event>();
	
	struct color_info {
		rgba number = white;
		rgba highlight = white;

		color_info(const messages::health_event& h) {
			if (h.target == messages::health_event::target_type::HEALTH) {
				if (h.effective_amount > 0) {
					number = red;
					highlight = white;
				}
				else {
					number = green;
					highlight = green;
				}
			}
			else if (h.target == messages::health_event::target_type::PERSONAL_ELECTRICITY) {
				if (h.effective_amount > 0) {
					number = turquoise;
					highlight = turquoise;
				}
				else {
					number = cyan;
					highlight = cyan;
				}
			}
			else if (h.target == messages::health_event::target_type::CONSCIOUSNESS) {
				if (h.effective_amount > 0) {
					number = orange;
					highlight = orange;
				}
				else {
					number = yellow;
					highlight = yellow;
				}
			}
		}
	};

	if (settings.acquire_flying_numbers) {
		auto& flying_numbers = get<flying_number_indicator_system>();

		for (const auto& h : healths) {
			auto make_vn_input = [&]() {
				flying_number_indicator_system::number::input vn;

				vn.impact_velocity = h.impact_velocity;
				vn.maximum_duration_seconds = 0.7f;

				return vn;
			};

			const bool destroyed = h.special_result != messages::health_event::result_type::NONE;

			if (h.target == messages::health_event::target_type::HEALTH) {
				if (h.effective_amount > 0) {
					if (destroyed) {
						auto vn = make_vn_input();
						vn.text = "Death";
						vn.color = red;

						if (const auto subject = cosm[h.subject]) {
							if (const auto transform = subject.find_logic_transform()) {
								vn.pos = transform->pos;
								flying_numbers.add(vn);
							}
						}
					}
				}
			}
			else if (h.target == messages::health_event::target_type::CONSCIOUSNESS) {
				if (h.effective_amount > 0) {
					if (destroyed) {
						auto vn = make_vn_input();
						vn.text = "Unconscious";
						vn.color = orange;
					}
				}
			}

			const auto number_value = static_cast<int>(h.effective_amount);

			auto vn = make_vn_input();
			vn.text = std::to_string(std::abs(number_value ? number_value : 1));
			const auto cols = color_info(h);
			vn.color = cols.number;
			vn.pos = h.point_of_impact;

			flying_numbers.add(vn);
		}
	}

	if (settings.acquire_highlights) {
		auto& highlights = get<pure_color_highlight_system>();

		for (const auto& h : healths) {
			if (augs::is_nonzero(h.effective_amount)) {
				const auto cols = color_info(h);

				pure_color_highlight_system::highlight::input new_highlight;

				new_highlight.target = h.subject;
				new_highlight.starting_alpha_ratio = 1.f;// std::min(1.f, h.ratio_effective_to_maximum * 5);

				new_highlight.maximum_duration_seconds = 0.10f;
				new_highlight.color = cols.highlight;

				highlights.add(new_highlight);
			}
		}
	}
}