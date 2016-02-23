#include "testbed.h"
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

#include "augs/file.h"
#include "misc/time.h"

#include "rendering_scripts/testbed_rendering.h"

#include "texture_baker/font.h"

#include "gui/text/printer.h"

using namespace augs;

namespace scene_builders {
	void testbed::initialize(world& world) {
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

		resource_manager.create(assets::texture_id::MOTORCYCLE_FRONT, L"hypersomnia/gfx/motorcycle_front.png");
		resource_manager.create(assets::texture_id::MOTORCYCLE_INSIDE, L"hypersomnia/gfx/motorcycle_inside.png");

		resource_manager.create(assets::texture_id::TRUCK_FRONT, L"hypersomnia/gfx/truck_front.png");
		resource_manager.create(assets::texture_id::TRUCK_INSIDE, L"hypersomnia/gfx/truck_inside.png");

		resource_manager.create(assets::texture_id::TEST_SPRITE, L"hypersomnia/gfx/frog.png");
		resource_manager.create(assets::texture_id::MOTOR, L"hypersomnia/gfx/motor.png");

		resource_manager.create(assets::texture_id::ASSAULT_RIFLE, L"hypersomnia/gfx/assault_rifle.png");
		resource_manager.create(assets::texture_id::SHOTGUN, L"hypersomnia/gfx/shotgun.png");

		auto& font = resource_manager.create(assets::font_id::GUI_FONT);
		font.open("hypersomnia/Kubasta.ttf", 16, L" ABCDEFGHIJKLMNOPRSTUVWXYZQabcdefghijklmnoprstuvwxyzq0123456789.!@#$%^&*()_+-=[];'\\,./{}:\"|<>?");

		resource_manager.create_sprites_indexed(
			assets::texture_id::TORSO_MOVING_FIRST,
			assets::texture_id::TORSO_MOVING_LAST,
			L"hypersomnia/gfx/sprite");

		resource_manager.create(assets::atlas_id::GAME_WORLD_ATLAS,
			resources::manager::atlas_creation_mode::FROM_ALL_TEXTURES
			| resources::manager::atlas_creation_mode::FROM_ALL_FONTS);

		resource_manager.create(assets::shader_id::DEFAULT_VERTEX, L"hypersomnia/shaders/default.vsh", graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::DEFAULT_FRAGMENT, L"hypersomnia/shaders/default.fsh", graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::DEFAULT, assets::shader_id::DEFAULT_VERTEX, assets::shader_id::DEFAULT_FRAGMENT);

		resource_manager.create_inverse_with_flip(assets::animation_id::TORSO_MOVE,
			assets::texture_id::TORSO_MOVING_FIRST,
			assets::texture_id::TORSO_MOVING_LAST,
			20.0f);

		auto& player_response = resource_manager.create(assets::animation_response_id::TORSO_SET);
		player_response[messages::animation_response_message::MOVE] = assets::animation_id::TORSO_MOVE;

		auto camera = world.create_entity("camera");
		auto crosshair = world.create_entity("crosshair");
		auto player = world.create_entity("player");
		auto crate = world.create_entity("crate");
		auto crate2 = world.create_entity("crate2");
		auto crate3 = world.create_entity("crate3");
		auto crate4 = world.create_entity("crate4");

		for (int x = -4 * 1; x < 4 * 1; ++x)
		{
			auto frog = world.create_entity("frog");
			ingredients::sprite(frog, vec2(100 + x * 40, 400), assets::texture_id::TEST_SPRITE, augs::white, render_layer::DYNAMIC_BODY);
			ingredients::crate_physics(frog);
		}

		auto car = prefabs::create_car(world, vec2(-300, 0));
		auto car2 = prefabs::create_car(world, vec2(-800, 0));
		auto car3 = prefabs::create_car(world, vec2(-1300, 0));


		auto motorcycle = prefabs::create_motorcycle(world, vec2(0, -600));

		ingredients::camera(camera, window_rect.w, window_rect.h);

		auto bg_size = assets::get_size(assets::texture_id::TEST_BACKGROUND);

		for (int x = -4*10; x < 4 * 10; ++x)
			for (int y = -4 * 10; y < 4 * 10; ++y)
			{
				auto background = world.create_entity();
				ingredients::sprite(background, vec2(x, y) * (bg_size + vec2(1500, 550)), assets::texture_id::TEST_BACKGROUND, augs::white, render_layer::GROUND);
				//ingredients::static_crate_physics(background);

				auto street = world.create_entity();
				ingredients::sprite_scalled(street, vec2(x, y) * (bg_size + vec2(1500, 700)) - vec2(1500, 700), 
					vec2(3000, 3000),
					assets::texture_id::TEST_BACKGROUND, augs::gray1, render_layer::UNDER_GROUND);
			}


		ingredients::wsad_player_crosshair(crosshair);
		ingredients::wsad_player(player, crosshair, camera);

		ingredients::wsad_player_physics(player);

		ingredients::character_inventory(player);

		ingredients::sprite_scalled(crate, vec2(200, 300), vec2i(100, 100)/3, assets::texture_id::CRATE, augs::white, render_layer::DYNAMIC_BODY);
		ingredients::crate_physics(crate);
		
		ingredients::sprite_scalled(crate2, vec2(400, 400), vec2i(100, 100), assets::texture_id::CRATE, augs::white, render_layer::DYNAMIC_BODY);
		ingredients::crate_physics(crate2);
		
		ingredients::sprite_scalled(crate4, vec2(500, 0), vec2i(100, 100), assets::texture_id::CRATE, augs::white, render_layer::DYNAMIC_BODY);
		ingredients::crate_physics(crate4);

		input_system::context active_context;
		active_context.map_key_to_intent(window::event::keys::W, intent_type::MOVE_FORWARD);
		active_context.map_key_to_intent(window::event::keys::S, intent_type::MOVE_BACKWARD);
		active_context.map_key_to_intent(window::event::keys::A, intent_type::MOVE_LEFT);
		active_context.map_key_to_intent(window::event::keys::D, intent_type::MOVE_RIGHT);

		active_context.map_event_to_intent(window::event::raw_mousemotion, intent_type::MOVE_CROSSHAIR);
		active_context.map_key_to_intent(window::event::keys::LMOUSE, intent_type::CROSSHAIR_PRIMARY_ACTION);
		active_context.map_key_to_intent(window::event::keys::RMOUSE, intent_type::CROSSHAIR_SECONDARY_ACTION);
		
		active_context.map_key_to_intent(window::event::keys::E, intent_type::USE_BUTTON);
		active_context.map_key_to_intent(window::event::keys::SPACE, intent_type::SPACE_BUTTON);
		
		active_context.map_key_to_intent(window::event::keys::G, intent_type::THROW_PRIMARY_ITEM);
		active_context.map_key_to_intent(window::event::keys::H, intent_type::HOLSTER_PRIMARY_ITEM);
		
		active_context.map_key_to_intent(window::event::keys::LSHIFT, intent_type::SWITCH_LOOK);

		active_context.map_key_to_intent(window::event::keys::Z, intent_type::START_PICKING_UP_ITEMS);

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
			world.parent_overworld.configure_stepping(60.0, 500);
			world.parent_overworld.delta_timer.set_stepping_speed_multiplier(1.0);
			augs::create_directory("sessions/");
			augs::create_directory("sessions/" + augs::get_timestamp());

			world.get_system<input_system>().raw_window_input_player.player.record("sessions/" + augs::get_timestamp() + "/recorded.inputs");
			world.get_system<input_system>().crosshair_intent_player.player.record("sessions/" + augs::get_timestamp() + "/recorded_crosshair.inputs");
		}
	}

	void testbed::perform_logic_step(world& world) {
		auto inputs = world.get_message_queue<messages::crosshair_intent_message>();

		for (auto& it : inputs) {
			bool draw = false;
			if (it.intent == intent_type::CROSSHAIR_PRIMARY_ACTION) {
				keep_drawing = it.pressed_flag;
				draw = true;
			}

			if (draw || (it.intent == intent_type::MOVE_CROSSHAIR && keep_drawing)) {
				auto ent = world.create_entity("drawn_sprite");
				ingredients::sprite_scalled(ent, it.crosshair_world_pos, vec2(10, 10), assets::texture_id::BLANK);
			}
		}

	}

	void testbed::drawcalls_after_all_cameras(world& world) {
		auto& target = renderer::get_current();
		graphics::gui::text::quick_print_format(target.triangles, L"Be welcomed in Hypersomnia, Architect.", graphics::gui::text::style(assets::font_id::GUI_FONT, augs::violet), vec2i(200-1, 200), 0, nullptr);
		graphics::gui::text::quick_print_format(target.triangles, L"Be welcomed in Hypersomnia, Architect.", graphics::gui::text::style(assets::font_id::GUI_FONT, augs::violet), vec2i(200+1, 200), 0, nullptr);
		graphics::gui::text::quick_print_format(target.triangles, L"Be welcomed in Hypersomnia, Architect.", graphics::gui::text::style(assets::font_id::GUI_FONT, augs::violet), vec2i(200, 200 - 1), 0, nullptr);
		graphics::gui::text::quick_print_format(target.triangles, L"Be welcomed in Hypersomnia, Architect.", graphics::gui::text::style(assets::font_id::GUI_FONT, augs::violet), vec2i(200, 200+1), 0, nullptr);


		graphics::gui::text::quick_print_format(target.triangles, L"Be welcomed in Hypersomnia, Architect.", graphics::gui::text::style(), vec2i(200, 200), 0, nullptr);
		target.call_triangles();
	}

	void testbed::execute_drawcalls_for_camera(messages::camera_render_request_message msg) {
		rendering_scripts::testbed_rendering(msg);
	}
}