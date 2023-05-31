#include "game/stateless_systems/portal_system.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/for_each_entity.h"
#include "game/detail/snap_interpolation_to_logical.h"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "game/messages/pure_color_highlight_message.h"
#include "game/detail/melee/like_melee.h"

auto calc_unit_progress_per_step(const augs::delta& dt, const real32 time_ms) {
	const auto seconds_to_complete = time_ms / 1000;
	const auto steps_to_complete = seconds_to_complete / dt.in_seconds();
	return (steps_to_complete == 0) ? 100.0f : (1.0f / steps_to_complete);
}

template <class H>
void do_shake(const H& typed_contacted_entity, const sentience_shake& shake_def) {
	const auto& cosm = typed_contacted_entity.get_cosmos();
	const auto& clk = cosm.get_clock();
	const auto now = clk.now;

	if (auto sentience = typed_contacted_entity.template find<components::sentience>()) {
		if (const auto sentience_def = typed_contacted_entity.template find<invariants::sentience>()) {
			shake_def.apply(
				now,
				*sentience_def,
				*sentience
			);
		}
	}
};

template <class H>
static bool skip_portal_effects(const H& typed_contacted_entity) {
	/* Skip all effects for shells and bullets to avoid effect spam */

	return 
		typed_contacted_entity.template has<components::remnant>()
		|| typed_contacted_entity.template has<components::missile>()
	;
}

template <class H>
static void play_begin_entering_effects(const logic_step step, const H& typed_contacted_entity, const components::portal& portal) {
	if (skip_portal_effects(typed_contacted_entity)) {
		return;
	}

	const auto predictability = predictable_only_by(typed_contacted_entity);

	const bool is_instant_entry = portal.enter_time_ms <= 0.f;

	if (is_instant_entry) {
		return;
	}

	{
		auto effect = portal.begin_entering_particles;

		effect.start(
			step,
			particle_effect_start_input::at_entity(typed_contacted_entity).face_velocity(),
			predictability
		);
	}

	{
		auto effect = portal.begin_entering_sound;

		effect.start(
			step,
			sound_effect_start_input::at_listener(typed_contacted_entity).face_velocity(),
			predictability
		);
	}

	if (portal.begin_entering_highlight_ms != 0.0f) {
		messages::pure_color_highlight h;
		h.subject = typed_contacted_entity.get_id();
		h.input.maximum_duration_seconds = portal.begin_entering_highlight_ms / 1000;
		h.input.starting_alpha_ratio = 1.0f;
		h.input.color = portal.rings_effect.value.inner_color;
		h.input.use_sqrt = false;
		step.post_message(h);
	}
};

template <class H>
static void play_enter_effects(const logic_step step, const H& typed_contacted_entity, const components::portal& portal) {
	if (skip_portal_effects(typed_contacted_entity)) {
		return;
	}

	do_shake(typed_contacted_entity, portal.enter_shake);

	const auto contacted_entity_transform = typed_contacted_entity.get_logic_transform();
	const auto predictability = predictable_only_by(typed_contacted_entity);

	{
		auto effect = portal.enter_particles;

		effect.start(
			step,
			particle_effect_start_input::fire_and_forget(contacted_entity_transform),
			predictability
		);
	}

	{
		auto effect = portal.enter_sound;

		auto start_input = sound_effect_start_input::fire_and_forget(contacted_entity_transform);

		const bool is_instant_exit = portal.travel_time_ms <= 0.f;

		if (!is_instant_exit) {
			/* 
				Set listener only if they have the chance to hear the sound.
				Otherwise it's pointless as they will instantly appear elsewhere.
			*/

			start_input.set_listener(typed_contacted_entity);
		}

		effect.start(
			step,
			start_input,
			predictability
		);
	}
};

template <class H>
static void play_exit_effects(const logic_step step, const H& typed_contacted_entity, const components::portal& portal) {
	/* Always post pure color highlight even for small objects, no problem with that */

	if (portal.begin_entering_highlight_ms != 0.0f) {
		messages::pure_color_highlight h;
		h.subject = typed_contacted_entity.get_id();
		h.input.maximum_duration_seconds = portal.exit_highlight_ms / 1000;
		h.input.starting_alpha_ratio = 1.0f;
		h.input.color = portal.rings_effect.value.inner_color;
		h.input.use_sqrt = false;
		step.post_message(h);
	}

	if (skip_portal_effects(typed_contacted_entity)) {
		return;
	}

	do_shake(typed_contacted_entity, portal.exit_shake);

	const bool is_instant_exit = portal.travel_time_ms <= 0.f;

	const auto predictability = is_instant_exit ? predictable_only_by(typed_contacted_entity) : always_predictable_v;

	{
		auto effect = portal.exit_particles;

		effect.start(
			step,
			particle_effect_start_input::at_entity(typed_contacted_entity).face_velocity(),
			predictability
		);
	}

	{
		auto effect = portal.exit_sound;

		effect.start(
			step,
			sound_effect_start_input::at_listener(typed_contacted_entity).face_velocity(),
			predictability
		);
	}
};

