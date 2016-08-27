#include "testbed.h"
#include "game/ingredients/ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/assets/texture_id.h"

#include "game/systematic/input_system.h"
#include "game/systematic/render_system.h"
#include "game/systematic/gui_system.h"
#include "game/components/sentience_component.h"
#include "game/components/attitude_component.h"
#include "game/components/name_component.h"
#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/enums/party_category.h"

#include "game/messages/intent_message.h"
#include "game/detail/inventory_utils.h"

#include "rendering_scripts/all.h"

#include "texture_baker/font.h"

#include "augs/misc/machine_entropy.h"
#include "game/transcendental/cosmic_entropy.h"
#include "game/transcendental/viewing_session.h"
#include "game/transcendental/step.h"
#include "game/game_window.h"

#include "game/detail/world_camera.h"
#include "augs/gui/text/printer.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/time_utils.h"

#include "game/transcendental/cosmic_delta.h"

namespace scene_managers {
	void testbed::populate_world_with_entities(cosmos& cosm) {
		cosm.advance_deterministic_schemata(cosmic_entropy(), [this](fixed_step& step) { populate(step); }, [](fixed_step&) {});
	}

	void testbed::populate(fixed_step& step) {
		auto& world = step.cosm;
		const auto crate = prefabs::create_crate(world, vec2(200, 200 + 300), vec2i(100, 100) / 3);
		const auto crate2 = prefabs::create_crate(world, vec2(400, 200 + 400), vec2i(300, 300));
		const auto crate4 = prefabs::create_crate(world, vec2(500, 200 + 0), vec2i(100, 100));

		for (int x = -4; x < 4; ++x) {
			for (int y = -4; y < 4; ++y) {
				auto obstacle = prefabs::create_crate(world, vec2(2000 + x * 300, 2000 + y * 300), vec2i(100, 100));
			}
		}

		for (int x = -4 * 1; x < 4 * 1; ++x)
		{
			auto frog = world.create_entity("frog");
			ingredients::sprite(frog, vec2(100 + x * 40, 200 + 400), assets::texture_id::TEST_SPRITE, augs::white, render_layer::DYNAMIC_BODY);
			ingredients::see_through_dynamic_body(frog);
			frog.add_standard_components();
		}

		const auto car = prefabs::create_car(world, components::transform(-300, -600, -90));
		const auto car2 = prefabs::create_car(world, components::transform(-800, -600, -90));
		const auto car3 = prefabs::create_car(world, components::transform(-1300, -600, -90));

		const auto motorcycle = prefabs::create_motorcycle(world, components::transform(0, -600, -90));
		prefabs::create_motorcycle(world, components::transform(100, -600, -90));

		const auto bg_size = assets::get_size(assets::texture_id::TEST_BACKGROUND);

		//for (int x = -4 * 10; x < 4 * 10; ++x)
		//	for (int y = -4 * 10; y < 4 * 10; ++y)
		//	{
		//		auto background = world.create_entity("bg[-]");
		//		ingredients::sprite(background, vec2(-1000, 0) + vec2(x, y) * (bg_size + vec2(1500, 550)), assets::texture_id::TEST_BACKGROUND, augs::white, render_layer::GROUND);
		//		//ingredients::standard_static_body(background);
		//
		//		auto street = world.create_entity("street[-]");
		//		ingredients::sprite_scalled(street, vec2(-1000, 0) + vec2(x, y) * (bg_size + vec2(1500, 700)) - vec2(1500, 700),
		//			vec2(3000, 3000),
		//			assets::texture_id::TEST_BACKGROUND, augs::gray1, render_layer::UNDER_GROUND);
		//
		//		background.add_standard_components();
		//		street.add_standard_components();
		//	}

		const int num_characters = 6;

		std::vector<entity_handle> new_characters;

		for (int i = 0; i < num_characters; ++i) {
			auto new_character = prefabs::create_character(world, vec2(i * 300 , 0), vec2(1920, 1080), typesafe_sprintf("player%x", i));

			new_characters.push_back(new_character);

			if (i == 0) {
				new_character.get<components::sentience>().health.value = 800;
				new_character.get<components::sentience>().health.maximum = 800;
			}
			if (i == 1) {
				new_character.get<components::transform>().pos.set(2800, 700);
				new_character.get<components::attitude>().parties = party_category::RESISTANCE_CITIZEN;
				new_character.get<components::attitude>().hostile_parties = party_category::METROPOLIS_CITIZEN;
				new_character.get<components::attitude>().maximum_divergence_angle_before_shooting = 25;
				new_character.get<components::sentience>().minimum_danger_amount_to_evade = 20;
				new_character.get<components::sentience>().health.value = 300;
				new_character.get<components::sentience>().health.maximum = 300;
				//ingredients::standard_pathfinding_capability(new_character);
				//ingredients::soldier_intelligence(new_character);
				new_character.recalculate_basic_processing_categories();
			}
			if (i == 2) {
				new_character.get<components::sentience>().health.value = 38;
			}
			if (i == 5) {
				new_character.get<components::attitude>().parties = party_category::METROPOLIS_CITIZEN;
				new_character.get<components::attitude>().hostile_parties = party_category::RESISTANCE_CITIZEN;
				new_character.get<components::attitude>().maximum_divergence_angle_before_shooting = 25;
				new_character.get<components::sentience>().minimum_danger_amount_to_evade = 20;
				new_character.get<components::sentience>().health.value = 300;
				new_character.get<components::sentience>().health.maximum = 300;
				//ingredients::standard_pathfinding_capability(new_character);
				//ingredients::soldier_intelligence(new_character);
				new_character.recalculate_basic_processing_categories();
			}
		}

		name_entity(new_characters[0], entity_name::PERSON, L"Attacker");

		inject_input_to(new_characters[current_character]);

		prefabs::create_sample_suppressor(world, vec2(300, -500));

		const bool many_charges = false;

		const auto rifle = prefabs::create_sample_rifle(step, vec2(100, -500),
			prefabs::create_sample_magazine(step, vec2(100, -650), many_charges ? "10" : "0.3",
				prefabs::create_cyan_charge(world, vec2(0, 0), many_charges ? 1000 : 30)));

		const auto rifle2 = prefabs::create_sample_rifle(step, vec2(100, -500 + 50),
			prefabs::create_sample_magazine(step, vec2(100, -650), true ? "10" : "0.3",
				prefabs::create_cyan_charge(world, vec2(0, 0), true ? 1000 : 30)));

		prefabs::create_sample_rifle(step, vec2(100, -500 + 100));

		prefabs::create_pistol(step, vec2(300, -500 + 50));

		const auto pis2 = prefabs::create_pistol(step, vec2(300, 50),
			prefabs::create_sample_magazine(step, vec2(100, -650), "0.4",
				prefabs::create_green_charge(world, vec2(0, 0), 40)));

		const auto submachine = prefabs::create_submachine(step, vec2(500, -500 + 50),
			prefabs::create_sample_magazine(step, vec2(100 - 50, -650), many_charges ? "10" : "0.5", prefabs::create_pink_charge(world, vec2(0, 0), many_charges ? 500 : 50)));

		prefabs::create_submachine(step, vec2(0, -1000),
			prefabs::create_sample_magazine(step, vec2(100 - 50, -650), many_charges ? "10" : "0.5", prefabs::create_pink_charge(world, vec2(0, 0), many_charges ? 500 : 50)));

		prefabs::create_submachine(step, vec2(150, -1000 + 150),
			prefabs::create_sample_magazine(step, vec2(100 - 50, -650), many_charges ? "10" : "0.5", prefabs::create_pink_charge(world, vec2(0, 0), many_charges ? 500 : 50)));

		prefabs::create_submachine(step, vec2(300, -1000 + 300),
			prefabs::create_sample_magazine(step, vec2(100 - 50, -650), many_charges ? "10" : "0.5", prefabs::create_pink_charge(world, vec2(0, 0), many_charges ? 500 : 50)));

		prefabs::create_submachine(step, vec2(450, -1000 + 450),
			prefabs::create_sample_magazine(step, vec2(100 - 50, -650), many_charges ? "10" : "0.5", prefabs::create_pink_charge(world, vec2(0, 0), many_charges ? 500 : 50)));


		prefabs::create_sample_magazine(step, vec2(100 - 50, -650));
		prefabs::create_sample_magazine(step, vec2(100 - 100, -650), "0.30");
		//prefabs::create_pink_charge(world, vec2(100, 100));
		//prefabs::create_pink_charge(world, vec2(100, -400));
		//prefabs::create_pink_charge(world, vec2(150, -400));
		//prefabs::create_pink_charge(world, vec2(200, -400));
		prefabs::create_cyan_charge(world, vec2(150, -500));
		prefabs::create_cyan_charge(world, vec2(200, -500));

		prefabs::create_cyan_urban_machete(world, vec2(100, 100));
		const auto second_machete = prefabs::create_cyan_urban_machete(world, vec2(0, 0));

		const auto backpack = prefabs::create_sample_backpack(world, vec2(200, -650));
		prefabs::create_sample_backpack(world, vec2(200, -750));

		perform_transfer({ backpack, new_characters[0][slot_function::SHOULDER_SLOT] }, step);
		perform_transfer({ submachine, new_characters[0][slot_function::PRIMARY_HAND] }, step);
		perform_transfer({ rifle, new_characters[0][slot_function::SECONDARY_HAND] }, step);

		if (num_characters > 1) {
			name_entity(new_characters[1], entity_name::PERSON, L"Enemy");
			perform_transfer({ rifle2, new_characters[1][slot_function::PRIMARY_HAND] }, step);
		}

		if (num_characters > 2) {
			name_entity(new_characters[2], entity_name::PERSON, L"Swordsman");
			perform_transfer({ second_machete, new_characters[2][slot_function::PRIMARY_HAND] }, step);
		}

		if (num_characters > 3) {
			name_entity(new_characters[3], entity_name::PERSON, L"Medic");
			perform_transfer({ pis2, new_characters[3][slot_function::PRIMARY_HAND] }, step);
		}

		if (num_characters > 5) {
			const auto new_item = prefabs::create_submachine(step, vec2(0, -1000),
				prefabs::create_sample_magazine(step, vec2(100 - 50, -650), true ? "10" : "0.5", prefabs::create_pink_charge(world, vec2(0, 0), true ? 500 : 50)));

			perform_transfer({ new_item, new_characters[5][slot_function::PRIMARY_HAND] }, step);
		}

		//draw_bodies.push_back(crate2);
		//draw_bodies.push_back(new_characters[0]);
		//draw_bodies.push_back(backpack);

		world.significant.meta.settings.visibility.epsilon_ray_distance_variation = 0.001;
		world.significant.meta.settings.visibility.epsilon_threshold_obstacle_hit = 10;
		world.significant.meta.settings.visibility.epsilon_distance_vertex_hit = 1;

		world.significant.meta.settings.pathfinding.draw_memorised_walls = 1;
		world.significant.meta.settings.pathfinding.draw_undiscovered = 1;

		world.significant.meta.settings.enable_interpolation = true;

		const auto& id_vector = to_id_vector(new_characters);
		characters.assign(id_vector.begin(), id_vector.end());
		// _controlfp(0, _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL);
	}


