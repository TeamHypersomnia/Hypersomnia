#include "augs/misc/randomization.h"
#include "augs/drawing/make_sprite_points.h"
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
		[&](const auto subject) {
			auto& movement_path = subject.template get<components::movement_path>();
			const auto& movement_path_def = subject.template get<invariants::movement_path>();

			auto& transform = subject.template get<components::transform>();

			const auto& rotation_speed = movement_path_def.continuous_rotation_speed;

			if (!augs::is_epsilon(rotation_speed)) {
				transform.rotation += rotation_speed * delta.in_seconds();
			}

			if (movement_path_def.rect_bounded.is_enabled) {
				const auto& pos = transform.pos;
				const auto tip_pos = *subject.find_logical_tip();

				const auto& def = movement_path_def.rect_bounded.value;
				const auto size = def.rect_size;

				const auto origin = movement_path.origin;
				const auto bound = xywh::center_and_size(origin.pos, size);

				const auto global_time = cosm.get_total_seconds_passed() + real32(subject.get_guid());
				const auto global_time_sine = std::sin(global_time * 2);

				const auto max_speed_boost = def.sine_speed_boost;
				const auto speed_boost = static_cast<real32>(global_time_sine * global_time_sine * max_speed_boost);

				const auto fov_half_degrees = real32((360 - 90) / 2);
				const auto max_avoidance_speed = 20 + speed_boost / 2;
				const auto cohesion_mult = 0.05f;
				const auto alignment_mult = 0.08f;

				const auto base_speed = def.base_speed;

				const auto min_speed = base_speed + speed_boost;
				const auto max_speed = base_speed + max_speed_boost;

				const auto current_dir = vec2::from_degrees(transform.rotation);

				const float comfort_zone_radius = 50.f;
				const float cohesion_zone_radius = 60.f;

				auto for_each_neighbor_within = [&](const auto radius, auto callback) {
					thread_local visible_entities neighbors;

					neighbors.clear();
					neighbors.acquire_non_physical({
						cosm,
						camera_cone(tip_pos),
						vec2::square(radius * 2),

						false
					});

					for (const auto& a : neighbors.all) {
						cosm[a].dispatch_on_having<components::movement_path>([&](const auto typed_neighbor) {
							if (typed_neighbor.get_id() != subject.get_id()) {
								const auto neighbor_tip = *typed_neighbor.find_logical_tip();
								const auto offset = neighbor_tip - tip_pos;

								const auto facing = current_dir.degrees_between(offset);

								if (facing < fov_half_degrees) {
									callback(typed_neighbor);
								}
							}
							/* Otherwise, don't measure against itself */
						});
					}
				};

				auto velocity = current_dir * min_speed;


				{
					auto greatest_avoidance = vec2::zero;

					for_each_neighbor_within(comfort_zone_radius, [&](const auto typed_neighbor) {
						const auto& neighbor_path_def = typed_neighbor.template get<invariants::movement_path>();
						const auto& neighbor_path = typed_neighbor.template get<components::movement_path>();

						if (neighbor_path_def.rect_bounded.is_enabled) {
							const auto neighbor_transform = typed_neighbor.get_logic_transform();
							const auto neighbor_vel = vec2::from_degrees(neighbor_transform.rotation) * neighbor_path.last_speed;
							const auto neighbor_tip = *typed_neighbor.find_logical_tip();

							const auto avoidance = augs::immediate_avoidance(
								tip_pos,
								vec2(velocity).set_length(movement_path.last_speed),
								neighbor_tip,
								neighbor_vel,
								comfort_zone_radius,
								max_avoidance_speed * neighbor_path.last_speed / max_speed
							);

							greatest_avoidance = std::max(avoidance, greatest_avoidance);
						}
					});

					velocity += greatest_avoidance;
				}


				{
					vec2 average_pos;
					vec2 average_vel;

					unsigned counted_neighbors = 0;

					for_each_neighbor_within(comfort_zone_radius, [&](const auto typed_neighbor) {
						if (typed_neighbor.get_flavour_id() != subject.get_flavour_id()) {
							return;
						}

						const auto& neighbor_path_def = typed_neighbor.template get<invariants::movement_path>();

						if (neighbor_path_def.rect_bounded.is_enabled) {
							const auto neighbor_transform = typed_neighbor.get_logic_transform();
							const auto& neighbor_path = typed_neighbor.template get<components::movement_path>();
							average_pos += neighbor_transform.pos;
							average_vel += vec2::from_degrees(neighbor_transform.rotation) * neighbor_path.last_speed;
							++counted_neighbors;
						}
					});

					if (counted_neighbors) {
						average_pos /= counted_neighbors;
						average_vel /= counted_neighbors;

						if (cohesion_mult != 0.f) {
							const auto target_vector = average_pos - pos;
							const auto dist_from_center = target_vector.length();

							//velocity += vec2(target_vector).set_length(max_cohesion_speed * dist_from_center/cohesion_zone_radius);

							velocity += augs::homing_correction(
								velocity * std::min(1.f, dist_from_center / cohesion_zone_radius) * cohesion_mult,
								target_vector
							);
						}

						if (alignment_mult != 0.f) {
							const auto target_vector = average_vel;

							velocity += augs::homing_correction(
								velocity * alignment_mult,
								target_vector
							);
						}
					}
				}


				auto& anim_state = subject.template get<components::animation>().state;

				const auto total_speed = velocity.length();

				{
					const auto edges = bound.make_edges();

					auto min_dist = std::numeric_limits<real32>::max();

					for (auto& e : edges) {
						const auto dist = tip_pos.distance_from_segment_sq(e[0], e[1]);

						if (dist < min_dist) {
							min_dist = dist;
						}
					}

					min_dist = std::sqrt(min_dist);

					/* Finally, correct velocities against the walls */

					auto dir_mult = std::max(0.f, 1.f - min_dist / 30.f) / 5;

					if (!bound.hover(tip_pos)) {
						/* Protect from going outside */
						dir_mult = 1.f;
					}

					velocity +=
						augs::seek(
							velocity,
							pos,
							origin.pos,
							velocity.length()
						) * dir_mult;
					;
				}

				movement_path.last_speed = total_speed;

				transform.pos += velocity * delta.in_seconds();
				transform.rotation = velocity.degrees();//augs::interp(transform.rotation, velocity.degrees(), 50.f * delta.in_seconds());

				const auto speed_mult = total_speed / max_speed;
				anim_state.frame_elapsed_ms += delta.in_milliseconds() * speed_mult;
			}

			npo.infer_cache_for(subject);
		}
	);
}
