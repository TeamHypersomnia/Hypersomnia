#include "physics_component.h"

#include <Box2D\Box2D.h>

#include "graphics/renderer.h"
#include "fixtures_component.h"

#include "math/vec2.h"
#include "game/cosmos.h"
#include "game/temporary_systems/physics_system.h"
#include "ensure.h"
#include "game/entity_handle.h"

typedef components::physics P;

template<bool C>
bool component_synchronizer<C, P>::is_constructed() const {
	return handle.get_cosmos().temporary_systems.get<physics_system>().is_constructed_rigid_body(handle);
}

template<bool C>
typename maybe_const_ref<C, rigid_body_cache>::type& component_synchronizer<C, P>::get_cache() const {
	return handle.get_cosmos().temporary_systems.get<physics_system>().get_rigid_body_cache(handle);
}

template<bool C>
template <class = typename std::enable_if<!C>::type>
void component_synchronizer<C, P>::set_body_type(components::physics::type t) {
	component.body_type = t;
	complete_resubstantialization();
}

template<bool C>
template <class = typename std::enable_if<!C>::type>
void component_synchronizer<C, P>::set_activated(bool flag) {
	component.activated = flag;
	complete_resubstantialization();
}

template<bool C>
template <class = typename std::enable_if<!C>::type>
void component_synchronizer<C, P>::set_velocity(vec2 pixels) {
	component.velocity = pixels;

	if (!is_constructed())
		return;

	get_cache().body->SetLinearVelocity(pixels * PIXELS_TO_METERSf);
}

template<bool C>
template <class = typename std::enable_if<!C>::type>
void component_synchronizer<C, P>::set_linear_damping(float damping) {
	component.linear_damping = damping;

	if (!is_constructed())
		return;

	get_cache().body->SetLinearDamping(damping);
}

template<bool C>
template <class = typename std::enable_if<!C>::type>
void component_synchronizer<C, P>::set_angular_damping(float damping) {
	component.angular_damping = damping;

	if (!is_constructed())
		return;

	get_cache().body->SetAngularDamping(damping);
}

template<bool C>
template <class = typename std::enable_if<!C>::type>
void component_synchronizer<C, P>::set_linear_damping_vec(vec2 damping) {
	component.linear_damping_vec = damping;

	if (!is_constructed())
		return;

	get_cache().body->SetLinearDampingVec(damping);
}

template<bool C>
template <class = typename std::enable_if<!C>::type>
void component_synchronizer<C, P>::apply_force(vec2 pixels) {
	apply_force(pixels, vec2(0, 0), true);
}

template<bool C>
template <class = typename std::enable_if<!C>::type>
void component_synchronizer<C, P>::apply_force(vec2 pixels, vec2 center_offset, bool wake) {
	ensure(is_constructed());

	if (pixels.is_epsilon(2.f))
		return;

	vec2 force = pixels * PIXELS_TO_METERSf;
	vec2 location = get_cache().body->GetWorldCenter() + (center_offset * PIXELS_TO_METERSf);

	get_cache().body->ApplyForce(force, location, wake);

	if (renderer::get_current().debug_draw_forces && force.non_zero()) {
		auto& lines = renderer::get_current().logic_lines;
		lines.draw_green(location * METERS_TO_PIXELSf + force * METERS_TO_PIXELSf, location * METERS_TO_PIXELSf);
	}
}

template<bool C>
template <class = typename std::enable_if<!C>::type>
void component_synchronizer<C, P>::apply_impulse(vec2 pixels) {
	apply_impulse(pixels, vec2(0, 0), true);
}

template<bool C>
template <class = typename std::enable_if<!C>::type>
void component_synchronizer<C, P>::apply_impulse(vec2 pixels, vec2 center_offset, bool wake) {
	ensure(is_constructed());

	if (pixels.is_epsilon(2.f))
		return;

	vec2 force = pixels * PIXELS_TO_METERSf;
	vec2 location = get_cache().body->GetWorldCenter() + (center_offset * PIXELS_TO_METERSf);

	get_cache().body->ApplyLinearImpulse(force, location, true);

	if (renderer::get_current().debug_draw_forces && force.non_zero()) {
		auto& lines = renderer::get_current().logic_lines;
		lines.draw_green(location * METERS_TO_PIXELSf + force * METERS_TO_PIXELSf, location * METERS_TO_PIXELSf);
	}
}

