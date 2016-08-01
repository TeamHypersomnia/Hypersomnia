#pragma once
#include <array>
#include "augs/ensure.h"
#include "augs/templates.h"

namespace augs {
	template<class Enum, class T>
	class enum_associative_array {
		typedef std::array<T, size_t(Enum::COUNT)> arr_type;

		std::array<bool, size_t(Enum::COUNT)> is_set;
		arr_type raw;

		void clear_flags() {
			std::fill(is_set.begin(), is_set.end(), false);
		}
	public:
		template<bool is_const>
		class basic_iterator {
			typedef maybe_const_ptr_t<is_const, enum_associative_array> ptr_type;
			typedef std::pair<Enum, maybe_const_ref_t<is_const, T>> ref_type;
			
			ptr_type ptr = nullptr;
			size_t idx = 0;

			//template<class Enum, class T>
			friend class enum_associative_array;

		public:
			basic_iterator(ptr_type ptr, size_t idx) : ptr(ptr), idx(idx) {}

			const basic_iterator operator++(int)
			{
				iterator temp = *this;
				++*this;
				return temp;
			}

			basic_iterator& operator++()
			{
				++idx;

				while (!ptr->is_set[idx] && idx < size_t(Enum::COUNT))
					++idx;
				
				return *this;
			}

			bool operator!=(const basic_iterator& b) const {
				return idx != b.idx;
			}

			ref_type operator*() const {
				ensure(idx < ptr->capacity());
				return { Enum(idx), ptr->raw[idx] };
			}
		};

		typedef basic_iterator<false> iterator;
		typedef basic_iterator<true> const_iterator;

		template<bool>
		friend class basic_iterator;

		enum_associative_array() {
			clear_flags();
		}

		iterator begin() {
			return iterator(this, 0);
		}

		iterator end() {
			return iterator(this, size_t(Enum::COUNT));
		}

		const_iterator begin() const {
			return const_iterator(this, 0);
		}

		const_iterator end() const {
			return const_iterator(this, size_t(Enum::COUNT));
		}

		iterator find(Enum enum_idx) {
			size_t i = size_t(enum_idx);
			ensure(i < capacity());

			if (is_set[i])
				return iterator(this, i);

			return end();
		}

		const_iterator find(Enum enum_idx) const {
			size_t i = size_t(enum_idx);
			ensure(i < capacity());

			if (is_set[i])
				return const_iterator(this, i);

			return end();
		}

		T& at(Enum enum_idx) {
			size_t i = size_t(enum_idx);
			ensure(i < capacity() && is_set[i]);
			return raw[i];
		}

		const T& at(Enum enum_idx) const {
			size_t i = size_t(enum_idx);
			ensure(i < capacity() && is_set[i]);
			return raw[i];
		}

		T& operator[](Enum enum_idx) {
			size_t i = size_t(enum_idx);

			if (!is_set[i]) {
				is_set[i] = true;
			}

			return raw[i];
		}

		const T& operator[](Enum enum_idx) const {
			return at(enum_idx);
		}

		size_t capacity() const {
			return raw.size();
		}

		void clear() {
			for_each([](T& v) {
				v.second = T();
			});

			clear_flags();
		}
	};
}