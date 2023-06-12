#include "missile_system.h"
#include "augs/math/steering.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/for_each_entity.h"

#include "game/messages/collision_message.h"
#include "game/messages/queue_deletion.h"

#include "game/detail/entity_scripts.h"

#include "game/components/missile_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/transform_component.h"
#include "game/components/driver_component.h"
#include "game/components/fixtures_component.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/components/sentience_component.h"
#include "game/components/attitude_component.h"
#include "game/components/sender_component.h"
#include "game/components/explosive_component.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/create_entity.hpp"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"

#include "game/detail/physics/physics_scripts.h"

#include "game/assets/ids/asset_ids.h"

#include "game/enums/filters.h"

#include "game/stateless_systems/sound_existence_system.h"
#include "game/detail/organisms/startle_nearbly_organisms.h"
#include "game/detail/explosive/detonate.h"
#include "game/detail/melee/like_melee.h"
#include "game/messages/thunder_effect.h"
#include "game/detail/physics/physics_queries.h"
#include "game/detail/physics/infer_damping.hpp"

#define USER_RICOCHET_COOLDOWNS 0
#define LOG_RICOCHETS 0

template <class... Args>
void RIC_LOG(Args&&... args) {
#if LOG_RICOCHETS
	LOG(std::forward<Args>(args)...);
#else
	((void)args, ...);
#endif
}

#if LOG_RICOCHETS
#define RIC_LOG_NVPS LOG_NVPS
#else
#define RIC_LOG_NVPS RIC_LOG
#endif

#include "game/detail/missile/missile_utils.h"
#include "game/detail/missile/missile_collision.h"
#include "game/detail/missile/missile_ricochet.h"

using namespace augs;