template <class H>
static void propagate_teleport_state_to_children(const H& typed_contacted_entity) {
	const auto& special = typed_contacted_entity.template get<components::rigid_body>().get_special();

	auto propagate = [&special](const auto child_item) {
		if (auto rigid = child_item.template find<components::rigid_body>()) {
			auto& s = rigid.get_special();

			s.teleport_progress = special.teleport_progress;
			s.teleport_progress_falloff_speed = special.teleport_progress_falloff_speed;
			s.inside_portal = special.inside_portal;
			s.teleport_decrease_opacity_to = special.teleport_decrease_opacity_to;
		}
	};

	typed_contacted_entity.for_each_contained_item_recursive(propagate);
}

void portal_system::finalize_portal_exit(const logic_step step, const entity_handle teleported, const bool reinfer_colliders) {
	auto& cosm = step.get_cosmos();

	teleported.dispatch_on_having_all<components::rigid_body>([&](const auto typed_contacted_entity) {
		auto rigid_body = typed_contacted_entity.template get<components::rigid_body>();
		auto& special = rigid_body.get_special();

		const auto portal_handle = cosm[special.inside_portal];

		if (portal_handle.dead()) {
			LOG("DEAD PORTAL");
			return;
		}

		const auto& portal = portal_handle.template get<components::portal>();

		const auto portal_exit = cosm[portal.portal_exit];

		const auto contacted_entity_transform = typed_contacted_entity.get_logic_transform();
		const auto portal_entry_transform = portal_handle.get_logic_transform();

		if (portal_exit.alive()) {
			if (const auto portal_exit_transform = portal_exit.find_logic_transform()) {
				if (const auto portal_exit_portal = portal_exit.template find<components::portal>()) {
					auto final_transform = transformr { 
						portal_exit_transform->pos,
						contacted_entity_transform.rotation
					};

					const auto radius = portal_exit.get_logical_size().smaller_side() / 2;

					rigid_body.restore_velocities();

					auto portal_exit_direction = portal_exit_transform->get_direction();

					auto considered_velocity = rigid_body.get_velocity();

					if (considered_velocity == vec2::zero) {
						considered_velocity = vec2(1.0f, 0.0f);
					}

					switch (portal_exit_portal->exit_direction) {
						case portal_exit_direction::PORTAL_DIRECTION:
							break;

						case portal_exit_direction::ENTERING_VELOCITY:
							portal_exit_direction = considered_velocity;
							portal_exit_direction.normalize();
							break;

						case portal_exit_direction::REVERSE_ENTERING_VELOCITY:
							portal_exit_direction = considered_velocity;
							portal_exit_direction.normalize();
							portal_exit_direction = -portal_exit_direction;
							break;

						default:
							break;
					};

					switch (portal_exit_portal->exit_position) {
						case portal_exit_position::PORTAL_CENTER_PLUS_ENTERING_OFFSET:
							final_transform.pos += contacted_entity_transform.pos - portal_entry_transform.pos;
							break;
						case portal_exit_position::PORTAL_BOUNDARY:
							final_transform.pos += portal_exit_direction * radius;
							break;
						default:
							break;
					}

					/*
						Safe to set transforms here.
						From b2Body::SetTransform docs:

						"Note: contacts are updated on the next call to b2World::Step."
					*/

					const auto impulses = portal_exit_portal->exit_impulses;

					const bool should_set_rotation_to_new_vel = 
						typed_contacted_entity.template has<components::missile>()
						|| ::is_like_thrown_melee(typed_contacted_entity)
					;

					if (const bool is_sentient = typed_contacted_entity.template has<components::sentience>()) {
						rigid_body.apply_linear(portal_exit_direction, impulses.character_exit_impulse);

						if (const auto movement = typed_contacted_entity.template find<components::movement>()) {
							//movement->linear_inertia_ms += impulses.character_exit_inertia_ms;
							movement->portal_inertia_ms += impulses.character_exit_inertia_ms;
						}
					}
					else {
						rigid_body.apply_linear(portal_exit_direction, impulses.object_exit_impulse);

						if (should_set_rotation_to_new_vel) {
							final_transform.rotation = rigid_body.get_velocity().degrees();
						}
						else {
							rigid_body.apply_angular(impulses.object_exit_angular_impulse);
						}
					}

					typed_contacted_entity.set_logic_transform(final_transform);
					::snap_interpolated_to(typed_contacted_entity, final_transform);

					play_exit_effects(step, typed_contacted_entity, *portal_exit_portal);

					special.teleport_progress = 0.0f;
					special.teleport_progress_falloff_speed = 0.0f;

					special.inside_portal.unset();

					const auto cooldown = portal_exit_portal->exit_cooldown_ms;

					if (cooldown > 0.0f) {
						special.dropped_or_created_cooldown.set(
							cooldown,
							cosm.get_timestamp()
						);

						special.during_cooldown_ignore_collision_with = portal_exit.get_id();
					}

					propagate_teleport_state_to_children(typed_contacted_entity);

					if (reinfer_colliders) {
						typed_contacted_entity.infer_item_physics_recursive();
					}
				}
			}
		}
	});
}

