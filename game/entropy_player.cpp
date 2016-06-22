#include "entropy_player.h"

bool entropy_player::found_recording() const {
	return augs::file_exists(L"recorded.inputs");
}

void entropy_player::replay_found_recording() {
	unmapped_intent_player.player.load_recording("recorded.inputs");
	unmapped_intent_player.player.replay();

	crosshair_intent_player.player.load_recording("recorded_crosshair.inputs");
	crosshair_intent_player.player.replay();

	gui_item_transfer_intent_player.player.load_recording("gui_transfers.inputs");
	gui_item_transfer_intent_player.player.replay();
}

void entropy_player::record_and_save_this_session() {
	augs::create_directory("sessions/");
	augs::create_directory("sessions/" + augs::get_timestamp());

	unmapped_intent_player.player.record("sessions/" + augs::get_timestamp() + "/recorded.inputs");
	crosshair_intent_player.player.record("sessions/" + augs::get_timestamp() + "/recorded_crosshair.inputs");
	gui_item_transfer_intent_player.player.record("sessions/" + augs::get_timestamp() + "/gui_transfers.inputs");
}

bool entropy_player::is_replaying() const {
	return unmapped_intent_player.player.is_replaying();
}
