#include "spell_data.h"
#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/components/sentience_component.h"
#include "game/components/render_component.h"
#include "game/components/wandering_pixels_component.h"
#include "game/components/damage_component.h"
#include "game/components/sound_response_component.h"
#include "game/messages/create_particle_effect.h"
#include "game/messages/exploding_ring.h"
#include "game/systems_stateless/particles_existence_system.h"
#include "game/systems_stateless/sound_existence_system.h"
#include "game/systems_stateless/visibility_system.h"

#include "game/ingredients/ingredients.h"
#include "game/assets/sound_response_id.h"

#include "game/detail/entity_scripts.h"
#include "game/enums/filters.h"

#include "game/messages/damage_message.h"

#include "augs/graphics/renderer.h"

spell_data get_spell_data(const spell_type spell) {
	spell_data d;
	
	switch (spell) {
	case spell_type::HASTE:
		d.cooldown_ms = 5000;
		d.incantation = L"treximo";
		d.perk_duration_seconds = 33;
		d.personal_electricity_required = 60;
		break;

	case spell_type::FURY_OF_THE_AEONS:
		d.cooldown_ms = 2000;
		d.incantation = L"mania aiones";
		d.personal_electricity_required = 100;
		break;

	case spell_type::ELECTRIC_TRIAD:
		d.cooldown_ms = 3000;
		d.incantation = L"energeia triada";
		d.personal_electricity_required = 120;
		break;

	case spell_type::ULTIMATE_WRATH_OF_THE_AEONS:
		d.cooldown_ms = 2000;
		d.casting_time_ms = 3000;
		d.incantation = L"megalyteri aiones via";
		d.personal_electricity_required = 260;
		break;

	case spell_type::ELECTRIC_SHIELD:
		d.cooldown_ms = 5000;
		d.incantation = L"energeia aspida";
		d.perk_duration_seconds = 60;
		d.personal_electricity_required = 50;
		break;
	
	default: 
		LOG("Unknown spell: %x", static_cast<int>(spell)); 
		break;
	}

	return std::move(d);
}

spell_appearance get_spell_appearance(const spell_type spell) {
	spell_appearance a;

	const rgba turqoise_spell_color = turquoise;
	const rgba blue_spell_border = cyan; //{ 0, 128, 209, 255 };
	const rgba green_spell_color = { 0, 200, 0, 255 };

	switch (spell) {
	case spell_type::HASTE:
		a.icon = assets::texture_id::SPELL_HASTE_ICON;
		a.border_col = green_spell_color;
		break;

	case spell_type::FURY_OF_THE_AEONS:
		a.icon = assets::texture_id::SPELL_FURY_OF_THE_AEONS_ICON;
		a.border_col = blue_spell_border;
		break;

	case spell_type::ELECTRIC_TRIAD:
		a.icon = assets::texture_id::SPELL_ELECTRIC_TRIAD_ICON;
		a.border_col = blue_spell_border;
		break;

	case spell_type::ULTIMATE_WRATH_OF_THE_AEONS:
		a.icon = assets::texture_id::SPELL_ULTIMATE_WRATH_OF_THE_AEONS_ICON;
		a.border_col = blue_spell_border;
		break;

	case spell_type::ELECTRIC_SHIELD:
		a.icon = assets::texture_id::SPELL_ELECTRIC_SHIELD_ICON;
		a.border_col = turqoise_spell_color;
		break;
	
	default: 
		LOG("Unknown spell: %x", static_cast<int>(spell)); 
		break;
	}

	return std::move(a);
}

