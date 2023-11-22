#pragma once

namespace augs {
	template <class derived, class Rr, bool always_force_set = true>
	class settable_commandizer : protected settable_as_current_mixin<derived, always_force_set> {
		static constexpr bool is_const = std::is_const_v<derived>;

	public:
		using base = settable_as_current_mixin<derived, always_force_set>;

		using op_type = settable_as_current_op_type;

		template <bool C = !is_const, class = std::enable_if_t<C>>
		void set_as_current(Rr& r) {
			auto& self = static_cast<derived&>(*this);

			r.template push_object_command(
				self,
				op_type::SET
			);
		}

		template <bool C = is_const, class = std::enable_if_t<C>>
		void set_as_current(Rr& r) const {
			const auto& self = static_cast<const derived&>(*this);

			r.template push_object_command(
				self,
				op_type::SET
			);
		}

		template <class R>
		static void mark_current(R& r) {
			r.template push_static_object_command<derived>(
				op_type::MARK
			);
		}

		template <class R>
		static void set_current_to_none(R& r) {
			r.template push_static_object_command<derived>(
				op_type::SET_TO_NONE
			);
		}

		template <class R>
		static void set_current_to_previous(R& r) {
			r.template push_static_object_command<derived>(
				op_type::SET_TO_PREVIOUS
			);
		}

		template <class R>
		static void set_current_to_marked(R& r) {
			r.template push_static_object_command<derived>(
				op_type::SET_TO_MARKED
			);
		}

		template <class A>
		static void perform_static(const A access, const op_type op) {
			using S = decltype(op);

			switch (op) {
				case S::MARK: base::mark_current(); break;
				case S::SET_TO_NONE: base::set_current_to_none(access); break;
				case S::SET_TO_PREVIOUS: base::set_current_to_previous(access); break;
				case S::SET_TO_MARKED: base::set_current_to_marked(access); break;
				default: break;
			}
		}

		template <class E, class A>
		static void perform_impl(E& self, const A access, const op_type op) {
			using S = decltype(op);

			switch (op) {
				case S::SET: self.set_as_current(access); break;
				default: break;
			}
		}

		template <class A>
		void perform(const A access, const op_type op) {
			auto& self = static_cast<base&>(*this);
			perform_impl(self, access, op);
		}

		template <class A>
		void perform(const A access, const op_type op) const {
			const auto& self = static_cast<const base&>(*this);
			perform_impl(self, access, op);
		}
	};
}
