#pragma once
#include <array>
#include "augs/ensure.h"

#include "augs/misc/trivial_pair.h"
template<class ForwardIt, class T, class Compare = std::less<>>
ForwardIt binary_find(ForwardIt first, ForwardIt last, const T& value, Compare comp = {})
{
	// Note: BOTH type T and the type after ForwardIt is dereferenced 
	// must be implicitly convertible to BOTH Type1 and Type2, used in Compare. 
	// This is stricter then lower_bound requirement (see above)

	first = std::lower_bound(first, last, value);
	return first != last && !comp(value, *first) ? first : last;
}

namespace augs  {
	template<class T, int const_count>
	class constant_size_vector {
		typedef std::array<T, const_count> arr_type;

		arr_type raw;
		size_t count;
	public:
		typedef typename arr_type::iterator iterator;
		typedef typename arr_type::const_iterator const_iterator;
		typedef T value_type;
		static constexpr size_t array_size = sizeof(T) * const_count;
		
		constant_size_vector() : count(0) {

		}

		template <class Iter>
		constant_size_vector(Iter first, Iter last) {
			assign(first, last);
		}

		constant_size_vector(std::initializer_list<T> l) : constant_size_vector(l.begin(), l.end()) {}

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

		T& front() {
			return raw[0];
		}

		const T& front() const {
			return raw[0];
		}

		T& back() {
			return raw[count - 1];
		}

		const T& back() const {
			return raw[count - 1];
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

		void insert(iterator position, const T& obj) {
			ensure(position >= begin());
			ensure(count < capacity());
			++count;
			std::copy(position, end()-1, position+1);
			*position = obj;
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

		T* data() {
			return &raw[0];
		}

		const T* data() const {
			return &raw[0];
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

		bool empty() const {
			return size() == 0;
		}

		size_t capacity() const {
			return const_count;
		}

		void reserve(size_t) {
			// no-op
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


	template<class Key, class Value, int const_count>
	class constant_size_associative_vector : private constant_size_vector<trivial_pair<Key, Value>, const_count> {
		typedef trivial_pair<Key, Value> elem_type;
		typedef constant_size_vector<trivial_pair<Key, Value>, const_count> base;
	public:
		using base::clear;
		using base::capacity;
		using base::size;
		using base::empty;

		void erase(const Key& i) {
			auto it = binary_find(begin(), end(), i, [](const elem_type& a, const elem_type& b) { return a.first < b.first; });
			ensure(it != end());
			base::erase(it);
		}

		Value& operator[](const Key& i) {
			auto it = binary_find(begin(), end(), i, [](const elem_type& a, const elem_type& b) { return a.first < b.first; });
			
			if (it == end()) {
				insert(it, elem_type(i, Value()));
			}

			return (*it).second;
		}

		const Value& operator[](const Key& i) const {
			auto it = binary_find(begin(), end(), i, [](const elem_type& a, const elem_type& b) { return a.first < b.first; });
			ensure(it != end());
			return (*it).second;
		}
	};
}