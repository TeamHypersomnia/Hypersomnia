#pragma once
#include <array>
#include "augs/ensure.h"

#include "augs/misc/simple_pair.h"
#include "augs/templates/folded_finders.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/templates/traits/is_comparable.h"

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

namespace augs {
	template <class T, unsigned const_count>
	class constant_size_vector_base {
	public:
		using value_type = T;

	protected:
		using size_type = unsigned;

		static constexpr bool is_trivially_copyable = std::is_trivially_copyable_v<value_type>;

		using value_array = std::array<T, const_count>;

		using storage_type = std::aligned_storage_t<
			sizeof(value_type) * const_count, alignof(value_type)
		>;
		
		size_type count = 0;
		storage_type raw;

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
			ensure_less(count, capacity());
			construct_at(count++, obj);
		}

		template <class... Args>
		void emplace_back(Args&&... args) {
			ensure_less(count, capacity());
			construct_at(count++, std::forward<Args>(args)...);
		}

		value_type& operator[](const std::size_t i) {
#if !IS_PRODUCTION_BUILD
			ensure_less(static_cast<size_type>(i), count);
#endif
			return nth(static_cast<size_type>(i));
		}

		const value_type& operator[](const std::size_t i) const {
#if !IS_PRODUCTION_BUILD
			ensure_less(static_cast<size_type>(i), count);
#endif
			return nth(static_cast<size_type>(i));
		}

		value_type& at(const std::size_t i) {
			ensure_less(static_cast<size_type>(i), count);
			return nth(static_cast<size_type>(i));
		}

		const value_type& at(const std::size_t i) const {
			ensure_less(static_cast<size_type>(i), count);
			return nth(static_cast<size_type>(i));
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
			ensure_geq(last, first);
			ensure_geq(first, begin());	
			ensure_leq(last, end());

			std::move(last, end(), first);
			resize(size() - (last - first));
			return first;
		}

		iterator erase(const iterator position) {
			ensure_geq(position, begin());
		   	ensure_leq(position, end());

			std::move(position + 1, end(), position);
			resize(size() - 1);
			return position;
		}

		void insert(const iterator where, const value_type& obj) {
			const auto new_elements_count = 1;

			ensure_geq(where, begin());
			ensure_leq(count + new_elements_count, capacity());

			std::move(where, end(), where + 1);
			construct_at(static_cast<size_type>(where - begin()), obj);

			count += new_elements_count;
		}

		template <class Iter>
		void insert(iterator where, Iter first, const Iter last) {
			const auto new_elements_count = static_cast<size_type>(last - first);
			
			ensure_geq(where, begin());
			ensure_leq(count + new_elements_count, capacity());

			std::move(where, end(), where + (last - first));
			
			while (first != last) {
				construct_at(static_cast<size_type>(where - begin()), *first);

				++first;
				++where;
			}

			count += new_elements_count;
		}

		void resize(const std::size_t s) {
			ensure_leq(s, capacity());
			auto diff = static_cast<int>(s);
			diff -= static_cast<int>(size());

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

		std::size_t size() const {
			return static_cast<std::size_t>(count);
		}

		static constexpr std::size_t max_size() {
			return const_count;
		}

		bool empty() const {
			return size() == 0;
		}

		static constexpr auto capacity() {
			return const_count;
		}

		void reserve(const std::size_t s) {
			ensure_leq(s, max_size());
			// no-op
		}

		void pop_back() {
			ensure_greater(count, 0);
			
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

		operator std::vector<value_type>() const {
			return { begin(), end() };
		}
	};

	template <class T, unsigned N>
	class constant_size_vector<T, N, std::enable_if_t<std::is_trivially_copyable_v<T>>>
		: public constant_size_vector_base<T, N> {

	public:
		using base = constant_size_vector_base<T, N>;
		using base::constant_size_vector_base;
		using base::insert;
		using base::assign;
		using base::begin;
		using base::end;
		using value_type = typename base::value_type;
		using base::operator std::vector<value_type>;

		constant_size_vector() = default;
	};

	template <class T, unsigned N>
	class constant_size_vector<T, N, std::enable_if_t<!std::is_trivially_copyable_v<T>>>
		: public constant_size_vector_base<T, N> {
	public:
		using base = constant_size_vector_base<T, N>;
		using base::constant_size_vector_base;
		using base::insert;
		using base::assign;
		using base::clear;
		using base::begin;
		using base::end;
		using value_type = typename base::value_type;
		using base::operator std::vector<value_type>;

		constant_size_vector() = default;

		constant_size_vector(const constant_size_vector& b) {
			insert(begin(), b.begin(), b.end());
		}

		constant_size_vector& operator=(const constant_size_vector& b) {
			assign(b.begin(), b.end());

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

	template <class T, unsigned C, class = std::enable_if_t<is_comparable_v<T, T>>>
	bool operator==(
		const constant_size_vector<T, C>& a,
		const constant_size_vector<T, C>& b
	) {
		return ranges_equal(a, b);
	}
}