/*
	Disable float/int warnings, this is just a content script
*/
#if PLATFORM_WINDOWS
#pragma warning(disable : 4244)
#endif
#include "augs/templates/algorithm_templates.h"
#include "game/assets/ids/asset_ids.h"
#include "game/assets/all_logical_assets.h"

#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/scenes/testbed.h"
#include "test_scenes/ingredients/ingredients.h"
#include "test_scenes/test_scenes_content.h"

#include "game/transcendental/cosmos.h"
#include "game/organization/all_component_includes.h"
#include "game/organization/all_messages_includes.h"

#include "game/stateless_systems/input_system.h"
#include "game/stateless_systems/particles_existence_system.h"
#include "game/stateless_systems/car_system.h"
#include "game/stateless_systems/driver_system.h"

#include "game/enums/party_category.h"
#include "game/detail/describers.h"

#include "game/messages/intent_message.h"
#include "game/detail/inventory/perform_transfer.h"

#include "view/viewables/image_cache.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmic_delta.h"

namespace test_scenes {
	entity_id testbed::populate(const loaded_image_caches_map& caches, const logic_step step) const {
		auto& world = step.get_cosmos();
		
#if TODO
		const auto car = prefabs::create_car(step, components::transform( { 1490, 340 }, -180));
		const auto car2 = prefabs::create_car(step, components::transform({ 1490, 340 + 400 }, -180));
		const auto car3 = prefabs::create_car(step, components::transform({ 1490, 340 + 800 }, -180));

		const auto riding_car = prefabs::create_car(step, components::transform({ 850, 1000 }, -90));

		const auto riding_car2 = prefabs::create_car(step, components::transform({ -850 + 1000, -8200 }, -90 + 180));
#endif

		const int num_characters = 10;

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

		std::vector<components::transform> character_transforms = {
			{ { 0, 300 }, 0 },
			{ { 254, 211 }, 68 },
			{ { 1102, 213 }, 110 },
			{ { 1102, 413 }, 110 },
			{ { -100, 20000 }, 0 },
			{ { 1200, 15000 }, 0 },
			{ { -300, 20000 }, 0 },
			{ { -300, -2000 }, 0 },
			{ { -400, -2000 }, 0 },
			{ { -500, -2000 }, 0 }
		};

		std::vector<entity_id> new_characters;
		new_characters.resize(num_characters);

		auto character = [&](const size_t i) {
			return i < new_characters.size() ? world[new_characters.at(i)] : world[entity_id()];
		};

		for (int i = 0; i < num_characters; ++i) {
			auto transform = character_transforms[i];

			const bool is_metropolis = i % 2 == 0;
			const auto new_character = (is_metropolis ? prefabs::create_metropolis_soldier : prefabs::create_resistance_soldier)(step, transform, typesafe_sprintf("player%x", i));

			new_characters[i] = new_character;

			auto& sentience = new_character.get<components::sentience>();

			sentience.get<consciousness_meter_instance>().set_maximum_value(400);
			sentience.get<consciousness_meter_instance>().set_value(400);

			sentience.get<personal_electricity_meter_instance>().set_maximum_value(400);
			sentience.get<personal_electricity_meter_instance>().set_value(400);

			new_character.get<components::attitude>().maximum_divergence_angle_before_shooting = 25;

			if (i == 0 || i == 1) {
				/* Let's have two OP characters */
				sentience.get<health_meter_instance>().set_value(10000);
				sentience.get<health_meter_instance>().set_maximum_value(10000);

				sentience.get<personal_electricity_meter_instance>().set_value(10000);
				sentience.get<personal_electricity_meter_instance>().set_maximum_value(10000);

				const auto rifle = (is_metropolis ? prefabs::create_sample_rifle : prefabs::create_vindicator)(
					step, vec2(100, -500), prefabs::create_sample_magazine(step, vec2(100, -650), prefabs::create_steel_charge(step, vec2(0, 0)), 30)
				);

				if (i == 1) {
					new_character.get<components::crosshair>().base_offset.x = -500;
				}

				perform_transfer(item_slot_transfer_request::standard(rifle, new_character.get_primary_hand()), step);
			}

			if (i == 7 || i == 8 || i == 9) {
				/* Give some stuff to three test characters */
				if (i == 9) {
					const auto rifle = prefabs::create_sample_rifle(step, vec2(100, -500),
						prefabs::create_sample_magazine(step, vec2(100, -650),
							prefabs::create_cyan_charge(step, vec2(0, 0))));

					perform_transfer(item_slot_transfer_request::standard(rifle, new_character.get_primary_hand()), step);
				}
				else {
					const auto rifle = (i == 7 ? prefabs::create_sample_rifle : prefabs::create_sample_rifle)(step, vec2(100, -500),
						prefabs::create_sample_magazine(step, vec2(100, -650),
							prefabs::create_cyan_charge(step, vec2(0, 0))));

					perform_transfer(item_slot_transfer_request::standard(rifle, new_character.get_primary_hand()), step);
				}

				const auto backpack = (is_metropolis ? prefabs::create_sample_backpack : prefabs::create_brown_backpack)(step, vec2(200, -650));
				perform_transfer(item_slot_transfer_request::standard(backpack, new_character[slot_function::SHOULDER]), step);
			}

			fill_range(sentience.learned_spells, true);
		}

		{
			const vec2 coords[] = {
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
					const auto l = create_test_scene_entity(world, test_static_lights::STRONG_LAMP);
					l.set_logic_transform(components::transform(light_pos));

					auto& light = l.get<components::light>();

					light.color = light_cyan;
				}

				{
					create_test_scene_entity(world, test_wandering_pixels_decorations::WANDERING_PIXELS, [&](const auto e){
						auto& w = e.template get<components::wandering_pixels>();

						w.colorize = light_cyan;
						w.particles_count = 150;
						w.set_reach(xywh(light_pos.x- 350, light_pos.y-350, 500, 500));
					});
				}
			}
		}

