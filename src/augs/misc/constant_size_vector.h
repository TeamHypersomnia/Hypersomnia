#pragma once
#include <array>
#include "augs/ensure.h"
#include "augs/zeroed_pod.h"

#include "augs/misc/trivially_copyable_pair.h"

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
	struct introspection_access;

	template<class T, size_t const_count>
	class constant_size_vector {
		typedef std::array<T, const_count> arr_type;

		// GEN INTROSPECTOR class augs::constant_size_vector class T size_t const_count
		size_t count;
		arr_type raw;
		// END GEN INTROSPECTOR

		friend struct augs::introspection_access;

	public:
		typedef typename arr_type::iterator iterator;
		typedef typename arr_type::const_iterator const_iterator;
		static constexpr bool is_string_type = std::is_same<zeroed_pod_internal_type_t<T>, char>::value || std::is_same<zeroed_pod_internal_type_t<T>, wchar_t>::value;
		typedef typename std::basic_string<zeroed_pod_internal_type_t<T>> string_type;
		typedef T value_type;
		static constexpr size_t array_size = sizeof(T) * const_count;
		
		constant_size_vector() : count(0) {

		}

		template <class Iter>
		constant_size_vector(Iter first, Iter last) {
			assign(first, last);
		}

		constant_size_vector(std::initializer_list<T> l) : constant_size_vector(l.begin(), l.end()) {}

		template<bool _is_string_type = is_string_type, class = std::enable_if_t<_is_string_type>>
		constant_size_vector(const std::basic_string<zeroed_pod_internal_type_t<T>>& s) : constant_size_vector(s.begin(), s.end()) {}

		constant_size_vector& operator=(const constant_size_vector&) = default;

		template<bool _is_string_type = is_string_type, class = std::enable_if_t<_is_string_type>>
		constant_size_vector& operator=(const string_type& s) {
			assign(s.begin(), s.end());
			return *this;
		}

		template<bool _is_string_type = is_string_type, class = std::enable_if_t<_is_string_type>>
		constant_size_vector& operator+=(const string_type& s) {
			insert(end(), s.begin(), s.end());
			return *this;
		}

		template <class Iter>
		void assign(const Iter first, const Iter last) {
			clear();
			insert(end(), first, last);
		}

		void push_back(const T& obj) {
			ensure(count < capacity());
			raw[count++] = obj;
		}

		template <class... Args>
		void emplace_back(Args&&... args) {
			ensure(count < capacity());
			raw[count++] = T(std::forward<Args>(args)...);
		}

		T& operator[](const size_t i) {
			return raw[i];
		}

		const T& operator[](const size_t i) const {
			return raw[i];
		}

		T& at(const size_t i) {
			return raw.at(i);
		}

		const T& at(const size_t i) const {
			return raw.at(i);
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

		iterator erase(const iterator first, const iterator last) {
			ensure(last >= first && first >= begin() && last <= end());
			std::copy(last, end(), first);
			resize(size() - (last - first));
			return first;
		}

		iterator erase(const iterator position) {
			ensure(position >= begin() && position <= end());
			std::copy(position + 1, end(), position);
			resize(size() - 1);
			return position;
		}

		void insert(const iterator where, const T& obj) {
			const auto new_elements_count = 1;

			ensure(where >= begin());
			ensure(count + new_elements_count <= capacity());

			std::copy(where, end(), where + 1);
			*where = obj;

			count += new_elements_count;
		}

		template <class Iter>
		void insert(const iterator where, const Iter first, const Iter last) {
			const auto new_elements_count = last - first;
			
			ensure(where >= begin());
			ensure(count + new_elements_count <= capacity());

			std::copy(where, end(), where + (last-first));
			std::copy(first, last, where);

			count += new_elements_count;
		}

		void resize(const size_t s) {
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

		std::size_t size() const {
			return count;
		}

		constexpr std::size_t max_size() const {
			return raw.max_size();
		}

		bool empty() const {
			return size() == 0;
		}

		size_t capacity() const {
			return const_count;
		}

		void reserve(const size_t) {
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

		template<bool _is_string_type = is_string_type, class = std::enable_if_t<_is_string_type>>
		operator string_type() const {
			return{ begin(), end() };
		}
	};

	template <size_t const_count>
	using constant_size_string = constant_size_vector<zeroed_pod<char>, const_count>;

	template <size_t const_count>
	using constant_size_wstring = constant_size_vector<zeroed_pod<wchar_t>, const_count>;
}