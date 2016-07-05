#include "fixtures_component.h"
#include "physics_component.h"
#include <Box2D\Dynamics\b2Fixture.h>
#include <Box2D/Box2D.h>
#include "ensure.h"
#include <algorithm>

#include <numeric>
#include <string>
#include <functional>
#include "game/cosmos.h"

typedef components::fixtures F;

template<bool C>
template <class = typename std::enable_if<!is_const>::type>
void component_synchronizer<C, F>::set_owner_body(basic_entity_handle<C> owner) {
	auto& cosmos = owner.get_cosmos();
	auto former_owner = cosmos[component.owner_body];

	if (former_owner.alive()) {
		remove_element(former_owner.get<components::physics>().get_data().fixture_entities, handle);
		cosmos.complete_resubstantialization(former_owner);
	}

	component.owner_body = owner;

	remove_element(owner.get<components::physics>().get_data().fixture_entities, handle);
	owner.get<components::physics>().get_data().fixture_entities.push_back(handle);

	cosmos.complete_resubstantialization(handle);
}

template<bool C>
typename maybe_const_ref<C, colliders_cache>::type& component_synchronizer<C, F>::get_cache() const {
	auto& cosmos = handle.get_cosmos();
	return cosmos.temporary_systems.get<physics_system>().get_colliders_cache(handle);
}

template<bool C>
vec2 component_synchronizer<C, F>::get_aabb_size() const {
	return get_aabb_rect().get_size();
}

template<bool C>
augs::rects::ltrb<float> component_synchronizer<C, F>::get_aabb_rect() const {
	b2AABB aabb;
	aabb.lowerBound.Set(FLT_MAX, FLT_MAX);
	aabb.upperBound.Set(-FLT_MAX, -FLT_MAX);

	for (auto& c : get_cache().fixtures_per_collider)
		for (auto f : c)
			aabb.Combine(aabb, f->GetAABB(0));

	return augs::rects::ltrb<float>(aabb.lowerBound.x, aabb.lowerBound.y, aabb.upperBound.x, aabb.upperBound.y).scale(METERS_TO_PIXELSf);
}

template<bool C>
size_t component_synchronizer<C, F>::get_num_colliders() const {
	return get_cache().fixtures_per_collider.size();
}

template<bool C>
template <class = typename std::enable_if<!is_const>::type>
void component_synchronizer<C, F>::set_offset(F::offset_type t, components::transform off) {
	component.offsets_for_created_shapes[static_cast<int>(t)] = off;
	complete_resubstantialization();
}

template<bool C>
template <class = typename std::enable_if<!is_const>::type>
void component_synchronizer<C, F>::rebuild_density(size_t index) {
	for (auto f : get_cache().fixtures_per_collider[index])
		f->SetDensity(component.colliders[index].density * component.colliders[index].density_multiplier);

	get_cache().fixtures_per_collider[0][0]->GetBody()->ResetMassData();
}

template<bool C>
template <class = typename std::enable_if<!is_const>::type>
void component_synchronizer<C, F>::set_density(float d, size_t index) {
	component.colliders[index].density = d;

	if (!is_constructed())
		return;

	rebuild_density(index);
}

template<bool C>
template <class = typename std::enable_if<!is_const>::type>
void component_synchronizer<C, F>::set_density_multiplier(float mult, size_t index) {
	component.colliders[index].density_multiplier = mult;

	if (!is_constructed())
		return;

	rebuild_density(index);
}

template<bool C>
template <class = typename std::enable_if<!is_const>::type>
void component_synchronizer<C, F>::set_activated(bool flag) {
	component.activated = flag;
	complete_resubstantialization();
}

template<bool C>
template <class = typename std::enable_if<!is_const>::type>
void component_synchronizer<C, F>::set_friction(float fr, size_t index) {
	component.colliders[index].friction = fr;

	if (!is_constructed())
		return;

	for (auto f : get_cache().fixtures_per_collider[index])
		f->SetFriction(fr);
}

template<bool C>
template <class = typename std::enable_if<!is_const>::type>
void component_synchronizer<C, F>::set_restitution(float r, size_t index) {
	component.colliders[index].restitution = r;

	if (!is_constructed())
		return;

	for (auto f : get_cache().fixtures_per_collider[index])
		f->SetRestitution(r);
}

template<bool C>
float component_synchronizer<C, F>::get_friction(size_t index) const {
	return component.colliders[index].friction;
}

template<bool C>
float component_synchronizer<C, F>::get_restitution(size_t index) const {
	return component.colliders[index].restitution;
}

template<bool C>
float component_synchronizer<C, F>::get_density(size_t index) const {
	return component.colliders[index].density;
}

template<bool C>
float component_synchronizer<C, F>::get_density_multiplier(size_t index) const {
	return component.colliders[index].density_multiplier;
}

template<bool C>
bool component_synchronizer<C, F>::is_activated() const {
	return component.activated;
}

template<bool C>
bool component_synchronizer<C, F>::is_constructed() const {
	return handle.get_cosmos().temporary_systems.get<physics_system>().is_constructed_colliders(handle);
}

template<bool C>
basic_entity_handle<C> component_synchronizer<C, F>::get_owner_body() const {
	return handle.get_cosmos()[component.owner_body];
}

template<bool C>
components::transform component_synchronizer<C, F>::get_offset(F::offset_type t) const {
	component.offsets_for_created_shapes[static_cast<int>(t)];
}

template<bool C>
components::transform component_synchronizer<C, F>::get_total_offset() const {
	return std::accumulate(component.offsets_for_created_shapes.begin(), component.offsets_for_created_shapes.end(), components::transform());
}

template class component_synchronizer<false, F>;
template class component_synchronizer<true, F>;