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
		double_type secs = 0.0;
		double_type ms = 0.0;

		default_t secsf = 0.f;
		default_t msf = 0.f;
		// END GEN INTROSPECTOR
		
		explicit delta(const double_type secs) :
			secs(secs),
			ms(secs * 1000),
			secsf(static_cast<default_t>(secs)),
			msf(static_cast<default_t>(ms))
		{}

	public:
		static delta zero;

		static delta steps_per_second(const unsigned steps) {
			return delta{ steps ? 1.0 / steps : 0.0 };
		}

		static delta from_milliseconds(const double_type ms) {
			return delta{ ms / 1000 };
		}

		bool operator==(const delta& b) const {
			return secs == b.secs;
		}

		template <class T = default_t>
		T in_seconds() const {
			if constexpr(std::is_same_v<T, default_t>) {
				return { secsf };
			}
			else if constexpr(std::is_same_v<T, double_type>) {
				return { secs };
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
				return { ms };
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
			*this = delta { secs * scalar };
			return *this;
		}

		template <class T>
		auto& operator/=(const T scalar) {
			*this = delta { secs / scalar };
			return *this;
		}

		auto in_steps_per_second() const {
			return static_cast<unsigned>(1.0 / secs);
		}
	};
}
