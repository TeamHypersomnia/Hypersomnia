#pragma once
namespace augs {
	namespace entity_system {
		class entity;
	}
}

namespace messages {
	struct message {
		augs::entity_system::entity* subject;
		message(augs::entity_system::entity* subject = nullptr) : subject(subject) {}
	};
}