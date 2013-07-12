#pragma once
#include <vector>
#include "signature_matcher.h"

namespace augmentations {
	namespace entity_system {
		class entity;
		class assemblage_bucket;
		class system {
			std::vector<entity*> targets;
		
		public:
			signature_matcher_bitset components_signature;
			
			/* add an entity to system 
			base function just push_backs entity to targets
			*/
			virtual void add(entity*);
			/* remove entity from system 
			base function just removes entity from targets
			there's no sense in introducing remove_n function as complexity is the same using remove_if with multiple objects for deletion
			*/
			virtual void remove(entity*);

			/* process all entity targets */
			virtual void process();
		};
	}
}