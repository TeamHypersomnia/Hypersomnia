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

namespace archetypes {
	void wsad_player_legs(augs::entity_id e) {
		components::sprite sprite;
		components::render render;
		components::animation animation;
		components::transform transform;



	}

	void wsad_player_crosshair(augs::entity_id e) {
		components::sprite sprite;
		components::render render;
		components::transform transform;
		components::input input;
		components::crosshair crosshair;

		sprite.set(assets::texture_id::TEST_CROSSHAIR, pixel_32(255, 0, 0, 255));

		input.add(messages::intent_message::MOVE_CROSSHAIR);
		input.add(messages::intent_message::CROSSHAIR_PRIMARY_ACTION);
		input.add(messages::intent_message::CROSSHAIR_SECONDARY_ACTION);

		render.layer = 0;
		render.interpolate = false;

		crosshair.sensitivity.set(3, 3);

		e->add(render);
		e->add(sprite);
		e->add(transform);
		e->add(input);
		e->add(crosshair);
	}

	void wsad_player(augs::entity_id e, augs::entity_id crosshair_entity, augs::entity_id camera_entity) {
		components::sprite sprite;
		components::render render;
		components::transform transform;
		components::input input;
		components::movement movement;
		components::lookat lookat;

		movement.input_acceleration.set(400, 400);

		sprite.set(assets::texture_id::TEST_PLAYER, pixel_32(255, 255, 255, 255));

		input.add(messages::intent_message::MOVE_BACKWARD);
		input.add(messages::intent_message::MOVE_FORWARD);
		input.add(messages::intent_message::MOVE_LEFT);
		input.add(messages::intent_message::MOVE_RIGHT);

		render.layer = 1;

		lookat.target = crosshair_entity;
		lookat.look_mode = components::lookat::look_type::POSITION;

		e->add(transform);
		e->add(input);
		e->add(render);
		e->add(sprite);
		e->add(movement);
		e->add(lookat);
		
		components::camera::configure_camera_player_crosshair(camera_entity, e, crosshair_entity);
	}
}