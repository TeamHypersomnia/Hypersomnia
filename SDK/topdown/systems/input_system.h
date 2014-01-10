#pragma once
#include "entity_system/processing_system.h"
#include "../components/input_component.h"
#include "../messages/intent_message.h"

using namespace augmentations;
using namespace entity_system;

namespace augmentations {
	namespace window {
		class glwindow;
	}
}
/* input to inna domena eventów bo mamy taki sam payload a masa zupelnie co innego znaczacych enumow 
zwykle message to beda po prostu message_component i sorted vector przyjmowanych enumow

rozwazyc generyczny message component a w bazowym message enum type
key message z booleanem
mouse message z vec2 i juz jebac ze sean proponowal w jednym to trzymac

ew jakas hybryda ze obok siebie istnieja message i input componenty

jednak bedzie input component jako normalny component bo wlasnie o to chodzi w componentach ze one serve purpose of grouping
a tak to bys musial zapierdalac po wszystkich ktore maja message component a to by bylo chujowe

*/
struct input_system : public processing_system_templated<components::input> {
	int quit_flag;

	void process_entities(world&) override;
	
	struct context {
		std::unordered_map<int, int> raw_id_to_intent;
		bool enabled;
		context();

		void set_intent(unsigned, messages::intent_message::intent_type);
	};

	std::vector<context*> active_contexts;
	void add_context(context*);
	void clear_contexts();

	window::glwindow& input_window;
	input_system(window::glwindow&);

	/* returns true if successfully mapped raw input to a high level intent */
	bool post_intent_from_raw_id(world& owner, const context&, unsigned id, bool state = true);

	void post(messages::intent_message incoming_event, world& owner);

	void clear() override;
};