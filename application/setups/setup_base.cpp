#include "setup_base.h"

bool setup_base::process_exit_key(const augs::machine_entropy::local_type& local) {
	for (auto& n : local) {
		if (n.was_key_pressed(exit_key)) {
			should_quit = true;
			
			return true;
		}
	}

	return false;
}
