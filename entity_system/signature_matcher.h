#pragma once
#include <set>
#include <bitset>
#include "type_registry.h"
#include "../options.h"

namespace augmentations {
	namespace entity_system {
		class world;

		/* used to detect whenever a certain system needs to process given entity */
		class signature_matcher_bitset {
			std::bitset<MAXIMUM_COMPONENTS> signature;
		public:
			signature_matcher_bitset(const std::vector<registered_type>& = std::vector<registered_type>());
			
			void add(const registered_type&);
			void add(const std::vector<registered_type>& = std::vector<registered_type>());
			void remove(const registered_type&);
			void remove(const std::vector<registered_type>& = std::vector<registered_type>());

			bool matches(const signature_matcher_bitset& bigger) const;
		};
		
	}
}