std::wstring describe_spell(
	const const_entity_handle caster,
	const spell_type spell
) {
	const auto spell_data = get_spell_data(spell);

	const auto properties = typesafe_sprintf(
		L"Incantation: [color=yellow]%x[/color]\nPE to cast: [color=vscyan]%x[/color]\nCooldown: [color=vscyan]%x[/color]",
		std::wstring(spell_data.incantation), 
		spell_data.personal_electricity_required, 
		spell_data.cooldown_ms
	);

	std::wstring description;

	switch (spell) {
	case spell_type::HASTE:
		description = typesafe_sprintf(
			L"[color=green]Haste[/color]\n%x\n[color=vsdarkgray]Increases movement speed for %x seconds.[/color]", 
			properties, 
			spell_data.perk_duration_seconds
		);
		break;

	case spell_type::FURY_OF_THE_AEONS:
		description = typesafe_sprintf(
			L"[color=cyan]Fury of the Aeons[/color]\n%x\n[color=vsdarkgray]Causes instant damage around the caster.[/color]", 
			properties
		);
		break;

	case spell_type::ELECTRIC_TRIAD:
		description = typesafe_sprintf(
			L"[color=cyan]Electric Triad[/color]\n%x\n[color=vsdarkgray]Spawns three electric missiles\nhoming towards hostile entities.[/color]", 
			properties
		);
		break;

	case spell_type::ULTIMATE_WRATH_OF_THE_AEONS:
		description = typesafe_sprintf(
			L"[color=cyan]Ultimate Wrath of the Aeons[/color]\n%x\n[color=vsdarkgray]Causes massive damage around the caster.\nRequires delay to initiate.[/color]", 
			properties
		);
		break;

	case spell_type::ELECTRIC_SHIELD:
		description = typesafe_sprintf(
			L"[color=turquoise]Electric Shield[/color]\n%x\n[color=vsdarkgray]For %x seconds, damage is absorbed\nby [/color][color=cyan]Personal Electricity[/color][color=vsdarkgray] instead of [/color][color=red]Health[/color][color=vsdarkgray].[/color]", 
			properties, 
			spell_data.perk_duration_seconds
		);
		break;

	default: 
		LOG("Unknown spell: %x", static_cast<int>(spell)); 
		break;
	}

	return std::move(description);
}

