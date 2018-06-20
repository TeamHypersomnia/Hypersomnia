#include "augs/misc/randomization.h"
#include "augs/math/steering.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

#include "game/components/transform_component.h"
#include "game/components/sprite_component.h"

#include "game/messages/interpolation_correction_request.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/will_soon_be_deleted.h"
#include "game/detail/visible_entities.h"

#include "game/stateless_systems/movement_path_system.h"

void movement_path_system::advance_paths(const logic_step step) const {
	auto& cosm = step.get_cosmos();
	const auto delta = step.get_delta();

	auto& npo = cosm.get_solvable_inferred({}).tree_of_npo;

	cosm.for_each_having<components::movement_path>(
		[&](const auto t) {
			auto& movement_path = t.template get<components::movement_path>();
			const auto& movement_path_def = t.template get<invariants::movement_path>();

			auto& transform = t.template get<components::transform>();

			const auto& rotation_speed = movement_path_def.continuous_rotation_speed;

			if (!augs::is_epsilon(rotation_speed)) {
				transform.rotation += rotation_speed * delta.in_seconds();
			}

			if (movement_path_def.rect_bounded.is_enabled) {
				const auto& pos = transform.pos;
				const auto tip_pos = pos + vec2(*t.find_logical_width(), 0).rotate(transform.rotation, vec2());

				const auto& data = movement_path_def.rect_bounded.value;
				const auto size = data.rect_size;

				const auto origin = movement_path.origin;
				const auto bound = xywh::center_and_size(origin.pos, size);

				const auto edges = bound.make_edges();

				const auto global_time = cosm.get_total_seconds_passed() + real32(t.get_guid());
				const auto global_time_sine = std::sin(global_time * 2);

				const auto max_speed_boost = 100;
				const auto speed_boost = static_cast<real32>(global_time_sine * global_time_sine * max_speed_boost);

				const auto max_avoidance_speed = 30 + speed_boost / 2;
				const auto min_speed = 100 + speed_boost;
				const auto max_speed = min_speed + max_speed_boost;

				const auto current_dir = vec2::from_degrees(transform.rotation);

				const float comfort_zone_radius = 70.f;

				thread_local visible_entities neighbors;

				neighbors.clear();
				neighbors.acquire_non_physical({
					cosm,
					camera_cone(pos),
					vec2::square(comfort_zone_radius),

					false
				});

				auto velocity = current_dir * min_speed;

				auto greatest_avoidance = vec2::zero;

				for (const auto& a : neighbors.all) {
					cosm[a].dispatch_on_having<components::movement_path>([&](const auto typed_neighbor) {
						if (typed_neighbor.get_id() == t.get_id()) {
							/* Don't measure against itself */
							return;
						}

						const auto& neighbor_path_def = typed_neighbor.template get<invariants::movement_path>();
						const auto& neighbor_path = typed_neighbor.template get<components::movement_path>();
					   
						if (neighbor_path_def.rect_bounded.is_enabled) {
							const auto neighbor_transform = typed_neighbor.get_logic_transform();
							const auto neighbor_vel = vec2::from_degrees(neighbor_transform.rotation) * neighbor_path.last_speed;

							const auto avoidance = augs::calc_danger_avoidance_proportional(
								pos,
								neighbor_transform.pos,
								neighbor_vel,
								comfort_zone_radius,
								neighbor_path.last_speed / max_speed
							) * max_avoidance_speed;

							/* const auto avoidance = augs::calc_homing( */
							/* 	velocity, */
							/* 	pos, */
							/* 	neighbor_transform.pos */
							/* ); */

							LOG_NVPS(t.get_guid(), velocity, pos, neighbor_transform.pos, avoidance);

							greatest_avoidance = std::max(avoidance, greatest_avoidance);
						}
					});
				}

				velocity += greatest_avoidance;

				auto& anim_state = t.template get<components::animation>().state;

				const auto total_speed = velocity.length();

				{
					auto min_dist = std::numeric_limits<real32>::max();

					for (auto& e : edges) {
						const auto dist = tip_pos.distance_from_segment_sq(e[0], e[1]);

						if (dist < min_dist) {
							min_dist = dist;
						}
					}

					min_dist = std::sqrt(min_dist);

					//const auto new_current_dir = vec2(velocity).normalize();
					//const auto new_current_degrees = new_current_dir.degrees();

					/* Finally, correct velocities against the walls */

					//const auto center_dir = (origin.pos - pos).normalize();
					auto dir_mult = std::max(0.f, 1.f - min_dist / 20.f) / 5;

					if (!bound.hover(tip_pos)) {
						/* Protect from going outside */
						dir_mult = 1.f;
					}

					velocity +=
						augs::calc_homing_dir(
							velocity,
							origin.pos - pos
						).set_length(max_avoidance_speed * dir_mult);
					;
						/* const auto center_homing_dir = augs::calc_homing_dir( */
						/* 	velocity, */
						/* 	origin.pos - pos */
						/* ); */

						/* const auto center_homing_degrees = center_homing_dir.degrees(); */

					/* const auto lerped_degrees = augs::interp(new_current_degrees, center_homing_degrees, dir_mult); */
					/* const auto target_dir = vec2::from_degrees(lerped_degrees); */

					/* velocity = target_dir * velocity.length(); */
				}

				movement_path.last_speed = total_speed;

				transform.pos += velocity * delta.in_seconds();
				transform.rotation = velocity.degrees();

				const auto speed_mult = total_speed / max_speed;
				anim_state.frame_elapsed_ms += delta.in_milliseconds() * speed_mult;
			}

			npo.infer_cache_for(t);
		}
	);
}
