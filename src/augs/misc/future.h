#pragma once
#include <future>
#if WEB_SINGLETHREAD
#include "augs/ensure.h"
#endif

namespace augs {
#if WEB_SINGLETHREAD
	template <class T>
	struct future {
		T obj;
		bool v = true;

		bool valid() const {
			return v;
		}

		T get() {
			ensure(valid());
			v = false;
			return std::move(obj);
		}

		void wait() {}
	};

	template <>
	struct future<void> {
		bool v = true;

		bool valid() const {
			return v;
		}

		void get() {
			ensure(valid());
			v = false;
		}

		void wait() {}
	};
#else
	template <class T>
	using future = std::future<T>;
#endif

}
