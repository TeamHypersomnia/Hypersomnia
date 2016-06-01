#pragma once
#include "math/vec2.h"
#include "entity_system/entity_id.h"

float rotation(augs::entity_id of);
vec2 orientation(augs::entity_id of);
vec2 position(augs::entity_id of);
vec2 direction(augs::entity_id to, augs::entity_id from);
vec2 direction_norm(augs::entity_id to, augs::entity_id from);
bool is_physical(augs::entity_id);
vec2 velocity(augs::entity_id);
float distance(augs::entity_id from, augs::entity_id to);
float distance_sq(augs::entity_id from, augs::entity_id to);