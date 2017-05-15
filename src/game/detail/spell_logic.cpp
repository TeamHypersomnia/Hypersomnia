#include "spell_logic.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/components/sentience_component.h"
#include "game/components/render_component.h"
#include "game/components/wandering_pixels_component.h"
#include "game/components/damage_component.h"
#include "game/messages/create_particle_effect.h"
#include "game/systems_stateless/particles_existence_system.h"
#include "game/systems_stateless/sound_existence_system.h"

#include "game/ingredients/ingredients.h"
#include "game/assets/sound_buffer_id.h"

#include "game/detail/entity_scripts.h"
#include "game/enums/filters.h"

#include "game/detail/explosions.h"
#include "game/assets/spell.h"

bool are_additional_conditions_for_casting_fulfilled(
	const assets::spell_id spell,
	const const_entity_handle caster
) {
	switch (spell) {
	case assets::spell_id::ELECTRIC_TRIAD: {
		constexpr float standard_triad_radius = 800.f;

		const bool is_any_hostile_in_proximity = get_closest_hostiles(
			caster,
			caster,
			standard_triad_radius,
			filters::bullet()
		).size() > 0;

		return is_any_hostile_in_proximity;
	}

	default:
		return true;
	}
}

void perform_spell_logic(
	const logic_step step,
	const assets::spell_id spell,
	const entity_handle caster,
	components::sentience& sentience,
	const augs::stepped_timestamp when_casted,
	const augs::stepped_timestamp now
) {
	auto& cosmos = step.cosm;
	const auto& metas = step.input.metas_of_assets;

	const auto spell_data = metas[spell];
	const auto dt = cosmos.get_fixed_delta();
	const auto caster_transform = caster.get_logic_transform();
	
	const auto ignite_sparkle_particles = [&]() {
		particle_effect_input burst;

		burst.effect.id = assets::particle_effect_id::CAST_SPARKLES;
		burst.effect.modifier.colorize = spell_data.border_col;
		
		burst.create_particle_effect_entity(
			step,
			caster_transform,
			caster
		).add_standard_components(step);
	};

	const auto ignite_charging_particles = [&](const rgba col) {
		particle_effect_input burst;

		burst.effect.id = assets::particle_effect_id::CAST_CHARGING;
		burst.effect.modifier.colorize = col;
		burst.effect.modifier.scale_lifetimes = 1.3f;
		burst.effect.modifier.homing_target = caster;

		burst.create_particle_effect_entity(
			step,
			caster_transform,
			caster
		).add_standard_components(step);
	};
	
	const auto play_sound = [&](const assets::sound_buffer_id effect, const float gain = 1.f) {
		sound_effect_input in;
		in.delete_entity_after_effect_lifetime = true;
		in.direct_listener = caster;
		in.effect.id = effect;
		in.effect.modifier.gain = gain;

		in.create_sound_effect_entity(step, caster_transform, entity_id()).add_standard_components(step);
	};

	const auto play_standard_sparkles_sound = [&]() {
		play_sound(assets::sound_buffer_id::CAST_SUCCESSFUL);
	};

	switch (spell) {
	case assets::spell_id::HASTE:
		ignite_sparkle_particles();
		play_standard_sparkles_sound();

		sentience.haste.timing.set_for_duration(static_cast<float>(spell_data.perk_duration_seconds * 1000), now); 

		break;

	case assets::spell_id::ELECTRIC_SHIELD:
		ignite_sparkle_particles();
		play_standard_sparkles_sound();

		sentience.electric_shield.timing.set_for_duration(static_cast<float>(spell_data.perk_duration_seconds * 1000), now); 

		break;

	case assets::spell_id::FURY_OF_THE_AEONS:
		if (when_casted == now) {
			ignite_sparkle_particles();
			play_standard_sparkles_sound();
			play_sound(assets::sound_buffer_id::EXPLOSION, 1.2f);

			sentience.shake_for_ms = 400.f;
			sentience.time_of_last_shake = now;

			standard_explosion_input in;
			in.effective_radius = 250.f;
			in.damage = 88.f;
			in.impact_force = 150.f;
			in.inner_ring_color = cyan;
			in.outer_ring_color = white;
			in.sound_effect = assets::sound_buffer_id::EXPLOSION;
			in.sound_gain = 1.2f;

			in.standard_explosion(step, caster_transform, caster);
		}

		break;

	case assets::spell_id::ULTIMATE_WRATH_OF_THE_AEONS:
	{
		const auto seconds_passed = (now - when_casted).in_seconds(dt);
		const auto first_at = augs::stepped_timestamp{ when_casted.step + static_cast<unsigned>(1.3f / dt.in_seconds()) };
		const auto second_at = augs::stepped_timestamp{ when_casted.step + static_cast<unsigned>(1.8f / dt.in_seconds()) };
		const auto third_at = augs::stepped_timestamp{ when_casted.step + static_cast<unsigned>(2.3f / dt.in_seconds()) };
		
		standard_explosion_input in;
		in.damage = 88.f;
		in.inner_ring_color = cyan;
		in.outer_ring_color = white;

		if (now == when_casted) {
			ignite_sparkle_particles();
			ignite_charging_particles(cyan);
			ignite_charging_particles(white);
			play_standard_sparkles_sound();
			play_sound(assets::sound_buffer_id::CAST_CHARGING);
		}
		else if (now == first_at) {
			sentience.shake_for_ms = 400.f;
			sentience.time_of_last_shake = now;
			
			in.effective_radius = 200.f;
			in.impact_force = 150.f;
			in.sound_gain = 1.2f;
			in.sound_effect = assets::sound_buffer_id::EXPLOSION;

			in.standard_explosion(step, caster_transform, caster);
		}
		else if (now == second_at) {
			sentience.shake_for_ms = 500.f;
			sentience.time_of_last_shake = now;

			in.effective_radius = 400.f;
			in.impact_force = 200.f;
			in.sound_gain = 1.0f;
			in.sound_effect = assets::sound_buffer_id::GREAT_EXPLOSION;

			in.standard_explosion(step, caster_transform, caster);
		}
		else if (now == third_at) {
			sentience.shake_for_ms = 600.f;
			sentience.time_of_last_shake = now;

			in.effective_radius = 600.f;
			in.impact_force = 250.f;
			in.sound_gain = 1.2f;
			in.sound_effect = assets::sound_buffer_id::GREAT_EXPLOSION;

			in.standard_explosion(step, caster_transform, caster);
		}

		break;
	}
	case assets::spell_id::ELECTRIC_TRIAD:
		ignite_sparkle_particles();
		play_standard_sparkles_sound();

		{
			constexpr float standard_triad_radius = 800.f;

			const auto hostiles = get_closest_hostiles(
				caster,
				caster,
				standard_triad_radius,
				filters::bullet()
			);

			for (size_t i = 0; i < 3 && i < hostiles.size(); ++i) {
				const auto next_hostile = cosmos[hostiles[i]];
				LOG_NVPS(next_hostile.get_id());
				const auto energy_ball = cosmos.create_entity("energy_ball");

				auto new_energy_ball_transform = caster_transform;
				
				new_energy_ball_transform.rotation = 
					(next_hostile.get_logic_transform().pos - caster_transform.pos).degrees();

				ingredients::add_sprite(
					energy_ball, 
					assets::game_image_id::ENERGY_BALL, 
					cyan, 
					render_layer::FLYING_BULLETS
				);

				ingredients::add_bullet_round_physics(
					step, 
					energy_ball,
					new_energy_ball_transform
				);

				auto& damage = energy_ball += components::damage();

				damage.destruction_particle_effect_response.id = assets::particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION;
				damage.destruction_particle_effect_response.modifier.colorize = cyan;

				damage.bullet_trace_particle_effect_response.id = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
				damage.bullet_trace_particle_effect_response.modifier.colorize = cyan;

				damage.muzzle_leave_particle_effect_response.id = assets::particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION;
				damage.muzzle_leave_particle_effect_response.modifier.colorize = cyan;

				auto& trace_modifier = damage.bullet_trace_sound_response.modifier;

				trace_modifier.max_distance = 1020.f;
				trace_modifier.reference_distance = 100.f;
				trace_modifier.gain = 1.3f;
				trace_modifier.repetitions = -1;
				trace_modifier.fade_on_exit = false;

				damage.bullet_trace_sound_response.id = assets::sound_buffer_id::ELECTRIC_PROJECTILE_FLIGHT;
				damage.destruction_sound_response.id = assets::sound_buffer_id::ELECTRIC_DISCHARGE_EXPLOSION;

				damage.homing_towards_hostile_strength = 1.0f;
				damage.particular_homing_target = next_hostile;
				damage.amount = 42;
				damage.sender = caster;

				const auto energy_ball_velocity = vec2().set_from_degrees(new_energy_ball_transform.rotation) * 2000;
				energy_ball.get<components::rigid_body>().set_velocity(energy_ball_velocity);

				energy_ball.add_standard_components(step);
			}
		}

		break;

	default: 
		LOG("Unknown spell: %x", static_cast<int>(spell)); 
		break;
	}
}

