#include "game/build_settings.h"

#include "one_entity.h"
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

namespace test_scenes {
	void one_entity::populate(const logic_step step) {
#if BUILD_TEST_SCENES 1
		auto& world = step.cosm;

		//prefabs::create_force_grenade(step, { 254, 611 });
		//prefabs::create_force_grenade(step, { 254, 711 });
		//prefabs::create_force_grenade(step, { 254, 811 });

		const int num_characters = 1;

		std::vector<entity_id> new_characters;
		new_characters.resize(num_characters);

		auto character = [&](const size_t i) {
			return i < new_characters.size() ? world[new_characters.at(i)] : world[entity_id()];
		};

		for (int i = 0; i < num_characters; ++i) {
			components::transform transform;

			if (i == 0) {
				//torso_set = assets::animation_response_id::TORSO_SET;
			}
			else if (i == 1) {
				//torso_set = assets::animation_response_id::VIOLET_TORSO_SET;
				transform.pos.x += 200;
			}

			const auto new_character = prefabs::create_sample_complete_character(step, transform, typesafe_sprintf("player%x", i), 0);

			new_characters[i] = new_character;

			if (i == 0) {
				new_character.get<components::sentience>().health.set_value(100);
				new_character.get<components::sentience>().health.set_maximum_value(100);
				new_character.get<components::attitude>().parties = party_category::RESISTANCE_CITIZEN;
				new_character.get<components::attitude>().hostile_parties = party_category::METROPOLIS_CITIZEN;
			}
			else if (i == 1) {
				new_character.get<components::sentience>().health.set_value(100);
				new_character.get<components::sentience>().health.set_maximum_value(100);
				new_character.get<components::attitude>().parties = party_category::METROPOLIS_CITIZEN;
				new_character.get<components::attitude>().hostile_parties = party_category::RESISTANCE_CITIZEN;
			}
		}

		//const auto amplifier = prefabs::create_amplifier_arm(step, vec2(-300, -500 + 50));

		//const auto backpack = prefabs::create_sample_backpack(step, vec2(100, -150));

		//const auto rifle2 = prefabs::create_sample_bilmer2000(step, vec2(100, -500 + 50),
		//	prefabs::create_sample_magazine(step, vec2(100, -650), false ? "10" : "0.3",
		//		prefabs::create_cyan_charge(step, vec2(0, 0), false ? 1000 : 5)));
		
		//prefabs::create_rocket_launcher(step, { -100, 0, -180 }, prefabs::create_force_rocket(step, {}));
		prefabs::create_motorcycle(step, {0, 0, -90});

		//prefabs::create_force_rocket(step, { 0, 100 });
		//prefabs::create_force_rocket(step, { 100, 100 });
		//prefabs::create_force_rocket(step, { 200, 100 });

		//prefabs::create_kek9(step, vec2(300, -500 + 50));
		//
		//prefabs::create_kek9(step, vec2(100, -500),
		//	prefabs::create_small_magazine(step, vec2(100, -650), "0.4",
		//		prefabs::create_pink_charge(step, vec2(0, 0), 30)));

		//perform_transfer({ backpack, character(0)[slot_function::SHOULDER] }, step);

		//const auto rifle = prefabs::create_sample_rifle(step, vec2(100, -500),
		//	prefabs::create_sample_magazine(step, vec2(100, -650), false ? "10" : "0.3",
		//		prefabs::create_cyan_charge(step, vec2(0, 0), false ? 1000 : 30)));
		//
		//
		//prefabs::create_submachine(step, vec2(100, -700),
		//	prefabs::create_sample_magazine(step, vec2(100, -650), true ? "10" : "0.3",
		//		prefabs::create_cyan_charge(step, vec2(0, 0), true ? 1000 : 30)));

		characters.assign(new_characters.begin(), new_characters.end());

		select_character(character(0));
		// _controlfp(0, _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL);
#endif
	}

	entity_id one_entity::get_selected_character() const {
		return selected_character;
	}

	void one_entity::select_character(const entity_id h) {
		selected_character = h;
	}
	
	void one_entity::control_character_selection(const augs::machine_entropy::local_type& local) {
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
}
