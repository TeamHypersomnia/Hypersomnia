#include "testbed.h"
#include "game/ingredients/ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/assets/texture_id.h"

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

#include "game/messages/intent_message.h"
#include "game/detail/inventory_utils.h"

#include "texture_baker/font.h"

#include "augs/misc/machine_entropy.h"
#include "game/transcendental/cosmic_entropy.h"
#include "game/transcendental/viewing_session.h"
#include "game/transcendental/step.h"

#include "game/detail/world_camera.h"
#include "augs/gui/text/printer.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/time_utils.h"

#include "game/transcendental/cosmic_delta.h"

#include "augs/graphics/renderer.h"

namespace scene_managers {
	void testbed::populate_world_with_entities(cosmos& cosm, const vec2i screen_size) {
		cosm.advance_deterministic_schemata(cosmic_entropy(), [&](logic_step& step) { populate(step, screen_size); }, [](const_logic_step&) {});
	}

	void testbed::populate(logic_step& step, const vec2i screen_size) {
		auto& world = step.cosm;
		//const auto crate = prefabs::create_crate(world, vec2(200, 200 + 300), vec2i(100, 100) / 3);
		//const auto crate2 = prefabs::create_crate(world, vec2(400, 200 + 400), vec2i(300, 300));
		//const auto crate4 = prefabs::create_crate(world, vec2(500, 200 + 0), vec2i(100, 100));
		//
		//crates.push_back(crate);
		//crates.push_back(crate2);
		//crates.push_back(crate4);

		//for (int x = -4; x < 4; ++x) {
		//	for (int y = -4; y < 4; ++y) {
		//		auto obstacle = prefabs::create_crate(world, vec2(2000 + x * 300, 2000 + y * 300), vec2i(100, 100));
		//		crates.push_back(obstacle);
		//	}
		//}

		//for (int x = -4 * 1; x < 4 * 1; ++x)
		//{
		//	auto frog = world.create_entity("frog");
		//	ingredients::sprite(frog, vec2(100 + x * 40, 200 + 400), assets::texture_id::TEST_SPRITE, augs::white, render_layer::SMALL_DYNAMIC_BODY);
		//	ingredients::see_through_dynamic_body(frog);
		//	name_entity(frog, entity_name::CRATE);
		//	frog.add_standard_components();
		//}

		const auto car = prefabs::create_car(world, components::transform( 1490, 340, -180));
		const auto car2 = prefabs::create_car(world, components::transform(1490, 340 + 400, -180));
		const auto car3 = prefabs::create_car(world, components::transform(1490, 340 + 800, -180));

		const auto motorcycle = prefabs::create_motorcycle(world, components::transform(250, 400, -90 + 180));
		//prefabs::create_motorcycle(world, components::transform(100, -600, -90));
		const auto main_character_motorcycle = prefabs::create_motorcycle(world, components::transform(900, 48200, -90));
		
		const auto riding_car = prefabs::create_car(world, components::transform(850, 44200, -90));

		const auto riding_car2 = prefabs::create_car(world, components::transform(-850 + 1000, -8200, -90 + 180));
		const auto motorcycle2 = prefabs::create_motorcycle(world, components::transform(-1150 + 1000, -8200, -90 + 180));

		const int num_characters = 4 + 3 + 3 + 2;

		std::vector<entity_id> new_characters;
		new_characters.resize(num_characters);

		auto character = [&](const size_t i) {
			return i < new_characters.size() ? world[new_characters.at(i)] : world[entity_id()];
		};

		for (int i = 0; i < num_characters; ++i) {
			assets::animation_response_id torso_set = assets::animation_response_id::TORSO_SET;
			components::transform transform;

			if (i == 0) {
				//transform = { 0, 300, 0 };
				torso_set = assets::animation_response_id::TORSO_SET;
			}
			else if (i == 1 || i == 2) {
				if (i == 1) {
					transform = { 254, 211, 68 };
				}
				if (i == 2) {
					transform = { 1102, 213, 110 };
				}

				torso_set = assets::animation_response_id::VIOLET_TORSO_SET;
			}
			else if (i == 3) {
				torso_set = assets::animation_response_id::VIOLET_TORSO_SET;
			}

			// three rebels

			else if (i == 4) {
				transform = { -100, 20000, 0 };

				torso_set = assets::animation_response_id::BLUE_TORSO_SET;
			}
			else if (i == 5) {
				transform = { 1200, 15000, 0 };

				torso_set = assets::animation_response_id::BLUE_TORSO_SET;
			}
			else if (i == 6) {
				transform = { -300, 20000, 0 };

				torso_set = assets::animation_response_id::BLUE_TORSO_SET;
			}

			// three metropolitan soldiers
			else if (i == 7) {
				transform = { -300, -3000, 0 };

				torso_set = assets::animation_response_id::VIOLET_TORSO_SET;
			}
			else if (i == 8) {
				transform = { -400, -3000, 0 };

				torso_set = assets::animation_response_id::VIOLET_TORSO_SET;
			}
			else if (i == 9) {
				transform = { -500, -3000, 0 };

				torso_set = assets::animation_response_id::VIOLET_TORSO_SET;
			}



			const auto new_character = prefabs::create_character(world, transform, screen_size, typesafe_sprintf("player%x", i), torso_set);

			new_characters[i] = new_character;

			if (i == 0) {
				new_character.get<components::sentience>().health.value = 100;
				new_character.get<components::sentience>().health.maximum = 100;
				new_character.get<components::attitude>().parties = party_category::RESISTANCE_CITIZEN;
				new_character.get<components::attitude>().hostile_parties = party_category::METROPOLIS_CITIZEN;
			}
			if (i == 1) {
				new_character.get<components::attitude>().maximum_divergence_angle_before_shooting = 25;
				new_character.get<components::sentience>().minimum_danger_amount_to_evade = 20;
				new_character.get<components::sentience>().health.value = 300;
				new_character.get<components::sentience>().health.maximum = 300;
				//ingredients::standard_pathfinding_capability(new_character);
				//ingredients::soldier_intelligence(new_character);
				new_character.recalculate_basic_processing_categories();
			}
			if (i == 2) {
				new_character.get<components::sentience>().health.value = 100;
			}
			if (i == 5) {
				new_character.get<components::attitude>().maximum_divergence_angle_before_shooting = 25;
				new_character.get<components::sentience>().minimum_danger_amount_to_evade = 20;
				new_character.get<components::sentience>().health.value = 300;
				new_character.get<components::sentience>().health.maximum = 300;
				//ingredients::standard_pathfinding_capability(new_character);
				//ingredients::soldier_intelligence(new_character);
				new_character.recalculate_basic_processing_categories();
			}

			if (
				i == 4 || i == 5 || i == 6
				) {
				const auto rifle = prefabs::create_sample_rifle(step, vec2(100, -500),
					prefabs::create_sample_magazine(step, vec2(100, -650), "0.4",
						(i == 5 ? prefabs::create_pink_charge : prefabs::create_cyan_charge)(world, vec2(0, 0), 30)));

				name_entity(new_character, entity_name::PERSON, L"Rebel");
				perform_transfer({ rifle, new_character[slot_function::PRIMARY_HAND] }, step);

				new_character.get<components::attitude>().parties = party_category::RESISTANCE_CITIZEN;
				new_character.get<components::attitude>().hostile_parties = party_category::METROPOLIS_CITIZEN;
			}

			if (
				i == 7 || i == 8 || i == 9
				) {

				name_entity(new_character, entity_name::PERSON, L"Hunter");
				
				if (i == 8) {
					name_entity(new_character, entity_name::PERSON, L"Commander");
				}

				if (i == 9) {
					const auto rifle = prefabs::create_kek9(step, vec2(100, -500),
						prefabs::create_small_magazine(step, vec2(100, -650), "3.4",
							prefabs::create_pink_charge(world, vec2(0, 0), 300)));

					perform_transfer({ rifle, new_character[slot_function::PRIMARY_HAND] }, step);
				}
				else {
					const auto rifle = (i == 7 ? prefabs::create_submachine : prefabs::create_sample_bilmer2000)(step, vec2(100, -500),
						prefabs::create_sample_magazine(step, vec2(100, -650), "3.4",
							prefabs::create_pink_charge(world, vec2(0, 0), 300)));

					perform_transfer({ rifle, new_character[slot_function::PRIMARY_HAND] }, step);
				}

				const auto backpack = prefabs::create_sample_backpack(world, vec2(200, -650));
				perform_transfer({ backpack, new_character[slot_function::SHOULDER_SLOT] }, step);
			}

			if (i == 10) {
				driver_system().assign_car_ownership(new_character, riding_car2);
			}

			if (i == 11) {
				driver_system().assign_car_ownership(new_character, motorcycle2);
			}
		}

		{


		}

		// street wandering pixels
		{
			const auto reach = xywh(0, 0, 1500, 32000);

			{
				const auto e = world.create_entity("wandering_pixels");
				auto& w = e += components::wandering_pixels();
				auto& r = e += components::render();

				r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

				w.face.set(assets::texture_id(int(assets::texture_id::BLANK)), cyan);
				w.face.size.set(1, 1);
				w.count = 200;
				w.reach = reach;
				e.add_standard_components();
			}

			{
				const auto e = world.create_entity("wandering_pixels");
				auto& w = e += components::wandering_pixels();
				auto& r = e += components::render();

				r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

				w.face.set(assets::texture_id(int(assets::texture_id::BLINK_FIRST) + 2), cyan);
				//w.face.size.set(1, 1);
				w.count = 80;
				w.reach = reach;
				e.add_standard_components();
			}

			{
				const auto e = world.create_entity("wandering_pixels");
				auto& w = e += components::wandering_pixels();
				auto& r = e += components::render();

				r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

				w.face.set(assets::texture_id(int(assets::texture_id::BLINK_FIRST) + 2), cyan);
				//w.face.size.set(1, 1);
				w.count = 80;
				w.reach = reach;
				e.add_standard_components();
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
				prefabs::create_crate(world, c + vec2(-100, 400) );
				prefabs::create_crate(world, c + vec2(300, 300) );
				prefabs::create_crate(world, c + vec2(100, -200) );

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
					l.add_standard_components();
				}


				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::texture_id(int(assets::texture_id::BLANK)), light_cyan);
					w.face.size.set(1, 1);
					w.count = 50;
					w.reach = xywh(light_pos.x- 250, light_pos.y-250, 500, 500);
					e.add_standard_components();
				}

				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::texture_id(int(assets::texture_id::BLINK_FIRST) + 2), light_cyan);
					//w.face.size.set(1, 1);
					w.count = 20;
					w.reach = xywh(light_pos.x - 150, light_pos.y - 150, 300, 300);
					e.add_standard_components();
				}

				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::texture_id(int(assets::texture_id::BLINK_FIRST) + 2), light_cyan);
					//w.face.size.set(1, 1);
					w.count = 20;
					w.reach = xywh(light_pos.x - 25, light_pos.y - 25, 50, 50);
					e.add_standard_components();
				}
			}
		}

		{
			//{
			//	const auto l = world.create_entity("l");
			//	l += components::transform(0, 300);
			//	auto& light = l += components::light();
			//	light.color = red;
			//	l.add_standard_components();
			//}
			//{
			//	const auto l = world.create_entity("l");
			//	l += components::transform(300, 300);
			//	auto& light = l += components::light();
			//	light.color = green;
			//	l.add_standard_components();
			//}
			//{
			//	const auto l = world.create_entity("l");
			//	l += components::transform(600, 300);
			//	auto& light = l += components::light();
			//	light.color = blue;
			//	l.add_standard_components();
			//}
			{
				const auto l = world.create_entity("l");
				l += components::transform(164.f - 8.f + 90.f, 220);
				auto& light = l += components::light();
				light.color = cyan;
				light.max_distance.base_value = 4500.f;
				light.wall_max_distance.base_value = 4000.f;
				l.add_standard_components();
			}
			{
				const auto l = world.create_entity("l");
				l += components::transform(1164.f + 24.f - 90.f, 220);
				auto& light = l += components::light();
				light.color = orange;
				light.max_distance.base_value = 4500.f;
				light.wall_max_distance.base_value = 4000.f;
				l.add_standard_components();
			}
			{
				const auto left_reach = xywh(164.f - 8.f + 90.f - 550, 220 - 250, 1000, 600);
				const auto right_reach = xywh(1164.f - 8.f + 90.f - 600, 220 - 250, 1000, 600);

				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::texture_id(int(assets::texture_id::BLINK_FIRST) + 2), cyan);
					//w.face.size.set(1, 1);
					w.count = 20;
					w.reach = left_reach;
					e.add_standard_components();
				}

				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::texture_id(int(assets::texture_id::BLINK_FIRST) + 2), orange);
					//w.face.size.set(1, 1);
					w.count = 20;
					w.reach = right_reach;
					e.add_standard_components();
				}

				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::texture_id(int(assets::texture_id::BLANK)), cyan);
					w.face.size.set(1, 1);
					w.count = 50;
					w.reach = left_reach;
					e.add_standard_components();
				}

				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::texture_id(int(assets::texture_id::BLANK)), orange);
					w.face.size.set(1, 1);
					w.count = 50;
					w.reach = right_reach;
					e.add_standard_components();
				}

				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::texture_id(int(assets::texture_id::BLANK)), cyan);
					w.face.size.set(2, 2);
					w.count = 30;
					w.reach = left_reach;
					e.add_standard_components();
				}

				{
					const auto e = world.create_entity("wandering_pixels");
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.face.set(assets::texture_id(int(assets::texture_id::BLANK)), orange);
					w.face.size.set(2, 2);
					w.count = 30;
					w.reach = right_reach;
					e.add_standard_components();
				}

				//{
				//	const auto e = world.create_entity("wandering_pixels");
				//	auto& w = e += components::wandering_pixels();
				//	auto& r = e += components::render();
				//
				//	r.layer = render_layer::WANDERING_PIXELS_EFFECTS;
				//
				//	w.face.set(assets::texture_id(int(assets::texture_id::WANDERING_CROSS)), cyan);
				//	w.count = 15;
				//	w.reach = xywh(164.f - 8.f + 90.f - 100, 220 - 100, 200, 200);
				//	e.add_standard_components();
				//}
				//
				//{
				//	const auto e = world.create_entity("wandering_pixels");
				//	auto& w = e += components::wandering_pixels();
				//	auto& r = e += components::render();
				//
				//	r.layer = render_layer::WANDERING_PIXELS_EFFECTS;
				//
				//	w.face.set(assets::texture_id(int(assets::texture_id::WANDERING_CROSS)), orange);
				//	w.count = 15;
				//	w.reach = xywh(1164.f - 8.f + 90.f - 100, 220 - 100, 200, 200);
				//	e.add_standard_components();
				//}
			}

			{
				const auto l = world.create_entity("l");
				l += components::transform(164.f - 8.f, -700);
				auto& light = l += components::light();
				light.color = cyan;
				//light.linear.base_value = 0.000005f;
				//light.quadratic.base_value = 0.000025f;
				light.max_distance.base_value = 4500.f;
				light.wall_max_distance.base_value = 4000.f;
				l.add_standard_components();
			}
			{
				const auto l = world.create_entity("l");
				l += components::transform(1164.f + 24.f, -700);
				auto& light = l += components::light();
				light.color = orange;
				light.max_distance.base_value = 4500.f;
				light.wall_max_distance.base_value = 4000.f;
				l.add_standard_components();
			}

			messages::create_particle_effect effect;
			effect.place_of_birth = components::transform(-164, 500, 0);
			effect.input.effect = assets::particle_effect_id::WANDERING_SMOKE;
			effect.input.randomize_position_within_radius = 500.f;
			effect.input.single_displacement_duration_ms.set(400.f, 1500.f);

			particles_existence_system().create_particle_effect_entity(world, effect).add_standard_components();

			{
				const auto e = world.create_entity("tiled_floor");
				
				e += components::transform(0.f, -1200.f);
				auto& tile_layer_instance = e += components::tile_layer_instance();
				auto& render = e += components::render();
				
				render.layer = render_layer::TILED_FLOOR;
				tile_layer_instance.id = assets::tile_layer_id::METROPOLIS_FLOOR;


				e.add_standard_components();
			}

			{
				const auto e = world.create_entity("have_a_pleasant");

				e += components::transform(164.f - 8.f, -60.f - 20.f);
				auto& render = e += components::render();
				auto& sprite = e += components::sprite();

				render.layer = render_layer::EFFECTS;
				sprite.set(assets::texture_id::HAVE_A_PLEASANT);

				e.add_standard_components();

				prefabs::create_brick_wall(world, components::transform(3 + 1 + 1100, -32 - 96), { 160, 160 });
				prefabs::create_brick_wall(world, components::transform(3 + 1 + 1100 + 160, -32 - 96), { 160, 160 });
				prefabs::create_brick_wall(world, components::transform(3 + 1 + 1100 + 160, -32 - 96 + 160), { 160, 160 });
				prefabs::create_brick_wall(world, components::transform(3 + 1 + 1100, -32 - 96 + 160), { 160, 160 });


				prefabs::create_brick_wall(world, components::transform(-3 -16 + 100, -32 - 96), { 160, 160 });
				prefabs::create_brick_wall(world, components::transform(-3 -16 + 100 + 160, -32 - 96), { 160, 160 });
				prefabs::create_brick_wall(world, components::transform(-3 -16 + 100 + 160, -32 - 96 + 160), { 160, 160 });
				prefabs::create_brick_wall(world, components::transform(-3 -16 + 100, -32 - 96 + 160), { 160, 160 });

				for (int b = 0; b < 8; ++b) {
					prefabs::create_brick_wall(world, components::transform(3 + 1 + 1100 + 160 + 160, -32 - 96 + 160 - 160.f * b, 90), { 160, 160 });
					prefabs::create_brick_wall(world, components::transform(-3 - 16 + 100 - 160, -32 - 96 + 160 - 160*b, 90), { 160, 160 });
				}

				const auto bg_size = assets::get_size(assets::texture_id::TEST_BACKGROUND);

				const int num_floors = 10 * 10;
				const int side = sqrt(num_floors) / 2;

				for (int x = -side; x < side; ++x) {
					for (int y = -side; y < side * 16; ++y)
					{
						//auto background = world.create_entity("bg[-]");
						//ingredients::sprite(background, vec2(-1000, 0) + vec2(x, y) * (bg_size + vec2(1500, 550)), assets::texture_id::TEST_BACKGROUND, augs::white, render_layer::GROUND);
						//ingredients::standard_static_body(background);

						auto street = world.create_entity("street[-]");
						ingredients::sprite(street, { bg_size * vec2(x, y) },
							assets::texture_id::TEST_BACKGROUND, augs::gray1, render_layer::GROUND);

						//background.add_standard_components();
						street.add_standard_components();
					}
				}

				const auto size = assets::get_size(assets::texture_id::ROAD_FRONT_DIRT);

				{
					auto road_dirt = world.create_entity("road_dirt[-]");
					ingredients::sprite(road_dirt, vec2(-3 - 16 + 100 + 160 + 80 + size.x / 2, -32 - 96 + 160 + 80 - size.y / 2),
						assets::texture_id::ROAD_FRONT_DIRT, white, render_layer::ON_TILED_FLOOR);

					road_dirt.add_standard_components();
				}

				for (int r = 0; r < 38; ++r) {
					const auto size = assets::get_size(assets::texture_id::ROAD);

					auto road = world.create_entity("road[-]");
					ingredients::sprite(road, vec2(-3 - 16 + 100 + 160 + 80 + size.x / 2, -32 - 96 + 160 + 80 + size.y / 2 + size.y*r),
						assets::texture_id::ROAD, white, render_layer::ON_GROUND);

					road.add_standard_components();
				}
			}

			{
				const auto e = world.create_entity("awakening");

				e += components::transform(164.f - 8.f, -60.f -20.f + 40.f);
				auto& render = e += components::render();
				auto& sprite = e += components::sprite();

				render.layer = render_layer::EFFECTS;
				sprite.set(assets::texture_id::AWAKENING);
				sprite.effect = components::sprite::special_effect::COLOR_WAVE;

				e.add_standard_components();
			}

			{
				const auto e = world.create_entity("metropolis");

				e += components::transform(1164.f + 24.f, -60.f);
				auto& render = e += components::render();
				auto& sprite = e += components::sprite();

				render.layer = render_layer::EFFECTS;
				sprite.set(assets::texture_id::METROPOLIS);

				e.add_standard_components();
			}
		}

		if (character(0).alive()) {
			name_entity(character(0), entity_name::PERSON, L"Newborn");

			driver_system().assign_car_ownership(character(0), main_character_motorcycle);
			main_character_motorcycle.get<components::car>().accelerating = true;
		}

		if (character(3).alive()) {
			driver_system().assign_car_ownership(character(3), riding_car);
			riding_car.get<components::car>().accelerating = true;
		}

		select_character(character(0));

		prefabs::create_sample_suppressor(world, vec2(300, -500));

		const bool many_charges = false;

		const auto rifle = prefabs::create_sample_rifle(step, vec2(100, -500),
			prefabs::create_sample_magazine(step, vec2(100, -650), many_charges ? "10" : "0.3",
				prefabs::create_cyan_charge(world, vec2(0, 0), many_charges ? 1000 : 30)));

		const auto rifle2 = prefabs::create_sample_bilmer2000(step, vec2(100, -500 + 50),
			prefabs::create_sample_magazine(step, vec2(100, -650), true ? "10" : "0.3",
				prefabs::create_cyan_charge(world, vec2(0, 0), true ? 1000 : 30)));

		prefabs::create_sample_rifle(step, vec2(100, -500 + 100));

		prefabs::create_kek9(step, vec2(300, -500 + 50));

		const auto pis2 = prefabs::create_kek9(step, vec2(300, 50),
			prefabs::create_small_magazine(step, vec2(100, -650), "0.4",
				prefabs::create_cyan_charge(world, vec2(0, 0), 40)));

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
		const auto second_machete = prefabs::create_cyan_urban_machete(world, vec2(0, 300));

		const auto backpack = prefabs::create_sample_backpack(world, vec2(200, -650));
		prefabs::create_sample_backpack(world, vec2(200, -750));

		//perform_transfer({ backpack, character(0)[slot_function::SHOULDER_SLOT] }, step);
		//perform_transfer({ submachine, character(0)[slot_function::PRIMARY_HAND] }, step);
		//perform_transfer({ rifle, character(0)[slot_function::SECONDARY_HAND] }, step);

		if (character(1).alive()) {
			name_entity(character(1), entity_name::PERSON, L"Sentinel");
			perform_transfer({ rifle2, character(1)[slot_function::PRIMARY_HAND] }, step);
		}

		if (character(2).alive()) {
			name_entity(character(2), entity_name::PERSON, L"Sentinel");
			perform_transfer({ second_machete, character(2)[slot_function::PRIMARY_HAND] }, step);
		}

		if (character(3).alive()) {
			name_entity(character(3), entity_name::PERSON, L"Medic");
			perform_transfer({ pis2, character(3)[slot_function::PRIMARY_HAND] }, step);
		}

		characters.assign(new_characters.begin(), new_characters.end());
		// _controlfp(0, _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL);
	}

	entity_id testbed::get_selected_character() const {
		return selected_character;
	}

	void testbed::select_character(const entity_id h) {
		selected_character = h;
	}

	void testbed::configure_view(viewing_session& session) const {
		auto& active_context = session.context;

		active_context.map_key_to_intent(window::event::keys::key::W, intent_type::MOVE_FORWARD);
		active_context.map_key_to_intent(window::event::keys::key::S, intent_type::MOVE_BACKWARD);
		active_context.map_key_to_intent(window::event::keys::key::A, intent_type::MOVE_LEFT);
		active_context.map_key_to_intent(window::event::keys::key::D, intent_type::MOVE_RIGHT);

		active_context.map_event_to_intent(window::event::message::mousemotion, intent_type::MOVE_CROSSHAIR);
		active_context.map_key_to_intent(window::event::keys::key::LMOUSE, intent_type::CROSSHAIR_PRIMARY_ACTION);
		active_context.map_key_to_intent(window::event::keys::key::RMOUSE, intent_type::CROSSHAIR_SECONDARY_ACTION);

		active_context.map_key_to_intent(window::event::keys::key::E, intent_type::USE_BUTTON);
		active_context.map_key_to_intent(window::event::keys::key::LSHIFT, intent_type::SPRINT);

		active_context.map_key_to_intent(window::event::keys::key::G, intent_type::THROW_PRIMARY_ITEM);
		active_context.map_key_to_intent(window::event::keys::key::H, intent_type::HOLSTER_PRIMARY_ITEM);

		active_context.map_key_to_intent(window::event::keys::key::BACKSPACE, intent_type::SWITCH_LOOK);

		active_context.map_key_to_intent(window::event::keys::key::LCTRL, intent_type::START_PICKING_UP_ITEMS);

		active_context.map_key_to_intent(window::event::keys::key::SPACE, intent_type::SPACE_BUTTON);
		active_context.map_key_to_intent(window::event::keys::key::MOUSE4, intent_type::SWITCH_TO_GUI);
		
		active_context.map_key_to_intent(window::event::keys::key::F, intent_type::SWITCH_WEAPON_LASER);

		active_context.map_key_to_intent(window::event::keys::key::_0, intent_type::HOTBAR_BUTTON_0);
		active_context.map_key_to_intent(window::event::keys::key::_1, intent_type::HOTBAR_BUTTON_1);
		active_context.map_key_to_intent(window::event::keys::key::_2, intent_type::HOTBAR_BUTTON_2);
		active_context.map_key_to_intent(window::event::keys::key::_3, intent_type::HOTBAR_BUTTON_3);
		active_context.map_key_to_intent(window::event::keys::key::_4, intent_type::HOTBAR_BUTTON_4);
		active_context.map_key_to_intent(window::event::keys::key::_5, intent_type::HOTBAR_BUTTON_5);
		active_context.map_key_to_intent(window::event::keys::key::_6, intent_type::HOTBAR_BUTTON_6);
		active_context.map_key_to_intent(window::event::keys::key::_7, intent_type::HOTBAR_BUTTON_7);
		active_context.map_key_to_intent(window::event::keys::key::_8, intent_type::HOTBAR_BUTTON_8);
		active_context.map_key_to_intent(window::event::keys::key::_9, intent_type::HOTBAR_BUTTON_9);

		active_context.map_key_to_intent(window::event::keys::key::Q, intent_type::PREVIOUS_HOTBAR_SELECTION_SETUP);
	}

	//if (raw_input.key == augs::window::event::keys::key::F7) {
	//	auto target_folder = "saves/" + augs::get_timestamp();
	//	augs::create_directories(target_folder);
	//
	//	main_cosmos.save_to_file(target_folder + "/" + "save.state");
	//}
	//if (raw_input.key == augs::window::event::keys::key::F4) {
	//	cosmos cosm_with_guids;
	//	cosm_with_guids.significant = stashed_cosmos.significant;
	//	cosm_with_guids.remap_guids();
	//
	//	ensure(stashed_delta.get_write_pos() == 0);
	//	cosmic_delta::encode(cosm_with_guids, main_cosmos, stashed_delta);
	//
	//	stashed_delta.reset_read_pos();
	//	cosmic_delta::decode(cosm_with_guids, stashed_delta);
	//	stashed_delta.reset_write_pos();
	//
	//	main_cosmos = cosm_with_guids;
	//}
	//if (raw_input.key == augs::window::event::keys::key::F8) {
	//	main_cosmos.profiler.duplication.new_measurement();
	//	stashed_cosmos = main_cosmos;
	//	main_cosmos.profiler.duplication.end_measurement();
	//}
	//if (raw_input.key == augs::window::event::keys::key::F9) {
	//	main_cosmos = stashed_cosmos;
	//}
	//if (raw_input.key == augs::window::event::keys::key::F10) {
	//}

	void testbed::control_character_selection(const augs::machine_entropy::local_type& local) {
		for (const auto& raw_input : local) {
			if (raw_input.was_any_key_pressed()) {
				if (raw_input.key == augs::window::event::keys::key::CAPSLOCK) {
					++current_character_index;
					current_character_index %= characters.size();

					select_character(characters[current_character_index]);
				}
			}
		}
	}

	//auto& cosmos = cosm;
	//auto& ln = augs::renderer::get_current().logic_lines;
	//
	//for (auto crate : crates) {
	//	auto& fixes = cosmos[crate].get<components::fixtures>();
	//
	//	auto& dest = fixes.get_modifiable_destruction_data({ 0, 0 });
	//
	//	for (auto scar : dest.scars) {
	//		ln.draw_cyan(scar.first_impact, scar.depth_point);
	//	}
	//}

	//for (auto& tested : draw_bodies) {
	//	auto& s = tested.get<components::physics_definition>();
	//
	//	auto& lines = renderer::get_current().logic_lines;
	//
	//	auto vv = s.fixtures[0].debug_original;
	//
	//	for (int i = 0; i < vv.size(); ++i) {
	//		auto& tt = tested.logic_transform();
	//		auto pos = tt.pos;
	//
	//		lines.draw_cyan((pos + vv[i]).rotate(tt.rotation, pos), (pos + vv[(i + 1) % vv.size()]).rotate(tt.rotation, pos));
	//	}
	//}

	//auto ff = (new_characters[1].get<components::pathfinding>().get_current_navigation_point() - position(new_characters[1])).set_length(15000);
	//new_characters[1].get<components::physics>().apply_force(ff);

	// LOG("F: %x", ff);
}
