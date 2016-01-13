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
#include "game_framework/components/trigger_detector_component.h"
#include "game_framework/components/driver_component.h"

#include "game_framework/game/body_helper.h"

#include "game_framework/globals/filters.h"
#include "game_framework/globals/input_profiles.h"

namespace archetypes {
	void wsad_player_physics(augs::entity_id e) {
		helpers::physics_info info;
		info.from_renderable(e);

		info.filter = filters::controlled_character();
		info.density = 0.1;
		info.fixed_rotation = false;
		info.angular_damping = 5;
		info.angled_damping = true;
		info.air_resistance = 0.6;

		helpers::create_physics_component(info, e, b2_dynamicBody);

		components::movement& movement = e->get<components::movement>();

		movement.input_acceleration.set(5000, 5000);
		movement.max_accel_len = 5000;
		movement.max_speed_animation = 1000;
		movement.braking_damping = 18;
	}

	void wsad_player_legs(augs::entity_id legs, augs::entity_id player) {
		components::sprite sprite;
		components::render render;
		components::animation animation;
		components::transform transform;
	}

	void wsad_player_crosshair(augs::entity_id e) {
		components::sprite sprite;
		components::render render;
		components::transform transform;
		components::crosshair crosshair;

		sprite.set(assets::texture_id::TEST_CROSSHAIR, pixel_32(255, 0, 0, 255));

		render.layer = components::render::render_layer::CROSSHAIR;
		render.interpolate = false;

		crosshair.sensitivity.set(3, 3);

		e->add(render);
		e->add(sprite);
		e->add(transform);
		e->add(input_profiles::crosshair());
		e->add(crosshair);
	}

	void wsad_player(augs::entity_id e, augs::entity_id crosshair_entity, augs::entity_id camera_entity) {
		components::sprite sprite;
		components::render render;
		components::animation animation;
		components::animation_response animation_response;
		components::transform transform;
		components::movement movement;
		components::lookat lookat;
		components::children children;
		components::trigger_detector detector;
		components::driver driver;

		children.map_sub_entity(crosshair_entity, components::children::CHARACTER_CROSSHAIR);

		animation_response.response = assets::animation_response_id::TORSO_SET;

		movement.input_acceleration.set(400, 400);
		movement.add_animation_receiver(e, false);

		sprite.set(assets::texture_id::TEST_PLAYER, pixel_32(255, 255, 255, 255));

		render.layer = components::render::render_layer::CHARACTER;

		lookat.target = crosshair_entity;
		lookat.look_mode = components::lookat::look_type::POSITION;
		lookat.use_physical_motor = true;

		e->add(transform);
		e->add(input_profiles::character());
		e->add(render);
		e->add(animation);
		e->add(animation_response);
		e->add(sprite);
		e->add(movement);
		e->add(lookat);
		e->add(detector);
		e->add(driver);
		
		components::camera::configure_camera_player_crosshair(camera_entity, e, crosshair_entity);
	}
}