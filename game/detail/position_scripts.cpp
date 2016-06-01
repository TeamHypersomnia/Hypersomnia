#include "position_scripts.h"
#include "entity_system/entity.h"
#include "game/components/transform_component.h"

vec2 position(augs::entity_id e) {
	return e->get<components::transform>().pos;
}

vec2 direction(augs::entity_id a, augs::entity_id b) {
	return position(a) - position(b);
}

vec2 direction_norm(augs::entity_id a, augs::entity_id b) {
	return direction(a, b).normalize();
}

float distance_sq(augs::entity_id a, augs::entity_id b) {
	return direction(a, b).length_sq();
}

float distance(augs::entity_id a, augs::entity_id b) {
	return sqrt(distance_sq(a, b));
}
