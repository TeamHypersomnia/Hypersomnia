#pragma once

namespace augs {
	template <class derived>
	class settable_as_current_mixin {
		static derived* current_instance;

		void set_as_current_impl() {

		}

	public:
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

		void set_as_current() {
			auto& self = static_cast<derived&>(*this);

			if (!is_current()) {
				self.set_as_current_impl();
				current_instance = &self;
			}
		}
	};

	template <class T>
	T* settable_as_current_mixin<T>::current_instance = nullptr;
}