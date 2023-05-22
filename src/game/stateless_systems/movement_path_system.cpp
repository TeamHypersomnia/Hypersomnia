#include "augs/misc/randomization.h"
#include "augs/math/steering.h"
#include "augs/math/make_rect_points.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/for_each_entity.h"

#include "game/messages/interpolation_correction_request.h"
#include "game/messages/queue_deletion.h"
#include "game/messages/will_soon_be_deleted.h"
#include "game/detail/visible_entities.h"

#include "game/stateless_systems/movement_path_system.h"
#include "game/inferred_caches/tree_of_npo_cache.hpp"
#include "game/inferred_caches/organism_cache.hpp"
#include "game/inferred_caches/organism_cache_query.hpp"
#include "game/detail/get_hovered_world_entity.h"

void movement_path_system::advance_paths(const logic_step step) const {
	if (!step.get_settings().simulate_decorative_organisms) {
		return;
	}

	auto& cosm = step.get_cosmos();
	const auto delta = step.get_delta();

	auto& step_rng = step.step_rng;

	auto& grids = cosm.get_solvable_inferred({}).organisms;

	static const auto fov_half_degrees = real32((360 - 90) / 2);
	static const auto fov_half_degrees_cos = repro::cos(fov_half_degrees);

	cosm.for_each_having<components::movement_path>(
		[&](const auto& subject) {
			const auto& movement_path_def = subject.template get<invariants::movement_path>();

			const auto& rotation_speed = movement_path_def.continuous_rotation_speed;

			if (augs::is_nonzero(rotation_speed)) {
				auto& transform = subject.template get<components::transform>();
				transform.rotation += rotation_speed * delta.in_seconds();
			}

			if (movement_path_def.organism_wandering.is_enabled) {
				auto& movement_path = subject.template get<components::movement_path>();

				const auto& transform = subject.template get<components::transform>();
				const auto& pos = transform.pos;
				const auto tip_pos = subject.get_logical_tip(transform);

				const auto& def = movement_path_def.organism_wandering.value;

				const auto origin = cosm[movement_path.origin];

				if (origin.dead()) {
					movement_path.origin = ::get_hovered_world_entity(
						cosm,
						transform.pos,
						[&](const auto area_id) {
							if (const auto area_entity = cosm[area_id]) {
								if (const auto area = area_entity.template find<invariants::area_marker>()) {
									if (area->type == area_marker_type::ORGANISM_AREA) {
										return true;
									}
								}
							}

							return false;
						},
						render_layer_filter::whitelist(render_layer::AREA_MARKERS),
						accuracy_type::EXACT,
						{ { tree_of_npo_type::RENDERABLES } }
					);

					// LOG("New origin for %x: %x", subject, cosm[movement_path.origin]);

					/*
						Return so that we notice a frozen organism 
						if for some reason origin gets reset every frame.
					*/

					return;
				}

				const auto global_time = cosm.get_total_seconds_passed() + real32(subject.get_id().raw.indirection_index);
				const auto global_time_sine = repro::sin(real32(global_time * 2));

				const auto max_speed_boost = def.sine_speed_boost;
				const auto boost_mult = static_cast<real32>(global_time_sine * global_time_sine);
				const auto speed_boost = boost_mult * max_speed_boost;

				const auto max_avoidance_speed = 20 + speed_boost / 2;
				const auto max_startle_speed = 250 + 4*speed_boost;
				const auto max_lighter_startle_speed = 200 + 4*speed_boost;

				const auto cohesion_mult = 0.05f;
				const auto alignment_mult = 0.08f;

				const auto base_speed = def.base_speed;

				const auto min_speed = base_speed + speed_boost;
				const auto max_speed = base_speed + max_speed_boost;

				const auto current_dir = transform.get_direction();

				const real32 comfort_zone_radius = movement_path_neighbor_query_radius_v;
				const real32 cohesion_zone_radius = 60.f;

				const auto current_speed_mult = movement_path.last_speed / max_speed;
				const auto wandering_sine = repro::sin(real32(global_time / def.sine_wandering_period * current_speed_mult)) * def.sine_wandering_amplitude * current_speed_mult;
				const auto perpendicular_dir = current_dir.perpendicular_cw();

				const auto subject_avoidance_rank = def.avoidance_rank;

				auto for_each_neighbor_within = [&](const auto radius, auto callback) {
					if (!def.enable_flocking) {
						return;
					}

					constexpr auto max_handled_organisms = std::size_t(3);

					auto cell_callback = [&](const auto& cell) {
						const auto& orgs = cell.organisms;
						const auto cnt = std::min(orgs.size(), max_handled_organisms);

						for (std::size_t i = 0; i < cnt; ++i) {
							const auto org_id = orgs[i];

							if (org_id == subject.get_id()) {
								/* Don't measure against itself */
								continue;
							}

							const auto typed_neighbor = cosm[org_id];
							const auto neighbor_transform = typed_neighbor.get_logic_transform();
							const auto neighbor_tip = typed_neighbor.get_logical_tip(neighbor_transform);
							const auto offset_dir = (neighbor_tip - tip_pos).normalize();

							const auto facing = current_dir.dot(offset_dir);

							/*
								Facing can be between -1 (180) and 1 (0)
								fov_half_degrees_cos = -0.70710...
								thus facing must be gequal than fov_half_degrees_cos.
							*/

							if (facing >= fov_half_degrees_cos) {
								callback(typed_neighbor, neighbor_transform, neighbor_tip);
							}
						}
					};

					grids.for_each_cell_of_grid(
						origin.get_id(),
						ltrb::center_and_size(tip_pos, vec2::square(radius * 2)),
						cell_callback
					);
				};

				auto velocity = current_dir * min_speed + perpendicular_dir * wandering_sine;

				real32 total_startle_applied = 0.f;

				auto do_startle = [&](const auto type, const auto damping, const auto steer_mult, const auto max_speed) {
					auto& startle = movement_path.startle[type];
					//const auto desired_vel = vec2(startle).trim_length(max_speed);
					const auto desired_vel = startle;
					const auto total_steering = vec2((desired_vel - velocity) * steer_mult * (0.02f + (0.16f * boost_mult))).trim_length(max_speed);

					total_startle_applied += total_steering.length() / velocity.length();

					velocity += total_steering;

					startle.damp(delta.in_seconds(), vec2::square(damping));
				};

				do_startle(startle_type::LIGHTER, 0.2f, 0.1f, max_lighter_startle_speed);
				do_startle(startle_type::IMMEDIATE, 5.f, 1.f, max_startle_speed);

				vec2 average_pos;
				vec2 average_vel;

				unsigned counted_neighbors = 0;

				{
					auto greatest_avoidance = vec2::zero;

					for_each_neighbor_within(comfort_zone_radius, [&](const auto& typed_neighbor, const auto& neighbor_transform, const auto& neighbor_tip) {
						const auto& neighbor_wandering_def = typed_neighbor.template get<invariants::movement_path>().organism_wandering;
						const auto& neighbor_path = typed_neighbor.template get<components::movement_path>();

						if (neighbor_wandering_def.is_enabled) {
							if (subject_avoidance_rank > neighbor_wandering_def.value.avoidance_rank) {
								/* Don't care about lesser species. */
								return;
							}

							const auto neighbor_vel = neighbor_transform.get_direction() * neighbor_path.last_speed;

							const auto avoidance = augs::immediate_avoidance(
								tip_pos,
								current_dir * movement_path.last_speed,
								neighbor_tip,
								neighbor_vel,
								comfort_zone_radius,
								max_avoidance_speed * neighbor_path.last_speed / max_speed
							);

							greatest_avoidance = std::max(avoidance, greatest_avoidance);

							if (typed_neighbor.get_flavour_id() == subject.get_flavour_id()) {
								average_pos += neighbor_transform.pos;
								average_vel += neighbor_vel;
								++counted_neighbors;
							}
						}
					});

					velocity += greatest_avoidance;
				}

				if (counted_neighbors) {
					average_pos /= counted_neighbors;
					average_vel /= counted_neighbors;

					if (cohesion_mult != 0.f) {
						const auto total_cohesion = cohesion_mult * total_startle_applied;

						velocity += augs::arrive(
							velocity,
							pos,
							average_pos,
							velocity.length(),
							cohesion_zone_radius
						) * total_cohesion;
					}

					if (alignment_mult != 0.f) {
						const auto desired_vel = average_vel.set_length(velocity.length());
						const auto steering = desired_vel - velocity;

						velocity += steering * alignment_mult;
					}
				}

				const auto total_speed = velocity.length();

				const auto bound_avoidance = origin.dispatch([&](const auto& typed_origin) {
					if (const auto tr = typed_origin.find_logic_transform()) {
						if (const auto size = typed_origin.get_logical_size(); size.area() > 0) {
							if (const auto area = typed_origin.template find<invariants::area_marker>()) {
								return augs::steer_to_avoid_edges(
									velocity,
									tip_pos,
									augs::make_rect_points(tr->pos, size, tr->rotation),
									tr->pos,
									60.f,
									0.2f
								);
							}
						}
					}

					return augs::steer_to_avoid_result { vec2::zero, false };
				});

				auto bound_avoidance_vector = bound_avoidance.seek_vector;
				velocity += bound_avoidance_vector;

				for (auto& startle : movement_path.startle) {
					/* 
						Decrease startle vectors when nearing the bounds,
						to avoid a glitch where fish is conflicted about where to go.
					*/

					if (startle +  bound_avoidance_vector * 6 < startle) {
						startle += bound_avoidance_vector * 6;
					}

					if (bound_avoidance.hit_edge) {
						startle = step_rng.randval(0, 1) == 0 ? startle.perpendicular_cw() : startle.perpendicular_ccw();
					}
				}

				movement_path.last_speed = total_speed;

				{
					const auto speed_mult = total_speed / max_speed;
					const auto elapsed_anim_ms = delta.in_milliseconds() * speed_mult;

					{
						const auto& bubble_effect = def.bubble_effect;

						if (bubble_effect.id.is_set()) {
							/* Resolve bubbles and bubble intervals */

							auto& next_in_ms = movement_path.next_bubble_in_ms;
							
							auto choose_new_interval = [&step_rng, &next_in_ms, &def]() {
								const auto interval = def.base_bubble_interval_ms;
								const auto h = interval / 1.5f;

								next_in_ms = step_rng.randval(interval - h, interval + h);
							};

							if (next_in_ms < 0.f) {
								choose_new_interval();
							}
							else {
								next_in_ms -= elapsed_anim_ms;

								if (next_in_ms < 0.f) {
									bubble_effect.start(
										step,
										particle_effect_start_input::orbit_local(subject, transformr(vec2(subject.get_logical_size().x / 3, 0), 0)),
										always_predictable_v
									);
								}
							}
						}
					}


					auto& anim_state = subject.template get<components::animation>().state;
					anim_state.frame_elapsed_ms += elapsed_anim_ms;
				}

				{
					auto& mut_transform = subject.template get<components::transform>();
					mut_transform.rotation = velocity.degrees();//augs::interp(transform.rotation, velocity.degrees(), 50.f * delta.in_seconds());

					const auto old_position = transform.pos;
					const auto new_position = old_position + velocity * delta.in_seconds();

					grids.recalculate_cell_for(origin, subject.get_id(), old_position, new_position);

					mut_transform.pos = new_position;
				}
			}
		}
	);
}
