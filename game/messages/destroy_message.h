#pragma once
#include "message.h"

namespace messages {
	struct destroy_message : public message {
		bool only_children = false;

		bool operator<(const destroy_message& b) const {
			return subject < b.subject;
		}

		bool operator==(const destroy_message& b) const {
			return subject == b.subject;
		}

		destroy_message(augs::entity_id s) : message(s) {}
	};
}