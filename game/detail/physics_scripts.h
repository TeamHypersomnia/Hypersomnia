#pragma once
#include <vector>
#include "math/vec2.h"
#include "game/entity_handle_declaration.h"
#include "game/entity_id.h"

void resolve_density_of_associated_fixtures(entity_handle);

entity_id get_owner_friction_field(const_entity_handle);
entity_id get_owner_body_entity(const_entity_handle sub_entity);
bool is_entity_physical(const_entity_handle);
bool are_connected_by_friction(const_entity_handle child, const_entity_handle parent);
std::vector<b2Vec2> get_world_vertices(const_entity_handle subject, bool meters = true, int fixture_num = 0);
