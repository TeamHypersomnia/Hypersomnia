#pragma once
#include <set>
#include <bitset>
#include "type_registry.h"
#include "../options.h"

namespace augmentations {
	namespace entity_system {
		class world;
		
		//class signature_matcher {
		//	type_pack signature;
		//public:
		//
		//	signature_matcher(const type_pack&);
		//
		//	/* optimized match, O(n1 log n2) */
		//	bool matches(const signature_matcher& bigger) const;
		//
		//	/* unoptimized match, O(n1 n2) */
		//	//bool matches(const type_pack& bigger) const;
		//	//
		//	//type_pack get_types() const;
		//};

		class signature_matcher_bitset {
			std::bitset<MAXIMUM_COMPONENTS> signature;
		public:

			signature_matcher_bitset(const std::vector<registered_type>& = std::vector<registered_type>());
			
			/* optimized match, O(1) */
			bool matches(const signature_matcher_bitset& bigger) const;
			void add(const registered_type&);
			void add(const std::vector<registered_type>& = std::vector<registered_type>());
			void remove(const registered_type&);
			void remove(const std::vector<registered_type>& = std::vector<registered_type>());
			
			/* the reason why is there no unoptimized equivalent for bitset matcher is because preprocessing must take place anyway 
				so we can construct the signature as well */
			// bool matches(const std::vector<registered_type>& bigger) const;
		};
		
	}
}