#pragma once
#include "misc/machine_entropy.h"

class entropy_player {
public:
	augs::machine_entropy acquire_machine_entropy_for_this_step();

	void replay_found_recording();
	void record_and_save_this_session();

	bool found_recording() const;
	bool is_replaying() const;
};