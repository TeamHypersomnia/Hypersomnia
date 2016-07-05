#include "fixtures_component.h"
#include "physics_component.h"
#include <Box2D\Dynamics\b2Fixture.h>
#include <Box2D/Box2D.h>
#include "ensure.h"
#include <algorithm>

#include <numeric>
#include <string>
#include <functional>

template<bool C>
using c = component_synchronizer<C, components::fixtures>;

template<bool C>
template <class = typename std::enable_if<!is_const>::type>
void component_synchronizer<C, components::fixtures>::set_owner_body(basic_entity_handle<C> owner) {
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
/*
fixtures& fixtures::operator=(const fixtures& f) {
	initialize_from_definition(f.get_definition());
}

fixtures::fixtures(const fixtures& f) {
	initialize_from_definition(f.get_definition());
}

fixtures::fixtures(const colliders_definition& def) {
	initialize_from_definition(def);
}

void fixtures::initialize_from_definition(const colliders_definition& def) {
	black = def;
	colliders_white_box::operator=(def);

	destroy_fixtures();

	if (should_fixtures_exist_now())
		build_fixtures();
}

colliders_definition fixtures::get_definition() const {
	colliders_definition output;
	output.colliders_black_box::operator=(black);
	output.colliders_white_box::operator=(*this);
	return output;
}

b2Body* fixtures::get_body() const {
	return black.owner_body.get<components::physics>().black_detail.body;
}

entity_id fixtures::get_body_entity() const {
	return black.owner_body;
}

vec2 fixtures::get_aabb_size() const {
	return get_aabb_rect().get_size();
}

augs::rects::ltrb<float> fixtures::get_aabb_rect() const {
	b2AABB aabb;
	aabb.lowerBound.Set(FLT_MAX, FLT_MAX);
	aabb.upperBound.Set(-FLT_MAX, -FLT_MAX);

	for (auto& c : black_detail.fixtures_per_collider)
		for (auto f : c)
			aabb.Combine(aabb, f->GetAABB(0));

	return augs::rects::ltrb<float>(aabb.lowerBound.x, aabb.lowerBound.y, aabb.upperBound.x, aabb.upperBound.y).scale(METERS_TO_PIXELSf);
}

size_t fixtures::get_num_colliders() const {
	return black_detail.fixtures_per_collider.size();
}

void fixtures::destroy_fixtures() {
	if (black_detail.fixtures_per_collider.empty())
		return;

	auto* body_previously_owning_fixtures = black_detail.fixtures_per_collider[0][0]->GetBody();

	for (auto& c : black_detail.fixtures_per_collider)
		for (auto f : c)
			body_previously_owning_fixtures->DestroyFixture(f);

	black_detail.fixtures_per_collider.clear();
}

void fixtures::build_fixtures() {
	ensure(black_detail.fixtures_per_collider.empty());


}

void fixtures::set_offset(offset_type t, components::transform off) {
	black.offsets_for_created_shapes[static_cast<int>(t)] = off;

	destroy_fixtures();

	if (should_fixtures_exist_now())
		build_fixtures();
}

void fixtures::rebuild_density(size_t index) {
	for (auto f : black_detail.fixtures_per_collider[index])
		f->SetDensity(black.colliders[index].density * black.colliders[index].density_multiplier);

	get_body()->ResetMassData();
}

void fixtures::set_density(float d, size_t index) {
	black.colliders[index].density = d;

	if (!syncable_black_box_exists())
		return;

	rebuild_density(index);
}

void fixtures::set_density_multiplier(float mult, size_t index) {
	black.colliders[index].density_multiplier = mult;

	if (!syncable_black_box_exists())
		return;

	rebuild_density(index);
}

void fixtures::set_activated(bool flag) {
	black.activated = flag;

	destroy_fixtures();

	if (should_fixtures_exist_now())
		build_fixtures();
}

void fixtures::set_friction(float fr, size_t index) {
	black.colliders[index].friction = fr;

	if (!syncable_black_box_exists())
		return;

	for (auto f : black_detail.fixtures_per_collider[index])
		f->SetFriction(fr);
}

void fixtures::set_restitution(float r, size_t index) {
	black.colliders[index].restitution = r;

	if (!syncable_black_box_exists())
		return;

	for (auto f : black_detail.fixtures_per_collider[index])
		f->SetRestitution(r);
}

void fixtures::set_owner_body(entity_id e) {
	if (black.owner_body.alive())
		remove_element(black.owner_body.get<components::physics>().black_detail.fixture_entities, black_detail.all_fixtures_owner);

	black.owner_body = e;

	if (e.alive())
		e.get<components::physics>().black_detail.fixture_entities.push_back(black_detail.all_fixtures_owner);

	destroy_fixtures();

	if (should_fixtures_exist_now())
		build_fixtures();
}

float fixtures::get_friction(size_t index) const {
	return black.colliders[index].friction;
}

float fixtures::get_restitution(size_t index) const {
	return black.colliders[index].restitution;
}

float fixtures::get_density(size_t index) const {
	return black.colliders[index].density;
}

float fixtures::get_density_multiplier(size_t index) const {
	return black.colliders[index].density_multiplier;
}

bool fixtures::is_activated() const {
	return black.activated;
}

entity_id fixtures::get_owner_body() const {
	return black.owner_body;
}

components::transform fixtures::get_offset(offset_type t) const {
	black.offsets_for_created_shapes[static_cast<int>(t)];
}

components::transform fixtures::get_total_offset() const {
	return std::accumulate(black.offsets_for_created_shapes.begin(), black.offsets_for_created_shapes.end(), components::transform());
}

bool fixtures::should_fixtures_exist_now() const {
	return black_detail.parent_system != nullptr && black.activated && black.owner_body.get<components::physics>().is_activated();
}

bool fixtures::syncable_black_box_exists() const {
	return black_detail.fixtures_per_collider.size() > 0;
}*/

template class component_synchronizer<false, components::fixtures>;
template class component_synchronizer<true, components::fixtures>;