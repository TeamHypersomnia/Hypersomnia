#pragma once
#include <vector>
#include <unordered_map>

namespace augmentations {
	namespace util {
		class input_context {

		public:
			bool map_to_action();
		};

		class input_system {
		public:

			std::vector<input_context*> contexts;
		};
	}
}