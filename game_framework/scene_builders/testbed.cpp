#include "testbed.h"
#include "../ingredients/ingredients.h"

#include "entity_system/world.h"
#include "window_framework/window.h"

#include "game_framework/resources/manager.h"
#include "game_framework/assets/texture.h"
#include "game_framework/assets/atlas.h"

#include "game_framework/systems/input_system.h"
#include "game_framework/systems/render_system.h"
#include "game_framework/systems/gui_system.h"
#include "game_framework/components/position_copying_component.h"
#include "game_framework/components/physics_definition_component.h"
#include "game_framework/components/item_component.h"
#include "game_framework/components/name_component.h"

#include "game_framework/messages/crosshair_intent_message.h"
#include "game_framework/messages/item_slot_transfer_request.h"

#include "../detail/inventory_slot.h"
#include "../detail/inventory_utils.h"

#include "augs/file.h"
#include "misc/time.h"

#include "rendering_scripts/all.h"
#include "resource_setups/all.h"

#include "texture_baker/font.h"

#include "gui/text/printer.h"
#include "log.h"
using namespace augs;

namespace scene_builders {
	void testbed::load_resources() {
		resource_setups::load_standard_atlas();
	}

	void testbed::populate_world_with_entities(world& world) {
		auto& window = *window::glwindow::get_current();
		auto window_rect = window.get_screen_rect();

		world.get_system<gui_system>().resize(vec2i(window_rect.w, window_rect.h));

		auto crate = prefabs::create_crate(world, vec2(200, 300), vec2i(100, 100) / 3);
		auto crate2 = prefabs::create_crate(world, vec2(400, 400), vec2i(300, 300));
		auto crate4 = prefabs::create_crate(world, vec2(500, 0), vec2i(100, 100));

		for (int x = -4 * 1; x < 4 * 1; ++x)
		{
			auto frog = world.create_entity("frog");
			ingredients::sprite(frog, vec2(100 + x * 40, 400), assets::texture_id::TEST_SPRITE, augs::white, render_layer::DYNAMIC_BODY);
			ingredients::crate_physics(frog);
		}

		auto car = prefabs::create_car(world, components::transform(-300, -600, -90));
		auto car2 = prefabs::create_car(world, components::transform(-800, -600, -90));
		auto car3 = prefabs::create_car(world, components::transform(-1300, -600, -90));

		auto motorcycle = prefabs::create_motorcycle(world, components::transform(0, -600, -90));
		prefabs::create_motorcycle(world, components::transform(100, -600, -90));

		world_camera = world.create_entity("camera");
		ingredients::camera(world_camera, window_rect.w, window_rect.h);

		auto bg_size = assets::get_size(assets::texture_id::TEST_BACKGROUND);

		for (int x = -4*10; x < 4 * 10; ++x)
			for (int y = -4 * 10; y < 4 * 10; ++y)
			{
				auto background = world.create_entity("bg");
				ingredients::sprite(background, vec2(x, y) * (bg_size + vec2(1500, 550)), assets::texture_id::TEST_BACKGROUND, augs::white, render_layer::GROUND);
				//ingredients::static_crate_physics(background);

				auto street = world.create_entity("street");
				ingredients::sprite_scalled(street, vec2(x, y) * (bg_size + vec2(1500, 700)) - vec2(1500, 700), 
					vec2(3000, 3000),
					assets::texture_id::TEST_BACKGROUND, augs::gray1, render_layer::UNDER_GROUND);
			}

		const int num_characters = 1;

		for (int i = 0; i < num_characters; ++i) {
			auto new_character = prefabs::create_character(world, vec2(i * 100, 0));
			new_character.set_debug_name(typesafe_sprintf("player%x", i));
			
			characters.push_back(new_character);
		}

		ingredients::inject_window_input_to_character(characters[current_character], world_camera);

		prefabs::create_sample_suppressor(world, vec2(300, -500));

		auto rifle = prefabs::create_sample_rifle(world, vec2(100, -500));
		prefabs::create_sample_rifle(world, vec2(100, -500 + 50));
		prefabs::create_sample_rifle(world, vec2(100, -500 + 100));

		prefabs::create_pistol(world, vec2(300, -500 + 50));
		prefabs::create_submachine(world, vec2(500, -500 + 50));


		auto mag = prefabs::create_sample_magazine(world, vec2(100, -650));
		mag[slot_function::ITEM_DEPOSIT]->space_available = to_space_units("100000");
		mag[slot_function::ITEM_DEPOSIT]->items_inside[0]->get<components::item>().charges = 1000;
		
		prefabs::create_sample_magazine(world, vec2(100 - 50, -650));
		prefabs::create_sample_magazine(world, vec2(100 - 100, -650));
		//prefabs::create_pink_charge(world, vec2(100, 100));
		//prefabs::create_pink_charge(world, vec2(100, -400));
		//prefabs::create_pink_charge(world, vec2(150, -400));
		//prefabs::create_pink_charge(world, vec2(200, -400));
		prefabs::create_cyan_charge(world, vec2(150, -500));
		prefabs::create_cyan_charge(world, vec2(200, -500));

		auto backpack = prefabs::create_sample_backpack(world, vec2(200, -650));
		prefabs::create_sample_backpack(world, vec2(200, -750));

		messages::item_slot_transfer_request r;
		r.item = backpack;
		r.target_slot = characters[0][slot_function::SHOULDER_SLOT];

		world.post_message(r);

		r.item = mag;
		r.target_slot = rifle[slot_function::GUN_DETACHABLE_MAGAZINE];

		world.post_message(r);

		r.item = rifle;
		r.target_slot = characters[0][slot_function::PRIMARY_HAND];
		
		world.post_message(r);

		r.item = mag[slot_function::ITEM_DEPOSIT]->items_inside[0];
		r.specified_quantity = 1;
		r.target_slot = rifle[slot_function::GUN_CHAMBER];

		world.post_message(r);

		input_system::context active_context;
		active_context.map_key_to_intent(window::event::keys::W, intent_type::MOVE_FORWARD);
		active_context.map_key_to_intent(window::event::keys::S, intent_type::MOVE_BACKWARD);
		active_context.map_key_to_intent(window::event::keys::A, intent_type::MOVE_LEFT);
		active_context.map_key_to_intent(window::event::keys::D, intent_type::MOVE_RIGHT);

		active_context.map_event_to_intent(window::event::mousemotion, intent_type::MOVE_CROSSHAIR);
		active_context.map_key_to_intent(window::event::keys::LMOUSE, intent_type::CROSSHAIR_PRIMARY_ACTION);
		active_context.map_key_to_intent(window::event::keys::RMOUSE, intent_type::CROSSHAIR_SECONDARY_ACTION);
		
		active_context.map_key_to_intent(window::event::keys::E, intent_type::USE_BUTTON);
		active_context.map_key_to_intent(window::event::keys::LSHIFT, intent_type::WALK);
		
		active_context.map_key_to_intent(window::event::keys::G, intent_type::THROW_PRIMARY_ITEM);
		active_context.map_key_to_intent(window::event::keys::H, intent_type::HOLSTER_PRIMARY_ITEM);
		
	    active_context.map_key_to_intent(window::event::keys::BACKSPACE, intent_type::SWITCH_LOOK);

		active_context.map_key_to_intent(window::event::keys::LCTRL, intent_type::START_PICKING_UP_ITEMS);
		active_context.map_key_to_intent(window::event::keys::CAPSLOCK, intent_type::SWITCH_CHARACTER);

		active_context.map_key_to_intent(window::event::keys::SPACE, intent_type::SPACE_BUTTON);
		active_context.map_key_to_intent(window::event::keys::MOUSE4, intent_type::SWITCH_TO_GUI);

		auto& input = world.get_system<input_system>();
		input.add_context(active_context);

		if (input.found_recording()) {
			world.parent_overworld.configure_stepping(60, 500);
			world.parent_overworld.delta_timer.set_stepping_speed_multiplier(6.00);

			input.replay_found_recording();

			world.get_system<render_system>().enable_interpolation = false;
		}
		else {
			world.parent_overworld.configure_stepping(60, 500);
			world.parent_overworld.delta_timer.set_stepping_speed_multiplier(1.0);

			world.get_system<render_system>().enable_interpolation = true;
			input.record_and_save_this_session();
		}

		//draw_bodies.push_back(crate2);
		//draw_bodies.push_back(characters[0]);
		//draw_bodies.push_back(backpack);
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
				//auto ent = world.create_entity("drawn_sprite");
				//ingredients::sprite_scalled(ent, it.crosshair_world_pos, vec2(10, 10), assets::texture_id::BLANK);
			}
		}


		auto key_inputs = world.get_message_queue<messages::unmapped_intent_message>();

		for (auto& it : key_inputs) {
			if (it.intent == intent_type::SWITCH_CHARACTER && it.pressed_flag) {
				++current_character;
				current_character %= characters.size();
				
				ingredients::inject_window_input_to_character(characters[current_character], world_camera);
			}
		}

		for (auto& tested : draw_bodies) {
			auto& s = tested->get<components::physics_definition>();

			auto& lines = renderer::get_current().logic_lines;

			auto vv = s.fixtures[0].debug_original;

			for (int i = 0; i < vv.size(); ++i) {
				auto& tt = tested->get<components::transform>();
				auto pos = tt.pos;

				lines.draw_cyan((pos + vv[i]).rotate(tt.rotation, pos), (pos + vv[(i + 1) % vv.size()]).rotate(tt.rotation, pos));
			}
		}
	}

	void testbed::drawcalls_after_all_cameras(world& world) {
		auto& target = renderer::get_current();
		using namespace gui::text;

		//quick_print_format(target.triangles, L"Be welcomed in Hypersomnia, Architect.", style(assets::font_id::GUI_FONT, violet), vec2i(200-1, 200), 0, nullptr);
		//quick_print_format(target.triangles, L"Be welcomed in Hypersomnia, Architect.", style(assets::font_id::GUI_FONT, violet), vec2i(200+1, 200), 0, nullptr);
		//quick_print_format(target.triangles, L"Be welcomed in Hypersomnia, Architect.", style(assets::font_id::GUI_FONT, violet), vec2i(200, 200 - 1), 0, nullptr);
		//quick_print_format(target.triangles, L"Be welcomed in Hypersomnia, Architect.", style(assets::font_id::GUI_FONT, violet), vec2i(200, 200+1), 0, nullptr);
		//
		quick_print_format(target.triangles, augs::to_wstring(typesafe_sprintf("Entities: %x", world.entities_count())), style(assets::GUI_FONT, rgba(255, 255, 255, 50)), vec2i(0, 0), 0, nullptr);
		target.call_triangles();
	}

	void testbed::execute_drawcalls_for_camera(messages::camera_render_request_message msg) {
		rendering_scripts::standard_rendering(msg);
	}
}