#pragma once
#include <vector>
#include <functional>
#include <cassert>
#include "component_bitset_matcher.h"

#include "misc/deterministic_timing.h"

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
		
		virtual void add(augs::entity_id);
		virtual void remove(augs::entity_id);
		
		void clear();

		deterministic_timestamp get_current_timestamp() const;

		int randval(int min, int max);
		unsigned randval(unsigned min, unsigned max);
		float randval(float min, float max);

		float randval(float minmax);

		unsigned randval(std::pair<unsigned, unsigned>);
		float randval(std::pair<float, float>);

		double frame_timestamp_seconds() const;
		double delta_seconds() const;
		double delta_milliseconds() const;
		double fixed_delta_milliseconds() const;
		double view_interpolation_ratio() const;
		augs::renderer& get_renderer();

		bool unset_or_passed(augs::deterministic_timeout&) const;
		bool was_set_and_passed(augs::deterministic_timeout&) const;
		bool passed(augs::deterministic_timeout&) const;

		bool check_timeout_and_reset(augs::deterministic_timeout&);

		float get_milliseconds_left(augs::deterministic_timeout&) const;
		float get_percentage_left(augs::deterministic_timeout&) const;

		void reset(augs::deterministic_timeout&);
		
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