void missile_system::advance_penetrations(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto si = cosm.get_si();

	const auto now = cosm.get_timestamp();
	(void)now;

	thread_local std::vector<b2Fixture*> hits;
	//const auto& delta = step.get_delta();

	cosm.for_each_having<components::missile>(
		[&](const auto& it) {
			auto& missile = it.template get<components::missile>();
			auto rigid_body = it.template get<components::rigid_body>();

			//auto& missile_def = it.template get<invariants::missile>();

			/*
				If it at any point penetrated, expire missiles with small velocity. 
			*/

			auto expire_at = [&](const vec2 point) { 
				if (missile.deleted_already) {
					return;
				}

				auto tr = it.get_logic_transform();
				tr.pos = point;

				missile.penetration_distance_remaining = 0.0f;
				missile.saved_point_of_impact_before_death = tr;
				missile.deleted_already = true;

				step.queue_deletion_of(it, "Missile penetration expiration");
			};

#if 0
			if (missile.during_penetration || missile.penetration_distance_remaining < missile.starting_penetration_distance) {
				const auto expire_penetrating_below_speed = 2500.0f;
				const auto expire_penetrating_below_speed_sq = expire_penetrating_below_speed * expire_penetrating_below_speed;

				const auto speed_sq = rigid_body.get_velocity().length_sq();
				//LOG_NVPS(now.step, *it.find_logical_tip(), rigid_body.get_velocity(), std::sqrt(speed_sq));

				if (speed_sq < expire_penetrating_below_speed_sq) {
					const auto maybe_tip = it.find_logical_tip();

					expire_at(maybe_tip ? *maybe_tip : vec2::zero);
					return;
				}
			}
#endif

			if (!missile.during_penetration) {
				return;
			}

			const auto maybe_tip = it.find_logical_tip();

			if (!maybe_tip.has_value()) {
				return;
			}

			const auto& tip = *maybe_tip;

			if (tip == missile.prev_tip_position) {
				//LOG("SAME TIP");
				return;
			}

			const auto filter = filters[predefined_filter_type::PENETRATING_PROGRESS_QUERY];

			const auto p1 = missile.prev_tip_position;
			const auto p2 = tip;

			const auto p1_meters = si.get_meters(p1);
			const auto p2_meters = si.get_meters(p2);

			const auto eps = vec2(2, 2);

			if (DEBUG_DRAWING.draw_penetration) {
				const auto sp = missile.saved_point_of_impact_before_death.pos;

				DEBUG_PERSISTENT_LINES.emplace_back(yellow, sp, sp + vec2(0, 100));
				DEBUG_PERSISTENT_LINES.emplace_back(cyan, p1, p2);
			}

			hits.clear();

			/* Fill forward facing hits */

			{
				/* 
					First test the point overlap, so that if we're inside a fixture,
					it appears as the first one, so the order of penetration 'events' is preserved.
				*/

				physics.for_each_in_aabb(
					si,
					p1 - eps,
					p1 + eps,
					filter,
					[&](b2Fixture& fix) {
						if (fix.TestPoint(b2Vec2(p1_meters))) {
							fix.penetrated_forward = true;
							fix.forward_point = b2Vec2(p1);

							hits.push_back(std::addressof(fix));
						}

						return callback_result::CONTINUE;
					}
				);

				const auto results = physics.ray_cast_all_intersections(p1_meters, p2_meters, filter);

				for (const auto& result : results) {
					auto f = result.what_fixture;
					f->penetrated_forward = true;
					f->forward_point = b2Vec2(si.get_pixels(result.intersection));
					hits.push_back(f);
				}
			}

			/* Fill backward facing hits */

			{
				physics.for_each_in_aabb(
					si,
					p2 - eps,
					p2 + eps,
					filter,
					[&](b2Fixture& fix) {
						if (fix.TestPoint(b2Vec2(p2_meters))) {
							fix.penetrated_backward = true;
							fix.backward_point = b2Vec2(p2);

							hits.push_back(std::addressof(fix));
						}

						return callback_result::CONTINUE;
					}
				);

				bool saved_first = false;

				const auto results = physics.ray_cast_all_intersections(p2_meters, p1_meters, filter);

				for (const auto& result : results) {
					auto f = result.what_fixture;
					f->penetrated_backward = true;
					f->backward_point = b2Vec2(si.get_pixels(result.intersection));
					hits.push_back(f);

					if (!saved_first) {
						saved_first = true;
						missile.potential_exit = vec2(f->backward_point);
					}
				}

				if (!saved_first) {
					missile.potential_exit = tip;
				}
			}

			for (auto& fixture_ptr : hits) {
				if (fixture_ptr == nullptr) {
					continue;
				}

				auto& fixture = *fixture_ptr;

				if (fixture.penetration_processed_flag) {
					continue;
				}

				fixture.penetration_processed_flag = true;

				float penetrability = 1.0f;

				if (const auto handle = cosm[fixture.GetUserData()]) {
					if (const auto fixtures = handle.template find<invariants::fixtures>()) {
						penetrability = fixtures->penetrability;
					}

					if (const auto body = handle.template find<components::rigid_body>()) {
						penetrability *= body.get_special().penetrability;
					}
				}
				else {
					continue;
				}

				const auto considered_p1 = fixture.penetrated_forward ? vec2(fixture.forward_point) : p1;
				const auto considered_p2 = fixture.penetrated_backward ? vec2(fixture.backward_point) : p2;

				if (penetrability <= 0.0f) {
					expire_at(considered_p1);
					break;
				}
				else {
					const auto offset = considered_p2 - considered_p1;
					const auto full_penetrated_distance = offset.length() / penetrability;

					auto& remaining = missile.penetration_distance_remaining;
					const auto required = full_penetrated_distance;

					//LOG_NVPS(full_penetrated_distance, remaining);

					if (remaining > required) {
						remaining -= required;
						rigid_body.infer_damping();
					}
					else {
						const auto ratio_travelled = remaining / required;
						expire_at(considered_p1 + offset * ratio_travelled);
						break;
					}
				}
			}

			if (const auto cache = ::find_colliders_cache(it)) {
				/* 
					Note we can't just conclude that there are no collisions anymore
					if there is no raycast hit between the consecutive tips.

					Consider a long but slow bullet that moves a tiny bit each step.
				*/

				bool any_intersection = false;

				for (auto cfix : cache->constructed_fixtures) {
					if (any_intersection) {
						break;
					}

					physics.for_each_intersection_with_shape_meters_generic(
						si,
						cfix->GetShape(),
						cfix->m_body->GetTransform(),
						filter,
						[&any_intersection](auto...) { any_intersection = true; return callback_result::ABORT; }
					);
				}

				if (!any_intersection) {
					missile.during_penetration = false;
					it.infer_colliders();
					//LOG("STEP: %x NO HITS BETWEEN %x and %x! EXIT, REM: %x", now.step, p1, p2, missile.penetration_distance_remaining);
				}
			}

			/* Cleanup */

			for (auto& fixture : hits) {
				if (fixture == nullptr) {
					continue;
				}

				fixture->penetration_processed_flag = false;
				fixture->penetrated_forward = false;
				fixture->penetrated_backward = false;
			}

			missile.prev_tip_position = tip;
		}
	);
}

