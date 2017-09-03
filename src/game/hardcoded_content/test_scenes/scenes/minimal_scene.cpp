#include "minimal_scene.h"

#include "game/enums/party_category.h"

#include "game/hardcoded_content/test_scenes/ingredients/ingredients.h"
#include "game/hardcoded_content/test_scenes/test_scenes_content.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/transcendental/logic_step.h"

#include "game/detail/inventory/inventory_utils.h"

namespace test_scenes {
	void minimal_scene::populate(const logic_step step) {
		auto& world = step.cosm;
		world.set_fixed_delta(60);

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

			const auto new_character = prefabs::create_sample_complete_character(step, transform, typesafe_sprintf("player%x", i), 1);

			new_characters[i] = new_character;

			if (i == 0) {
				new_character.get<components::sentience>().get<health_meter_instance>().set_value(100);
				new_character.get<components::sentience>().get<health_meter_instance>().set_maximum_value(100);
				new_character.get<components::attitude>().parties = party_category::RESISTANCE_CITIZEN;
				new_character.get<components::attitude>().hostile_parties = party_category::METROPOLIS_CITIZEN;
			}
			else if (i == 1) {
				new_character.get<components::sentience>().get<health_meter_instance>().set_value(100);
				new_character.get<components::sentience>().get<health_meter_instance>().set_maximum_value(100);
				new_character.get<components::attitude>().parties = party_category::METROPOLIS_CITIZEN;
				new_character.get<components::attitude>().hostile_parties = party_category::RESISTANCE_CITIZEN;
			}

			auto& sentience = new_character.get<components::sentience>();


			fill_container(sentience.learned_spells, true);
			//for_each_through_std_get(
			//	sentience.spells,
			//	[](auto& spell) {
			//		spell.common.learned = true;
			//	}
			//);
		}

		//const auto amplifier = prefabs::create_amplifier_arm(step, vec2(-300, -500 + 50));

		//const auto backpack = prefabs::create_sample_backpack(step, vec2(100, -150));

		const auto rifle2 = prefabs::create_sample_rifle(step, vec2(100, -500 + 50),
			prefabs::create_sample_magazine(step, vec2(100, -650), true ? "10" : "0.3",
				prefabs::create_cyan_charge(step, vec2(0, 0), true ? 1000 : 5)));
		
		//prefabs::create_rocket_launcher(step, { -100, 0, -180 }, prefabs::create_force_rocket(step, {}));
		//prefabs::create_motorcycle(step, {0, 0, -90});

		//prefabs::create_force_rocket(step, { 0, 100 });
		//prefabs::create_force_rocket(step, { 100, 100 });
		//prefabs::create_force_rocket(step, { 200, 100 });

		//prefabs::create_sample_rifle(step, vec2(300, -500 + 50));
		//
		//prefabs::create_sample_rifle(step, vec2(100, -500),
		//	prefabs::create_sample_magazine(step, vec2(100, -650), "0.4",
		//		prefabs::create_cyan_charge(step, vec2(0, 0), 30)));

		//perform_transfer({ backpack, character(0)[slot_function::SHOULDER] }, step);

		//const auto rifle = prefabs::create_sample_rifle(step, vec2(100, -500),
		//	prefabs::create_sample_magazine(step, vec2(100, -650), false ? "10" : "0.3",
		//		prefabs::create_cyan_charge(step, vec2(0, 0), false ? 1000 : 30)));
		//
		//
		//prefabs::create_sample_rifle(step, vec2(100, -700),
		//	prefabs::create_sample_magazine(step, vec2(100, -650), true ? "10" : "0.3",
		//		prefabs::create_cyan_charge(step, vec2(0, 0), true ? 1000 : 30)));

		// _controlfp(0, _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL);
		load_test_scene_sentience_properties(
			world.significant.meta.global
		);

		auto& global_assets = world.get_global_assets();
		global_assets.cast_unsuccessful_sound.id = assets::sound_buffer_id::CAST_UNSUCCESSFUL;
		global_assets.ped_shield_impact_sound.id = assets::sound_buffer_id::EXPLOSION;
		global_assets.ped_shield_destruction_sound.id = assets::sound_buffer_id::GREAT_EXPLOSION;
		global_assets.exhausted_smoke_particles.id = assets::particle_effect_id::EXHAUSTED_SMOKE;
		global_assets.exploding_ring_smoke = assets::particle_effect_id::EXPLODING_RING_SMOKE;
		global_assets.exploding_ring_sparkles = assets::particle_effect_id::EXPLODING_RING_SPARKLES;
		global_assets.thunder_remnants = assets::particle_effect_id::THUNDER_REMNANTS;

		std::get<electric_triad>(world.get_global_state().spells).missile_definition = prefabs::create_electric_missile_def(step, {});
	}
}
