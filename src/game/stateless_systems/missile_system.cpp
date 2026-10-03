#include <cstdint>
#include <algorithm>
#include <limits>
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
#include "game/detail/decals/penetration_fatigue.h"
#include "game/detail/physics/calc_penetrability.hpp"
#include "game/detail/missile/penetration_path.h"

using namespace augs;

/*
	Fixture entries closer than this to where the step began are the fixture
	the bullet was already in, not a new one. Only guards against float noise:
	the fixture a penetration began in is recognized by its entity anyway.
*/
static constexpr real32 PENETRATION_ENTRY_EPSILON_PX = 0.1f;

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

			/*
				Process the fixtures in the order the bullet meets them,
				so that it dies in - and hits - the right ones.
			*/
			hits.erase(std::remove(hits.begin(), hits.end(), nullptr), hits.end());

			/*
				A character standing between walls on this step's path is only hit
				once this system is done - by its contact in detonate_colliding_missiles.
				So as not to charge - or even die in - the walls behind them first,
				this step's walk stops at the first such character and resumes from there.
			*/
			const auto character_cut = [&]() -> std::optional<vec2> {
				std::optional<vec2> nearest;
				auto nearest_dist_sq = std::numeric_limits<real32>::max();

				const auto character_filter = filters[predefined_filter_type::PENETRATING_BULLET];

				for (const auto& result : physics.ray_cast_all_intersections(p1_meters, p2_meters, character_filter)) {
					const auto character = cosm[result.what_fixture->GetUserData()];

					if (character.dead()) {
						continue;
					}

					const auto* const sentience = character.template find<components::sentience>();

					if (sentience == nullptr) {
						continue;
					}

					const bool already_hit_by_this_bullet =
						sentience->ignore_bullet == entity_id(it.get_id())
						&& sentience->ignore_bullet_when_born == missile.when_fired
					;

					if (already_hit_by_this_bullet) {
						continue;
					}

					if (missile_surface_info(it, character, step.get_settings().friendly_fire).should_ignore_altogether()) {
						continue;
					}

					const auto point = si.get_pixels(result.intersection);
					const auto dist_sq = (point - p1).length_sq();

					/*
						Where the walk already stopped last step. Cutting there again
						would stall the walk for good if that contact never got processed.
					*/
					if (dist_sq <= PENETRATION_SEAM_TOLERANCE_PX * PENETRATION_SEAM_TOLERANCE_PX) {
						continue;
					}

					if (dist_sq < nearest_dist_sq) {
						nearest_dist_sq = dist_sq;
						nearest = point;
					}
				}

				return nearest;
			}();

			const auto walk_end = character_cut.has_value() ? *character_cut : p2;
			const auto walk_end_dist = (walk_end - p1).length();

			auto entry_dist_sq_of = [&](const b2Fixture* const f) {
				const auto entry = f->penetrated_forward ? vec2(f->forward_point) : p1;
				return (entry - p1).length_sq();
			};

			auto owner_index_of = [&](const b2Fixture* const f) {
				if (const auto owner = cosm[f->GetUserData()]) {
					return owner.get_id().raw.indirection_index;
				}

				return std::numeric_limits<decltype(entity_id().raw.indirection_index)>::max();
			};

			std::sort(
				hits.begin(),
				hits.end(),
				[&](const b2Fixture* const a, const b2Fixture* const b) {
					const auto dist_a = entry_dist_sq_of(a);
					const auto dist_b = entry_dist_sq_of(b);

					if (dist_a != dist_b) {
						return dist_a < dist_b;
					}

					/* Ties are broken by identity, never by the order of the spatial query. */
					const auto owner_a = owner_index_of(a);
					const auto owner_b = owner_index_of(b);

					if (owner_a != owner_b) {
						return owner_a < owner_b;
					}

					return a->index_in_component < b->index_in_component;
				}
			);

			const auto path_dir = vec2(p2 - p1).normalize();

			const b2Fixture* previous_fixture = nullptr;
			auto previous_exit = p1;

			for (auto& fixture_ptr : hits) {
				auto& fixture = *fixture_ptr;

				if (fixture.penetration_processed_flag) {
					continue;
				}

				fixture.penetration_processed_flag = true;

				const auto surface = cosm[fixture.GetUserData()];

				if (surface.dead()) {
					continue;
				}

				const auto surface_owner = surface.get_id();
				const auto penetrability = ::calc_penetrability(surface);

				const auto considered_p1 = fixture.penetrated_forward ? vec2(fixture.forward_point) : p1;

				if ((considered_p1 - p1).length() > walk_end_dist) {
					/* Beyond the character - will be walked next step. */
					continue;
				}

				const auto considered_p2 = [&]() {
					const auto exit = fixture.penetrated_backward ? vec2(fixture.backward_point) : p2;

					if ((exit - p1).length() > walk_end_dist) {
						return walk_end;
					}

					return exit;
				}();

				{
					/*
						Entered during this very step, rather than
						being the fixture the bullet is already in.
					*/
					const bool entered_now =
						fixture.penetrated_forward
						&& (considered_p1 - p1).length_sq() > PENETRATION_ENTRY_EPSILON_PX * PENETRATION_ENTRY_EPSILON_PX
					;

					/* Merely the next part of the wall the bullet is going through. */
					const bool continues_wall =
						previous_fixture != nullptr
						&& ::same_wall(*previous_fixture, fixture)
						&& (considered_p1 - previous_exit).dot(path_dir) <= PENETRATION_SEAM_TOLERANCE_PX
					;

					/* One bullet never hits the same entity twice in a row. */
					const bool same_entity = surface_owner == missile.last_penetrated_surface;

					if (entered_now && !continues_wall && !same_entity) {
						::on_missile_entered_next_surface(
							step,
							it,
							surface,
							fixture,
							considered_p1,
							path_dir
						);
					}

					missile.last_penetrated_surface = surface_owner;
					previous_fixture = std::addressof(fixture);
					previous_exit = considered_p2;
				}

				if (penetrability <= 0.0f) {
					expire_at(considered_p1);
					break;
				}
				else {
					const auto offset = considered_p2 - considered_p1;

					/*
						Discounted by the decals already covering this stretch.
						The gift over the bullet's lifetime is capped in total.
					*/
					const auto max_gift = ::calc_remaining_fatigue_gift(
						missile.starting_penetration_distance,
						missile.penetration_fatigue_gift_used
					);

					const auto cost = ::calc_penetration_cost_px(
						cosm,
						surface_owner,
						considered_p1,
						considered_p2,
						penetrability,
						max_gift,
						missile.when_fired.step
					);

					auto& remaining = missile.penetration_distance_remaining;
					const auto required = cost.cost;

					if (remaining > required) {
						missile.penetration_fatigue_gift_used += cost.gifted;

						remaining -= required;
						rigid_body.infer_damping();
					}
					else {
						/*
							The bullet dies inside. An exact walk here:
							a ratio would smear the decal tunnel's discount
							uniformly over the whole stretch.
						*/
						const auto reach = ::calc_penetration_reach_px(
							cosm,
							surface_owner,
							considered_p1,
							vec2(offset).normalize(),
							remaining,
							max_gift,
							penetrability,
							missile.when_fired.step
						);

						missile.penetration_fatigue_gift_used += reach.gifted;

						expire_at(considered_p1 + vec2(offset).set_length(std::min(reach.reach, offset.length())));
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

				/*
					A walk cut at a character has yet to reach the walls behind them -
					the next step resumes it from the cut.
				*/
				if (!any_intersection && !character_cut.has_value()) {
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

			missile.prev_tip_position = walk_end;
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

		if (surface_handle.dead() || missile_handle.dead()) {
			continue;
		}

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

		if (surface_handle.dead() || missile_handle.dead()) {
			continue;
		}

		missile_handle.dispatch_on_having_all<invariants::missile>([&](const auto& typed_missile) {
			auto& missile = typed_missile.template get<components::missile>();
			const auto& missile_def = typed_missile.template get<invariants::missile>();

			const auto info = missile_surface_info(typed_missile, surface_handle, step.get_settings().friendly_fire);

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
					missile.last_penetrated_surface = surface_handle.get_id();
					typed_missile.infer_colliders();
					typed_missile.template get<components::rigid_body>().set_velocity(result->impact_velocity);

					//const auto& clk = cosm.get_clock();
					//const auto& now = clk.now;
					//LOG("BEGAN step %x at %x with v=%x", now.step, result->transform_of_impact.pos, it.collider_impact_velocity);

					missile.prev_tip_position = result->transform_of_impact.pos;

					auto shifted_bullet_tr = result->transform_of_impact;

					auto vel = result->impact_velocity;
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

					const auto info = missile_surface_info(typed_melee, surface_handle, step.get_settings().friendly_fire);

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
						m.when_fired = typed_melee.when_last_transferred();
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

					if (dist_remaining != dist_starting && dist_starting > 0.0f) {
						considered_lifetime *= repro::sqrt(std::max(0.0f, dist_remaining / dist_starting));
					}

					const auto fuse_delay_steps = [&]() {
						const auto steps = considered_lifetime / delta.in_milliseconds();

						/* Negated so that a NaN takes the safe branch. */
						if (!(steps > 0.0f)) {
							return uint32_t(0);
						}

						return static_cast<uint32_t>(std::min(steps, 1e9f));
					}();
					const auto when_detonates = missile.when_fired.step + fuse_delay_steps;

					bool force_detonate = false;

					if (missile.force_detonate_in_ms >= 0.f) {
						const auto dt_ms = delta.in_milliseconds();

						if (missile.force_detonate_in_ms > dt_ms) {
							missile.force_detonate_in_ms -= dt_ms;
						}
						else {
							force_detonate = true;
						}
					}

					if (force_detonate || now.step >= when_detonates) {
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
					sender_capability && sender_capability.template has<components::sentience>() ? sender_capability : entity_handle::dead_handle(cosm)
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