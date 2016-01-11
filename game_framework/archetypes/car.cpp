#include "archetypes.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "game_framework/components/chase_component.h"
#include "game_framework/components/camera_component.h"
#include "game_framework/components/input_component.h"
#include "game_framework/components/crosshair_component.h"
#include "game_framework/components/sprite_component.h"
#include "game_framework/components/movement_component.h"
#include "game_framework/components/lookat_component.h"
#include "game_framework/components/animation_component.h"
#include "game_framework/components/animation_response_component.h"
#include "game_framework/components/physics_component.h"
#include "game_framework/components/children_component.h"
#include "game_framework/components/car_component.h"

#include "game_framework/game/body_helper.h"

#include "game_framework/globals/filters.h"

namespace prefabs {
	augs::entity_id create_car(augs::world& world, vec2 pos) {
		auto front = world.create_entity();
		{
			components::sprite sprite;
			components::render render;
			components::transform transform;
			transform.pos = pos;

			sprite.set(assets::texture_id::CAR_FRONT, augs::pixel_32(0, 255, 255));
			sprite.size.x = 100;
			sprite.size.y = 50;

			render.layer = components::render::render_layer::DYNAMIC_BODY;

			front->add(sprite);
			front->add(render);
			front->add(transform);
			
			helpers::physics_info info;
			info.from_renderable(front);

			info.filter = filters::dynamic_object();
			info.density = 0.6f;
			info.angular_damping = 5;
			info.angled_damping = true;
			info.linear_damping = 10;
			info.sensor = false;

			helpers::create_physics_component(info, front, b2_dynamicBody);
		}

		auto interior = world.create_entity();

		components::children interior_children;

		{
			components::sprite sprite;
			components::render render;
			components::transform transform;
			components::car car;
			components::chase chase;
			
			render.layer = components::render::render_layer::CAR_INTERIOR;

			chase.set_target(front);
			chase.chase_type = chase.ORBIT;
			chase.chase_rotation = true;

			sprite.set(assets::texture_id::CAR_INSIDE, augs::pixel_32(122, 122, 122, 255));
			sprite.size.x = 80;
			sprite.size.y = 70;
			
			chase.rotation_orbit_offset.set(0, front->get<components::sprite>().size.y / 2 + sprite.size.y / 2);

			interior->add(sprite);
			interior->add(render);
			interior->add(transform);
			interior->add(car);
			interior->add(chase);
		}

		interior_children.map_sub_entity(front, components::children::CAR_FRONT);
		interior->add(interior_children);

		return interior;
	}

}