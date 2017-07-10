#pragma once

namespace augs {
	template <class derived>
	class settable_as_current_mixin {
		static derived* current_instance;

		bool set_as_current_impl() {
			return true;
		}

	public:
		static bool current_exists() {
			return current_instance != nullptr;
		}

		static derived& get_current() {
			return *current_instance;
		}

		static void set_current_to_none() {
			current_instance = nullptr;
		}

		bool is_current() const {
			const auto& self = static_cast<const derived&>(*this);

			return current_instance == &self;
		}

		bool set_as_current() {
			auto& self = static_cast<derived&>(*this);

			if (!is_current()) {
				current_instance = &self;
				return self.set_as_current_impl();
			}

			return true;
		}
	};

	template <class T>
	T* settable_as_current_mixin<T>::current_instance = nullptr;
}