void missile_system::ricochet_missiles(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto& events = step.get_queue<messages::collision_message>();

	for (const auto& it : events) {
		{
			const bool interested = it.type == messages::collision_message::event_type::BEGIN_CONTACT;

			if (!interested || it.one_is_sensor) {
				continue;
			}
		}

		const auto surface_handle = cosm[it.subject];
		const auto missile_handle = cosm[it.collider];

		missile_handle.dispatch_on_having_all<invariants::missile>([&](const auto& typed_missile) {
			::ricochet_missile_against_surface(
				step,

				typed_missile, 
				surface_handle,

				it.normal,
				it.collider_impact_velocity,
				it.point
			);
		});
	}
}

void missile_system::detonate_colliding_missiles(const logic_step step) {
	auto access = allocate_new_entity_access();

	auto& cosm = step.get_cosmos();
	const auto& events = step.get_queue<messages::collision_message>();

	for (const auto& it : events) {
		if (it.one_is_sensor) {
			continue;
		}

		const auto type = [&it]() -> std::optional<missile_collision_type> {
			switch (it.type) {
				case messages::collision_message::event_type::BEGIN_CONTACT:
					return missile_collision_type::CONTACT_START;
				case messages::collision_message::event_type::PRE_SOLVE:
					return missile_collision_type::PRE_SOLVE;
				default:
					return std::nullopt;
			}
		}();

		if (!type.has_value()) {
			continue;
		}

		const auto surface_handle = cosm[it.subject];
		const auto missile_handle = cosm[it.collider];

		missile_handle.dispatch_on_having_all<invariants::missile>([&](const auto& typed_missile) {
			auto& missile = typed_missile.template get<components::missile>();
			const auto& missile_def = typed_missile.template get<invariants::missile>();

			const auto info = missile_surface_info(typed_missile, surface_handle);

			if (const auto result = collide_missile_against_surface(
				access,
				step,

				typed_missile, 
				surface_handle,

				missile_def,
				missile,

				*type,

				info,

				it.indices,

				it.normal,
				it.collider_impact_velocity,
				it.point
			)) {
				missile.saved_point_of_impact_before_death = result->transform_of_impact;
				missile.deleted_already = result->deleted_already;

				if (result->penetration_began) {
					missile.during_penetration = true;
					typed_missile.infer_colliders();
					typed_missile.template get<components::rigid_body>().set_velocity(it.collider_impact_velocity);

					//const auto& clk = cosm.get_clock();
					//const auto& now = clk.now;
					//LOG("BEGAN step %x at %x with v=%x", now.step, result->transform_of_impact.pos, it.collider_impact_velocity);

					missile.prev_tip_position = result->transform_of_impact.pos;

					auto shifted_bullet_tr = result->transform_of_impact;

					auto vel = it.collider_impact_velocity;
					vel.normalize();

					const auto w = typed_missile.get_logical_size().x;
					shifted_bullet_tr.pos -= vel * (w / 2);
					typed_missile.set_logic_transform(shifted_bullet_tr);
				}
			}
		});

		/* 
			Treat melee weapons as special kind of missiles.
	   	*/

		if (*type == missile_collision_type::CONTACT_START) {
			missile_handle.dispatch_on_having_all<invariants::melee>([&](const auto& typed_melee) {
				const auto& clk = cosm.get_clock();
				const auto& now = clk.now;

				if (is_like_thrown_melee(typed_melee)) {
					auto& melee = typed_melee.template get<components::melee>();

					auto cooldown_passes = [&](augs::stepped_timestamp& stamp, const int cooldown = 2) {
						return !(now.step <= stamp.step + cooldown);
					};

					auto try_pass_cooldown = [&](augs::stepped_timestamp& stamp, const int cooldown = 2) {
						if (!cooldown_passes(stamp, cooldown)) {
							return false;
						}

						stamp = now;
						return true;
					};

					const auto info = missile_surface_info(typed_melee, surface_handle);

					if (info.should_ignore_altogether()) {
						return;
					}

					if (is_like_thrown_melee(surface_handle) && try_pass_cooldown(melee.when_clashed, 5)) {
						const auto& from = surface_handle;
						const auto& what = typed_melee;

						const auto& throw_def = typed_melee.template get<invariants::melee>().throw_def;
						const auto& from_throw_def = from.template get<invariants::melee>().throw_def;
						const auto& other_clash = from_throw_def.clash;

						const auto clash_impulse = other_clash.impulse;

						if (clash_impulse > 0.f) {
							const auto clash_dir = -vec2(it.collider_impact_velocity).normalize();
							const auto& rigid_body = what.template get<components::rigid_body>();

							const auto total_vel = clash_dir * clash_impulse;
							rigid_body.set_velocity(total_vel);

							{
								const auto vel_degrees = total_vel.degrees();
								const auto s = augs::sgn(vel_degrees);

								rigid_body.set_angular_velocity(s * throw_def.clash_angular_speed);
							}

							const auto eff_dir = vec2(clash_dir).perpendicular_cw();
							const auto eff_transform = transformr(it.point, eff_dir.degrees());

							other_clash.particles.start(
								step,
								particle_effect_start_input::fire_and_forget(eff_transform),
								never_predictable_v
							);

							{
								const bool avoid_clashing_same_sound =
									other_clash.sound.id == throw_def.clash.sound.id
									&& !cooldown_passes(from.template get<components::melee>().when_clashed)
								;

								if (!avoid_clashing_same_sound) {
									other_clash.sound.start(
										step,
										sound_effect_start_input::fire_and_forget(eff_transform),
										never_predictable_v
									);
								}
							}

							{
								auto msg = messages::thunder_effect(never_predictable_v);
								auto& th = msg.payload;

								th.delay_between_branches_ms = {10.f, 25.f};
								th.max_branch_lifetime_ms = {40.f, 65.f};
								th.branch_length = {10.f, 120.f};

								th.max_all_spawned_branches = 40;
								th.max_branch_children = 2;

								th.first_branch_root = eff_transform;
								th.branch_angle_spread = 40.f;

								th.color = white;

								step.post_message(msg);
							}
						}

						return;
					}

					const bool sentient = surface_handle.template has<components::sentience>();
					const bool interested = sentient || info.surface_is_held_item;

					if (!interested) {
						return;
					}

					if (sentient && !try_pass_cooldown(melee.when_inflicted_damage)) {
						return;
					}

					if (info.surface_is_held_item && !try_pass_cooldown(melee.when_passed_held_item)) {
						return;
					}

					const auto& melee_def = typed_melee.template get<invariants::melee>();
					const auto& throw_def = melee_def.throw_def;

					auto simulated_missile_def = invariants::missile();
					auto simulated_missile = components::missile();

					{
						auto& m = simulated_missile_def;

						m.damage = throw_def.damage;
						m.damage_upon_collision = true;
						m.destroy_upon_damage = false;
						m.constrain_lifetime = false;
						m.damage_falloff = false;
					}

					{
						auto& m = simulated_missile;
						m.deleted_already = false;
						m.power_multiplier_of_sender = 1.f;
						m.headshot_multiplier_of_sender = throw_def.headshot_multiplier;
						m.head_radius_multiplier_of_sender = throw_def.head_radius_multiplier;
					}

					if (const auto result = collide_missile_against_surface(
						access,
						step,

						typed_melee, 
						surface_handle,

						simulated_missile_def,
						simulated_missile,

						*type,

						info,

						it.indices,

						it.normal,
						it.collider_impact_velocity,
						it.point
					)) {
						if (sentient) {
#if UNSET_SENDER_AFTER_DEALING_DAMAGE
							{
								auto& sender = typed_melee.template get<components::sender>();
								sender.unset();
							}
#endif

							const auto boomerang_impulse = throw_def.boomerang_impulse;

							const auto& rigid_body = typed_melee.template get<components::rigid_body>();
							const auto boomerang_dir = result->transform_of_impact.get_direction() * -1;

							const auto total_vel = boomerang_dir * boomerang_impulse.linear;
							rigid_body.set_velocity(total_vel);

							rigid_body.apply_angular_impulse(boomerang_impulse.angular * rigid_body.get_mass());
						}
					}
				}
			});
		}
	}
}

