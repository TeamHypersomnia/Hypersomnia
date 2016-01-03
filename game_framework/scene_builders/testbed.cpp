#include "testbed.h"
#include "../archetypes/archetypes.h"

#include "entity_system/world.h"
#include "window_framework/window.h"

#include "game_framework/resources/manager.h"
#include "game_framework/assets/texture.h"
#include "game_framework/assets/atlas.h"

#include "game_framework/systems/input_system.h"
#include "game_framework/components/chase_component.h"

#include "utilities/file.h"

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
		
		resource_manager.create(assets::atlas_id::GAME_WORLD_ATLAS, resources::manager::atlas_creation_mode::FROM_ALL_TEXTURES);

		resource_manager.create(assets::shader_id::DEFAULT_VERTEX, L"hypersomnia/data/shaders/default.vsh", graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::DEFAULT_FRAGMENT, L"hypersomnia/data/shaders/default.fsh", graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::DEFAULT, assets::shader_id::DEFAULT_VERTEX, assets::shader_id::DEFAULT_FRAGMENT);

		auto background = world.create_entity();
		auto camera = world.create_entity();
		crosshair = world.create_entity();
		auto player = world.create_entity();

		archetypes::camera(camera, window_rect.w, window_rect.h);
		archetypes::sprite(background, vec2(0, 0), assets::texture_id::TEST_BACKGROUND);
		archetypes::wsad_player_crosshair(crosshair);
		archetypes::wsad_player(player, crosshair, camera);

		input_system::context active_context;
		active_context.map_event_to_intent(window::event::mousemotion, messages::intent_message::AIM);
		active_context.map_key_to_intent(window::event::keys::W, messages::intent_message::MOVE_FORWARD);
		active_context.map_key_to_intent(window::event::keys::S, messages::intent_message::MOVE_BACKWARD);
		active_context.map_key_to_intent(window::event::keys::A, messages::intent_message::MOVE_LEFT);
		active_context.map_key_to_intent(window::event::keys::D, messages::intent_message::MOVE_RIGHT);
		active_context.map_key_to_intent(window::event::keys::LMOUSE, messages::intent_message::SHOOT);

		world.get_system<input_system>().add_context(active_context);

		if (augs::file_exists(L"myinputslol.dat")) {
			world.get_system<input_system>().player.load_recording("myinputslol.dat");
			world.get_system<input_system>().player.replay();
		}
		else {
			world.get_system<input_system>().player.record("myinputslol.dat");
		}

		world.parent_overworld.configure_stepping(60.0, 5);
		world.parent_overworld.accumulator.set_time_multiplier(1.0);
	}

	void testbed::perform_logic_step(world& world) {
		auto inputs = world.get_message_queue<messages::unmapped_intent_message>();

		for (auto& it : inputs) {
			if (it.intent == messages::intent_message::SHOOT) {
				keep_drawing = it.pressed_flag;
				it.intent = messages::intent_message::AIM;
			}

			if (it.intent == messages::intent_message::AIM && keep_drawing) {
				auto ent = world.create_entity();
				archetypes::sprite_scalled(ent, crosshair->get<components::transform>().pos, vec2(10, 10), assets::texture_id::BLANK);
			}
		}

	}

	void testbed::draw(world& world) {

	}
}