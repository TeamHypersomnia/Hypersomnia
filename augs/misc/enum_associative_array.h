#pragma once
#include <bitset>
#include "augs/ensure.h"
#include "augs/templates/maybe_const.h"

namespace augs {
	template<class Enum, class T>
	class enum_associative_array {
		typedef std::array<T, size_t(Enum::COUNT)> arr_type;

		std::bitset<size_t(Enum::COUNT)> is_set;
		arr_type raw;

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

				while (idx < size_t(Enum::COUNT) && !ptr->is_set.test(idx))
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

			if (is_set.test(i))
				return iterator(this, i);

			return end();
		}

		const_iterator find(Enum enum_idx) const {
			size_t i = size_t(enum_idx);
			ensure(i < capacity());

			if (is_set.test(i))
				return const_iterator(this, i);

			return end();
		}

		T& at(Enum enum_idx) {
			size_t i = size_t(enum_idx);
			ensure(i < capacity() && is_set.test(i));
			return raw[i];
		}

		const T& at(Enum enum_idx) const {
			size_t i = size_t(enum_idx);
			ensure(i < capacity() && is_set.test(i));
			return raw[i];
		}

		T& operator[](Enum enum_idx) {
			size_t i = size_t(enum_idx);

			if (!is_set.test(i)) {
				is_set.set(i);
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

			is_set = std::bitset<size_t(Enum::COUNT)>();
		}
	};
}