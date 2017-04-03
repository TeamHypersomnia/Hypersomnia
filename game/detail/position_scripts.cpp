#include "position_scripts.h"
#include "game/components/transform_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/fixtures_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/physics/physics_scripts.h"
#include "game/transcendental/cosmos.h"

vec2 position(const_entity_handle e) {
	return e.get_logic_transform().pos;
}

float rotation(const_entity_handle e) {
	return e.get_logic_transform().rotation;
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
	return e.get_effective_velocity();
}

float speed(const_entity_handle e) {
	return velocity(e).length();
}

bool is_entity_physical(const_entity_handle e) {
	return e.has<components::fixtures>() || e.has<components::rigid_body>();
}

float distance_sq(const_entity_handle a, const_entity_handle b) {
	return direction(a, b).length_sq();
}

float distance(const_entity_handle a, const_entity_handle b) {
	return sqrt(distance_sq(a, b));
}

void set_velocity(entity_handle h, vec2 v) {
	h.get<components::rigid_body>().set_velocity(v);
}