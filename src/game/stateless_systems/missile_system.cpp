#include "missile_system.h"
#include "augs/math/steering.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/for_each_entity.h"

#include "game/messages/collision_message.h"
#include "game/messages/queue_deletion.h"
#include "game/messages/damage_message.h"

#include "game/detail/inventory/perform_transfer.h"
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

		if (type == std::nullopt) {
			continue;
		}

		const auto surface_handle = cosm[it.subject];
		const auto missile_handle = cosm[it.collider];

		missile_handle.dispatch_on_having_all<invariants::missile>([&](const auto& typed_missile) {
			auto& missile = typed_missile.template get<components::missile>();
			const auto& missile_def = typed_missile.template get<invariants::missile>();

			const auto info = missile_surface_info(typed_missile, surface_handle);

			if (const auto result = collide_missile_against_surface(
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
				missile.damage_charges_before_destruction = result->new_charges_value;
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
						m.damage_charges_before_destruction = 1;
						m.power_multiplier_of_sender = 1.f;
					}

					if (const auto result = collide_missile_against_surface(
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
		[&](const auto it) {
			auto& missile = it.template get<components::missile>();
			auto& missile_def = it.template get<invariants::missile>();
		
			const bool already_detonated_in_this_step = 
				missile.damage_charges_before_destruction == 0
			;

			if (missile_def.constrain_lifetime && !already_detonated_in_this_step) {
				if (!missile.when_fired.was_set()) {
					missile.when_fired = now;
				}
				else {
					const auto fuse_delay_steps = static_cast<unsigned>(missile_def.max_lifetime_ms / delta.in_milliseconds());
					const auto when_detonates = missile.when_fired.step + fuse_delay_steps;

					if (now.step >= when_detonates) {
						const auto current_tr = it.get_logic_transform();

						missile.saved_point_of_impact_before_death = current_tr;
						detonate_if(it, current_tr.pos, step);
						step.post_message(messages::queue_deletion(it));
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
					: cosm[get_closest_hostile(it, sender_attitude, detection_radius, filters[predefined_filter_type::FLYING_ITEM])]
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