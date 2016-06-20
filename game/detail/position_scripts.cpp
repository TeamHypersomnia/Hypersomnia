#include "position_scripts.h"
#include "game/entity_id.h"
#include "game/components/transform_component.h"
#include "game/components/physics_component.h"

vec2 position(entity_id e) {
	return e.get<components::transform>().pos;
}

float rotation(entity_id e) {
	return e.get<components::transform>().rotation;
}

vec2 orientation(entity_id of) {
	return vec2().set_from_degrees(rotation(of));
}

vec2 direction(entity_id a, entity_id b) {
	return position(a) - position(b);
}

vec2 direction_norm(entity_id a, entity_id b) {
	return direction(a, b).normalize();
}

vec2 velocity(entity_id e) {
	return components::physics::get_owner_body_entity(e).get<components::physics>().velocity();
}

float speed(entity_id e) {
	return velocity(e).length();
}

bool is_physical(entity_id e) {
	return components::physics::is_entity_physical(e);
}

float distance_sq(entity_id a, entity_id b) {
	return direction(a, b).length_sq();
}

float distance(entity_id a, entity_id b) {
	return sqrt(distance_sq(a, b));
}
