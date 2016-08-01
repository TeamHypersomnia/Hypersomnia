#pragma once
#include <array>
#include "augs/ensure.h"

namespace augs  {
	template<class T, int const_count>
	class constant_size_vector {
		typedef std::array<T, const_count> arr_type;

		arr_type raw;
		size_t count = 0;
	public:
		typedef typename arr_type::iterator iterator;
		typedef typename arr_type::const_iterator const_iterator;
		typedef T value_type;
		
		constant_size_vector() = default;

		template <class Iter>
		constant_size_vector(Iter first, Iter last) {
			assign(first, last);
		}

		template <class Iter>
		void assign(Iter first, Iter last) {
			clear();

			while (first != last)
				push_back(*first++);
		}

		void push_back(const T& obj) {
			ensure(count < capacity());
			raw[count++] = obj;
		}

		T& operator[](size_t i) {
			return raw[i];
		}

		const T& operator[](size_t i) const {
			return raw[i];
		}

		iterator begin() {
			return raw.begin();
		}

		iterator end() {
			return raw.begin() + size();
		}

		iterator erase(iterator first, iterator last) {
			ensure(last >= first && first >= begin() && last <= end());
			std::copy(last, end(), first);
			resize(size() - (last - first));
			return first;
		}

		iterator erase(iterator position) {
			ensure(position >= begin() && position <= end());
			std::copy(position + 1, end(), position);
			resize(size() - 1);
			return position;
		}

		void resize(size_t s) {
			ensure(s <= capacity());
			int diff = s;
			diff -= size();

			if (diff > 0) {
				while (diff--) {
					push_back(T());
				}
			}
			else if (diff < 0) {
				diff = -diff;
				
				while (diff--) {
					pop_back();
				}
			}
		}

		const_iterator begin() const {
			return raw.begin();
		}

		const_iterator end() const {
			return raw.begin() + size();
		}

		size_t size() const {
			return count;
		}

		size_t capacity() const {
			return const_count;
		}

		void pop_back() {
			ensure(count > 0);
			raw[count-1] = T();
			--count;
		}

		void clear() {
			for (auto& e : raw) {
				e = T();
			}

			count = 0;
		}
	};
}