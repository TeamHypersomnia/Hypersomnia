#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "rigid_body_component.h"
#include "all_inferred_state_component.h"

#include <Box2D/Box2D.h>

#include "fixtures_component.h"

#include "augs/math/vec2.h"
#include "game/inferential_systems/physics_system.h"
#include "augs/ensure.h"
#include "game/debug_drawing_settings.h"

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
	return handle.get_cosmos().inferential.get<physics_system>().is_inferred_state_created_for_rigid_body(handle);
}

template<bool C>
maybe_const_ref_t<C, rigid_body_cache>& basic_physics_synchronizer<C>::get_cache() const {
	return handle.get_cosmos().inferential.get<physics_system>().get_rigid_body_cache(handle);
}

void component_synchronizer<false, P>::reinference() const {
	handle.get_cosmos().partial_reinference<physics_system>(handle);
}

void component_synchronizer<false, P>::set_body_type(const rigid_body_type t) const {
	get_raw_component().body_type = t;
	reinference();
}

void component_synchronizer<false, P>::set_activated(const bool flag) const {
	if (flag == get_raw_component().activated) {
		return;
	}

	get_raw_component().activated = flag;

	if (!flag) {
		get_raw_component().velocity.reset();
		get_raw_component().angular_velocity = 0.f;
	}

	reinference();
}

void component_synchronizer<false, P>::set_bullet_body(const bool flag) const {
	get_raw_component().bullet = flag;
	reinference();
}

void component_synchronizer<false, P>::set_velocity(const vec2 pixels) const {
	get_raw_component().velocity = to_meters(pixels);

	if (!is_constructed())
		return;

	get_cache().body->SetLinearVelocity(get_raw_component().velocity);
}

void component_synchronizer<false, P>::set_angular_velocity(const float degrees) const {
	get_raw_component().angular_velocity = DEG_TO_RAD<float> * degrees;

	if (!is_constructed())
		return;

	get_cache().body->SetLinearVelocity(get_raw_component().velocity);
}

void component_synchronizer<false, P>::set_linear_damping(const float damping) const {
	get_raw_component().linear_damping = damping;

	if (!is_constructed())
		return;

	get_cache().body->SetLinearDamping(damping);
}


void component_synchronizer<false, P>::set_angular_damping(const float damping) const {
	get_raw_component().angular_damping = damping;

	if (!is_constructed())
		return;

	get_cache().body->SetAngularDamping(damping);
}

void component_synchronizer<false, P>::set_linear_damping_vec(const vec2 damping) const {
	get_raw_component().linear_damping_vec = damping;

	if (!is_constructed())
		return;

	get_cache().body->SetLinearDampingVec(damping);
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

	const auto body = get_cache().body;
	auto& data = get_raw_component();

	const auto force = handle.get_cosmos().get_fixed_delta().in_seconds() * to_meters(pixels);
	const auto location = vec2(body->GetWorldCenter() + to_meters(center_offset));

	body->ApplyLinearImpulse(
		force, 
		location, 
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

	const auto body = get_cache().body;
	auto& data = get_raw_component();

	const vec2 force = to_meters(pixels);
	const vec2 location = body->GetWorldCenter() + to_meters(center_offset);

	body->ApplyLinearImpulse(force, location, true);
	data.angular_velocity = body->GetAngularVelocity();
	data.velocity = body->GetLinearVelocity();

	if (DEBUG_DRAWING.draw_forces && force.non_zero()) {
		DEBUG_PERSISTENT_LINES.emplace_back(green, to_pixels(location) + to_pixels(force), to_pixels(location));
	}
}

void component_synchronizer<false, P>::apply_angular_impulse(const float imp) const {
	ensure(is_constructed());
	const auto body = get_cache().body;
	auto& data = get_raw_component();

	body->ApplyAngularImpulse(imp, true);
	data.angular_velocity = body->GetAngularVelocity();
}

template<bool C>
float basic_physics_synchronizer<C>::get_mass() const {
	ensure(is_constructed());
	return get_cache().body->GetMass();
}

template<bool C>
float basic_physics_synchronizer<C>::get_angle() const {
	return get_raw_component().sweep.a * RAD_TO_DEG<float>;
}

template<bool C>
float basic_physics_synchronizer<C>::get_angular_velocity() const {
	return get_raw_component().angular_velocity * RAD_TO_DEG<float>;
}

template<bool C>
float basic_physics_synchronizer<C>::get_inertia() const {
	ensure(is_constructed());
	return get_cache().body->GetInertia();
}

template<bool C>
vec2 basic_physics_synchronizer<C>::get_position() const {
	return to_pixels(get_raw_component().transform.p);
}

template<bool C>
vec2 basic_physics_synchronizer<C>::get_mass_position() const {
	ensure(is_constructed());
	return to_pixels(get_cache().body->GetWorldCenter());
}

template<bool C>
vec2 basic_physics_synchronizer<C>::velocity() const {
	return to_pixels(get_raw_component().velocity);
}

template<bool C>
vec2 basic_physics_synchronizer<C>::get_world_center() const {
	ensure(is_constructed());
	return to_pixels(get_cache().body->GetWorldCenter());
}

template<bool C>
rigid_body_type basic_physics_synchronizer<C>::get_body_type() const {
	return get_raw_component().body_type;
}

template<bool C>
bool basic_physics_synchronizer<C>::is_activated() const {
	return get_raw_component().activated;
}

void component_synchronizer<false, P>::set_transform(const entity_id id) const {
	set_transform(handle.get_cosmos()[id].get_logic_transform());
}

void component_synchronizer<false, P>::set_transform(const components::transform& transform) const {
	const auto body = get_cache().body;
	auto& data = get_raw_component();

	data.set_transform(
		handle.get_cosmos().significant.meta.global.si,
		transform
	);

	if (!is_constructed()) {
		return;
	}

	body->m_xf = get_raw_component().transform;
	body->m_sweep = get_raw_component().sweep;
}

template<bool C>
bool basic_physics_synchronizer<C>::test_point(const vec2 v) const {
	ensure(is_constructed());
	return get_cache().body->TestPoint(to_meters(v));
}

template class basic_physics_synchronizer<false>;
template class basic_physics_synchronizer<true>;