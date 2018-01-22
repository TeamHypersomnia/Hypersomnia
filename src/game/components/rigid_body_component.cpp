#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "rigid_body_component.h"
#include "all_inferred_state_component.h"

#include <Box2D/Box2D.h>

#include "fixtures_component.h"

#include "augs/math/vec2.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "augs/ensure.h"
#include "game/debug_drawing_settings.h"

template <bool C>
float basic_physics_synchronizer<C>::get_mass() const {
	return body().GetMass();
}

template <bool C>
float basic_physics_synchronizer<C>::get_degrees() const {
	return get_radians() * RAD_TO_DEG<float>;
}

template <bool C>
float basic_physics_synchronizer<C>::get_radians() const {
	return body().GetAngle();
}

template <bool C>
float basic_physics_synchronizer<C>::get_degree_velocity() const {
	return get_radian_velocity() * RAD_TO_DEG<float>;
}

template <bool C>
float basic_physics_synchronizer<C>::get_radian_velocity() const {
	return body().GetAngularVelocity();
}

template <bool C>
float basic_physics_synchronizer<C>::get_inertia() const {
	return body().GetInertia();
}

template <bool C>
vec2 basic_physics_synchronizer<C>::get_position() const {
	return to_pixels(body().GetPosition());
}

template <bool C>
components::transform basic_physics_synchronizer<C>::get_transform() const {
	return { get_position(), get_degrees() };
}

template <bool C>
vec2 basic_physics_synchronizer<C>::get_mass_position() const {
	return to_pixels(body().GetWorldCenter());
}

template <bool C>
vec2 basic_physics_synchronizer<C>::get_velocity() const {
	return to_pixels(body().GetLinearVelocity());
}

template <bool C>
vec2 basic_physics_synchronizer<C>::get_world_center() const {
	return to_pixels(body().GetWorldCenter());
}

template <bool C>
bool basic_physics_synchronizer<C>::test_point(const vec2 v) const {
	return body().TestPoint(b2Vec2(to_meters(v)));
}

template <bool C>
const b2Body& basic_physics_synchronizer<C>::body() const {
	return *get_cache().body.get(); 
}

using P = components::rigid_body;

components::rigid_body::rigid_body(
	const si_scaling si,
	const components::transform t
) {
	set_transform(si, t);
}

void components::rigid_body::set_transform(
	const si_scaling si,
	const components::transform& t
) {
	t.to_si_space(si).to_box2d_transforms(transform, sweep);
}

template<bool C>
bool basic_physics_synchronizer<C>::is_constructed() const {
	return get_cache().is_constructed();
}

template <bool C>
const rigid_body_cache& basic_physics_synchronizer<C>::get_cache() const {
	return handle.get_cosmos().get_solvable_inferred().physics.get_rigid_body_cache(handle);
}

template <bool C>
damping_info basic_physics_synchronizer<C>::calculate_damping_info(const definitions::rigid_body& def) const {
	damping_info damping = def.damping;

	if (const auto* const maybe_movement = handle.template find<components::movement>()) {
		const auto& movement = *maybe_movement; /* Convenience */

		const bool is_inert = movement.make_inert_for_ms > 0.f;

		if (is_inert) {
			damping.linear = 2;
		}
		else {
			damping.linear = movement.standard_linear_damping;
		}

		const auto requested_by_input = movement.get_force_requested_by_input();

		if (requested_by_input.non_zero()) {
			if (movement.was_sprint_effective) {
				if (!is_inert) {
					damping.linear /= 4;
				}
			}
		}

		/* the player feels less like a physical projectile if we brake per-axis */
		if (movement.enable_braking_damping && !(movement.make_inert_for_ms > 0.f)) {
			damping.linear_axis_aligned += vec2(
				requested_by_input.x_non_zero() ? 0.f : movement.braking_damping,
				requested_by_input.y_non_zero() ? 0.f : movement.braking_damping
			);
		}
	}

	return damping;
}

rigid_body_cache& component_synchronizer<false, P>::get_cache() const {
	return handle.get_cosmos().get_solvable_inferred({}).physics.get_rigid_body_cache(handle);
}

