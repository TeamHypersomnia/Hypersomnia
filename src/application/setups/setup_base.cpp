#include "setup_base.h"

bool setup_base::process_exit(const augs::machine_entropy::local_type& local) {
	using namespace augs::window::event;

	for (auto& n : local) {
		if (
			n.msg == message::close
			|| n.msg == message::quit
			|| (n.msg == message::syskeydown && n.key.key == keys::key::F4)
#if !IS_PRODUCTION_BUILD // for even faster exit
			|| n.was_key_pressed(exit_key)
#endif
		) {
			should_quit = true;

			return true;
		}
	}

	return false;
}
