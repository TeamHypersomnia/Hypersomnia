#pragma once

namespace augmentations {
	namespace entity_system {
		class entity;
	}
}

namespace messages {
	struct message {
		augmentations::entity_system::entity* subject;
	};
}