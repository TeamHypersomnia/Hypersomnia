#pragma once
#include <type_traits>

namespace augs {
	template <class M, template <class> class C, class S, class... K>
	class pool;

	template <class, class... K>
	struct pooled_object_id;

	template <class, class... K>
	struct unversioned_id;

	template <class S, class... K>
	struct pool_undo_free_input;

	template <class>
	struct is_pool : std::false_type {};

	template <class M, template <class> class C, class S, class... K>
	struct is_pool<pool<M, C, S, K...>> : std::true_type {};

	template <class T>
	constexpr bool is_pool_v = is_pool<T>::value;
}
