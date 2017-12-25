#if PLATFORM_UNIX
#include <experimental/propagate_const>

namespace augs {
	template <class T>
	using propagate_const = std::experimental::propagate_const<T>;
}

#else

#include <utility>
#include <type_traits>

namespace augs {
	/* Unsafe (about constructors), incomplete wrapper for compatibility only */
	template <class T>
	class propagate_const;

	template <class T>
	class propagate_const<T*> {
		T* p;

	public:
		propagate_const(T* const t = nullptr) : p(t) {};

		T* operator->() {
			return p;
		}

		const T* operator->() const {
			return p;
		}

		T* get() {
			return p;
		}

		const T* get() const {
			return p;
		}

		operator T*() {
			return p;
		}

		operator const T*() const {
			return p;
		}
	};
}

#endif