void missile_system::detonate_expired_missiles(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto now = cosm.get_timestamp();
	const auto& delta = step.get_delta();

	cosm.for_each_having<components::missile>(
		[&](const auto& it) {
			auto& missile = it.template get<components::missile>();
			auto& missile_def = it.template get<invariants::missile>();
		
			if (missile_def.constrain_lifetime && !missile.deleted_already) {
				if (!missile.when_fired.was_set()) {
					missile.when_fired = now;
				}
				else {
					auto considered_lifetime = missile_def.max_lifetime_ms;

					const auto dist_remaining = missile.penetration_distance_remaining;
					const auto dist_starting = missile.starting_penetration_distance;

					if (dist_remaining != dist_starting && dist_starting != 0.0f) {
						considered_lifetime *= repro::sqrt(dist_remaining / dist_starting);
					}

					const auto fuse_delay_steps = static_cast<uint32_t>(considered_lifetime / delta.in_milliseconds());
					const auto when_detonates = missile.when_fired.step + fuse_delay_steps;

					if (now.step >= when_detonates) {
						const auto current_tr = it.get_logic_transform();

						missile.saved_point_of_impact_before_death = current_tr;
						detonate_if(it, current_tr.pos, step);
						step.queue_deletion_of(it, "Missile lifetime expiration");
					}
				}
			}

			const auto* const maybe_sender = it.template find<components::sender>();

			if (maybe_sender != nullptr && missile_def.homing_towards_hostile_strength > 0.f) {
				const auto sender_capability = cosm[maybe_sender->capability_of_sender];
				const auto sender_attitude = 
					sender_capability && sender_capability.template has<components::attitude>() ? sender_capability : entity_handle::dead_handle(cosm)
				;

				const auto particular_homing_target = cosm[missile.particular_homing_target];
				
				const auto detection_radius = 250.f;
				const auto closest_hostile = 
					particular_homing_target.alive() 
					? particular_homing_target 
					: cosm[get_closest_hostile(it, sender_attitude, detection_radius, filters[predefined_filter_type::FLYING_BULLET])]
				;

				const auto current_vel = it.template get<components::rigid_body>().get_velocity();
				const auto current_pos = it.get_logic_transform().pos;

				it.set_logic_transform({ current_pos, current_vel.degrees() });

				if (closest_hostile.alive()) {
					const auto hostile_pos = closest_hostile.get_logic_transform().pos;

					const auto homing_force = augs::seek( 
						current_vel,
						current_pos,
						hostile_pos,
						missile.initial_speed
					);

					it.template get<components::rigid_body>().apply_force(
						homing_force * missile_def.homing_towards_hostile_strength
					);
				}
			}
		}
	);
}