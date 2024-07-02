#pragma once
#include <mutex>

namespace augs {
#if WEB_SINGLETHREAD
	struct lock_dummy {
		template <class... T>
		lock_dummy(T...) {}
	};

	template <class... A>
	struct unique_lock {
		template <class... T>
		unique_lock(T...) {}
	};

	using mutex = int;
	using scoped_lock = lock_dummy;
#else
	using mutex = std::mutex;

	template <class... A>
	using scoped_lock = std::scoped_lock<A...>;

	template <class... A>
	using unique_lock = std::unique_lock<A...>;
#endif

}
