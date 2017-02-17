#include "explosions.h"
#include "game/assets/sound_buffer_id.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "augs/graphics/pixel.h"
#include "game/messages/exploding_ring.h"
#include "game/messages/damage_message.h"
#include "game/systems_stateless/visibility_system.h"
#include "game/systems_stateless/sound_existence_system.h"
#include "game/enums/filters.h"
#include "augs/graphics/renderer.h"
#include "game/components/sentience_component.h"
#include "game/transcendental/step.h"

void standard_explosion(
	const logic_step step,
	components::transform explosion_location,
	const entity_id subject_if_any,
	const float effective_radius,
	const float damage,
	const float impact_force,
	const rgba inner_ring_color,
	const rgba outer_ring_color,
	const assets::sound_buffer_id sound_effect,
	const float sound_gain
) {
	auto& cosmos = step.cosm;

	components::sound_existence::effect_input in;
	in.delete_entity_after_effect_lifetime = true;
	in.direct_listener = subject_if_any;
	in.effect = sound_effect;
	in.modifier.gain = sound_gain;

	sound_existence_system().create_sound_effect_entity(
		cosmos, 
		in, 
		explosion_location, 
		entity_id()
	).add_standard_components();

	const auto delta = cosmos.get_fixed_delta();
	const auto now = cosmos.get_timestamp();

	const auto effective_radius_sq = effective_radius*effective_radius;
	const auto subject = cosmos[subject_if_any];

	messages::visibility_information_request request;
	request.eye_transform = explosion_location;
	request.filter = filters::line_of_sight_query();
	request.square_side = effective_radius * 2;
	request.subject = subject_if_any;

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
				body_entity_id != subject.get_id()
				&& ((explosion_location.pos - point_a).length_sq() <= effective_radius_sq
					|| (explosion_location.pos - point_b).length_sq() <= effective_radius_sq)
				) {

				const auto it = affected_entities_of_bodies.insert(body_entity_id);
				const bool is_yet_unaffected = it.second;

				if (is_yet_unaffected) {
					const auto body_entity = cosmos[body_entity_id];
					const auto& affected_physics = body_entity.get<components::physics>();

					const auto impact = (point_b - explosion_location.pos).set_length(impact_force);
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

					auto* const maybe_sentience = body_entity.find<components::sentience>();

					if (maybe_sentience != nullptr) {
						maybe_sentience->shake_for_ms = 400.f;
						maybe_sentience->time_of_last_shake = now;
					}

					messages::damage_message damage_msg;

					damage_msg.inflictor = subject;
					damage_msg.subject = body_entity;
					damage_msg.amount = damage;
					damage_msg.impact_velocity = impact;
					damage_msg.point_of_impact = point_b;
					step.transient.messages.post(damage_msg);
				}
			}

			return query_callback_result::CONTINUE;
		}
		);
	}

	{
		messages::exploding_ring ring;

		ring.outer_radius_start_value = effective_radius / 2;
		ring.outer_radius_end_value = effective_radius;

		ring.inner_radius_start_value = 0.f;
		ring.inner_radius_end_value = effective_radius;

		ring.time_of_occurence = now.in_seconds(delta);
		ring.maximum_duration_seconds = 0.20f;

		ring.color = cyan;
		ring.center = request.eye_transform.pos;
		ring.visibility = std::move(response.vis[0]);

		step.transient.messages.post(ring);
	}

	{
		messages::exploding_ring ring;

		ring.outer_radius_start_value = effective_radius;
		ring.outer_radius_end_value = effective_radius / 2;

		ring.inner_radius_start_value = effective_radius / 1.5f;
		ring.inner_radius_end_value = effective_radius / 2;

		ring.time_of_occurence = now.in_seconds(delta);
		ring.maximum_duration_seconds = 0.20f;

		ring.color = white;
		ring.center = request.eye_transform.pos;
		ring.visibility = std::move(response.vis[0]);

		step.transient.messages.post(ring);
	}
}