#include "one_entity.h"
#include "../ingredients/ingredients.h"

#include "entity_system/world.h"
#include "window_framework/window.h"

#include "game_framework/resources/manager.h"
#include "game_framework/assets/texture.h"
#include "game_framework/assets/atlas.h"

#include "game_framework/systems/input_system.h"
#include "game_framework/systems/render_system.h"
#include "game_framework/components/position_copying_component.h"

#include "game_framework/messages/crosshair_intent_message.h"

#include "game_framework/messages/animation_response_message.h"

#include "rendering_scripts/testbed_rendering.h"

#include "augs/file.h"
#include "misc/time.h"

using namespace augs;

namespace scene_builders {
	void one_entity::initialize(world& world) {
		auto window_rect = window::glwindow::get_current()->get_screen_rect();

		resource_manager.destroy_everything();
		world.delete_all_entities();

		resource_manager.create(assets::texture_id::TEST_CROSSHAIR, std::wstring(L"hypersomnia/gfx/crosshair.png"));
		resource_manager.create(assets::texture_id::TEST_PLAYER, L"hypersomnia/gfx/walk_1.png");
		resource_manager.create(assets::texture_id::BLANK, L"hypersomnia/gfx/blank.png");
		resource_manager.create(assets::texture_id::TEST_BACKGROUND, L"hypersomnia/gfx/snow_textures/snow3.png");
		resource_manager.create(assets::texture_id::CRATE, L"hypersomnia/gfx/crate.png");
		resource_manager.create(assets::texture_id::CAR_INSIDE, L"hypersomnia/gfx/crate2.png");
		resource_manager.create(assets::texture_id::CAR_FRONT, L"hypersomnia/gfx/crate2.png");

		resource_manager.create_sprites_indexed(
			assets::texture_id::TORSO_MOVING_FIRST,
			assets::texture_id::TORSO_MOVING_LAST,
			L"hypersomnia/gfx/walk");

		resource_manager.create(assets::atlas_id::GAME_WORLD_ATLAS, resources::manager::atlas_creation_mode::FROM_ALL_TEXTURES);

		resource_manager.create(assets::shader_id::DEFAULT_VERTEX, L"hypersomnia/shaders/default.vsh", graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::DEFAULT_FRAGMENT, L"hypersomnia/shaders/default.fsh", graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::DEFAULT, assets::shader_id::DEFAULT_VERTEX, assets::shader_id::DEFAULT_FRAGMENT);

		resource_manager.create(assets::animation_id::TORSO_MOVE,
			assets::texture_id::TORSO_MOVING_FIRST,
			assets::texture_id::TORSO_MOVING_LAST,
			20.0f);

		auto& player_response = resource_manager.create(assets::animation_response_id::TORSO_SET);
		player_response[messages::animation_response_message::MOVE] = assets::animation_id::TORSO_MOVE;

		auto camera = world.create_entity();

		ingredients::camera(camera, window_rect.w, window_rect.h);

		auto bg_size = assets::get_size(assets::texture_id::TEST_BACKGROUND);

		auto background = world.create_entity();
		ingredients::sprite(background, vec2(500, 0), assets::texture_id::TEST_BACKGROUND);
		
		input_system::context active_context;
		active_context.map_event_to_intent(window::event::raw_mousemotion, messages::intent_message::MOVE_CROSSHAIR);
		active_context.map_key_to_intent(window::event::keys::LSHIFT, messages::intent_message::SWITCH_LOOK);
		active_context.map_key_to_intent(window::event::keys::W, messages::intent_message::MOVE_FORWARD);
		active_context.map_key_to_intent(window::event::keys::S, messages::intent_message::MOVE_BACKWARD);
		active_context.map_key_to_intent(window::event::keys::A, messages::intent_message::MOVE_LEFT);
		active_context.map_key_to_intent(window::event::keys::D, messages::intent_message::MOVE_RIGHT);
		active_context.map_key_to_intent(window::event::keys::LMOUSE, messages::intent_message::CROSSHAIR_PRIMARY_ACTION);
		active_context.map_key_to_intent(window::event::keys::RMOUSE, messages::intent_message::CROSSHAIR_SECONDARY_ACTION);
		active_context.map_key_to_intent(window::event::keys::E, messages::intent_message::RELEASE_CAR);
		active_context.map_key_to_intent(window::event::keys::E, messages::intent_message::PRESS_TRIGGER);
		active_context.map_key_to_intent(window::event::keys::SPACE, messages::intent_message::HAND_BRAKE);

		world.get_system<input_system>().add_context(active_context);

		if (augs::file_exists(L"recorded.inputs")) {
			world.parent_overworld.configure_stepping(60.0, 500);
			world.parent_overworld.delta_timer.set_stepping_speed_multiplier(6.00);

			world.get_system<input_system>().raw_window_input_player.player.load_recording("recorded.inputs");
			world.get_system<input_system>().raw_window_input_player.player.replay();
		
			world.get_system<input_system>().crosshair_intent_player.player.load_recording("recorded_crosshair.inputs");
			world.get_system<input_system>().crosshair_intent_player.player.replay();

			world.get_system<render_system>().enable_interpolation = false;
		}
		else {
			world.parent_overworld.configure_stepping(60.0, 5);
			world.parent_overworld.delta_timer.set_stepping_speed_multiplier(1.0);

			augs::create_directory("sessions/" + augs::get_timestamp());

			world.get_system<input_system>().raw_window_input_player.player.record("sessions/" + augs::get_timestamp() + "/recorded.inputs");
			world.get_system<input_system>().crosshair_intent_player.player.record("sessions/" + augs::get_timestamp() + "/recorded_crosshair.inputs");
		}
	}

	void one_entity::perform_logic_step(world& world) {

	}

	void one_entity::execute_drawcalls_for_camera(messages::camera_render_request_message msg) {
		rendering_scripts::testbed_rendering(msg);
	}
}