		{
			{
				const auto l = create_test_scene_entity(world, test_static_lights::STRONG_LAMP);
				l.set_logic_transform(components::transform(vec2(-44, 270)));
				auto& light = l.get<components::light>();
				light.color = cyan;
			}
			{
				const auto l = create_test_scene_entity(world, test_static_lights::STRONG_LAMP);
				l.set_logic_transform(components::transform(vec2(1098, 220)));
				auto& light = l.get<components::light>();
				light.color = orange;
			}
			{
				const auto l = create_test_scene_entity(world, test_static_lights::STRONG_LAMP);
				l.set_logic_transform(components::transform(vec2(223, -47)));
				auto& light = l.get<components::light>();
				light.color = cyan;
			}

			{
				const auto left_reach = xywh(164.f - 8.f + 90.f - 550, 220 - 250, 1000, 600);
				const auto right_reach = xywh(1164.f - 8.f + 90.f - 600, 220 - 250, 1000, 600);

				{
					create_test_scene_entity(world, test_wandering_pixels_decorations::WANDERING_PIXELS, [&](const auto e){

						auto& w = e.template get<components::wandering_pixels>();

						w.colorize = cyan;
						w.particles_count = 50;
						w.set_reach(left_reach);
					});
				}

				{
					create_test_scene_entity(world, test_wandering_pixels_decorations::WANDERING_PIXELS, [&](const auto e){

						auto& w = e.template get<components::wandering_pixels>();

						w.colorize = orange;
						w.particles_count = 50;
						w.set_reach(right_reach);
					});
				}
			}


			{
				create_test_scene_entity(world, test_sprite_decorations::HAVE_A_PLEASANT, components::transform(vec2(-42, -32)));
				create_test_scene_entity(world, test_sprite_decorations::AWAKENING, components::transform(vec2(-42, 8)));
				create_test_scene_entity(world, test_sprite_decorations::METROPOLIS, components::transform(vec2(1106, 3)));

				auto half = vec2(-64, -64);
				prefabs::create_brick_wall(step, half + vec2(0, 0));
				prefabs::create_brick_wall(step, half +vec2(128, 128));
				prefabs::create_brick_wall(step, half +vec2(128, 0));
				prefabs::create_brick_wall(step, half +vec2(0, 128));

				const auto horioff = 8 * 128;

				prefabs::create_brick_wall(step, half +vec2(horioff, 0) + vec2(0, 0));
				prefabs::create_brick_wall(step, half +vec2(horioff, 0) + vec2(128, 128));
				prefabs::create_brick_wall(step, half +vec2(horioff, 0) + vec2(128, 0));
				prefabs::create_brick_wall(step, half +vec2(horioff, 0) + vec2(0, 128));

				for (int b = 0; b < 10; ++b) {
					prefabs::create_brick_wall(step, components::transform(half +vec2(-128, -128 - b*128) + vec2(0, 256), 90));
					prefabs::create_brick_wall(step, components::transform(half +vec2(horioff + 256, -128 - b*128) + vec2(0, 256), 90));
				}

				{
					const vec2 bg_size = caches.at(to_image_id(test_scene_image_id::SOIL)).get_original_size();

					const auto num_roads = 10 * 10;
					const auto side = static_cast<int>(sqrt(num_roads) / 2);

					for (int x = -side; x < side; ++x) {
						for (int y = -side; y < side * 16; ++y) {
							create_test_scene_entity(world, test_sprite_decorations::SOIL, components::transform{ bg_size * vec2i(x, y) });
						}
					}
				}

				create_test_scene_entity(world, test_sprite_decorations::ROAD_DIRT, components::transform(vec2(468, 112)));

				for (int r = 0; r < 38; ++r) {
					const vec2 size = caches.at(to_image_id(test_scene_image_id::ROAD)).get_original_size();

					create_test_scene_entity(world, test_sprite_decorations::ROAD, components::transform{ vec2(468, 832+ size.y * r ) });
				}

				{
					const vec2 size = caches.at(to_image_id(test_scene_image_id::FLOOR)).get_original_size();

					for (int x = 0; x < 10; ++x) {
						for (int y = 0; y < 10; ++y) {
							create_test_scene_entity(world, test_sprite_decorations::FLOOR, components::transform(vec2(-64, -192) + size * vec2i(x, -y)));
						}
					}
				}

			}
		}

		prefabs::create_kek9(step, vec2(-800, -200),
			prefabs::create_sample_magazine(step, vec2(100, -650),
				prefabs::create_cyan_charge(step, vec2(0, 0))));

		prefabs::create_amplifier_arm(step, vec2(-300, -500 + 50));
		prefabs::create_cyan_urban_machete(step, vec2(100, 100));

		prefabs::create_sample_backpack(step, vec2(200, -750));
		prefabs::create_brown_backpack(step, vec2(280, -750));

		prefabs::create_rifle(step, vec2(280, -750), test_shootable_weapons::DATUM_GUN, prefabs::create_sample_magazine(step, vec2(100, -650), prefabs::create_cyan_charge(step, vec2(0, 0)), 25));
		prefabs::create_rifle(step, vec2(0, -100), test_shootable_weapons::LEWSII, prefabs::create_magazine(step, vec2(100, -650), test_container_items::LEWSII_MAG, prefabs::create_steel_charge(step, vec2(0, 0)), 10000));

		if (character(2).alive()) {
			const auto second_machete = prefabs::create_cyan_urban_machete(step, vec2(0, 300));
			perform_transfer(item_slot_transfer_request::standard(second_machete, character(2).get_primary_hand()), step);
		}

		return character(1);
	}
}
