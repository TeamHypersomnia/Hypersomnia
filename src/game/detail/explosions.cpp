#include "explosions.h"
#include "game/assets/sound_buffer_id.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "augs/graphics/pixel.h"
#include "game/messages/exploding_ring_input.h"
#include "game/messages/damage_message.h"
#include "game/systems_stateless/visibility_system.h"
#include "game/systems_stateless/sound_existence_system.h"
#include "game/enums/filters.h"
#include "augs/graphics/renderer.h"
#include "game/components/sentience_component.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"
#include "application/config_structs/debug_drawing_settings.h"

void standard_explosion_input::standard_explosion(
	const logic_step step,
	const components::transform explosion_location,
	const entity_id subject_if_any
) const {
	auto& cosmos = step.cosm;

	sound_effect_input sound_effect;
	sound_effect.delete_entity_after_effect_lifetime = true;
	sound_effect.direct_listener = subject_if_any;
	sound_effect.effect.id = this->sound_effect;
	sound_effect.effect.modifier.gain = sound_gain;

	sound_effect.create_sound_effect_entity(
		step, 
		explosion_location,
		entity_id()
	).add_standard_components(step);

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

	const auto& physics = cosmos.systems_inferred.get<physics_system>();

	std::unordered_set<unversioned_entity_id> affected_entities_of_bodies;

	for (auto i = 0u; i < response.vis[0].get_num_triangles(); ++i) {
		auto damaging_triangle = response.vis[0].get_world_triangle(i, request.eye_transform.pos);
		damaging_triangle[1] += (damaging_triangle[1] - damaging_triangle[0]).set_length(5);
		damaging_triangle[2] += (damaging_triangle[2] - damaging_triangle[0]).set_length(5);

		physics.for_each_intersection_with_triangle(
			cosmos.get_si(),
			damaging_triangle,
			filters::dynamic_object(),
			[&](
				const b2Fixture* const fix,
				const vec2 point_a,
				const vec2 point_b
			) {
				const auto body_entity_id = get_id_of_entity_of_body(fix);
				const bool is_self = 
					body_entity_id == subject.get_id()
					|| cosmos[body_entity_id].get_owning_transfer_capability() == subject.get_id()
				;

				if (is_self) {
					return callback_result::CONTINUE;
				}

				const bool in_range =
					(explosion_location.pos - point_a).length_sq() <= effective_radius_sq
					|| (explosion_location.pos - point_b).length_sq() <= effective_radius_sq
				;

				const bool should_be_affected = in_range;

				if (should_be_affected) {
					const auto it = affected_entities_of_bodies.insert(body_entity_id);
					const bool is_yet_unaffected = it.second;

					if (is_yet_unaffected) {
						const auto body_entity = cosmos[body_entity_id];
						const auto& affected_physics = body_entity.get<components::rigid_body>();

						auto impact = (point_b - explosion_location.pos).set_length(impact_force);
						const auto center_offset = (point_b - affected_physics.get_mass_position()) * 0.8f;

						messages::damage_message damage_msg;
						damage_msg.type = type;
						damage_msg.inflictor = subject;
						damage_msg.subject = body_entity;
						damage_msg.amount = damage;
						damage_msg.point_of_impact = point_b;

						switch (type) {
							case adverse_element_type::FORCE: {
								damage_msg.request_shake_for_ms = 400.f;
								damage_msg.impact_velocity = impact * 2.f;

								affected_physics.apply_impulse(
									impact, center_offset
								);

								auto& r = augs::renderer::get_current();

								if (r.debug.draw_explosion_forces) {
									r.persistent_lines.draw_cyan(
										affected_physics.get_mass_position() + center_offset,
										affected_physics.get_mass_position() + center_offset + impact
									);
								}

								// LOG("Impact %x dealt to: %x. Resultant angular: %x", impact, body_entity.get_debug_name(), affected_physics.get_angular_velocity());

								damage_msg.impact_velocity = impact;
							}
							break;

							case adverse_element_type::PED: {

							}
							break;

							case adverse_element_type::INTERFERENCE: {
								damage_msg.request_shake_for_ms = 800.f;
								damage_msg.impact_velocity = impact * 300.f;
							}
							break;

							default: {
								ensure(false && "Unknown adverse element type");
							}
							break;
						}

						step.transient.messages.post(damage_msg);
					}
				}

				return callback_result::CONTINUE;
			}
		);
	}

	{
		exploding_ring_input ring;

		ring.outer_radius_start_value = effective_radius / 2;
		ring.outer_radius_end_value = effective_radius;

		ring.inner_radius_start_value = 0.f;
		ring.inner_radius_end_value = effective_radius;
		
		ring.emit_particles_on_ring = true;

		ring.maximum_duration_seconds = 0.20f;

		ring.color = inner_ring_color;
		ring.center = request.eye_transform.pos;
		ring.visibility = std::move(response.vis[0]);

		step.transient.messages.post(ring);
	}

	{
		exploding_ring_input ring;

		ring.outer_radius_start_value = effective_radius;
		ring.outer_radius_end_value = effective_radius / 2;

		ring.inner_radius_start_value = effective_radius / 1.5f;
		ring.inner_radius_end_value = effective_radius / 2;
		
		ring.emit_particles_on_ring = true;

		ring.maximum_duration_seconds = 0.20f;

		ring.color = outer_ring_color;
		ring.center = request.eye_transform.pos;
		ring.visibility = std::move(response.vis[0]);

		step.transient.messages.post(ring);
	}
}