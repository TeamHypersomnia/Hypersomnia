#include "generated/setting_build_test_scenes.h"
#include "testbed.h"

#if BUILD_TEST_SCENES

#include "game/ingredients/ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/assets/game_image_id.h"

#include "game/systems_stateless/input_system.h"
#include "game/systems_stateless/render_system.h"
#include "game/systems_stateless/particles_existence_system.h"
#include "game/systems_stateless/gui_system.h"
#include "game/systems_stateless/car_system.h"
#include "game/systems_stateless/driver_system.h"
#include "game/components/sentience_component.h"
#include "game/components/attitude_component.h"
#include "game/components/name_component.h"
#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/enums/party_category.h"
#include "game/detail/describers.h"

#include "game/messages/intent_message.h"
#include "game/detail/inventory/inventory_utils.h"

#include "augs/image/font.h"

#include "augs/misc/machine_entropy.h"
#include "game/transcendental/cosmic_entropy.h"
#include "game/view/viewing_session.h"
#include "game/transcendental/logic_step.h"

#include "game/view/world_camera.h"
#include "augs/gui/text/printer.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/time_utils.h"

#include "game/transcendental/cosmic_delta.h"

#include "augs/graphics/renderer.h"
#include "game/test_scenes/resource_setups/all.h"
#endif
namespace test_scenes {
	void testbed::populate(const logic_step step) {
#if BUILD_TEST_SCENES
		auto& world = step.cosm;
		const auto& metas = step.input.metas_of_assets;
		
		//const auto crate = prefabs::create_crate(step, vec2(200, 200 + 300), vec2i(100, 100) / 3);
		//const auto crate2 = prefabs::create_crate(step, vec2(400, 200 + 400), vec2i(300, 300));
		//const auto crate4 = prefabs::create_crate(step, vec2(500, 200 + 0), vec2i(100, 100));
		//
		//crates.push_back(crate);
		//crates.push_back(crate2);
		//crates.push_back(crate4);

		//for (int x = -4; x < 4; ++x) {
		//	for (int y = -4; y < 4; ++y) {
		//		auto obstacle = prefabs::create_crate(step, vec2(2000 + x * 300, 2000 + y * 300), vec2i(100, 100));
		//		crates.push_back(obstacle);
		//	}
		//}

		//for (int x = -4 * 1; x < 4 * 1; ++x)
		//{
		//	auto frog = world.create_entity("frog");
		//	ingredients::add_sprite(frog, vec2(100 + x * 40, 200 + 400), assets::game_image_id::TEST_SPRITE, white, render_layer::SMALL_DYNAMIC_BODY);
		//	ingredients::add_see_through_dynamic_body(frog, pos);
		//	
		//	frog.add_standard_components(step);
		//}

		const auto car = prefabs::create_car(step, components::transform( 1490, 340, -180));
		const auto car2 = prefabs::create_car(step, components::transform(1490, 340 + 400, -180));
		const auto car3 = prefabs::create_car(step, components::transform(1490, 340 + 800, -180));

		const auto motorcycle = prefabs::create_motorcycle(step, components::transform(250, 400, -90 + 180));
		//prefabs::create_motorcycle(step, components::transform(100, -600, -90));
		//const auto main_character_motorcycle = prefabs::create_motorcycle(step, components::transform(900, 48200, -90));
		const auto main_character_motorcycle = prefabs::create_motorcycle(step, components::transform(900, 200, -90));
		
		const auto riding_car = prefabs::create_car(step, components::transform(850, 1000, -90));

		const auto riding_car2 = prefabs::create_car(step, components::transform(-850 + 1000, -8200, -90 + 180));
		const auto motorcycle2 = prefabs::create_motorcycle(step, components::transform(-1150 + 1000, -8200, -90 + 180));

		const int num_characters = 4 + 3 + 3 + 2;

		for (int i = 0; i < 10; ++i) {
			prefabs::create_force_grenade(step, { 254, 611 + i *100.f });
			prefabs::create_ped_grenade(step, { 204, 611 + i * 100.f });
			prefabs::create_interference_grenade(step, { 154, 611 + i * 100.f });
		}

		for (int i = 0; i < 10; ++i) {
			prefabs::create_force_grenade(step, { 654, -811 + i *100.f });
			prefabs::create_ped_grenade(step, { 604, -811 + i * 100.f });
			prefabs::create_interference_grenade(step, { 554, -811 + i * 100.f });
		}

		std::vector<entity_id> new_characters;
		new_characters.resize(num_characters);

		auto character = [&](const size_t i) {
			return i < new_characters.size() ? world[new_characters.at(i)] : world[entity_id()];
		};

		for (int i = 0; i < num_characters; ++i) {
			components::transform transform;

			if (i == 0) {
				transform = main_character_motorcycle.get_logic_transform();
				//transform = { 0, 300, 0 };
				//torso_set = assets::animation_response_id::TORSO_SET;
			}
			else if (i == 1 || i == 2) {
				if (i == 1) {
					transform = { 254, 211, 68 };
				}
				if (i == 2) {
					transform = { 1102, 213, 110 };
				}

				//torso_set = assets::animation_response_id::VIOLET_TORSO_SET;
			}
			else if (i == 3) {
				//torso_set = assets::animation_response_id::VIOLET_TORSO_SET;
				transform = riding_car.get_logic_transform();
			}

			// three rebels

			else if (i == 4) {
				transform = { -100, 20000, 0 };

				//torso_set = assets::animation_response_id::BLUE_TORSO_SET;
			}
			else if (i == 5) {
				transform = { 1200, 15000, 0 };

				//torso_set = assets::animation_response_id::BLUE_TORSO_SET;
			}
			else if (i == 6) {
				transform = { -300, 20000, 0 };

				//torso_set = assets::animation_response_id::BLUE_TORSO_SET;
			}

			// three metropolitan soldiers
			else if (i == 7) {
				transform = { -300, -2000, 0 };

				//torso_set = assets::animation_response_id::VIOLET_TORSO_SET;
			}
			else if (i == 8) {
				transform = { -400, -2000, 0 };

				//torso_set = assets::animation_response_id::VIOLET_TORSO_SET;
			}
			else if (i == 9) {
				transform = { -500, -2000, 0 };

				//torso_set = assets::animation_response_id::VIOLET_TORSO_SET;
			}
			else if(i == 10) {
				transform = riding_car2.get_logic_transform();
			}
			else if(i == 11) {
				transform = motorcycle2.get_logic_transform();
			}

			const auto new_character = prefabs::create_sample_complete_character(step, transform, typesafe_sprintf("player%x", i), i ? 2 : 0);

			new_characters[i] = new_character;

			if (i == 0) {
				new_character.get<components::sentience>().get<health_meter_instance>().set_value(100);
				new_character.get<components::sentience>().get<health_meter_instance>().set_maximum_value(100);
				new_character.get<components::attitude>().parties = party_category::RESISTANCE_CITIZEN;
				new_character.get<components::attitude>().hostile_parties = party_category::METROPOLIS_CITIZEN;
			}
			if (i == 1) {
				new_character.get<components::attitude>().maximum_divergence_angle_before_shooting = 25;
				new_character.get<components::sentience>().minimum_danger_amount_to_evade = 20;
				new_character.get<components::sentience>().get<health_meter_instance>().set_value(300);
				new_character.get<components::sentience>().get<health_meter_instance>().set_maximum_value(300);
				//ingredients::add_standard_pathfinding_capability(new_character);
				//ingredients::add_soldier_intelligence(new_character);
				new_character.recalculate_basic_processing_categories();
			}
			if (i == 2) {
				new_character.get<components::sentience>().get<health_meter_instance>().set_value(100);
			}
			if (i == 5) {
				new_character.get<components::attitude>().maximum_divergence_angle_before_shooting = 25;
				new_character.get<components::sentience>().minimum_danger_amount_to_evade = 20;
				new_character.get<components::sentience>().get<health_meter_instance>().set_value(300);
				new_character.get<components::sentience>().get<health_meter_instance>().set_maximum_value(300);
				//ingredients::add_standard_pathfinding_capability(new_character);
				//ingredients::add_soldier_intelligence(new_character);
				new_character.recalculate_basic_processing_categories();
			}

			if (
				i == 4 || i == 5 || i == 6
				) {
				const auto rifle = prefabs::create_sample_rifle(step, vec2(100, -500),
					prefabs::create_sample_magazine(step, vec2(100, -650), "0.4",
						(i == 5 ? prefabs::create_cyan_charge : prefabs::create_cyan_charge)(step, vec2(0, 0), 30)));

				
				perform_transfer({ rifle, new_character.get_primary_hand() }, step);

				new_character.get<components::attitude>().parties = party_category::RESISTANCE_CITIZEN;
				new_character.get<components::attitude>().hostile_parties = party_category::METROPOLIS_CITIZEN;
			}

			if (
				i == 7 || i == 8 || i == 9
				) {

				
				
				if (i == 8) {
					
				}

				if (i == 9) {
					const auto rifle = prefabs::create_sample_rifle(step, vec2(100, -500),
						prefabs::create_sample_magazine(step, vec2(100, -650), "3.4",
							prefabs::create_cyan_charge(step, vec2(0, 0), 300)));

					perform_transfer({ rifle, new_character.get_primary_hand() }, step);
				}
				else {
					const auto rifle = (i == 7 ? prefabs::create_sample_rifle : prefabs::create_sample_rifle)(step, vec2(100, -500),
						prefabs::create_sample_magazine(step, vec2(100, -650), "3.4",
							prefabs::create_cyan_charge(step, vec2(0, 0), 300)));

					perform_transfer({ rifle, new_character.get_primary_hand() }, step);
				}

				const auto backpack = prefabs::create_sample_backpack(step, vec2(200, -650));
				perform_transfer({ backpack, new_character[slot_function::SHOULDER] }, step);
			}

			if (i == 10) {
				driver_system().assign_car_ownership(new_character, riding_car2);
			}

			if (i == 11) {
				driver_system().assign_car_ownership(new_character, motorcycle2);
			}

			auto& sentience = new_character.get<components::sentience>();

			sentience.get<consciousness_meter_instance>().set_maximum_value(400);
			sentience.get<consciousness_meter_instance>().set_value(400);

			sentience.get<personal_electricity_meter_instance>().set_maximum_value(400);
			sentience.get<personal_electricity_meter_instance>().set_value(400);

			if (i == 0) {
				sentience.get<personal_electricity_meter_instance>().set_maximum_value(800);
				sentience.get<personal_electricity_meter_instance>().set_value(800);
			}

			fill_container(sentience.learned_spells, true);

			// for_each_through_std_get(
			// 	sentience.spells,
			// 	[](auto& spell) {
			// 		spell.common.learned = true;
			// 	}
			// );

			set_bbcoded_entity_description(new_character, L"Member of Atlantic nations.");
		}

		// street wandering pixels
		{
			const auto reach = xywh(0, 0, 1500, 32000);

			{
				const auto e = world.create_entity("wandering_pixels");
				auto& w = e += components::wandering_pixels();
				auto& r = e += components::render();

				r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

				w.face.set(assets::game_image_id(int(assets::game_image_id::BLANK)), vec2(1, 1), cyan);
				w.count = 200;
				w.reach = reach;
				e.add_standard_components(step);
			}

			{
				const auto e = world.create_entity("wandering_pixels");
				auto& w = e += components::wandering_pixels();
				auto& r = e += components::render();

				r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

				w.face.set(assets::game_image_id(int(assets::game_image_id::BLINK_FIRST) + 2), cyan);
				//w.face.size.set(1, 1);
				w.count = 80;
				w.reach = reach;
				e.add_standard_components(step);
			}

			{
				const auto e = world.create_entity("wandering_pixels");
				auto& w = e += components::wandering_pixels();
				auto& r = e += components::render();

				r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

				w.face.set(assets::game_image_id(int(assets::game_image_id::BLINK_FIRST) + 2), cyan);
				//w.face.size.set(1, 1);
				w.count = 80;
				w.reach = reach;
				e.add_standard_components(step);
			}
		}


		{
			vec2 coords[] = {
				{ 1200, 5400 },
				{ 1200, 10400 },
				{ 1200, 15400 },
				{ 1200, 20400 },
				{ -1, 25400 },
				{ 1200, 30400 },
			};

			for (const auto c : coords) {
				prefabs::create_crate(step, c + vec2(-100, 400) );
				prefabs::create_crate(step, c + vec2(300, 300) );
				prefabs::create_crate(step, c + vec2(100, -200) );

				const auto light_pos = c + vec2(0, 100);
				const auto light_cyan = c.x < 0 ? orange : rgba(30, 255, 255, 255);

				{
					const auto l = world.create_entity("l");
					l += components::transform(light_pos);
					auto& light = l += components::light();
					light.color = light_cyan;
					light.max_distance.base_value = 4500.f;
					light.constant.base_value = 0.15f;
					light.linear.base_value = 0.000005f;

					if (light_cyan == orange) {
						light.max_distance.base_value = 5500.f;
						light.constant.base_value = 0.10f;
						light.linear.base_value = 0.000002f;
					}

					light.wall_max_distance.base_value = 4000.f;
					l.add_standard_components(step);
				}


				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::game_image_id(int(assets::game_image_id::BLANK)), vec2(1, 1), light_cyan);
					w.count = 50;
					w.reach = xywh(light_pos.x- 250, light_pos.y-250, 500, 500);
					e.add_standard_components(step);
				}

				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::game_image_id(int(assets::game_image_id::BLINK_FIRST) + 2), light_cyan);
					w.count = 20;
					w.reach = xywh(light_pos.x - 150, light_pos.y - 150, 300, 300);
					e.add_standard_components(step);
				}

				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::game_image_id(int(assets::game_image_id::BLINK_FIRST) + 2), light_cyan);
					w.count = 20;
					w.reach = xywh(light_pos.x - 25, light_pos.y - 25, 50, 50);
					e.add_standard_components(step);
				}
			}
		}

		{
			//{
			//	const auto l = world.create_entity("l");
			//	l += components::transform(0, 300);
			//	auto& light = l += components::light();
			//	light.color = red;
			//	l.add_standard_components(step);
			//}
			//{
			//	const auto l = world.create_entity("l");
			//	l += components::transform(300, 300);
			//	auto& light = l += components::light();
			//	light.color = green;
			//	l.add_standard_components(step);
			//}
			//{
			//	const auto l = world.create_entity("l");
			//	l += components::transform(600, 300);
			//	auto& light = l += components::light();
			//	light.color = blue;
			//	l.add_standard_components(step);
			//}
			{
				const auto l = world.create_entity("l");
				l += components::transform(164.f - 8.f + 90.f, 220);
				auto& light = l += components::light();
				light.color = cyan;
				light.max_distance.base_value = 4500.f;
				light.wall_max_distance.base_value = 4000.f;
				l.add_standard_components(step);
			}
			{
				const auto l = world.create_entity("l");
				l += components::transform(1164.f + 24.f - 90.f, 220);
				auto& light = l += components::light();
				light.color = orange;
				light.max_distance.base_value = 4500.f;
				light.wall_max_distance.base_value = 4000.f;
				l.add_standard_components(step);
			}
			{
				const auto left_reach = xywh(164.f - 8.f + 90.f - 550, 220 - 250, 1000, 600);
				const auto right_reach = xywh(1164.f - 8.f + 90.f - 600, 220 - 250, 1000, 600);

				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::game_image_id(int(assets::game_image_id::BLINK_FIRST) + 2), cyan);
					w.count = 20;
					w.reach = left_reach;
					e.add_standard_components(step);
				}

				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::game_image_id(int(assets::game_image_id::BLINK_FIRST) + 2), orange);
					w.count = 20;
					w.reach = right_reach;
					e.add_standard_components(step);
				}

				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::game_image_id(int(assets::game_image_id::BLANK)), vec2(1, 1), cyan);
					w.count = 50;
					w.reach = left_reach;
					e.add_standard_components(step);
				}

				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::game_image_id(int(assets::game_image_id::BLANK)), vec2(1, 1), orange);
					w.count = 50;
					w.reach = right_reach;
					e.add_standard_components(step);
				}

				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::game_image_id(int(assets::game_image_id::BLANK)), vec2(2, 2), cyan);
					w.count = 30;
					w.reach = left_reach;
					e.add_standard_components(step);
				}

				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::game_image_id(int(assets::game_image_id::BLANK)), vec2(2, 2), orange);
					w.count = 30;
					w.reach = right_reach;
					e.add_standard_components(step);
				}

				//{
				//	const auto e = world.create_entity("wandering_pixels");
				//	auto& w = e += components::wandering_pixels();
				//	auto& r = e += components::render();
				//
				//	r.layer = render_layer::WANDERING_PIXELS_EFFECTS;
				//
				//	w.face.set(assets::game_image_id(int(assets::game_image_id::WANDERING_CROSS)), cyan);
				//	w.count = 15;
				//	w.reach = xywh(164.f - 8.f + 90.f - 100, 220 - 100, 200, 200);
				//	e.add_standard_components(step);
				//}
				//
				//{
				//	const auto e = world.create_entity("wandering_pixels");
				//	auto& w = e += components::wandering_pixels();
				//	auto& r = e += components::render();
				//
				//	r.layer = render_layer::WANDERING_PIXELS_EFFECTS;
				//
				//	w.face.set(assets::game_image_id(int(assets::game_image_id::WANDERING_CROSS)), orange);
				//	w.count = 15;
				//	w.reach = xywh(1164.f - 8.f + 90.f - 100, 220 - 100, 200, 200);
				//	e.add_standard_components(step);
				//}
			}

			{
				//const auto l = world.create_entity("l");
				//l += components::transform(164.f - 8.f, -700);
				//auto& light = l += components::light();
				//light.color = cyan;
				////light.linear.base_value = 0.000005f;
				////light.quadratic.base_value = 0.000025f;
				//light.max_distance.base_value = 4500.f;
				//light.wall_max_distance.base_value = 4000.f;
				//l.add_standard_components(step);
			}
			{
				const auto l = world.create_entity("l");
				l += components::transform(664.f + 24.f, -1100);
				auto& light = l += components::light();
				light.color = orange;
				light.max_distance.base_value = 4500.f;
				light.wall_max_distance.base_value = 4000.f;
				l.add_standard_components(step);
			}

			particles_existence_input effect;
			effect.effect.id = assets::particle_effect_id::WANDERING_SMOKE;
			effect.displace_source_position_within_radius = 500.f;
			effect.single_displacement_duration_ms.set(400.f, 1500.f);

			effect.create_particle_effect_entity(
				step, 
				components::transform(-164, 500, 0),
				entity_id()
			).add_standard_components(step);

			{
				const auto e = world.create_entity("tiled_floor");
				
				e += components::transform(0.f, -1200.f);
				auto& tile_layer_instance = e += components::tile_layer_instance();
				auto& render = e += components::render();
				
				render.layer = render_layer::TILED_FLOOR;
				tile_layer_instance.id = assets::tile_layer_id::METROPOLIS_FLOOR;


				e.add_standard_components(step);
			}

			{
				const auto e = world.create_entity("have_a_pleasant");

				ingredients::add_sprite(
					e,
					assets::game_image_id::HAVE_A_PLEASANT,
					white,
					render_layer::NEON_CAPTIONS
				);

				e += components::transform(164.f - 8.f, -60.f - 20.f);

				e.add_standard_components(step);

				prefabs::create_brick_wall(step, components::transform(3 + 1 + 1100, -32 - 96), { 160, 160 });
				prefabs::create_brick_wall(step, components::transform(3 + 1 + 1100 + 160, -32 - 96), { 160, 160 });
				prefabs::create_brick_wall(step, components::transform(3 + 1 + 1100 + 160, -32 - 96 + 160), { 160, 160 });
				prefabs::create_brick_wall(step, components::transform(3 + 1 + 1100, -32 - 96 + 160), { 160, 160 });


				prefabs::create_brick_wall(step, components::transform(-3 -16 + 100, -32 - 96), { 160, 160 });
				prefabs::create_brick_wall(step, components::transform(-3 -16 + 100 + 160, -32 - 96), { 160, 160 });
				prefabs::create_brick_wall(step, components::transform(-3 -16 + 100 + 160, -32 - 96 + 160), { 160, 160 });
				prefabs::create_brick_wall(step, components::transform(-3 -16 + 100, -32 - 96 + 160), { 160, 160 });

				for (int b = 0; b < 8; ++b) {
					prefabs::create_brick_wall(step, components::transform(3 + 1 + 1100 + 160 + 160, -32 - 96 + 160 - 160.f * b, 90), { 160, 160 });
					prefabs::create_brick_wall(step, components::transform(-3 - 16 + 100 - 160, -32 - 96 + 160 - 160*b, 90), { 160, 160 });
				}

				const vec2 bg_size = metas[assets::game_image_id::TEST_BACKGROUND].get_size();

				const int num_floors = 10 * 10;
				const int side = sqrt(num_floors) / 2;

				for (int x = -side; x < side; ++x) {
					for (int y = -side; y < side * 16; ++y)
					{
						//auto background = world.create_entity("bg[-]");
						//ingredients::add_sprite(background, vec2(-1000, 0) + vec2(x, y) * (bg_size + vec2(1500, 550)), assets::game_image_id::TEST_BACKGROUND, white, render_layer::GROUND);
						//ingredients::add_standard_static_body(background);

						auto street = world.create_entity("street[-]");
						ingredients::add_sprite(street,
							assets::game_image_id::TEST_BACKGROUND, gray1, render_layer::GROUND);

						street += components::transform{ bg_size * vec2(x, y) };

						//background.add_standard_components(step);
						street.add_standard_components(step);
					}
				}

				{
					const vec2 size = metas[assets::game_image_id::ROAD_FRONT_DIRT].get_size();

					auto road_dirt = world.create_entity("road_dirt[-]");
					ingredients::add_sprite(road_dirt,
						assets::game_image_id::ROAD_FRONT_DIRT, white, render_layer::ON_TILED_FLOOR);

					road_dirt += components::transform{ vec2(-3 - 16 + 100 + 160 + 80 + size.x / 2, -32 - 96 + 160 + 80 - size.y / 2) };

					road_dirt.add_standard_components(step);
				}

				for (int r = 0; r < 38; ++r) {
					const vec2 size = metas[assets::game_image_id::ROAD].get_size();

					auto road = world.create_entity("road[-]");
					ingredients::add_sprite(road,
						assets::game_image_id::ROAD, white, render_layer::ON_GROUND);

					road += components::transform{ vec2(-3 - 16 + 100 + 160 + 80 + size.x / 2, -32 - 96 + 160 + 80 + size.y / 2 + size.y*r) };

					road.add_standard_components(step);
				}
			}

			{
				const auto e = world.create_entity("awakening");
				auto& sprite = ingredients::add_sprite(
					e,
					assets::game_image_id::AWAKENING,
					white,
					render_layer::NEON_CAPTIONS
				);

				e += components::transform(164.f - 8.f, -60.f - 20.f + 40.f);

				sprite.effect = components::sprite::special_effect::COLOR_WAVE;

				e.add_standard_components(step);
			}

			{
				const auto e = world.create_entity("metropolis");

				ingredients::add_sprite(
					e,
					assets::game_image_id::METROPOLIS,
					white,
					render_layer::NEON_CAPTIONS
				);

				e += components::transform(1164.f + 24.f, -60.f);

				e.add_standard_components(step);
			}
		}

		if (character(0).alive()) {
			driver_system().assign_car_ownership(character(0), main_character_motorcycle);
			//main_character_motorcycle.get<components::car>().accelerating = true;
		}

		if (character(3).alive()) {
			driver_system().assign_car_ownership(character(3), riding_car);
			//riding_car.get<components::car>().accelerating = true;
		}

		prefabs::create_sample_suppressor(step, vec2(300, -500));

		const bool many_charges = false;

		const auto rifle = prefabs::create_sample_rifle(step, vec2(100, -500),
			prefabs::create_sample_magazine(step, vec2(100, -650), many_charges ? "10" : "0.3",
				prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 1000 : 30)));

		const auto rifle2 = prefabs::create_sample_rifle(step, vec2(100, -500 + 50),
			prefabs::create_sample_magazine(step, vec2(100, -650), true ? "10" : "0.3",
				prefabs::create_cyan_charge(step, vec2(0, 0), true ? 1000 : 30)));

		const auto amplifier = prefabs::create_amplifier_arm(step, vec2(-300, -500 + 50));
		prefabs::create_amplifier_arm(step, vec2(-370, + 50));

		prefabs::create_sample_rifle(step, vec2(100, -500 + 100), prefabs::create_sample_magazine(step, vec2(100, -650), many_charges ? "10" : "0.3",
			prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 1000 : 30)));

		prefabs::create_sample_rifle(step, vec2(200, -600 + 100), prefabs::create_sample_magazine(step, vec2(100, -650), many_charges ? "10" : "0.3",
				prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 1000 : 30)));
		prefabs::create_sample_rifle(step, vec2(300, -700 + 100), prefabs::create_sample_magazine(step, vec2(100, -650), many_charges ? "10" : "0.3",
				prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 1000 : 30)));
		prefabs::create_sample_rifle(step, vec2(400, -800 + 100), prefabs::create_sample_magazine(step, vec2(100, -650), many_charges ? "10" : "0.3",
				prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 1000 : 30)));
		prefabs::create_sample_rifle(step, vec2(500, -900 + 100), prefabs::create_sample_magazine(step, vec2(100, -650), many_charges ? "10" : "0.3",
				prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 1000 : 30)));

		prefabs::create_sample_rifle(step, vec2(700, -600 + 100), prefabs::create_sample_magazine(step, vec2(100, -650), many_charges ? "10" : "0.3",
				prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 1000 : 30)));
		prefabs::create_sample_rifle(step, vec2(800, -700 + 100), prefabs::create_sample_magazine(step, vec2(100, -650), many_charges ? "10" : "0.3",
				prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 1000 : 30)));
		prefabs::create_sample_rifle(step, vec2(900, -800 + 100), prefabs::create_sample_magazine(step, vec2(100, -650), many_charges ? "10" : "0.3",
				prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 1000 : 30)));

		prefabs::create_sample_rifle(step, vec2(300, -500 + 50));

		const auto pis2 = prefabs::create_sample_rifle(step, vec2(300, 50),
			prefabs::create_sample_magazine(step, vec2(100, -650), "0.4",
				prefabs::create_cyan_charge(step, vec2(0, 0), 40)));

		const auto submachine = prefabs::create_sample_rifle(step, vec2(500, -500 + 50),
			prefabs::create_sample_magazine(step, vec2(100 - 50, -650), many_charges ? "10" : "0.5", prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 500 : 50)));

		prefabs::create_sample_rifle(step, vec2(0, -1000),
			prefabs::create_sample_magazine(step, vec2(100 - 50, -650), many_charges ? "10" : "0.5", prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 500 : 50)));

		prefabs::create_sample_rifle(step, vec2(150, -1000 + 150),
			prefabs::create_sample_magazine(step, vec2(100 - 50, -650), many_charges ? "10" : "0.5", prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 500 : 50)));

		prefabs::create_sample_rifle(step, vec2(300, -1000 + 300),
			prefabs::create_sample_magazine(step, vec2(100 - 50, -650), many_charges ? "10" : "0.5", prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 500 : 50)));

		prefabs::create_sample_rifle(step, vec2(450, -1000 + 450),
			prefabs::create_sample_magazine(step, vec2(100 - 50, -650), many_charges ? "10" : "0.5", prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 500 : 50)));


		prefabs::create_sample_magazine(step, vec2(100 - 50, -650));
		prefabs::create_sample_magazine(step, vec2(100 - 100, -650), "0.30");
		prefabs::create_cyan_charge(step, vec2(150, -500));
		prefabs::create_cyan_charge(step, vec2(200, -500));

		prefabs::create_cyan_urban_machete(step, vec2(100, 100));
		const auto second_machete = prefabs::create_cyan_urban_machete(step, vec2(0, 300));

		const auto backpack = prefabs::create_sample_backpack(step, vec2(200, -650));
		prefabs::create_sample_backpack(step, vec2(200, -750));

		if (character(1).alive()) {
			perform_transfer({ rifle2, character(1).get_primary_hand() }, step);
		}

		if (character(2).alive()) {
			perform_transfer({ second_machete, character(2).get_primary_hand() }, step);
		}

		if (character(3).alive()) {
			perform_transfer({ pis2, character(3).get_primary_hand() }, step);
		}

		world.get_global_assets().cast_unsuccessful_sound.id = assets::sound_buffer_id::CAST_UNSUCCESSFUL;
		world.get_global_assets().ped_shield_impact_sound.id = assets::sound_buffer_id::EXPLOSION;
		world.get_global_assets().ped_shield_destruction_sound.id = assets::sound_buffer_id::GREAT_EXPLOSION;
		world.get_global_assets().exhausted_smoke_particles.id = assets::particle_effect_id::EXHAUSTED_SMOKE;
		
		set_standard_sentience_properties(
			world.significant.meta.global
		);

		auto& spells = world.get_global_state().spells;
		std::get<electric_triad>(spells).missile_definition = prefabs::create_electric_missile_def(step, {});

		{
			auto& in = std::get<fury_of_the_aeons>(spells).explosion;

			in.effective_radius = 250.f;
			in.damage = 88.f;
			in.impact_force = 150.f;
			in.inner_ring_color = cyan;
			in.outer_ring_color = white;
			in.sound_effect = assets::sound_buffer_id::EXPLOSION;
			in.sound_gain = 1.2f;
			in.type = adverse_element_type::FORCE;
		}


		// _controlfp(0, _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL);
#endif
	}
}