void component_synchronizer<false, P>::infer_caches() const {
	handle.get_cosmos().get_solvable_inferred({}).physics.infer_cache_for_rigid_body(handle);
}

void component_synchronizer<false, P>::set_velocity(const vec2 pixels) const {
	auto& v = get_raw_component().velocity;
	v = to_meters(pixels);

	if (!is_constructed()) {
		return;
	}

	get_cache().body->SetLinearVelocity(b2Vec2(v));
}

void component_synchronizer<false, P>::set_angular_velocity(const float degrees) const {
	auto& v = get_raw_component().angular_velocity;
	v = DEG_TO_RAD<float> * degrees;

	if (!is_constructed()) {
		return;
	}

	get_cache().body->SetAngularVelocity(v);
}

void component_synchronizer<false, P>::apply_force(const vec2 pixels) const {
	apply_force(pixels, vec2(0, 0), true);
}

void component_synchronizer<false, P>::apply_force(
	const vec2 pixels, 
	const vec2 center_offset, 
	const bool wake
) const {
	ensure(is_constructed());

	if (pixels.is_epsilon(2.f)) {
		return;
	}

	const auto body = get_cache().body.get();
	auto& data = get_raw_component();

	const auto force = handle.get_cosmos().get_fixed_delta().in_seconds() * to_meters(pixels);
	const auto location = vec2(body->GetWorldCenter() + b2Vec2(to_meters(center_offset)));

	body->ApplyLinearImpulse(
		b2Vec2(force), 
		b2Vec2(location), 
		wake
	);

	data.angular_velocity = body->GetAngularVelocity();
	data.velocity = body->GetLinearVelocity();

	if (DEBUG_DRAWING.draw_forces && force.non_zero()) {
		auto& lines = DEBUG_LOGIC_STEP_LINES;
		lines.emplace_back(green, to_pixels(location) + to_pixels(force), to_pixels(location));
	}
}

void component_synchronizer<false, P>::apply_impulse(const vec2 pixels) const {
	apply_impulse(pixels, vec2(0, 0), true);
}

void component_synchronizer<false, P>::apply_impulse(
	const vec2 pixels, 
	const vec2 center_offset, 
	const bool wake
) const {
	ensure(is_constructed());

	if (pixels.is_epsilon(2.f)) {
		return;
	}

	const auto body = get_cache().body.get();
	auto& data = get_raw_component();

	const vec2 force = to_meters(pixels);
	const vec2 location = vec2(body->GetWorldCenter()) + to_meters(center_offset);

	body->ApplyLinearImpulse(b2Vec2(force), b2Vec2(location), true);
	data.angular_velocity = body->GetAngularVelocity();
	data.velocity = body->GetLinearVelocity();

	if (DEBUG_DRAWING.draw_forces && force.non_zero()) {
		DEBUG_PERSISTENT_LINES.emplace_back(green, to_pixels(location) + to_pixels(force), to_pixels(location));
	}
}

void component_synchronizer<false, P>::apply_angular_impulse(const float imp) const {
	ensure(is_constructed());
	auto& body = *get_cache().body.get();
	auto& data = get_raw_component();

	body.ApplyAngularImpulse(imp, true);
	data.angular_velocity = body.GetAngularVelocity();
}


void component_synchronizer<false, P>::set_transform(const entity_id id) const {
	set_transform(handle.get_cosmos()[id].get_logic_transform());
}

void component_synchronizer<false, P>::set_transform(const components::transform& transform) const {
	auto& data = get_raw_component();

	data.set_transform(
		handle.get_cosmos().get_common_significant().si,
		transform
	);

	if (!is_constructed()) {
		return;
	}

	auto& body = *get_cache().body.get();

	if (!(body.m_xf == data.transform)) {
		body.m_xf = data.transform;
		body.m_sweep = data.sweep;

		b2BroadPhase* broadPhase = &body.m_world->m_contactManager.m_broadPhase;

		for (b2Fixture* f = body.m_fixtureList; f; f = f->m_next)
		{
			f->Synchronize(broadPhase, body.m_xf, body.m_xf);
		}
	}	
}


template class basic_physics_synchronizer<false>;
template class basic_physics_synchronizer<true>;