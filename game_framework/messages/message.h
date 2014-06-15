#pragma once
namespace augs {
	namespace entity_system {
		class entity;
	}
}

namespace messages {
	/* by default, all messages are considered to be of internal C++ framework use; 
	only by setting send_to_script flag in certain places in game_framework do we note that scripts should be notified of this event.
	*/
	struct message {
		augs::entity_system::entity* subject;
		bool send_to_scripts = false;

		message(augs::entity_system::entity* subject = nullptr) : subject(subject) {}
	};
}