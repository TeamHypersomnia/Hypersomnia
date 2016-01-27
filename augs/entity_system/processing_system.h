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
		
		void add(augs::entity_id);
		void remove(augs::entity_id);
		void clear();

		double delta_seconds();
		double delta_milliseconds();
		double view_interpolation_ratio();
		augs::renderer& get_renderer();

		/* you are required to override this function to specify components that this system needs to processing */
		virtual type_hash_vector get_needed_components() const;
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
	};
}