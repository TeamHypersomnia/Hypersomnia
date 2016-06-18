#pragma once
#include "math/vec2.h"
#include "game/entity_id.h"

float rotation(entity_id of);
vec2 orientation(entity_id of);
vec2 position(entity_id of);
vec2 direction(entity_id to, entity_id from);
vec2 direction_norm(entity_id to, entity_id from);
bool is_physical(entity_id);
vec2 velocity(entity_id);
float speed(entity_id);
float distance(entity_id from, entity_id to);
float distance_sq(entity_id from, entity_id to);
