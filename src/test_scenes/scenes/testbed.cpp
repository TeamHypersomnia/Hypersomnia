/*
	Disable float/int warnings, this is just a content script
*/
#pragma warning(disable : 4244)
#include "augs/templates/algorithm_templates.h"
#include "game/assets/ids/game_image_id.h"
#include "game/assets/all_logical_assets.h"

#include "test_scenes/test_scene_types.h"
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

#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmic_delta.h"

namespace test_scenes {
	void testbed::populate(cosmos_common_significant& common) const {
		populate_test_scene_types(common.logical_assets, common.all_entity_types);

		auto& common_assets = common.assets;
		common_assets.cast_unsuccessful_sound.id = assets::sound_buffer_id::CAST_UNSUCCESSFUL;
		common_assets.ped_shield_impact_sound.id = assets::sound_buffer_id::EXPLOSION;
		common_assets.ped_shield_destruction_sound.id = assets::sound_buffer_id::GREAT_EXPLOSION;
		common_assets.item_throw_sound.id = assets::sound_buffer_id::ITEM_THROW;
		common_assets.item_throw_sound.modifier.pitch = 1.15f;
		common_assets.item_throw_sound.modifier.gain = 0.8f;

		common_assets.exhausted_smoke_particles.id = assets::particle_effect_id::EXHAUSTED_SMOKE;
		common_assets.exploding_ring_smoke = assets::particle_effect_id::EXPLODING_RING_SMOKE;
		common_assets.exploding_ring_sparkles = assets::particle_effect_id::EXPLODING_RING_SPARKLES;
		common_assets.thunder_remnants = assets::particle_effect_id::THUNDER_REMNANTS;

		load_test_scene_sentience_properties(common);

		auto& spells = common.spells;
		//std::get<electric_triad>(spells).missile_definition = prefabs::create_electric_missile_def(step, {});
		// _controlfp(0, _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL);
	}

	entity_id testbed::populate(const logic_step step) const {
		auto& world = step.get_cosmos();
		const auto& metas = step.get_logical_assets();
		
		const auto car = prefabs::create_car(step, components::transform( 1490, 340, -180));
		const auto car2 = prefabs::create_car(step, components::transform(1490, 340 + 400, -180));
		const auto car3 = prefabs::create_car(step, components::transform(1490, 340 + 800, -180));

		const auto riding_car = prefabs::create_car(step, components::transform(850, 1000, -90));

		const auto riding_car2 = prefabs::create_car(step, components::transform(-850 + 1000, -8200, -90 + 180));

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
				transform = { 0, 300, 0 };
				//torso_set = assets::animation_response_id::TORSO_SET;
			}
			else if (i == 1 || i == 2) {
				if (i == 1) {
					transform = { 254, 211, 68 };
				}
				if (i == 2) {
					transform = { 1102, 213, 110 };
				}

			}
			else if (i == 3) {
				transform = riding_car.get_logic_transform();
			}

			else if (i == 4) {
				transform = { -100, 20000, 0 };
			}
			else if (i == 5) {
				transform = { 1200, 15000, 0 };
			}
			else if (i == 6) {
				transform = { -300, 20000, 0 };
			}

			// three metropolitan soldiers
			else if (i == 7) {
				transform = { -300, -2000, 0 };
			}
			else if (i == 8) {
				transform = { -400, -2000, 0 };
			}
			else if (i == 9) {
				transform = { -500, -2000, 0 };
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

			auto& sentience = new_character.get<components::sentience>();

			sentience.get<consciousness_meter_instance>().set_maximum_value(400);
			sentience.get<consciousness_meter_instance>().set_value(400);

			sentience.get<personal_electricity_meter_instance>().set_maximum_value(400);
			sentience.get<personal_electricity_meter_instance>().set_value(400);

			if (i == 0) {
				sentience.get<personal_electricity_meter_instance>().set_maximum_value(800);
				sentience.get<personal_electricity_meter_instance>().set_value(800);
			}

			fill_range(sentience.learned_spells, true);
		}

		// street wandering pixels
		auto wandering_pixels_frames = decltype(components::wandering_pixels::frames){};
		wandering_pixels_frames.resize(5);

		wandering_pixels_frames[0] = { assets::game_image_id::BLANK, vec2(1, 1), cyan };
		wandering_pixels_frames[1] = { assets::game_image_id::BLANK, vec2(2, 2), cyan };
		wandering_pixels_frames[2] = { assets::game_image_id(int(assets::game_image_id::CAST_BLINK_1) + 1), metas, cyan };
		wandering_pixels_frames[3] = { assets::game_image_id(int(assets::game_image_id::CAST_BLINK_1) + 2), metas, cyan };
		wandering_pixels_frames[4] = { assets::game_image_id::BLANK, vec2(2, 2), cyan };