	entity_id testbed::get_controlled_entity() const {
		return characters[current_character];
	}
	
	void testbed::inject_input_to(entity_handle h) {
		currently_controlled_character = h;
	}

	void testbed::configure_view(viewing_session& session) const {
		auto& active_context = session.input;

		active_context.map_key_to_intent(window::event::keys::W, intent_type::MOVE_FORWARD);
		active_context.map_key_to_intent(window::event::keys::S, intent_type::MOVE_BACKWARD);
		active_context.map_key_to_intent(window::event::keys::A, intent_type::MOVE_LEFT);
		active_context.map_key_to_intent(window::event::keys::D, intent_type::MOVE_RIGHT);

		active_context.map_event_to_intent(window::event::message::mousemotion, intent_type::MOVE_CROSSHAIR);
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

	}

	void testbed::control(const augs::machine_entropy::local_type& local, cosmos& main_cosmos) {
		for (const auto& raw_input : local) {
			if (raw_input.key_event == augs::window::event::PRESSED) {
				if (raw_input.key == augs::window::event::keys::F7) {
					auto target_folder = "saves/" + augs::get_timestamp();
					augs::create_directories(target_folder);

					main_cosmos.save_to_file(target_folder + "/" + "save.state");
				}
				if (raw_input.key == augs::window::event::keys::F4) {
					cosmos cosm_with_guids;
					cosm_with_guids.significant = stashed_cosmos;
					cosm_with_guids.remap_guids();

					ensure(stashed_delta.get_write_pos() == 0);
					cosmic_delta::encode(cosm_with_guids, main_cosmos, stashed_delta);

					stashed_delta.reset_read_pos();
					cosmic_delta::decode(cosm_with_guids, stashed_delta);
					stashed_delta.reset_write_pos();

					main_cosmos = cosm_with_guids;
				}
				if (raw_input.key == augs::window::event::keys::DASH) {
					show_profile_details = !show_profile_details;
				}
				if (raw_input.key == augs::window::event::keys::F8) {
					main_cosmos.profiler.duplication.new_measurement();
					stashed_cosmos = main_cosmos.significant;
					main_cosmos.profiler.duplication.end_measurement();
				}
				if (raw_input.key == augs::window::event::keys::F9) {
					main_cosmos = stashed_cosmos;
				}
				if (raw_input.key == augs::window::event::keys::F10) {
					main_cosmos.significant.meta.settings.enable_interpolation = !main_cosmos.significant.meta.settings.enable_interpolation;
				}
			}
		}
	}

