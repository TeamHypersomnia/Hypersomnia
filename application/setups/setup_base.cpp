#include "setup_base.h"

bool setup_base::process_exit_key(const augs::machine_entropy::local_type& local) {
	for (auto& n : local) {
		if (n.key == exit_key && n.was_key_pressed()) {
			should_quit = true;
			
			return true;
		}
	}

	return false;
}
