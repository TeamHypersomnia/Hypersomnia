#pragma once
#include <vector>
#include <functional>
#include <cassert>
#include "signature_matcher.h"
#include "world.h"

namespace augs {
	namespace entity_system {
		class processing_system {
			friend class world;
			friend class entity;

		public:
			signature_matcher_bitset components_signature;
			std::vector<entity_id> targets;
			std::vector<processing_system*> subsystems;

			/* add an entity to processing_system 
			base function just push_backs entity to targets
			*/
			virtual void add(entity_id);
			/* remove entity from processing_system 
			base function just removes entity from targets

			note: there's no sense in introducing remove_n function as complexity using "remove_if" once is the same as using "remove" multiple times
			*/
			virtual void remove(entity_id);

			/* you are required to override this function to specify components that this system needs to processing */
			virtual type_pack get_needed_components() const = 0;

			virtual void clear();
		};

		/* helper class removing necessity to override get_needed_components by specifying the types in the parameter pack */
		template<typename... needed_components>
		class processing_system_templated : public processing_system {
		public:
			virtual type_pack get_needed_components() const override {
				return templated_list<needed_components...>::get();
			}
		};
		
		template<typename... needed_components>
		class event_only_system_templated : public processing_system_templated<needed_components...> {
		public:
			virtual void add(entity_id) {

			}

			virtual void remove(entity_id) {

			}
		};
	}
}