template<bool C>
template <class = typename std::enable_if<!C>::type>
void component_synchronizer<C, P>::apply_angular_impulse(float imp) {
	ensure(is_constructed());
	get_cache().body->ApplyAngularImpulse(imp, true);
}

template<bool C>
float component_synchronizer<C, P>::get_mass() const {
	ensure(is_constructed());
	return get_cache().body->GetMass();
}

template<bool C>
float component_synchronizer<C, P>::get_angle() const {
	ensure(is_constructed());
	return get_cache().body->GetAngle() * RAD_TO_DEG;
}

template<bool C>
float component_synchronizer<C, P>::get_angular_velocity() const {
	ensure(is_constructed());
	return get_cache().body->GetAngularVelocity() * RAD_TO_DEG;
}

template<bool C>
float component_synchronizer<C, P>::get_inertia() const {
	ensure(is_constructed());
	return get_cache().body->GetInertia();
}

template<bool C>
vec2 component_synchronizer<C, P>::get_position() const {
	ensure(is_constructed());
	return METERS_TO_PIXELSf * get_cache().body->GetPosition();
}

template<bool C>
vec2 component_synchronizer<C, P>::get_mass_position() const {
	ensure(is_constructed());
	return METERS_TO_PIXELSf * get_cache().body->GetWorldCenter();
}

template<bool C>
vec2 component_synchronizer<C, P>::velocity() const {
	ensure(is_constructed());
	return vec2(get_cache().body->GetLinearVelocity()) * METERS_TO_PIXELSf;
}

template<bool C>
vec2 component_synchronizer<C, P>::get_world_center() const {
	ensure(is_constructed());
	return METERS_TO_PIXELSf * get_cache().body->GetWorldCenter();
}

template<bool C>
vec2 component_synchronizer<C, P>::get_aabb_size() const {
	ensure(is_constructed());
	b2AABB aabb;
	aabb.lowerBound.Set(FLT_MAX, FLT_MAX);
	aabb.upperBound.Set(-FLT_MAX, -FLT_MAX);

	b2Fixture* fixture = get_cache().body->GetFixtureList();

	while (fixture != nullptr) {
		aabb.Combine(aabb, fixture->GetAABB(0));
		fixture = fixture->GetNext();
	}

	return vec2(aabb.upperBound.x - aabb.lowerBound.x, aabb.upperBound.y - aabb.lowerBound.y);
}

template<bool C>
P::type component_synchronizer<C, P>::get_body_type() const {
	return component.body_type;
}

template<bool C>
bool component_synchronizer<C, P>::is_activated() const {
	return component.activated;
}

template<bool C>
template <class = typename std::enable_if<!C>::type>
void component_synchronizer<C, P>::set_transform(entity_id id) {
	set_transform(handle.get_cosmos()[id].get<components::transform>());
}

template<bool C>
template <class = typename std::enable_if<!C>::type>
void component_synchronizer<C, P>::set_transform(components::transform transform) {
	component.transform = transform;

	if (!is_constructed())
		return;

	get_cache().body->SetTransform(transform.pos * PIXELS_TO_METERSf, transform.rotation * DEG_TO_RAD);
}


template<bool C>
const std::vector<basic_entity_handle<C>>& component_synchronizer<C, P>::get_fixture_entities() const {
	return handle.get_cosmos().to_handle_vector(component.fixture_entities);
}

template<bool C>
bool component_synchronizer<C, P>::test_point(vec2 v) const {
	return get_cache().body->TestPoint(v * PIXELS_TO_METERSf);
}

template class component_synchronizer<false, P>;
template class component_synchronizer<true, P>;