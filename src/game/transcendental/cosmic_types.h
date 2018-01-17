#pragma once
namespace augs {
	template <template <class T> class, class... components>
	class component_aggregate;
}

namespace augs {
	template <class mapped_type, template <class> class make_container_type, class size_type>
	class pool;

	template <class, class>
	struct pooled_object_id;

	template <class, class>
	struct unversioned_id;
}

using cosmic_pool_size_type = unsigned short;

#if STATICALLY_ALLOCATE_ENTITIES_NUM
#include "augs/misc/constant_size_vector.h"
template <class T>
using cosmic_object_pool = augs::pool<T, of_size<5000>::make_constant_vector, cosmic_pool_size_type>;
#else
#include "augs/templates/type_mod_templates.h"
template <class T>
using cosmic_object_pool = augs::pool<T, make_vector, cosmic_pool_size_type>;
#endif

template <class T>
using cosmic_object_pool_id = augs::pooled_object_id<T, cosmic_pool_size_type>;

template <class T>
using cosmic_object_unversioned_id = augs::unversioned_id<T, cosmic_pool_size_type>;

template <class... components>
using cosmic_aggregate = augs::component_aggregate<cosmic_object_unversioned_id, components...>;