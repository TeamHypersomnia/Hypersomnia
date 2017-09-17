#pragma once
#include "augs/math/declare_math.h"

namespace augs {
	struct introspection_access;

	// TODO: usage of time should be uniform, 
	// should only store precomputed float/double counterparts,
	// without milliseconds

	class delta {
		friend struct introspection_access;
		friend class timer;
		using default_t = real32;

		// GEN INTROSPECTOR class augs::delta
		double secs = 0.0;
		double ms = 0.0;

		default_t secsf = 0.f;
		default_t msf = 0.f;
		// END GEN INTROSPECTOR
		
		explicit delta(const double secs) :
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

		bool operator==(const delta& b) const {
			return secs == b.secs;
		}

		template <class T = default_t>
		T in_seconds() const;
		
		template <class T = default_t>
		T in_milliseconds() const;

		template <>
		default_t in_milliseconds<default_t>() const {
			return { msf };
		}

		template <>
		default_t in_seconds<default_t>() const {
			return { secsf };
		}

		template <>
		double in_milliseconds<double>() const {
			return { ms };
		}

		template <>
		double in_seconds<double>() const {
			return { secs };
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
			*this = { secs * scalar };
			return *this;
		}

		template <class T>
		auto& operator/=(const T scalar) {
			*this = { secs / scalar };
			return *this;
		}
	};
}
