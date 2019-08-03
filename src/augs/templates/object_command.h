#pragma once

namespace augs {
	template <class T, class P>
	struct object_command {
		T* this_ptr = nullptr;
		P payload;

		template <class A>
		void operator()(const A access) const {
			this_ptr->perform(access, payload);
		}
	};

	template <class T, class P>
	struct static_object_command {
		P payload;

		template <class A>
		void operator()(const A access) const {
			T::perform_static(access, payload);
		}
	};
}
