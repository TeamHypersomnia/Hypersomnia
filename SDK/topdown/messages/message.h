#pragma once
namespace augmentations {
	namespace entity_system {
		class entity;
	}
}

namespace messages {
	struct message {
		augmentations::entity_system::entity* subject;
		message(augmentations::entity_system::entity* subject = nullptr) : subject(subject) {}
	};
}