		auto get_frames_col = [&](rgba col) {
			auto f = wandering_pixels_frames;

			for (auto& o : f) {
				o.color = col;
			}

			return f;
		};

		const auto duration_ms = 6000.f;

		{
			const auto reach = xywh(0, 0, 1500, 32000);

			{
				const auto e = create_test_scene_entity(world, test_scene_type::WANDERING_PIXELS);
				auto& w = e += components::wandering_pixels();
				auto& r = e += components::render();

				r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

				w.frames = get_frames_col(cyan);
				w.frame_duration_ms = duration_ms;
				w.particles_count = 200;
				w.reach = reach;
				e.add_standard_components(step);
			}

			{
				const auto e = create_test_scene_entity(world, test_scene_type::WANDERING_PIXELS);
				auto& w = e += components::wandering_pixels();
				auto& r = e += components::render();

				r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

				w.frames = get_frames_col(cyan);
				w.frame_duration_ms = duration_ms;
				//w.face.size.set(1, 1);
				w.particles_count = 80;
				w.reach = reach;
				e.add_standard_components(step);
			}

			{
				const auto e = create_test_scene_entity(world, test_scene_type::WANDERING_PIXELS);
				auto& w = e += components::wandering_pixels();
				auto& r = e += components::render();

				r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

				w.frames = get_frames_col(cyan);
				w.frame_duration_ms = duration_ms;
				//w.face.size.set(1, 1);
				w.particles_count = 80;
				w.reach = reach;
				e.add_standard_components(step);
			}
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
					const auto l = create_test_scene_entity(world, test_scene_type::LAMP);
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
					const auto e = create_test_scene_entity(world, test_scene_type::WANDERING_PIXELS);
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.frames = get_frames_col(light_cyan);
					w.frame_duration_ms = duration_ms;
					w.particles_count = 50;
					w.reach = xywh(light_pos.x- 250, light_pos.y-250, 500, 500);
					e.add_standard_components(step);
				}

				{
					const auto e = create_test_scene_entity(world, test_scene_type::WANDERING_PIXELS);
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.frames = get_frames_col(light_cyan);
					w.frame_duration_ms = duration_ms;
					w.particles_count = 20;
					w.reach = xywh(light_pos.x - 150, light_pos.y - 150, 300, 300);
					e.add_standard_components(step);
				}

