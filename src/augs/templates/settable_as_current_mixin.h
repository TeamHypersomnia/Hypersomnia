#pragma once

namespace augs {
	template <class derived>
	class settable_as_current_mixin {
		static derived* current_instance;
		static constexpr bool is_const = std::is_const_v<derived>;

	protected:
		using settable_as_current_base = settable_as_current_mixin<derived>;

		settable_as_current_mixin() = default;

		settable_as_current_mixin(settable_as_current_mixin&& b) {
			if (b.is_current()) {
				current_instance = static_cast<derived*>(this);
			}
		}

		settable_as_current_mixin& operator=(settable_as_current_mixin&& b) {
			unset_if_current();

			if (b.is_current()) {
				current_instance = static_cast<derived*>(this);
			}

			return *this;
		}

	public:
		static derived& get_current() {
			return *current_instance;
		}

		static void set_current_to_none() {
			derived::set_current_to_none_impl();
			current_instance = nullptr;
		}

		static bool current_exists() {
			return current_instance != nullptr;
		}

		template <class = std::enable_if_t<!is_const>>
		void unset_if_current() {
			if (is_current()) {
				set_current_to_none();
			}
		}

		template <class = std::enable_if_t<is_const>>
		void unset_if_current() const {
			if (is_current()) {
				set_current_to_none();
			}
		}

		bool is_current() const {
			const auto& self = static_cast<const derived&>(*this);

			return current_instance == std::addressof(self);
		}

		template <class = std::enable_if_t<!is_const>>
		bool set_as_current() {
			auto& self = static_cast<derived&>(*this);

			if (!is_current()) {
				current_instance = std::addressof(self);
				return self.set_as_current_impl();
			}

			return true;
		}

		template <class = std::enable_if_t<is_const>>
		bool set_as_current() const {
			const auto& self = static_cast<derived&>(*this);

			if (!is_current()) {
				current_instance = std::addressof(self);
				return self.set_as_current_impl();
			}

			return true;
		}
	};

	template <class T>
	T* settable_as_current_mixin<T>::current_instance = nullptr;
}