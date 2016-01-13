#pragma once
#include <vector>
#include <functional>
#include <cassert>
#include "component_bitset_matcher.h"

namespace augs {
	class renderer;

	class processing_system {
		friend class world;
		friend class overworld;
		friend class entity;
	public:
		world& parent_world;
		overworld& parent_overworld;

		component_bitset_matcher components_signature;
		std::vector<entity_id> targets;
		std::vector<processing_system*> subsystems;

		processing_system(world& parent_world);

		double per_second();
		double frame_time();
		double view_interpolation_ratio();
		augs::renderer& get_renderer();

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
		virtual type_hash_vector get_needed_components() const;

		virtual void clear();
	};

	/* helper class removing necessity to override get_needed_components by specifying the types in the parameter pack */
	template<typename... needed_components>
	class processing_system_templated : public processing_system {
	public:
		using processing_system::processing_system;

		virtual type_hash_vector get_needed_components() const override {
			return templated_list_to_hash_vector<needed_components...>::unpack();
		}
	};

	class event_only_system : public processing_system {
	public:
		using processing_system::processing_system;

		virtual void add(entity_id) {

		}

		virtual void remove(entity_id) {

		}
	};
}