bool are_additional_conditions_for_casting_fulfilled(
	const spell_type spell,
	const const_entity_handle caster
) {
	switch (spell) {
	case spell_type::ELECTRIC_TRIAD:
	{
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
	const spell_type spell,
	const entity_handle caster,
	components::sentience& sentience,
	const augs::stepped_timestamp when_casted,
	const augs::stepped_timestamp now
) {
	const auto spell_data = get_spell_data(spell);
	auto& cosmos = step.cosm;
	const auto dt = cosmos.get_fixed_delta();
	const auto caster_transform = caster.get_logic_transform();
	const auto appearance = get_spell_appearance(spell);
	
	const auto ignite_sparkle_particles = [&]() {
		messages::create_particle_effect burst;
		burst.subject = caster;
		burst.place_of_birth = caster_transform;
		burst.input.effect = assets::particle_effect_id::CAST_SPARKLES;
		burst.input.modifier.colorize = appearance.border_col;

		particles_existence_system().create_particle_effect_entity(cosmos, burst).add_standard_components();
	};

	const auto play_standard_sparkles_sound = [&]() {
		components::sound_existence::effect_input in;
		in.delete_entity_after_effect_lifetime = true;
		in.direct_listener = caster;
		in.effect = assets::sound_buffer_id::CAST_SUCCESSFUL;

		sound_existence_system().create_sound_effect_entity(cosmos, in, caster_transform, entity_id()).add_standard_components();
	};

	switch (spell) {
	case spell_type::HASTE:
		ignite_sparkle_particles();
		play_standard_sparkles_sound();

		sentience.haste.set_for_duration(static_cast<float>(spell_data.perk_duration_seconds * 1000), now); 

		break;

	case spell_type::ELECTRIC_SHIELD:
		ignite_sparkle_particles();
		play_standard_sparkles_sound();

		sentience.electric_shield.set_for_duration(static_cast<float>(spell_data.perk_duration_seconds * 1000), now); 

		break;

	case spell_type::FURY_OF_THE_AEONS:
		ignite_sparkle_particles();
		play_standard_sparkles_sound();

		{
			const auto transform = caster.get_logic_transform();

			constexpr auto effective_radius = 500.f;
			constexpr auto effective_radius_sq = effective_radius*effective_radius;

			messages::visibility_information_request request;
			request.eye_transform = transform;
			request.filter = filters::line_of_sight_query();
			request.square_side = effective_radius;
			request.subject = caster;

			const auto response = visibility_system().respond_to_visibility_information_requests(
				cosmos,
				{},
				{ request }
			);

			const auto& physics = cosmos.systems_temporary.get<physics_system>();

			std::unordered_set<unversioned_entity_id> affected_entities_of_bodies;

			for (auto i = 0u; i < response.vis[0].get_num_triangles(); ++i) {
				auto damaging_triangle = response.vis[0].get_world_triangle(i, request.eye_transform.pos);
				damaging_triangle[1] += (damaging_triangle[1] - damaging_triangle[0]).set_length(5);
				damaging_triangle[2] += (damaging_triangle[2] - damaging_triangle[0]).set_length(5);

				physics.for_each_intersection_with_triangle(
					damaging_triangle,
					filters::dynamic_object(), 
					[&](
						const b2Fixture* const fix, 
						const vec2 point_a, 
						const vec2 point_b
					) {
						const auto body_entity_id = get_id_of_entity_of_body(fix);
						
						if (
							body_entity_id != caster.get_id()
							&& ((caster_transform.pos - point_a).length_sq() <= effective_radius_sq
							|| (caster_transform.pos - point_b).length_sq() <= effective_radius_sq)
						) {

							const auto it = affected_entities_of_bodies.insert(body_entity_id);
							const bool is_yet_unaffected = it.second;

							if (is_yet_unaffected) {
								const auto body_entity = cosmos[body_entity_id];
								const auto& affected_physics = body_entity.get<components::physics>();
								
								const auto impact = (point_b - caster_transform.pos).set_length(150);
								const auto center_offset = (point_b - affected_physics.get_mass_position()) * 0.8f;

								{
									affected_physics.apply_impulse(
										impact, center_offset
									);
									
									auto& r = augs::renderer::get_current();

									if (r.debug_draw_explosion_forces) {
										r.persistent_lines.draw_cyan(
											affected_physics.get_mass_position() + center_offset,
											affected_physics.get_mass_position() + center_offset + impact
										);
									}

									// LOG("Impact %x dealt to: %x. Resultant angular: %x", impact, body_entity.get_debug_name(), affected_physics.get_angular_velocity());
								}

								messages::damage_message damage_msg;

								damage_msg.inflictor = caster;
								damage_msg.subject = body_entity;
								damage_msg.amount = 88;
								damage_msg.impact_velocity = impact;
								damage_msg.point_of_impact = point_b;
								step.transient.messages.post(damage_msg);
							}
						}

						return query_callback_result::CONTINUE;
					}
				);
			}

			messages::exploding_ring ring;
			ring.radius = effective_radius;
			ring.color = appearance.border_col;
			ring.center = transform.pos;
			ring.visibility = std::move(response.vis[0]);

			step.transient.messages.post(ring);
		}

		break;

	case spell_type::ULTIMATE_WRATH_OF_THE_AEONS:
		if (now == when_casted) {
			ignite_sparkle_particles();
			play_standard_sparkles_sound();
		}

		break;

	case spell_type::ELECTRIC_TRIAD:
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

				const auto energy_ball = cosmos.create_entity("energy_ball");

				auto new_energy_ball_transform = caster_transform;
				
				new_energy_ball_transform.rotation = 
					(next_hostile.get_logic_transform().pos - caster_transform.pos).degrees();

				ingredients::add_sprite(
					energy_ball, 
					new_energy_ball_transform, 
					assets::texture_id::ENERGY_BALL, 
					cyan, 
					render_layer::FLYING_BULLETS
				);

				ingredients::add_bullet_round_physics(energy_ball);

				{
					auto& response = energy_ball += components::particle_effect_response{ assets::particle_effect_response_id::ELECTRIC_PROJECTILE_RESPONSE };
					response.modifier.colorize = cyan;
				}

				{
					energy_ball += components::sound_response{ assets::sound_response_id::ELECTRIC_PROJECTILE_RESPONSE };
				}

				auto& damage = energy_ball += components::damage();
				damage.homing_towards_hostile_strength = 1.0f;
				damage.particular_homing_target = next_hostile;
				damage.amount = 42;
				damage.sender = caster;

				const auto energy_ball_velocity = vec2().set_from_degrees(new_energy_ball_transform.rotation) * 2000;
				energy_ball.get<components::physics>().set_velocity(energy_ball_velocity);

				energy_ball.add_standard_components();
			}
		}

		break;

	default: 
		LOG("Unknown spell: %x", static_cast<int>(spell)); 
		break;
	}
}

