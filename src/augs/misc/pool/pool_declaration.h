#pragma once
#include <type_traits>

namespace augs {
	template <class mapped_type, template <class> class make_container_type, class size_type>
	class pool;

	template <class>
	struct pooled_object_id;

	template <class>
	struct unversioned_id;

	template <class size_type>
	struct pool_undo_free_input;

	template <class>
	struct is_pool : std::false_type {};

	template <class mapped_type, template <class> class make_container_type, class size_type>
	struct is_pool<pool<mapped_type, make_container_type, size_type>> : std::true_type {};

	template <class T>
	constexpr bool is_pool_v = is_pool<T>::value;
}