	cosmic_entropy testbed::make_cosmic_entropy(const augs::machine_entropy::local_type& local, const input_context& context, cosmos& cosm) {
		cosmic_entropy result;

		auto& intents = result.entropy_per_entity[get_controlled_entity()];
		
		for (const auto& raw : local) {
			entity_intent mapped;
		
			if (result.make_intent(context, raw, mapped))
				intents.push_back(mapped);
		}

		return result;
	}

	void testbed::step_with_callbacks(const cosmic_entropy& cosmic_entropy_for_this_step, cosmos& cosm) {
		cosm.advance_deterministic_schemata(cosmic_entropy_for_this_step,
			[this](fixed_step& step) { pre_solve(step); },
			[this](fixed_step& step) { post_solve(step); }
		);
	}

	void testbed::pre_solve(fixed_step& step) {

	}

	void testbed::post_solve(fixed_step& step) {
		auto& cosmos = step.cosm;

		for (auto& it : step.messages.get_queue<messages::intent_message>()) {
			if (it.subject == characters[current_character] && it.intent == intent_type::SWITCH_CHARACTER && it.pressed_flag) {
				++current_character;
				current_character %= characters.size();

				inject_input_to(cosmos[characters[current_character]]);
			}
		}

		//for (auto& tested : draw_bodies) {
		//	auto& s = tested.get<components::physics_definition>();
		//
		//	auto& lines = renderer::get_current().logic_lines;
		//
		//	auto vv = s.fixtures[0].debug_original;
		//
		//	for (int i = 0; i < vv.size(); ++i) {
		//		auto& tt = tested.get<components::transform>();
		//		auto pos = tt.pos;
		//
		//		lines.draw_cyan((pos + vv[i]).rotate(tt.rotation, pos), (pos + vv[(i + 1) % vv.size()]).rotate(tt.rotation, pos));
		//	}
		//}

		//auto ff = (new_characters[1].get<components::pathfinding>().get_current_navigation_point() - position(new_characters[1])).set_length(15000);
		//new_characters[1].get<components::physics>().apply_force(ff);

		// LOG("F: %x", ff);
	}

