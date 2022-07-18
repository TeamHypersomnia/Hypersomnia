#include "minimal_scene.h"

#include "augs/templates/algorithm_templates.h"

#include "game/enums/faction_type.h"

#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/ingredients/ingredients.h"
#include "test_scenes/test_scenes_content.h"

#include "game/cosmos/cosmos.h"
#include "game/organization/all_component_includes.h"
#include "game/organization/all_messages_includes.h"
#include "game/cosmos/logic_step.h"

#include "game/detail/inventory/perform_transfer.h"
#include "view/viewables/image_cache.h"

#include "test_scenes/scenes/test_scene_node.h"
#include "game/modes/bomb_defusal.h"
#include "augs/math/cascade_aligner.h"
#include "game/modes/test_mode.h"

namespace test_scenes {
	void minimal_scene::setup(test_mode_ruleset& rs) {
		rs.spawned_faction = faction_type::METROPOLIS;
		rs.name = "Minimal test ruleset";
	}

	void minimal_scene::setup(bomb_defusal_ruleset& rs) {
		rs.bot_names = { "First", "Second", "Third", "Fourth", "Fifth", "Sixth" };

		rs.player_colors = {
			rgba(255, 34, 30, 255), // red
			rgba(0, 116, 255, 255), // blue
			rgba(0, 255, 0, 255), // green
			rgba(255, 255, 0, 255), // yellow
			rgba(255, 136, 0, 255), // orange
			rgba(121, 48, 255, 255), // purple
			rgba(255, 72, 255, 255), // pink
			rgba(0, 255, 255, 255), // cyan
			rgba(86, 34, 0, 255), // brown
			rgba(133, 133, 133, 255), // gray
			rgba(119, 187, 255, 255), // light blue
			rgba(17, 102, 68, 255), // dark green
			rgba(0, 0, 0, 255) // black
		};

		rs.name = "Minimal bomb ruleset";
		rs.bot_quota = 0;
	}

	void minimal_scene::populate(const loaded_image_caches_map& caches, const logic_step step) const {
		auto& world = step.get_cosmos();

		auto create = [&](auto&&... args) {
			return create_test_scene_entity(world, std::forward<decltype(args)>(args)...);
		};

		(void)create;

		auto get_size_of = [&caches](const auto id) {
			return vec2i(caches.at(to_image_id(id)).get_original_size());
		};

		auto give_weapon = [&](const auto& character, const auto w) {
			requested_equipment r;
			r.weapon = to_entity_flavour_id(w);

			if constexpr(std::is_same_v<const transformr&, decltype(character)>) {
				r.num_given_ammo_pieces = 2;
			}
			else {
				r.personal_deposit_wearable = to_entity_flavour_id(test_container_items::STANDARD_PERSONAL_DEPOSIT);
			}

			r.generate_for(character, step);
		};

		if (0)
		{
			for (int i = 0; i < 1; ++i) {
				transformr transform;

				if (i == 0) {
				}
				else if (i == 1) {
					transform.pos.x += 200;
				}

				const auto metropolis_type = test_controlled_characters::METROPOLIS_SOLDIER;
				const auto new_character = create(metropolis_type, transform);

				if (i == 0) {
					new_character.get<components::sentience>().get<health_meter_instance>().set_value(100);
					new_character.get<components::sentience>().get<health_meter_instance>().set_maximum_value(100);
					new_character.get<components::attitude>().official_faction = faction_type::RESISTANCE;
				}
				else if (i == 1) {
					new_character.get<components::sentience>().get<health_meter_instance>().set_value(100);
					new_character.get<components::sentience>().get<health_meter_instance>().set_maximum_value(100);
					new_character.get<components::attitude>().official_faction = faction_type::METROPOLIS;
				}

				auto& sentience = new_character.get<components::sentience>();


				fill_range(sentience.learnt_spells, true);
			}
		}

		if (1)
		{
			const vec2 floor_size = get_size_of(test_scene_image_id::FLOOR);
			const auto total_floor_size = floor_size * 10;
			const auto floor_origin = vec2(512, -768);

			auto floor_align = [&](const auto flavour_id) {
				return make_cascade_aligner(
					floor_origin,
					total_floor_size, 
					test_scene_node { world, flavour_id }
				);
			};

			floor_align(test_static_decorations::FLOOR).set_size(total_floor_size);
		}

		cosmic::reinfer_all_entities(world);

		give_weapon(transformr(), test_shootable_weapons::CYBERSPRAY);

		//give_weapon(transformr(), test_shootable_weapons::BILMER2000);

		create(test_point_markers::BOMB_DEFUSAL_SPAWN, transformr()).set_associated_faction(faction_type::METROPOLIS);

		// _controlfp(0, _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL);
	}
}
