#pragma once
#include <vector>
#include <memory>

#include "augs/misc/delta.h"

namespace augs {
	class action;
	
	class action_list {
	public:
		std::vector<std::unique_ptr<action>> actions;

		void update(const delta);

		void push_blocking(std::unique_ptr<action>);
		void push_non_blocking(std::unique_ptr<action>);

		float calculate_duration_ms() const;

		bool is_complete() const;
	};
}