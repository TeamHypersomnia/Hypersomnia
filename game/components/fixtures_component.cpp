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

void component_synchronizer<false, F>::set_owner_body(entity_id owner_id) const {
	auto& cosmos = handle.get_cosmos();
	auto owner = cosmos[owner_id];

	auto former_owner = cosmos[component.owner_body];

	if (former_owner.alive()) {
		remove_element(former_owner.get<components::physics>().component.fixture_entities, handle.get_id());
		cosmos.complete_resubstantialization(former_owner);
	}

	component.owner_body = owner;

	remove_element(owner.get<components::physics>().component.fixture_entities, handle.get_id());
	owner.get<components::physics>().component.fixture_entities.push_back(handle.get_id());

	cosmos.complete_resubstantialization(handle);
}

template<bool C>
maybe_const_ref_t<C, colliders_cache>& basic_fixtures_synchronizer<C>::get_cache() const {
	auto& cosmos = handle.get_cosmos();
	return cosmos.temporary_systems.get<physics_system>().get_colliders_cache(handle);
}

template<bool C>
vec2 basic_fixtures_synchronizer<C>::get_aabb_size() const {
	return get_aabb_rect().get_size();
}

template<bool C>
augs::rects::ltrb<float> basic_fixtures_synchronizer<C>::get_aabb_rect() const {
	std::vector<vec2> all_verts;
	
	for (auto& s : component.colliders) {
		for (auto& c : s.shape.convex_polys) {
			for (auto& v : c) {
				all_verts.push_back(v);
			}
		}
	}

	return augs::get_aabb(all_verts);
}

template<bool C>
size_t basic_fixtures_synchronizer<C>::get_num_colliders() const {
	return component.colliders.size();
}

template<bool C>
bool basic_fixtures_synchronizer<C>::is_friction_ground() const {
	return component.is_friction_ground;
}

template<bool C>
bool basic_fixtures_synchronizer<C>::standard_collision_resolution_disabled() const {
	return component.disable_standard_collision_resolution;
}

void component_synchronizer<false, F>::set_offset(colliders_offset_type t, components::transform off) const {
	component.offsets_for_created_shapes[static_cast<int>(t)] = off;
	complete_resubstantialization();
}

component_synchronizer<false, F>& component_synchronizer<false, F>::operator=(const F& f) {
	set_owner_body(f.owner_body);
	component = f;
	complete_resubstantialization();
	return *this;
}

void component_synchronizer<false, F>::rebuild_density(size_t index) const {
	for (auto f : get_cache().fixtures_per_collider[index])
		f->SetDensity(component.colliders[index].density * component.colliders[index].density_multiplier);

	get_cache().fixtures_per_collider[0][0]->GetBody()->ResetMassData();
}

void component_synchronizer<false, F>::set_density(float d, size_t index) const {
	component.colliders[index].density = d;

	if (!is_constructed())
		return;

	rebuild_density(index);
}

void component_synchronizer<false, F>::set_density_multiplier(float mult, size_t index) const {
	component.colliders[index].density_multiplier = mult;

	if (!is_constructed())
		return;

	rebuild_density(index);
}

void component_synchronizer<false, F>::set_activated(bool flag) const {
	component.activated = flag;
	complete_resubstantialization();
}

void component_synchronizer<false, F>::set_friction(float fr, size_t index) const {
	component.colliders[index].friction = fr;

	if (!is_constructed())
		return;

	for (auto f : get_cache().fixtures_per_collider[index])
		f->SetFriction(fr);
}

void component_synchronizer<false, F>::set_restitution(float r, size_t index) const {
	component.colliders[index].restitution = r;

	if (!is_constructed())
		return;

	for (auto f : get_cache().fixtures_per_collider[index])
		f->SetRestitution(r);
}

template<bool C>
float basic_fixtures_synchronizer<C>::get_friction(size_t index) const {
	return component.colliders[index].friction;
}

template<bool C>
float basic_fixtures_synchronizer<C>::get_restitution(size_t index) const {
	return component.colliders[index].restitution;
}

template<bool C>
float basic_fixtures_synchronizer<C>::get_density(size_t index) const {
	return component.colliders[index].density;
}

template<bool C>
float basic_fixtures_synchronizer<C>::get_density_multiplier(size_t index) const {
	return component.colliders[index].density_multiplier;
}

template<bool C>
bool basic_fixtures_synchronizer<C>::is_activated() const {
	return component.activated;
}

template<bool C>
bool basic_fixtures_synchronizer<C>::is_constructed() const {
	return handle.get_cosmos().temporary_systems.get<physics_system>().is_constructed_colliders(handle);
}

template<bool C>
basic_entity_handle<C> basic_fixtures_synchronizer<C>::get_owner_body() const {
	return handle.get_cosmos()[component.owner_body];
}

template<bool C>
components::transform basic_fixtures_synchronizer<C>::get_offset(colliders_offset_type t) const {
	return component.offsets_for_created_shapes[static_cast<int>(t)];
}

template<bool C>
components::transform basic_fixtures_synchronizer<C>::get_total_offset() const {
	return std::accumulate(component.offsets_for_created_shapes.begin(), component.offsets_for_created_shapes.end(), components::transform());
}

template class basic_fixtures_synchronizer<false>;
template class basic_fixtures_synchronizer<true>;