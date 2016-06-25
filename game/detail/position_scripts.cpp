#include "position_scripts.h"
#include "game/components/transform_component.h"
#include "game/components/physics_component.h"
#include "game/entity_handle.h"
#include "game/detail/physics_scripts.h"

vec2 position(const_entity_handle e) {
	return e.get<components::transform>().pos;
}

float rotation(const_entity_handle e) {
	return e.get<components::transform>().rotation;
}

vec2 orientation(const_entity_handle of) {
	return vec2().set_from_degrees(rotation(of));
}

vec2 direction(const_entity_handle a, const_entity_handle b) {
	return position(a) - position(b);
}

vec2 direction_norm(const_entity_handle a, const_entity_handle b) {
	return direction(a, b).normalize();
}

vec2 velocity(const_entity_handle e) {
	return e.make_handle(e.get_owner_body_entity()).get<components::physics>().velocity();
}

float speed(const_entity_handle e) {
	return velocity(e).length();
}

bool is_physical(const_entity_handle e) {
	return is_entity_physical(e);
}

float distance_sq(const_entity_handle a, const_entity_handle b) {
	return direction(a, b).length_sq();
}

float distance(const_entity_handle a, const_entity_handle b) {
	return sqrt(distance_sq(a, b));
}

void set_velocity(entity_handle h, vec2 v) {
	h.get<components::physics>().set_velocity(v);
}