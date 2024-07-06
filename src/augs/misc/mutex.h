#pragma once
#include <mutex>

namespace augs {
#if WEB_SINGLETHREAD
	struct lock_dummy {
		template <class... T>
		lock_dummy(T...) {}

		~lock_dummy() {}
	};

	template <class... A>
	struct unique_lock {
		template <class... T>
		unique_lock(T...) {}

		~unique_lock() {}
	};

	using mutex = int;
	using scoped_lock = lock_dummy;
#else
	using mutex = std::mutex;

	template <class... A>
	auto scoped_lock(A&&... args) {
		return std::scoped_lock(std::forward<A>(args)...);
	}

	template <class... A>
	using unique_lock = std::unique_lock<A...>;
#endif

}
