#pragma once

class cosmos;
class step_state;

class item_system {
public:
	void handle_trigger_confirmations_as_pick_requests(cosmos& cosmos, step_state& step);
	
	void handle_throw_item_intents(cosmos& cosmos, step_state& step);
	void handle_holster_item_intents(cosmos& cosmos, step_state& step);

	void translate_gui_intents_to_transfer_requests(cosmos& cosmos, step_state& step);
	static void consume_item_slot_transfer_requests(cosmos& cosmos, step_state& step);

	void process_mounting_and_unmounting(cosmos& cosmos, step_state& step);
};