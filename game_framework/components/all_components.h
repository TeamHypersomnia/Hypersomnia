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

//#include "misc/object_pool.h"

#define REGISTER_COMPONENT_TYPE(ccc, object) object.register_component<##ccc##>()
//#define OBJECT_POOL_SLOT_COUNT(ccc, ...) __VA_ARGS__
//#define OBJECT_POOL_CONSTRUCT(ccc, ...) augs::object_pool<##ccc##>(__VA_ARGS__)
//#define OBJECT_POOL_TYPE(ccc, ...) augs::object_pool<##ccc##>
#define ALL_COMPONENTS(COMPONENT_NAME, ...) \
COMPONENT_NAME(components::animate, __VA_ARGS__), \
COMPONENT_NAME(components::behaviour_tree, __VA_ARGS__), \
COMPONENT_NAME(components::camera, __VA_ARGS__), \
COMPONENT_NAME(components::chase, __VA_ARGS__), \
COMPONENT_NAME(components::children, __VA_ARGS__), \
COMPONENT_NAME(components::crosshair, __VA_ARGS__), \
COMPONENT_NAME(components::damage, __VA_ARGS__), \
COMPONENT_NAME(components::gun, __VA_ARGS__), \
COMPONENT_NAME(components::input, __VA_ARGS__), \
COMPONENT_NAME(components::lookat, __VA_ARGS__), \
COMPONENT_NAME(components::movement, __VA_ARGS__), \
COMPONENT_NAME(components::particle_emitter, __VA_ARGS__), \
COMPONENT_NAME(components::particle_group, __VA_ARGS__), \
COMPONENT_NAME(components::pathfinding, __VA_ARGS__), \
COMPONENT_NAME(components::physics, __VA_ARGS__), \
COMPONENT_NAME(components::render, __VA_ARGS__), \
COMPONENT_NAME(components::steering, __VA_ARGS__), \
COMPONENT_NAME(components::transform, __VA_ARGS__), \
COMPONENT_NAME(components::visibility, __VA_ARGS__)