	void testbed::view(const cosmos& cosmos, game_window& window, viewing_session& session, const augs::variable_delta& dt) const {
		session.fps_profiler.new_measurement();

		auto& target = renderer::get_current();

		target.clear_current_fbo();

		target.set_viewport({ 0, 0, static_cast<int>(session.camera.visible_world_area.x), static_cast<int>(session.camera.visible_world_area.y) });

		basic_viewing_step main_cosmos_viewing_step(cosmos, dt, target);
		view_cosmos(cosmos, main_cosmos_viewing_step, session.camera);

		auto summary = typesafe_sprintf(L"Entities: %x\n", cosmos.entities_count());

		using namespace augs::gui::text;

		const auto controlled = cosmos[get_controlled_entity()];

		const auto coords = controlled.get<components::transform>().pos;
		const auto vel = controlled.get<components::physics>().velocity();

		quick_print_format(target.triangles, typesafe_sprintf(L"Entities: %x\nX: %f2\nY: %f2\nVelX: %x\nVelY: %x\n", cosmos.entities_count(), coords.x, coords.y, vel.x, vel.y)
			+ session.summary() + cosmos.profiler.sorted_summary(show_profile_details), style(assets::font_id::GUI_FONT, rgba(255, 255, 255, 150)), vec2i(0, 0), 0);

		target.call_triangles();
		target.clear_triangles();


		session.triangles.measure(static_cast<double>(target.triangles_drawn_total));
		target.triangles_drawn_total = 0;

		window.swap_buffers();
		
		session.fps_profiler.end_measurement();
	}

	void testbed::view_cosmos(const cosmos& cosm, basic_viewing_step& step, world_camera& camera) const {
		auto& cosmos = cosm;

		auto character_chased_by_camera = cosmos[currently_controlled_character];

		camera.tick(step.get_delta(), character_chased_by_camera);
		
		viewing_step viewing(step, camera.get_state_for_drawing_camera(character_chased_by_camera));
		rendering_scripts::standard_rendering(viewing);
	}
}
