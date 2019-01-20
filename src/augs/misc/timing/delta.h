#pragma once
#include <type_traits>

#include "augs/math/declare_math.h"
#include "augs/templates/identity_templates.h"

namespace augs {
	struct introspection_access;

	class delta {
		friend struct introspection_access;
		friend class timer;
		using default_t = real32;
		using double_type = real64;

		// GEN INTROSPECTOR class augs::delta
		default_t secsf = 0.f;
		default_t msf = 0.f;
		// END GEN INTROSPECTOR
		
		explicit delta(const double_type secs) :
			secsf(static_cast<default_t>(secs)),
			msf(static_cast<default_t>(secs * 1000))
		{}

	public:
		static delta zero;

		static delta steps_per_second(const unsigned steps) {
			return delta{ steps ? 1.f / steps : 0.f };
		}

		static delta from_milliseconds(const double_type ms) {
			return delta{ ms / 1000.f };
		}

		bool operator==(const delta& b) const {
			return secsf == b.secsf;
		}

		template <class T = default_t>
		T in_seconds() const {
			if constexpr(std::is_same_v<T, default_t>) {
				return { secsf };
			}
			else if constexpr(std::is_same_v<T, double_type>) {
				return { secsf };
			}
			else {
				static_assert(always_false_v<T>, "Can't call in_seconds with this type.");
			}
		}

		template <class T = default_t>
		T in_milliseconds() const {
			if constexpr(std::is_same_v<T, default_t>) {
				return { msf };
			}
			else if constexpr(std::is_same_v<T, double_type>) {
				return { msf };
			}
			else {
				static_assert(always_false_v<T>, "Can't call in_milliseconds with this type.");
			}
		}
		
		template <class T>
		auto per_second(const T t) const {
			return t * in_seconds<T>();
		}

		template <class T>
		auto per_millisecond(const T t) const {
			return t * in_milliseconds<T>();
		}

		template <class T>
		auto& operator*=(const T scalar) {
			*this = delta { secsf * scalar };
			return *this;
		}

		template <class T>
		auto& operator/=(const T scalar) {
			*this = delta { secsf / scalar };
			return *this;
		}

		auto in_steps_per_second() const {
			return static_cast<unsigned>(1.f / secsf);
		}
	};
}
