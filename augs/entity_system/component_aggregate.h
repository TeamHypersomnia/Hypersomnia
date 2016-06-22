#pragma once
#include "templates.h"
#include "misc/object_pool_id.h"

namespace augs {
	template<class T>
	struct make_object_pool_id { typedef object_pool_id<T> type; };
	
	template <class... components>
	class component_aggregate {
	public:
		typename transform_types<std::tuple, make_object_pool_id, components...>::type component_ids;

		template <class component>
		auto& writable_id() {
			return std::get<object_pool_id<component>>(component_ids);
		}

		template <class component>
		auto get_id() const {
			return std::get<object_pool_id<component>>(component_ids);
		}

		unsigned long long processing_subject_categories = 0;
	};
}
