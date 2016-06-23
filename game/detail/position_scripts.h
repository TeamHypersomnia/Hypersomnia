#pragma once
#include "math/vec2.h"
#include "game/entity_handle_declaration.h"

float rotation(const_entity_handle of);
vec2 orientation(const_entity_handle of);
vec2 position(const_entity_handle of);
vec2 direction(const_entity_handle to, const_entity_handle from);
vec2 direction_norm(const_entity_handle to, const_entity_handle from);
bool is_physical(const_entity_handle);
vec2 velocity(const_entity_handle);
float speed(const_entity_handle);
float distance(const_entity_handle from, const_entity_handle to);
float distance_sq(const_entity_handle from, const_entity_handle to);

void set_velocity(entity_handle, vec2);