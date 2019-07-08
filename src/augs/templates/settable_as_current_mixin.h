#pragma once
#include "augs/templates/settable_as_current_op.h"

namespace augs {
	template <class derived, bool always_force_set = false>
	class settable_as_current_mixin {
		static derived* marked_instance;
		static derived* previous_instance;
		static derived* current_instance;

		static constexpr bool is_const = std::is_const_v<derived>;

		static void push_instance(derived* const n) {
			previous_instance = current_instance;
			current_instance = n;
		}

		template <class... Args>
		static void set_current_to(derived* const which, Args&&... args) {
			if (which == nullptr) {
				set_current_to_none(std::forward<Args>(args)...);
			}

			which->settable_as_current_mixin::set_as_current(std::forward<Args>(args)...);
		}

	protected:
		settable_as_current_mixin() = default;

		settable_as_current_mixin(settable_as_current_mixin&& b) noexcept {
			if (b.is_current()) {
				current_instance = static_cast<derived*>(this);
			}
		}

		settable_as_current_mixin& operator=(settable_as_current_mixin&& b) noexcept {
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

		static derived* find_current() {
			return current_instance;
		}

		static void mark_current() {
			marked_instance = current_instance;
		}

		template <class... Args>
		static void set_current_to_none(Args&&... args) {
			derived::set_current_to_none_impl(std::forward<Args>(args)...);
			push_instance(nullptr);
		}

		template <class... Args>
		static void set_current_to_previous(Args&&... args) {
			set_current_to(previous_instance, std::forward<Args>(args)...);
		}

		template <class... Args>
		static void set_current_to_marked(Args&&... args) {
			set_current_to(marked_instance, std::forward<Args>(args)...);
		}

		static bool current_exists() {
			return current_instance != nullptr;
		}

		template <class... Args>
		void unset_if_current(Args&&... args) const {
			if (is_current()) {
				set_current_to_none(std::forward<Args>(args)...);
			}
		}

		bool is_current() const {
			const auto& self = static_cast<const derived&>(*this);

			return current_instance == std::addressof(self);
		}

		template <class... Args, bool C = !is_const, class = std::enable_if_t<C>>
		bool set_as_current(Args&&... args) {
			auto& self = static_cast<derived&>(*this);

			if (!is_current() || always_force_set) {
				push_instance(std::addressof(self));
				return self.set_as_current_impl(std::forward<Args>(args)...);
			}

			return true;
		}

		template <class... Args, bool C = is_const, class = std::enable_if_t<C>>
		bool set_as_current(Args&&... args) const {
			const auto& self = static_cast<derived&>(*this);

			if (!is_current() || always_force_set) {
				push_instance(std::addressof(self));
				return self.set_as_current_impl(std::forward<Args>(args)...);
			}

			return true;
		}
	};

	template <class T, bool A>
	T* settable_as_current_mixin<T, A>::current_instance = nullptr;

	template <class T, bool A>
	T* settable_as_current_mixin<T, A>::previous_instance = nullptr;

	template <class T, bool A>
	T* settable_as_current_mixin<T, A>::marked_instance = nullptr;
}