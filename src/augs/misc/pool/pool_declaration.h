#pragma once

namespace augs {
	template <class mapped_type, template <class> class make_container_type, class size_type>
	class pool;

	template <class>
	struct pooled_object_id;

	template <class>
	struct unversioned_id;
}
