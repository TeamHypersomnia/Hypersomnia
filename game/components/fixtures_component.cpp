#include "fixtures_component.h"
#include "inferred_state_component.h"
#include "rigid_body_component.h"
#include <Box2D\Dynamics\b2Fixture.h>
#include <Box2D/Box2D.h>
#include "augs/ensure.h"
#include <algorithm>

#include <numeric>
#include <string>
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/physics/b2Fixture_index_in_component.h"

typedef components::fixtures F;

void component_synchronizer<false, F>::set_owner_body(const entity_id owner_id) const {
	handle.set_owner_body(owner_id);
}

template<bool C>
maybe_const_ref_t<C, colliders_cache>& basic_fixtures_synchronizer<C>::get_cache() const {
	auto& cosmos = handle.get_cosmos();
	return cosmos.systems_inferred.get<physics_system>().get_colliders_cache(handle);
}

template<bool C>
ltrb basic_fixtures_synchronizer<C>::get_local_aabb() const {
	ensure("Not implemented" && false);
	return {};
	//std::vector<vec2> all_verts;
	//
	//for (const auto& s : component.colliders) {
	//	for (const auto& c : s.shape.convex_polys) {
	//		for (const auto v : c.vertices) {
	//			all_verts.push_back(v);
	//		}
	//	}
	//}
	//
	//return augs::get_aabb(all_verts);
}

template<bool C>
size_t basic_fixtures_synchronizer<C>::get_num_colliders() const {
	return component.colliders.size();
}

template<bool C>
const convex_partitioned_collider& basic_fixtures_synchronizer<C>::get_collider_data(const size_t i) const {
	return component.colliders[i];
;}

template<bool C>
bool basic_fixtures_synchronizer<C>::is_friction_ground() const {
	return component.is_friction_ground;
}

template<bool C>
bool basic_fixtures_synchronizer<C>::standard_collision_resolution_disabled() const {
	return component.disable_standard_collision_resolution;
}

template<bool C>
bool basic_fixtures_synchronizer<C>::can_driver_shoot_through() const {
	return component.can_driver_shoot_through;
}

void component_synchronizer<false, F>::set_offset(
	const colliders_offset_type t, 
	const components::transform off
) const {
	component.offsets_for_created_shapes[t] = off;
	reinference();
}

component_synchronizer<false, F>& component_synchronizer<false, F>::operator=(const F& f) {
	component = f;
	reinference();
	return *this;
}

void component_synchronizer<false, F>::reinference() const {
	handle.get_cosmos().partial_reinference<physics_system>(handle);
}

void component_synchronizer<false, F>::rebuild_density(const size_t index) const {
	for (auto f : get_cache().fixtures_per_collider[index]) {
		f->SetDensity(component.colliders[index].density * component.colliders[index].density_multiplier);
	}

	get_cache().fixtures_per_collider[0][0]->GetBody()->ResetMassData();
}

void component_synchronizer<false, F>::set_density(
	const float d, 
	const size_t index
) const {
	component.colliders[index].density = d;

	if (!is_constructed()) {
		return;
	}

	rebuild_density(index);
}

convex_poly_destruction_data& component_synchronizer<false, F>::get_modifiable_destruction_data(const b2Fixture_index_in_component indices) {
	return component.colliders[indices.collider_index].shape.get<convex_partitioned_shape>().convex_polys[indices.convex_shape_index].destruction;
}

void component_synchronizer<false, F>::set_density_multiplier(
	const float mult, 
	const size_t index
) const {
	component.colliders[index].density_multiplier = mult;

	if (!is_constructed()) {
		return;
	}

	rebuild_density(index);
}

void component_synchronizer<false, F>::set_activated(const bool flag) const {
	component.activated = flag;
	reinference();
}

void component_synchronizer<false, F>::set_friction(
	const float fr, 
	const size_t index
) const {
	component.colliders[index].friction = fr;

	if (!is_constructed()) {
		return;
	}

	for (auto f : get_cache().fixtures_per_collider[index]) {
		f->SetFriction(fr);
	}
}

void component_synchronizer<false, F>::set_restitution(
	const float r, 
	const size_t index
) const {
	component.colliders[index].restitution = r;

	if (!is_constructed()) {
		return;
	}

	for (auto f : get_cache().fixtures_per_collider[index]) {
		f->SetRestitution(r);
	}
}

void component_synchronizer<false, F>::set_physical_material(
	const physical_material_type m, 
	const size_t index
) const {
	component.colliders[index].material = m;
}

template<bool C>
float basic_fixtures_synchronizer<C>::get_friction(const size_t index) const {
	return component.colliders[index].friction;
}

template<bool C>
float basic_fixtures_synchronizer<C>::get_restitution(const size_t index) const {
	return component.colliders[index].restitution;
}

template<bool C>
float basic_fixtures_synchronizer<C>::get_density(const size_t index) const {
	return get_base_density(index) * component.colliders[index].density_multiplier;
}

template<bool C>
float basic_fixtures_synchronizer<C>::get_base_density(const size_t index) const {
	return component.colliders[index].density;
}

template<bool C>
float basic_fixtures_synchronizer<C>::get_density_multiplier(const size_t index) const {
	return component.colliders[index].density_multiplier;
}

template<bool C>
bool basic_fixtures_synchronizer<C>::is_activated() const {
	return component.activated;
}

template<bool C>
bool basic_fixtures_synchronizer<C>::is_constructed() const {
	return handle.get_cosmos().systems_inferred.get<physics_system>().is_constructed_colliders(handle);
}

template<bool C>
basic_entity_handle<C> basic_fixtures_synchronizer<C>::get_owner_body() const {
	return handle.get_owner_body();
}

template<bool C>
components::transform basic_fixtures_synchronizer<C>::get_offset(const colliders_offset_type t) const {
	return component.offsets_for_created_shapes[t];
}

template<bool C>
components::transform basic_fixtures_synchronizer<C>::get_total_offset() const {
	return std::accumulate(component.offsets_for_created_shapes.begin(), component.offsets_for_created_shapes.end(), components::transform());
}

components::transform components::fixtures::transform_around_body(
	const const_entity_handle fe, 
	const components::transform& body_transform
) {
	const auto total_offset = fe.get<components::fixtures>().get_total_offset();
	
	components::transform displaced = body_transform + total_offset;
	displaced.pos.rotate(body_transform.rotation, body_transform.pos);

	return displaced;
}

template class basic_fixtures_synchronizer<false>;
template class basic_fixtures_synchronizer<true>;