#include "networked_testbed.h"
#include "game/ingredients/ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/assets/game_image_id.h"

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
#include "game/detail/inventory/inventory_utils.h"

#include "game/view/rendering_scripts/all.h"

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

namespace scene_builders {
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

	void networked_testbed::populate_world_with_entities(
		cosmos& cosm,
		const all_logical_metas_of_assets& metas_of_assets
	) {
		cosm.advance_deterministic_schemata(
			{ cosmic_entropy(), metas_of_assets }, 
			[this](const logic_step step) { 
				populate(step); 
			}, 
			[](const const_logic_step) {}
		);
	}

	void networked_testbed::populate(const logic_step step) {
		auto& world = step.cosm;
		const auto& metas = step.input.metas_of_assets;

		ensure(false && "networked testbed is out of service now");
		// _controlfp(0, _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL);
	}


	entity_id networked_testbed_client::get_selected_character() const {
		return selected_character;
	}
	
	void networked_testbed_client::select_character(const entity_id h) {
		selected_character = h;
	}
}
