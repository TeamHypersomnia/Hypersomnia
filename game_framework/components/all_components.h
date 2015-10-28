#pragma once
#include "behaviour_tree_component.h"
#include "visibility_component.h"
#include "pathfinding_component.h"
#include "animate_component.h"
#include "camera_component.h"
#include "chase_component.h"
#include "children_component.h"
#include "crosshair_component.h"
#include "damage_component.h"
#include "gun_component.h"
#include "input_component.h"
#include "lookat_component.h"
#include "movement_component.h"
#include "particle_emitter_component.h"
#include "particle_group_component.h"
#include "physics_component.h"
#include "steering_component.h"
#include "transform_component.h"
#include "render_component.h"

#include "misc/object_pool.h"

#define OBJECT_POOL_SLOT_COUNT(ccc, ...) __VA_ARGS__
#define OBJECT_POOL_CONSTRUCT(ccc, ...) augs::object_pool<components::##ccc##>(__VA_ARGS__)
#define OBJECT_POOL_TYPE(ccc, ...) augs::object_pool<components::##ccc##>
#define ALL_COMPONENTS(COMPONENT_NAME, ...) \
COMPONENT_NAME(animate, __VA_ARGS__), \
COMPONENT_NAME(behaviour_tree, __VA_ARGS__), \
COMPONENT_NAME(camera, __VA_ARGS__), \
COMPONENT_NAME(chase, __VA_ARGS__), \
COMPONENT_NAME(children, __VA_ARGS__), \
COMPONENT_NAME(crosshair, __VA_ARGS__), \
COMPONENT_NAME(damage, __VA_ARGS__), \
COMPONENT_NAME(gun, __VA_ARGS__), \
COMPONENT_NAME(input, __VA_ARGS__), \
COMPONENT_NAME(lookat, __VA_ARGS__), \
COMPONENT_NAME(movement, __VA_ARGS__), \
COMPONENT_NAME(particle_emitter, __VA_ARGS__), \
COMPONENT_NAME(particle_group, __VA_ARGS__), \
COMPONENT_NAME(pathfinding, __VA_ARGS__), \
COMPONENT_NAME(physics, __VA_ARGS__), \
COMPONENT_NAME(render, __VA_ARGS__), \
COMPONENT_NAME(steering, __VA_ARGS__), \
COMPONENT_NAME(transform, __VA_ARGS__), \
COMPONENT_NAME(visibility, __VA_ARGS__)