#include "physics_scripts.h"
#include "game/components/physics_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/item_component.h"
#include "game/components/driver_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/detail/inventory_slot_handle.h"
#include "position_scripts.h"

void resolve_density_of_associated_fixtures(entity_handle id) {
	auto& cosmos = id.get_cosmos();

	if (id.has<components::physics>()) {
		const auto& entities = id.get<components::physics>().get_fixture_entities();

		for (auto& f : entities) {
			if (f != id)
				resolve_density_of_associated_fixtures(f);
		}
	}

	auto& fixtures = id.get<components::fixtures>();

	float density_multiplier = 1.f;

	auto* item = id.find<components::item>();

	if (item != nullptr && cosmos[item->current_slot].alive() && cosmos[item->current_slot].should_item_inside_keep_physical_body())
		density_multiplier *= cosmos[item->current_slot].calculate_density_multiplier_due_to_being_attached();

	auto owner_body = id.get_owner_body();
	auto* driver = owner_body.find<components::driver>();

	if (driver) {
		if (cosmos[driver->owned_vehicle].alive()) {
			density_multiplier *= driver->density_multiplier_while_driving;
		}
	}

	for (size_t i = 0; i < fixtures.get_num_colliders(); ++i)
		fixtures.set_density_multiplier(density_multiplier, i);
}

bool are_connected_by_friction(const_entity_handle child, const_entity_handle parent) {
	auto& cosmos = child.get_cosmos();

	if (is_entity_physical(child) && is_entity_physical(parent)) {
		bool matched_ancestor = false;

		entity_id parent_body_entity = parent.get_owner_body();
		entity_id childs_ancestor_entity = child.get_owner_body().get_owner_friction_ground();

		while (cosmos[childs_ancestor_entity].alive()) {
			if (childs_ancestor_entity == parent_body_entity) {
				matched_ancestor = true;
				break;
			}

			childs_ancestor_entity = cosmos[childs_ancestor_entity].get_owner_friction_ground();
		}

		if (matched_ancestor)
			return true;
	}

	return false;
}

std::vector<b2Vec2> get_world_vertices(const_entity_handle subject, bool meters, int fixture_num) {
	std::vector<b2Vec2> output;

	auto& b = subject.get<components::physics>();

	const auto& verts = subject.get<components::fixtures>().get_data().colliders[0].shape.convex_polys[fixture_num].vertices;

	/* for every vertex in given fixture's shape */
	for (auto& v : verts) {
		auto position = b.get_position();
		/* transform angle to degrees */
		auto rotation = b.get_angle();

		/* transform vertex to current entity's position and rotation */
		vec2 out_vert = (vec2(v).rotate(rotation, b2Vec2(0, 0)) + position);

		if (meters) out_vert *= PIXELS_TO_METERSf;

		output.push_back(out_vert);
	}

	return output;
}
