#include "position_scripts.h"
#include "entity_system/entity.h"
#include "game/components/transform_component.h"
#include "game/components/physics_component.h"

vec2 position(augs::entity_id e) {
	return e->get<components::transform>().pos;
}

float rotation(augs::entity_id e) {
	return e->get<components::transform>().rotation;
}

vec2 orientation(augs::entity_id of) {
	return vec2().set_from_degrees(rotation(of));
}

vec2 direction(augs::entity_id a, augs::entity_id b) {
	return position(a) - position(b);
}

vec2 direction_norm(augs::entity_id a, augs::entity_id b) {
	return direction(a, b).normalize();
}

vec2 velocity(augs::entity_id e) {
	return components::physics::get_owner_body_entity(e)->get<components::physics>().velocity();
}

bool is_physical(augs::entity_id e) {
	return components::physics::is_entity_physical(e);
}

float distance_sq(augs::entity_id a, augs::entity_id b) {
	return direction(a, b).length_sq();
}

float distance(augs::entity_id a, augs::entity_id b) {
	return sqrt(distance_sq(a, b));
}
