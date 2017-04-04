#pragma once
#include <bitset>
#include "augs/ensure.h"
#include "augs/templates/maybe_const.h"
#include "augs/misc/introspect.h"

namespace augs {
	template<class Enum, class T>
	class enum_associative_array {
	public:
		typedef Enum key_type;
		typedef T mapped_type;
	private:

		typedef std::array<mapped_type, size_t(key_type::COUNT)> arr_type;

		// GEN INTROSPECTOR class augs::enum_associative_array class key_type class mapped_type
		std::bitset<size_t(key_type::COUNT)> is_set;
		arr_type raw;
		// END GEN INTROSPECTOR

		template <class F, class key_type, class A, class... Instances>
		friend void introspect_body(
			const augs::enum_associative_array<key_type, A>* const,
			F f,
			Instances&&... _t_
		);

		unsigned find_first_set_index(unsigned from) const {
			while (from < static_cast<size_t>(key_type::COUNT) && !is_set.test(from)) {
				++from;
			}

			return from;
		}

	public:
		template<bool is_const>
		class basic_iterator {
			typedef maybe_const_ptr_t<is_const, enum_associative_array> ptr_type;
			typedef std::pair<key_type, maybe_const_ref_t<is_const, mapped_type>> ref_type;
			
			ptr_type ptr = nullptr;
			size_t idx = 0;

			friend class enum_associative_array<Enum, T>;

		public:
			basic_iterator(const ptr_type ptr, const size_t idx) : ptr(ptr), idx(idx) {}

			const basic_iterator operator++(int) {
				const iterator temp = *this;
				++*this;
				return temp;
			}

			basic_iterator& operator++() {
				idx = ptr->find_first_set_index(idx + 1);
				return *this;
			}

			bool operator==(const basic_iterator& b) const {
				return idx == b.idx;
			}

			bool operator!=(const basic_iterator& b) const {
				return idx != b.idx;
			}

			ref_type operator*() const {
				ensure(idx < ptr->capacity());
				return { static_cast<key_type>(idx), ptr->raw[idx] };
			}
		};

		typedef basic_iterator<false> iterator;
		typedef basic_iterator<true> const_iterator;

		template<bool>
		friend class basic_iterator;

		iterator begin() {
			return iterator(this, find_first_set_index(0u));
		}

		iterator end() {
			return iterator(this, static_cast<size_t>(key_type::COUNT));
		}

		const_iterator begin() const {
			return const_iterator(this, find_first_set_index(0));
		}

		const_iterator end() const {
			return const_iterator(this, static_cast<size_t>(key_type::COUNT));
		}

		iterator find(const key_type enum_idx) {
			const size_t i = static_cast<size_t>(enum_idx);
			ensure(i < capacity());

			if (is_set.test(i)) {
				return iterator(this, i);
			}

			return end();
		}

		const_iterator find(const key_type enum_idx) const {
			const size_t i = static_cast<size_t>(enum_idx);
			ensure(i < capacity());

			if (is_set.test(i)) {
				return const_iterator(this, i);
			}

			return end();
		}

		mapped_type& at(const key_type enum_idx) {
			const size_t i = static_cast<size_t>(enum_idx);
			ensure(i < capacity() && is_set.test(i));
			return raw[i];
		}

		const mapped_type& at(const key_type enum_idx) const {
			const size_t i = static_cast<size_t>(enum_idx);
			ensure(i < capacity() && is_set.test(i));
			return raw[i];
		}

		mapped_type& operator[](const key_type enum_idx) {
			const size_t i = static_cast<size_t>(enum_idx);

			if (!is_set.test(i)) {
				is_set.set(i);
			}

			return raw[i];
		}

		const mapped_type& operator[](const key_type enum_idx) const {
			return at(enum_idx);
		}

		constexpr size_t capacity() const {
			return raw.size();
		}

		void clear() {
			for (auto& v : raw) {
				v.~mapped_type();
				new (&v) mapped_type;
			}

			is_set = std::bitset<static_cast<size_t>(key_type::COUNT)>();
		}
	};
}