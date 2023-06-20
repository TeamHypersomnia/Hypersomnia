#include "minimal_scene.h"

#include "augs/templates/algorithm_templates.h"

#include "game/cosmos/entity_handle.h"
#include "game/detail/inventory/generate_equipment.h"
#include "game/enums/faction_type.h"

#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/ingredients/ingredients.h"
#include "test_scenes/test_scenes_content.h"

#include "game/cosmos/cosmos.h"
#include "game/organization/all_component_includes.h"
#include "game/organization/all_messages_includes.h"
#include "game/cosmos/logic_step.h"

#include "view/viewables/image_cache.h"

#include "test_scenes/scenes/test_scene_node.h"
#include "game/modes/arena_mode.h"
#include "augs/math/cascade_aligner.h"
#include "game/modes/test_mode.h"
#include "game/inferred_caches/organism_cache.hpp"

namespace test_scenes {
	void minimal_scene::populate(const loaded_image_caches_map& caches, const logic_step step) const {
		auto& world = step.get_cosmos();

		auto access = allocate_new_entity_access();

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

			r.generate_for(access, character, step);
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

			floor_align(test_static_decorations::CYAN_FLOOR).set_size(total_floor_size);
		}

		cosmic::reinfer_all_entities(world);

		give_weapon(transformr(), test_shootable_weapons::CYBERSPRAY);

		//give_weapon(transformr(), test_shootable_weapons::BILMER2000);

		create(test_point_markers::TEST_TEAM_SPAWN, transformr()).set_associated_faction(faction_type::METROPOLIS);

		// _controlfp(0, _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL);
	}
}
