#pragma once
#include <array>
#include "augs/ensure.h"
#include "augs/zeroed_pod.h"

#include "augs/misc/trivially_copyable_pair.h"
#include "augs/templates/type_matching_and_indexing.h"

#include "augs/misc/declare_containers.h"

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

	template <class T, std::size_t const_count>
	class constant_size_vector_base {
	public:
		using value_type = T;

	protected:
		using size_type = std::size_t;

		static constexpr bool is_trivially_copyable = std::is_trivially_copyable_v<value_type>;

		using value_array = std::array<T, const_count>;

		using storage_type = std::array<
			std::conditional_t<is_trivially_copyable, 
				value_type,
				std::aligned_storage_t<sizeof(value_type), alignof(value_type)>
			>,
			const_count
		>;
		
		static_assert(
			!is_trivially_copyable || std::is_default_constructible_v<value_type>,
			"No support for a type that is trivially copyable but not default constructible."
		);

		// GEN INTROSPECTOR class augs::constant_size_vector_base class T std::size_t const_count
		size_type count = 0;
		storage_type raw;
		// END GEN INTROSPECTOR

		friend struct augs::introspection_access;

		auto& as_value_array() {
			return reinterpret_cast<value_array&>(raw);
		}

		const auto& as_value_array() const {
			return reinterpret_cast<const value_array&>(raw);
		}

		auto& nth(const size_type n) {
			return as_value_array()[n];
		}

		const auto& nth(const size_type n) const {
			return as_value_array()[n];
		}

		template <class... Args>
		void construct_at(const size_type n, Args&&... args) {
			new (&nth(n)) value_type(std::forward<Args>(args)...);
		}

	public:
		using iterator = typename value_array::iterator;
		using const_iterator = typename value_array::const_iterator;

		constant_size_vector_base() = default;

		template <class Iter>
		constant_size_vector_base(Iter first, Iter last) {
			assign(first, last);
		}

		constant_size_vector_base(std::initializer_list<value_type> l) : constant_size_vector_base(l.begin(), l.end()) {}

		template <class Iter>
		void assign(const Iter first, const Iter last) {
			clear();
			insert(end(), first, last);
		}

		void push_back(const value_type& obj) {
			ensure(count < capacity());
			construct_at(count++, obj);
		}

		template <class... Args>
		void emplace_back(Args&&... args) {
			ensure(count < capacity());
			construct_at(count++, std::forward<Args>(args)...);
		}

		value_type& operator[](const size_type i) {
			return nth(i);
		}

		const value_type& operator[](const size_type i) const {
			return nth(i);
		}

		value_type& at(const size_type i) {
			ensure(i < count);
			return nth(i);
		}

		const value_type& at(const size_type i) const {
			ensure(i < count);
			return nth(i);
		}

		value_type& front() {
			return nth(0);
		}

		const value_type& front() const {
			return nth(0);
		}

		value_type& back() {
			return nth(count - 1);
		}

		const value_type& back() const {
			return nth(count - 1);
		}

		iterator erase(const iterator first, const iterator last) {
			ensure(last >= first && first >= begin() && last <= end());
			std::move(last, end(), first);
			resize(size() - (last - first));
			return first;
		}

		iterator erase(const iterator position) {
			ensure(position >= begin() && position <= end());
			std::move(position + 1, end(), position);
			resize(size() - 1);
			return position;
		}

		void insert(const iterator where, const value_type& obj) {
			const auto new_elements_count = 1;

			ensure(where >= begin());
			ensure(count + new_elements_count <= capacity());

			std::move(where, end(), where + 1);
			construct_at(where - begin(), obj);

			count += new_elements_count;
		}

		template <class Iter>
		void insert(iterator where, Iter first, const Iter last) {
			const auto new_elements_count = last - first;
			
			ensure(where >= begin());
			ensure(count + new_elements_count <= capacity());

			std::move(where, end(), where + (last - first));
			
			while (first != last) {
				construct_at(where - begin(), *first);

				++first;
				++where;
			}

			count += new_elements_count;
		}

		void resize(const size_type s) {
			ensure(s <= capacity());
			int diff = s;
			diff -= size();

			if (diff > 0) {
				while (diff--) {
					push_back(value_type());
				}
			}
			else if (diff < 0) {
				diff = -diff;
				
				while (diff--) {
					pop_back();
				}
			}
		}

		value_type* data() {
			return &nth(0);
		}

		const value_type* data() const {
			return &nth(0);
		}

		iterator begin() {
			return as_value_array().begin();
		}

		iterator end() {
			return as_value_array().begin() + size();
		}

		const_iterator begin() const {
			return as_value_array().begin();
		}

		const_iterator end() const {
			return as_value_array().begin() + size();
		}

		size_type size() const {
			return count;
		}

		constexpr size_type max_size() const {
			return const_count;
		}

		bool empty() const {
			return size() == 0;
		}

		size_type capacity() const {
			return const_count;
		}

		void reserve(const size_type s) {
			ensure(s <= max_size());
			// no-op
		}

		void pop_back() {
			ensure(count > 0);
			
			if constexpr(!is_trivially_copyable) {
				nth(count - 1).~value_type();
			}

			--count;
		}

		void clear() {
			if constexpr(!is_trivially_copyable) {
				while (count) {
					pop_back();
				}
			}
			else {
				count = 0;
			}
		}
	};

	// GEN INTROSPECTOR class augs::constant_size_vector class T std::size_t const_count class dummy
	// INTROSPECT BASE augs::constant_size_vector_base<T, const_count>
	// END GEN INTROSPECTOR

	template <class T, std::size_t N>
	class constant_size_vector<T, N, std::enable_if_t<std::is_trivially_copyable_v<T>>>
		: public constant_size_vector_base<T, N> {
		using underlying_char_type = zeroed_pod_internal_type_t<T>;

		static constexpr bool should_act_like_string =
			is_one_of_v<underlying_char_type, char, wchar_t>
			;

		using string_type = std::basic_string<underlying_char_type>;

	public:
		using constant_size_vector_base::constant_size_vector_base;

		constant_size_vector() = default;

		template <class = std::enable_if_t<should_act_like_string>>
		constant_size_vector(const string_type& s) : constant_size_vector(s.begin(), s.end()) {}

		template <class = std::enable_if_t<should_act_like_string>>
		constant_size_vector& operator=(const string_type& s) {
			assign(s.begin(), s.end());
			return *this;
		}

		template <class = std::enable_if_t<should_act_like_string>>
		constant_size_vector& operator+=(const string_type& s) {
			insert(end(), s.begin(), s.end());
			return *this;
		}

		template <class = std::enable_if_t<should_act_like_string>>
		operator string_type() const {
			return{ begin(), end() };
		}
	};

	template <class T, std::size_t N>
	class constant_size_vector<T, N, std::enable_if_t<!std::is_trivially_copyable_v<T>>>
		: public constant_size_vector_base<T, N> {
	public:
		using constant_size_vector_base::constant_size_vector_base;
		
		constant_size_vector() = default;

		constant_size_vector(const constant_size_vector& b) {
			insert(begin(), b.begin(), b.end());
		}

		constant_size_vector& operator=(const constant_size_vector& b) {
			clear();
			insert(begin(), b.begin(), b.end());

			return *this;
		}

		constant_size_vector(constant_size_vector&& b) {
			insert(begin(), 
				std::make_move_iterator(b.begin()), 
				std::make_move_iterator(b.end())
			);

			b.count = 0;
		}

		constant_size_vector& operator=(constant_size_vector&& b) {
			clear();

			insert(begin(),
				std::make_move_iterator(b.begin()),
				std::make_move_iterator(b.end())
			);

			b.count = 0;

			return *this;
		}

		~constant_size_vector() {
			clear();
		}
	};

	template <std::size_t const_count>
	using constant_size_string = constant_size_vector<zeroed_pod<char>, const_count>;

	template <std::size_t const_count>
	using constant_size_wstring = constant_size_vector<zeroed_pod<wchar_t>, const_count>;
}

template <std::size_t I>
struct of_size {
	template <class T>
	using make_constant_vector = augs::constant_size_vector<T, I>;
};