				{
					const auto e = create_test_scene_entity(world, test_scene_type::WANDERING_PIXELS);
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.frames = get_frames_col(light_cyan);
					w.frame_duration_ms = duration_ms;
					w.particles_count = 20;
					w.reach = xywh(light_pos.x - 25, light_pos.y - 25, 50, 50);
					e.add_standard_components(step);
				}
			}
		}

		{
			{
				const auto l = create_test_scene_entity(world, test_scene_type::LAMP);
				l += components::transform(164.f - 8.f + 90.f, 220);
				auto& light = l += components::light();
				light.color = cyan;
				light.max_distance.base_value = 4500.f;
				light.wall_max_distance.base_value = 4000.f;
				l.add_standard_components(step);
			}
			{
				const auto l = create_test_scene_entity(world, test_scene_type::LAMP);
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
					const auto e = create_test_scene_entity(world, test_scene_type::WANDERING_PIXELS);
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.frames = get_frames_col(cyan);
					w.frame_duration_ms = duration_ms;
					w.particles_count = 20;
					w.reach = left_reach;
					e.add_standard_components(step);
				}

				{
					const auto e = create_test_scene_entity(world, test_scene_type::WANDERING_PIXELS);
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.frames = get_frames_col(orange);
					w.frame_duration_ms = duration_ms;
					w.particles_count = 20;
					w.reach = right_reach;
					e.add_standard_components(step);
				}

				{
					const auto e = create_test_scene_entity(world, test_scene_type::WANDERING_PIXELS);
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.frames = get_frames_col(cyan);
					w.frame_duration_ms = duration_ms;
					w.particles_count = 50;
					w.reach = left_reach;
					e.add_standard_components(step);
				}

				{
					const auto e = create_test_scene_entity(world, test_scene_type::WANDERING_PIXELS);
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.frames = get_frames_col(orange);
					w.frame_duration_ms = duration_ms;
					w.particles_count = 50;
					w.reach = right_reach;
					e.add_standard_components(step);
				}

				{
					const auto e = create_test_scene_entity(world, test_scene_type::WANDERING_PIXELS);
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.frames = get_frames_col(cyan);
					w.frame_duration_ms = duration_ms;
					w.particles_count = 30;
					w.reach = left_reach;
					e.add_standard_components(step);
				}

				{
					const auto e = create_test_scene_entity(world, test_scene_type::WANDERING_PIXELS);
					auto& w = e += components::wandering_pixels();
					auto& r = e += components::render();

					r.layer = render_layer::WANDERING_PIXELS_EFFECTS;

					w.frames = get_frames_col(orange);
					w.frame_duration_ms = duration_ms;
					w.particles_count = 30;
					w.reach = right_reach;
					e.add_standard_components(step);
				}
			}

			{
				const auto l = create_test_scene_entity(world, test_scene_type::LAMP);
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
				const auto e = create_test_scene_entity(world, test_scene_type::HAVE_A_PLEASANT);

				ingredients::add_sprite(metas, 
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

				const vec2 bg_size = metas.at(assets::game_image_id::TEST_BACKGROUND).get_size();

				const auto num_floors = 10 * 10;
				const auto side = static_cast<int>(sqrt(num_floors) / 2);

				for (int x = -side; x < side; ++x) {
					for (int y = -side; y < side * 16; ++y)
					{
						//auto background = create_test_scene_entity(world, test_scene_type::GROUND);
						//ingredients::add_sprite(metas, background, vec2(-1000, 0) + vec2(x, y) * (bg_size + vec2(1500, 550)), assets::game_image_id::TEST_BACKGROUND, white, render_layer::GROUND);
						//ingredients::add_standard_static_body(background);

						auto street = create_test_scene_entity(world, test_scene_type::STREET);
						ingredients::add_sprite(metas, street,
							assets::game_image_id::TEST_BACKGROUND, gray1, render_layer::GROUND);

						street += components::transform{ bg_size * vec2i(x, y) };

						//background.add_standard_components(step);
						street.add_standard_components(step);
					}
				}

				{
					const vec2 size = metas.at(assets::game_image_id::ROAD_FRONT_DIRT).get_size();

					auto road_dirt = create_test_scene_entity(world, test_scene_type::ROAD_DIRT);
					ingredients::add_sprite(metas, road_dirt,
						assets::game_image_id::ROAD_FRONT_DIRT, white, render_layer::ON_GROUND);

					road_dirt += components::transform{ vec2(-3 - 16 + 100 + 160 + 80 + size.x / 2, -32 - 96 + 160 + 80 - size.y / 2) };

					road_dirt.add_standard_components(step);
				}

				for (int r = 0; r < 38; ++r) {
					const vec2 size = metas.at(assets::game_image_id::ROAD).get_size();

					auto road = create_test_scene_entity(world, test_scene_type::ROAD);
					ingredients::add_sprite(metas, road,
						assets::game_image_id::ROAD, white, render_layer::ON_GROUND);

					road += components::transform{ vec2(-3 - 16 + 100 + 160 + 80 + size.x / 2, -32 - 96 + 160 + 80 + size.y / 2 + size.y*r) };

					road.add_standard_components(step);
				}
			}

			{
				const auto e = create_test_scene_entity(world, test_scene_type::AWAKENING);
				auto& sprite = ingredients::add_sprite(metas, 
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
				const auto e = create_test_scene_entity(world, test_scene_type::METROPOLIS);

				ingredients::add_sprite(metas, 
					e,
					assets::game_image_id::METROPOLIS,
					white,
					render_layer::NEON_CAPTIONS
				);

				e += components::transform(1164.f + 24.f, -60.f);

				e.add_standard_components(step);
			}
		}

		prefabs::create_sample_suppressor(step, vec2(300, -500));

		const bool many_charges = false;

		prefabs::create_kek9(step, vec2(-800, -200),
			prefabs::create_sample_magazine(step, vec2(100, -650), many_charges ? "10" : "0.3",
				prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 1000 : 30)));

		const auto rifle = prefabs::create_sample_rifle(step, vec2(100, -500),
			prefabs::create_sample_magazine(step, vec2(100, -650), many_charges ? "10" : "0.3",
				prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 1000 : 30)));

		const auto rifle2 = prefabs::create_sample_rifle(step, vec2(100, -500 + 50),
			prefabs::create_sample_magazine(step, vec2(100, -650), true ? "10" : "0.3",
				prefabs::create_cyan_charge(step, vec2(0, 0), true ? 1000 : 30)));

		const auto amplifier = prefabs::create_amplifier_arm(step, vec2(-300, -500 + 50));

		prefabs::create_sample_rifle(step, vec2(100, -500 + 100), prefabs::create_sample_magazine(step, vec2(100, -650), many_charges ? "10" : "0.3",
			prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 1000 : 30)));

		prefabs::create_sample_rifle(step, vec2(300, -500 + 50));

		const auto pis2 = prefabs::create_sample_rifle(step, vec2(300, 50),
			prefabs::create_sample_magazine(step, vec2(100, -650), "0.4",
				prefabs::create_cyan_charge(step, vec2(0, 0), 40)));

		const auto submachine = prefabs::create_sample_rifle(step, vec2(500, -500 + 50),
			prefabs::create_sample_magazine(step, vec2(100 - 50, -650), many_charges ? "10" : "0.5", prefabs::create_cyan_charge(step, vec2(0, 0), many_charges ? 500 : 50)));

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

		return character(0);
	}
}
