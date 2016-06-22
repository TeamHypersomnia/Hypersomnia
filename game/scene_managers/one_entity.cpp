#include "one_entity.h"
#include "game/ingredients/ingredients.h"

#include "game/cosmos.h"
#include "window_framework/window.h"

#include "game/resources/manager.h"
#include "game/assets/texture_id.h"
#include "game/assets/atlas_id.h"

#include "game/systems/input_system.h"
#include "game/systems/render_system.h"
#include "game/components/position_copying_component.h"

#include "game/messages/crosshair_intent_message.h"

#include "rendering_scripts/all.h"

#include "augs/filesystem/file.h"
#include "misc/time.h"

using namespace augs;

namespace scene_managers {
	void one_entity::load_resources() {
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
	}

	void one_entity::populate_world_with_entities(cosmos& world) {
		auto window_rect = window::glwindow::get_current()->get_screen_rect();

		auto& player_response = resource_manager.create(assets::animation_response_id::TORSO_SET);
		player_response[animation_response_type::MOVE] = assets::animation_id::TORSO_MOVE;

		auto camera = world.create_entity();

		ingredients::camera(camera, window_rect.w, window_rect.h);

		auto bg_size = assets::get_size(assets::texture_id::TEST_BACKGROUND);

		auto background = world.create_entity();
		ingredients::sprite(background, vec2(500, 0), assets::texture_id::TEST_BACKGROUND);
		
		input_system::context active_context;
		active_context.map_event_to_intent(window::event::mousemotion, intent_type::MOVE_CROSSHAIR);
		active_context.map_key_to_intent(window::event::keys::LSHIFT, intent_type::SWITCH_LOOK);
		active_context.map_key_to_intent(window::event::keys::W, intent_type::MOVE_FORWARD);
		active_context.map_key_to_intent(window::event::keys::S, intent_type::MOVE_BACKWARD);
		active_context.map_key_to_intent(window::event::keys::A, intent_type::MOVE_LEFT);
		active_context.map_key_to_intent(window::event::keys::D, intent_type::MOVE_RIGHT);
		active_context.map_key_to_intent(window::event::keys::LMOUSE, intent_type::CROSSHAIR_PRIMARY_ACTION);
		active_context.map_key_to_intent(window::event::keys::RMOUSE, intent_type::CROSSHAIR_SECONDARY_ACTION);
		active_context.map_key_to_intent(window::event::keys::SPACE, intent_type::HAND_BRAKE);

		auto& input = world.systems.get<input_system>();
		input.add_context(active_context);

		if (input.found_recording()) {
			world.parent_overworld.configure_stepping(60.0, 500);
			world.parent_overworld.delta_timer.set_stepping_speed_multiplier(6.00);

			input.replay_found_recording();
		
			world.systems.get<render_system>().enable_interpolation = false;
		}
		else {
			world.parent_overworld.configure_stepping(60.0, 5);
			world.parent_overworld.delta_timer.set_stepping_speed_multiplier(1.0);

			input.record_and_save_this_session();
		}
	}

	void one_entity::perform_logic_step(cosmos& world) {

	}

	void one_entity::execute_drawcalls_for_camera(messages::camera_render_request_message msg) {
		rendering_scripts::standard_rendering(msg);
	}
}