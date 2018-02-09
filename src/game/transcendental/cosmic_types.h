#pragma once
#include "augs/misc/pool/pool_declaration.h"

namespace augs {
	template <class mapped_type, template <class> class make_container_type, class size_type>
	class pool;

	template <class, class>
	struct pooled_object_id;

	template <class, class>
	struct unversioned_id;
}

using cosmic_pool_size_type = unsigned short;

template <class T>
using cosmic_object_pool_id = augs::pooled_object_id<T, cosmic_pool_size_type>;

template <class T>
using cosmic_object_unversioned_id = augs::unversioned_id<T, cosmic_pool_size_type>;
