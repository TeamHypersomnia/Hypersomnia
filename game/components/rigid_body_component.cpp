#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "rigid_body_component.h"
#include "all_inferred_state_component.h"

#include <Box2D\Box2D.h>

#include "augs/graphics/renderer.h"
#include "fixtures_component.h"

#include "augs/math/vec2.h"
#include "game/systems_inferred/physics_system.h"
#include "augs/ensure.h"
#include "application/config_structs/debug_drawing_settings.h"

typedef components::rigid_body P;

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
	return handle.get_cosmos().systems_inferred.get<physics_system>().is_inferred_state_created_for_rigid_body(handle);
}

template<bool C>
maybe_const_ref_t<C, rigid_body_cache>& basic_physics_synchronizer<C>::get_cache() const {
	return handle.get_cosmos().systems_inferred.get<physics_system>().get_rigid_body_cache(handle);
}

void component_synchronizer<false, P>::reinference() const {
	handle.get_cosmos().partial_reinference<physics_system>(handle);
}

void component_synchronizer<false, P>::set_body_type(const rigid_body_type t) const {
	get_data().body_type = t;
	reinference();
}

void component_synchronizer<false, P>::set_activated(const bool flag) const {
	if (flag == get_data().activated) {
		return;
	}

	get_data().activated = flag;

	if (!flag) {
		get_data().velocity.reset();
		get_data().angular_velocity = 0.f;
	}

	reinference();
}

void component_synchronizer<false, P>::set_bullet_body(const bool flag) const {
	get_data().bullet = flag;
	reinference();
}

void component_synchronizer<false, P>::set_velocity(const vec2 pixels) const {
	get_data().velocity = to_meters(pixels);

	if (!is_constructed())
		return;

	get_cache().body->SetLinearVelocity(get_data().velocity);
}

void component_synchronizer<false, P>::set_angular_velocity(const float degrees) const {
	get_data().angular_velocity = DEG_TO_RAD<float> * degrees;

	if (!is_constructed())
		return;

	get_cache().body->SetLinearVelocity(get_data().velocity);
}

void component_synchronizer<false, P>::set_linear_damping(const float damping) const {
	get_data().linear_damping = damping;

	if (!is_constructed())
		return;

	get_cache().body->SetLinearDamping(damping);
}


void component_synchronizer<false, P>::set_angular_damping(const float damping) const {
	get_data().angular_damping = damping;

	if (!is_constructed())
		return;

	get_cache().body->SetAngularDamping(damping);
}


void component_synchronizer<false, P>::set_linear_damping_vec(const vec2 damping) const {
	get_data().linear_damping_vec = damping;

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

	const auto force = handle.get_cosmos().get_fixed_delta().in_seconds() * to_meters(pixels);
	const auto location = vec2(get_cache().body->GetWorldCenter() + to_meters(center_offset));

	get_cache().body->ApplyLinearImpulse(
		force, 
		location, 
		wake
	);

	get_data().angular_velocity = get_cache().body->GetAngularVelocity();
	get_data().velocity = get_cache().body->GetLinearVelocity();

	if (augs::renderer::get_current().debug.draw_forces && force.non_zero()) {
		auto& lines = augs::renderer::get_current().logic_lines;
		lines.draw_green(to_pixels(location) + to_pixels(force), to_pixels(location));
	}
}

void component_synchronizer<false, P>::apply_impulse(const vec2 pixels) const {
	apply_impulse(pixels, vec2(0, 0), true);
}

void component_synchronizer<false, P>::apply_impulse(const vec2 pixels, const vec2 center_offset, const bool wake) const {
	ensure(is_constructed());

	if (pixels.is_epsilon(2.f)) {
		return;
	}

	const vec2 force = to_meters(pixels);
	const vec2 location = get_cache().body->GetWorldCenter() + to_meters(center_offset);

	get_cache().body->ApplyLinearImpulse(force, location, true);
	get_data().angular_velocity = get_cache().body->GetAngularVelocity();
	get_data().velocity = get_cache().body->GetLinearVelocity();

	if (augs::renderer::get_current().debug.draw_forces && force.non_zero()) {
		auto& lines = augs::renderer::get_current().persistent_lines;
		lines.draw_green(to_pixels(location) + to_pixels(force), to_pixels(location));
	}
}

void component_synchronizer<false, P>::apply_angular_impulse(const float imp) const {
	ensure(is_constructed());
	get_cache().body->ApplyAngularImpulse(imp, true);
	get_data().angular_velocity = get_cache().body->GetAngularVelocity();
}

template<bool C>
float basic_physics_synchronizer<C>::get_mass() const {
	ensure(is_constructed());
	return get_cache().body->GetMass();
}

template<bool C>
float basic_physics_synchronizer<C>::get_angle() const {
	return get_data().sweep.a * RAD_TO_DEG<float>;
}

template<bool C>
float basic_physics_synchronizer<C>::get_angular_velocity() const {
	return get_data().angular_velocity * RAD_TO_DEG<float>;
}

template<bool C>
float basic_physics_synchronizer<C>::get_inertia() const {
	ensure(is_constructed());
	return get_cache().body->GetInertia();
}

template<bool C>
vec2 basic_physics_synchronizer<C>::get_position() const {
	return to_pixels(get_data().transform.p);
}

template<bool C>
vec2 basic_physics_synchronizer<C>::get_mass_position() const {
	ensure(is_constructed());
	return to_pixels(get_cache().body->GetWorldCenter());
}

template<bool C>
vec2 basic_physics_synchronizer<C>::velocity() const {
	return to_pixels(get_data().velocity);
}

template<bool C>
vec2 basic_physics_synchronizer<C>::get_world_center() const {
	ensure(is_constructed());
	return to_pixels(get_cache().body->GetWorldCenter());
}

template<bool C>
rigid_body_type basic_physics_synchronizer<C>::get_body_type() const {
	return get_data().body_type;
}

template<bool C>
bool basic_physics_synchronizer<C>::is_activated() const {
	return get_data().activated;
}

void component_synchronizer<false, P>::set_transform(const entity_id id) const {
	set_transform(handle.get_cosmos()[id].get_logic_transform());
}

void component_synchronizer<false, P>::set_transform(const components::transform& transform) const {
	get_data().set_transform(
		handle.get_cosmos().significant.meta.settings.si,
		transform
	);

	if (!is_constructed()) {
		return;
	}

	get_cache().body->m_xf = get_data().transform;
	get_cache().body->m_sweep = get_data().sweep;
}

template<bool C>
bool basic_physics_synchronizer<C>::test_point(const vec2 v) const {
	ensure(is_constructed());
	return get_cache().body->TestPoint(to_meters(v));
}

template class basic_physics_synchronizer<false>;
template class basic_physics_synchronizer<true>;