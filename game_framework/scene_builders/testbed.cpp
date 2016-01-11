#include "testbed.h"
#include "../archetypes/archetypes.h"

#include "entity_system/world.h"
#include "window_framework/window.h"

#include "game_framework/resources/manager.h"
#include "game_framework/assets/texture.h"
#include "game_framework/assets/atlas.h"

#include "game_framework/systems/input_system.h"
#include "game_framework/components/chase_component.h"

#include "game_framework/messages/crosshair_intent_message.h"

#include "game_framework/messages/animation_response_message.h"

#include "utilities/file.h"
#include "misc/time.h"

using namespace augs;

namespace scene_builders {
	void testbed::initialize(world& world) {
		auto window_rect = window::glwindow::get_current()->get_screen_rect();

		resource_manager.destroy_everything();
		world.delete_all_entities();

		resource_manager.create(assets::texture_id::TEST_CROSSHAIR, std::wstring(L"hypersomnia/data/gfx/crosshair.png"));
		resource_manager.create(assets::texture_id::TEST_PLAYER, L"hypersomnia/data/gfx/walk_1.png");
		resource_manager.create(assets::texture_id::BLANK, L"hypersomnia/data/gfx/blank.png");
		resource_manager.create(assets::texture_id::TEST_BACKGROUND, L"hypersomnia/data/maps/snow_textures/snow3.png");
		resource_manager.create(assets::texture_id::CRATE, L"hypersomnia/data/gfx/crate.png");
		resource_manager.create(assets::texture_id::CAR_INSIDE, L"hypersomnia/data/gfx/crate2.png");
		resource_manager.create(assets::texture_id::CAR_FRONT, L"hypersomnia/data/gfx/crate2.png");

		resource_manager.create_sprites_indexed(
			assets::texture_id::TORSO_MOVING_FIRST,
			assets::texture_id::TORSO_MOVING_LAST,
			L"hypersomnia/data/gfx/walk");

		resource_manager.create(assets::atlas_id::GAME_WORLD_ATLAS, resources::manager::atlas_creation_mode::FROM_ALL_TEXTURES);

		resource_manager.create(assets::shader_id::DEFAULT_VERTEX, L"hypersomnia/data/shaders/default.vsh", graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::DEFAULT_FRAGMENT, L"hypersomnia/data/shaders/default.fsh", graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::DEFAULT, assets::shader_id::DEFAULT_VERTEX, assets::shader_id::DEFAULT_FRAGMENT);

		resource_manager.create(assets::animation_id::TORSO_MOVE,
			assets::texture_id::TORSO_MOVING_FIRST,
			assets::texture_id::TORSO_MOVING_LAST,
			20.0f);

		auto& player_response = resource_manager.create(assets::animation_response_id::TORSO_SET);
		player_response[messages::animation_response_message::MOVE] = assets::animation_id::TORSO_MOVE;

		auto background = world.create_entity();
		auto camera = world.create_entity();
		auto crosshair = world.create_entity();
		auto player = world.create_entity();
		auto crate = world.create_entity();
		auto crate2 = world.create_entity();

		auto car = prefabs::create_car(world, vec2(-200, 0));

		archetypes::camera(camera, window_rect.w, window_rect.h);
		archetypes::sprite(background, vec2(0, 0), assets::texture_id::TEST_BACKGROUND);
		
		archetypes::wsad_player_crosshair(crosshair);
		archetypes::wsad_player(player, crosshair, camera);

		archetypes::wsad_player_physics(player);

		archetypes::sprite_scalled(crate, vec2(100, 0), vec2i(100, 100), assets::texture_id::CRATE);
		archetypes::crate_physics(crate);

		archetypes::sprite_scalled(crate2, vec2(200, 0), vec2i(100, 100), assets::texture_id::CRATE);
		archetypes::crate_physics(crate2);

		input_system::context active_context;
		active_context.map_event_to_intent(window::event::raw_mousemotion, messages::intent_message::MOVE_CROSSHAIR);
		active_context.map_key_to_intent(window::event::keys::LSHIFT, messages::intent_message::SWITCH_LOOK);
		active_context.map_key_to_intent(window::event::keys::W, messages::intent_message::MOVE_FORWARD);
		active_context.map_key_to_intent(window::event::keys::S, messages::intent_message::MOVE_BACKWARD);
		active_context.map_key_to_intent(window::event::keys::A, messages::intent_message::MOVE_LEFT);
		active_context.map_key_to_intent(window::event::keys::D, messages::intent_message::MOVE_RIGHT);
		active_context.map_key_to_intent(window::event::keys::LMOUSE, messages::intent_message::CROSSHAIR_PRIMARY_ACTION);
		active_context.map_key_to_intent(window::event::keys::RMOUSE, messages::intent_message::CROSSHAIR_SECONDARY_ACTION);

		world.get_system<input_system>().add_context(active_context);

		world.parent_overworld.configure_stepping(60.0, 5);
		
		if (augs::file_exists(L"recorded.inputs")) {
			world.get_system<input_system>().raw_window_input_player.player.load_recording("recorded.inputs");
			world.get_system<input_system>().raw_window_input_player.player.replay();
		
			world.get_system<input_system>().crosshair_intent_player.player.load_recording("recorded_crosshair.inputs");
			world.get_system<input_system>().crosshair_intent_player.player.replay();
			
			world.parent_overworld.accumulator.set_time_multiplier(6.0);
		}
		else {
			augs::create_directory("sessions/" + augs::get_timestamp());

			world.get_system<input_system>().raw_window_input_player.player.record("sessions/" + augs::get_timestamp() + "/recorded.inputs");
			world.get_system<input_system>().crosshair_intent_player.player.record("sessions/" + augs::get_timestamp() + "/recorded_crosshair.inputs");
		}
	}

	void testbed::perform_logic_step(world& world) {
		auto inputs = world.get_message_queue<messages::crosshair_intent_message>();

		for (auto& it : inputs) {
			if (it.intent == messages::intent_message::CROSSHAIR_PRIMARY_ACTION) {
				keep_drawing = it.pressed_flag;
				it.intent = messages::intent_message::MOVE_CROSSHAIR;
			}

			if (it.intent == messages::intent_message::MOVE_CROSSHAIR && keep_drawing) {
				auto ent = world.create_entity();
				archetypes::sprite_scalled(ent, it.crosshair_world_pos, vec2(10, 10), assets::texture_id::BLANK);
			}
		}

	}

	void testbed::draw(world& world) {

	}
}