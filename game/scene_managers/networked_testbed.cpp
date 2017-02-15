#include "networked_testbed.h"
#include "game/ingredients/ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/assets/texture_id.h"

#include "game/systems_stateless/input_system.h"
#include "game/systems_stateless/render_system.h"
#include "game/systems_stateless/gui_system.h"
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

#include "game/detail/world_camera.h"
#include "augs/gui/text/printer.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/time_utils.h"

#include "game/transcendental/cosmic_delta.h"

namespace scene_managers {
	entity_id networked_testbed_server::assign_new_character() {
		for (auto& c : characters) {
			if (!c.occupied) {
				c.occupied = true;
				return c.id;
			}
		}

		ensure(false);
		return entity_id();
	}

	void networked_testbed_server::free_character(const entity_id id) {
		for (auto& c : characters) {
			if (c.id == id) {
				ensure(c.occupied);
				c.occupied = false;
				return;
			}
		}

		ensure(false);
	}

	void networked_testbed::populate_world_with_entities(cosmos& cosm) {
		cosm.advance_deterministic_schemata(cosmic_entropy(), [this](const logic_step step) { populate(step); }, [](const const_logic_step) {});
	}

	void networked_testbed::populate(const logic_step step) {
		auto& world = step.cosm;

		const auto crate = prefabs::create_crate(world, vec2(200, 200 + 300), vec2i(100, 100) / 3);
		const auto crate2 = prefabs::create_crate(world, vec2(400, 200 + 400), vec2i(300, 300));
		const auto crate4 = prefabs::create_crate(world, vec2(500, 200 + 0), vec2i(100, 100));

		for (int x = -4; x < 4; ++x) {
			for (int y = -4; y < 4; ++y) {
				auto obstacle = prefabs::create_crate(world, vec2(2000.f + x * 300.f, -1000.f +2000.f + y * 300.f), vec2i(100, 100));
			}
		}

		const auto car = prefabs::create_car(world, components::transform(-300, -600, -90));
		const auto car2 = prefabs::create_car(world, components::transform(-800, -600, -90));
		const auto car3 = prefabs::create_car(world, components::transform(-1300, -600, -90));

		const auto motorcycle = prefabs::create_motorcycle(world, components::transform(0, -600, -90));
		prefabs::create_motorcycle(world, components::transform(100, -600, -90));

		const vec2 bg_size = assets::get_size(assets::texture_id::TEST_BACKGROUND);

		const int num_floors = 10 * 10;
		const int side = sqrt(num_floors) / 2;

		for (int x = -side; x < side; ++x)
			for (int y = -side; y < side; ++y)
			{
				//auto background = world.create_entity("bg[-]");
				//ingredients::add_sprite(background, vec2(-1000, 0) + vec2(x, y) * (bg_size + vec2(1500, 550)), assets::texture_id::TEST_BACKGROUND, white, render_layer::GROUND);
				//ingredients::add_standard_static_body(background);

				auto street = world.create_entity("street[-]");
				ingredients::add_sprite(street, { bg_size * vec2(x, y) },
					assets::texture_id::TEST_BACKGROUND, gray1, render_layer::GROUND);

				//background.add_standard_components();
				street.add_standard_components();
			}

		const int num_characters = 6;

		std::vector<entity_id> new_characters;
		new_characters.resize(num_characters);

		auto character = [&](const size_t i) {
			return i < new_characters.size() ? world[new_characters.at(i)] : world[entity_id()];
		};

		{
			{
				const auto l = world.create_entity("l");
				l += components::transform();
				auto& light = l += components::light();
				light.color = red;
				l.add_standard_components();
			}
			{
				const auto l = world.create_entity("l");
				l += components::transform(300, 0);
				auto& light = l += components::light();
				light.color = green;
				l.add_standard_components();
			}
			{
				const auto l = world.create_entity("l");
				l += components::transform(600, 0);
				auto& light = l += components::light();
				light.color = blue;
				l.add_standard_components();
			}
			{
				const auto l = world.create_entity("l");
				l += components::transform(900, 0);
				auto& light = l += components::light();
				light.color = cyan;
				l.add_standard_components();
			}
			{
				const auto l = world.create_entity("l");
				l += components::transform(1200, 0);
				auto& light = l += components::light();
				light.color = orange;
				l.add_standard_components();
			}
		}

		for (int i = 0; i < num_characters; ++i) {
			const auto new_character = prefabs::create_character(world, vec2(i * 300.f, 0), vec2(1200, 800), typesafe_sprintf("player%x", i));

			new_characters[i] = new_character;

			if (i == 0) {
				new_character.get<components::sentience>().health.value = 100;
				new_character.get<components::sentience>().health.maximum = 100;
			}
			if (i == 1) {
				new_character.get<components::attitude>().parties = party_category::RESISTANCE_CITIZEN;
				new_character.get<components::attitude>().hostile_parties = party_category::METROPOLIS_CITIZEN;
				new_character.get<components::attitude>().maximum_divergence_angle_before_shooting = 25;
				new_character.get<components::sentience>().minimum_danger_amount_to_evade = 20;
				new_character.get<components::sentience>().health.value = 300;
				new_character.get<components::sentience>().health.maximum = 300;
				//ingredients::add_standard_pathfinding_capability(new_character);
				//ingredients::add_soldier_intelligence(new_character);
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
				//ingredients::add_standard_pathfinding_capability(new_character);
				//ingredients::add_soldier_intelligence(new_character);
				new_character.recalculate_basic_processing_categories();
			}
		}

		if (character(0).alive()) {
			name_entity(character(0), entity_name::PERSON, L"Attacker");
		}

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

		perform_transfer({ backpack, character(0)[slot_function::SHOULDER_SLOT] }, step);
		perform_transfer({ submachine, character(0)[slot_function::PRIMARY_HAND] }, step);
		perform_transfer({ rifle, character(0)[slot_function::SECONDARY_HAND] }, step);

		if (character(1).alive()) {
			name_entity(character(1), entity_name::PERSON, L"Enemy");
			perform_transfer({ rifle2, character(1)[slot_function::PRIMARY_HAND] }, step);
		}

		if (character(2).alive()) {
			name_entity(character(2), entity_name::PERSON, L"Swordsman");
			perform_transfer({ second_machete, character(2)[slot_function::PRIMARY_HAND] }, step);
		}

		if (character(3).alive()) {
			name_entity(character(3), entity_name::PERSON, L"Medic");
			perform_transfer({ pis2, character(3)[slot_function::PRIMARY_HAND] }, step);
		}

		if (character(5).alive()) {
			const auto new_item = prefabs::create_submachine(step, vec2(0, -1000),
				prefabs::create_sample_magazine(step, vec2(100 - 50, -650), true ? "10" : "0.5", prefabs::create_pink_charge(world, vec2(0, 0), true ? 500 : 50)));

			perform_transfer({ new_item, character(5)[slot_function::PRIMARY_HAND] }, step);
		}

		//draw_bodies.push_back(crate2);
		//draw_bodies.push_back(new_characters[0]);
		//draw_bodies.push_back(backpack);

		//world.significant.meta.settings.pathfinding.draw_memorised_walls = 1;
		//world.significant.meta.settings.pathfinding.draw_undiscovered = 1;

		characters.assign(new_characters.begin(), new_characters.end());
		// _controlfp(0, _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL);
	}


	entity_id networked_testbed_client::get_selected_character() const {
		return selected_character;
	}
	
	void networked_testbed_client::select_character(const entity_id h) {
		selected_character = h;
	}
}