void portal_system::advance_portal_logic(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto dt = cosm.get_fixed_delta();
	const auto& clk = cosm.get_clock();

	thread_local std::vector<entity_id> to_refresh_colliders;
	to_refresh_colliders.clear();

	cosm.for_each_having<components::portal>(
		[&](const auto& typed_portal_handle) {
			const auto& portal = typed_portal_handle.template get<components::portal>();

			const auto portal_exit = cosm[portal.portal_exit];

			if (const bool disabled = portal.custom_filter.maskBits == 0) {
				/* Other non-portal area sensors might reuse this entity type. */
				return;
			}

			const bool is_instant_exit = portal.travel_time_ms <= 0.f;
			const auto rigid_body = typed_portal_handle.template get<components::rigid_body>();

			auto advance_portal_entering = [&](const auto typed_contacted_entity) {
				if (typed_contacted_entity.template has<components::portal>()) {
					/* Don't teleport portals */
					return;
				}

				auto contacted_rigid = typed_contacted_entity.template get<components::rigid_body>();
				auto& special = contacted_rigid.get_special();

				if (special.dropped_or_created_cooldown.lasts(clk)) {
					if (special.during_cooldown_ignore_collision_with == typed_portal_handle.get_id()) {
						return;
					}
				}

				auto enter_portal = [&]() {
					if (portal_exit.alive()) {
						if (const auto portal_exit_transform = portal_exit.find_logic_transform()) {
							if (const auto portal_exit_marker = portal_exit.template find<components::marker>()) {
								play_enter_effects(step, typed_contacted_entity, portal);

								/* 
									Now it's the responsibility of physics_system to call back finalize_portal_exit,
									once special.teleport_progress becomes >= 1 again.
								*/

								special.teleport_progress = 0.0f;
								special.teleport_progress_falloff_speed = -::calc_unit_progress_per_step(dt, portal.travel_time_ms);

								special.inside_portal = typed_portal_handle.get_id();

								contacted_rigid.backup_velocities();
								contacted_rigid.set_velocity(vec2::zero);

								return true;
							}
						}
					}

					return false;
				};

				if (!special.inside_portal.is_set()) {
					/* STATE: Entering portal. */
					special.teleport_progress_falloff_speed = ::calc_unit_progress_per_step(dt, portal.enter_time_ms);
					special.teleport_decrease_opacity_to = portal.decrease_opacity_to;

					/* 2 * to account for physics system decreasing it every step */
					const auto progress_added = 2 * special.teleport_progress_falloff_speed;

					if (special.teleport_progress <= 0.f) {
						special.teleport_progress = progress_added;
						play_begin_entering_effects(step, typed_contacted_entity, portal);
					}
					else {
						special.teleport_progress += progress_added;
					}

					if (special.teleport_progress >= 1.0f) {
						if (enter_portal()) {
							if (is_instant_exit) {
								/* Teleporting finished. Reset progress variables. */
								const bool should_reinfer = false;

								finalize_portal_exit(step, typed_contacted_entity, should_reinfer);
							}
							else {
								to_refresh_colliders.push_back(typed_contacted_entity.get_id());
							}
						}
						else { 
							/* 
								Entering failed due to wrongly set portal exit.

								Keep it at one, never teleporting.
								This will simply make the character stay invisible instead of trapping them.
							*/

							special.teleport_progress = 1.0f;
						}
					}
				}

				/*
					Update progresses of rigid bodies of children independently
					in case they get detached by e.g. dropping them.
				*/

				::propagate_teleport_state_to_children(typed_contacted_entity);
			};

			for (auto ce = rigid_body.get_contact_list(); ce != nullptr; ce = ce->next) {
				auto ct = ce->contact;

				if (ct == nullptr) {
					continue;
				}

				if (!ct->IsTouching()) {
					continue;
				}

				if (ce->other) {
					/* 
						Note this will always default to the owning transfer capability,
						since we take the other b2Body, not fixture.
					*/

					const auto contacted_entity = cosm[ce->other->GetUserData()];

					if (contacted_entity.dead()) {
						continue;
					}

					contacted_entity.template dispatch_on_having_all<components::rigid_body>(advance_portal_entering);
				}
			}
		}
	);

	for (auto& refresh : to_refresh_colliders) {
		cosm[refresh].infer_item_physics_recursive();
	}
}
