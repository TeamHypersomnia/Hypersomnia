#include "one_entity.h"
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
	void one_entity::populate_world_with_entities(cosmos& cosm, const vec2i screen_size) {
		cosm.advance_deterministic_schemata(cosmic_entropy(), [&](logic_step& step) { populate(step, screen_size); }, [](const_logic_step&) {});
	}

	void one_entity::populate(logic_step& step, const vec2i screen_size) {
		auto& world = step.cosm;

		const int num_characters = 1;

		std::vector<entity_id> new_characters;
		new_characters.resize(num_characters);

		auto character = [&](const size_t i) {
			return i < new_characters.size() ? world[new_characters.at(i)] : world[entity_id()];
		};

		for (int i = 0; i < num_characters; ++i) {
			assets::animation_response_id torso_set = assets::animation_response_id::TORSO_SET;
			components::transform transform;

			if (i == 0) {
				torso_set = assets::animation_response_id::TORSO_SET;
			}
			const auto new_character = prefabs::create_character(world, transform, screen_size, typesafe_sprintf("player%x", i), torso_set);

			new_characters[i] = new_character;

			if (i == 0) {
				new_character.get<components::sentience>().health.value = 100;
				new_character.get<components::sentience>().health.maximum = 100;
				new_character.get<components::attitude>().parties = party_category::RESISTANCE_CITIZEN;
				new_character.get<components::attitude>().hostile_parties = party_category::METROPOLIS_CITIZEN;
			}
		}

		const auto backpack = prefabs::create_sample_backpack(world, vec2(200, -650));

		const auto rifle2 = prefabs::create_sample_bilmer2000(step, vec2(100, -500 + 50),
			prefabs::create_sample_magazine(step, vec2(100, -650), false ? "10" : "0.3",
				prefabs::create_cyan_charge(world, vec2(0, 0), false ? 1000 : 30)));
		
		prefabs::create_kek9(step, vec2(300, -500 + 50));

		prefabs::create_kek9(step, vec2(100, -500),
			prefabs::create_small_magazine(step, vec2(100, -650), "0.4",
				prefabs::create_pink_charge(world, vec2(0, 0), 30)));

		//perform_transfer({ backpack, character(0)[slot_function::SHOULDER_SLOT] }, step);
		//prefabs::create_sample_suppressor(world, vec2(300, -500));

		characters.assign(new_characters.begin(), new_characters.end());

		select_character(character(0));
		// _controlfp(0, _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL);
	}

	entity_id one_entity::get_selected_character() const {
		return selected_character;
	}

	void one_entity::select_character(const entity_id h) {
		selected_character = h;
	}

	void one_entity::configure_view(viewing_session& session) const {
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
