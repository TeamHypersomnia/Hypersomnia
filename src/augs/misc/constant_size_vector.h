#pragma once
#include <cstddef>
#include <array>
#include <cstring>
#include <iterator>
#include <new>
#include <utility>
#include "augs/ensure.h"
#include "augs/ensure_rel.h"

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

		using aligned_storage_type = std::aligned_storage_t<
			sizeof(value_type) , alignof(value_type)
		>;

		size_type count = 0;
		aligned_storage_type raw[const_count];

		void memset_zero() {
			std::memset(raw, 0, sizeof(raw));
		}

		auto* nth_ptr(const size_type n) {
			return std::launder(reinterpret_cast<T*>(raw + n));
		}

		const auto* nth_ptr(const size_type n) const {
			return std::launder(reinterpret_cast<const T*>(raw + n));
		}

		auto& nth(const size_type n) {
			return *std::launder(reinterpret_cast<T*>(raw + n));
		}

		const auto& nth(const size_type n) const {
			return *std::launder(reinterpret_cast<const T*>(raw + n));
		}

		template <class... Args>
		void construct_at(const size_type n, Args&&... args) {
			new (raw + n) value_type(std::forward<Args>(args)...);
		}

		void _pop_back() {
			--count;

			if constexpr(!is_trivially_copyable) {
				std::destroy_at(nth_ptr(count));
			}
		}

	public:
		using iterator = T*;
		using const_iterator = const T*;
		using reverse_iterator = std::reverse_iterator<T*>;
		using const_reverse_iterator = std::reverse_iterator<const T*>;

#if DEBUG_DESYNCS
		constant_size_vector_base() {
			memset_zero();
		}
#else
		constant_size_vector_base() = default;
#endif

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
			ensure_less(count, static_cast<size_type>(capacity()));
			construct_at(count++, obj);
		}

		void push_back(value_type&& obj) {
			ensure_less(count, static_cast<size_type>(capacity()));
			construct_at(count++, std::move(obj));
		}

		template <class... Args>
		value_type& emplace_back(Args&&... args) {
			ensure_less(count, static_cast<size_type>(capacity()));
			construct_at(count, std::forward<Args>(args)...);
			return nth(count++);
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
#if !IS_PRODUCTION_BUILD
			ensure_less(static_cast<size_type>(i), count);
#endif
			return nth(static_cast<size_type>(i));
		}

		const value_type& at(const std::size_t i) const {
#if !IS_PRODUCTION_BUILD
			ensure_less(static_cast<size_type>(i), count);
#endif
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

		template <class... Args>
		void emplace(const iterator where, Args&&... args) {
			const auto new_elements_count = 1;

			ensure_geq(where, begin());
			ensure_leq(count + new_elements_count, static_cast<size_type>(capacity()));

			std::move(where, end(), where + 1);
			construct_at(static_cast<size_type>(where - begin()), std::forward<Args>(args)...);

			count += new_elements_count;
		}

		void insert(const iterator where, const value_type& obj) {
			const auto new_elements_count = 1;

			ensure_geq(where, begin());
			ensure_leq(count + new_elements_count, static_cast<size_type>(capacity()));

			std::move(where, end(), where + 1);
			construct_at(static_cast<size_type>(where - begin()), obj);

			count += new_elements_count;
		}

		template <class Iter>
		void insert(iterator where, Iter first, const Iter last) {
			const auto new_elements_count = static_cast<size_type>(last - first);
			
			ensure_geq(where, begin());
			ensure_leq(count + new_elements_count, static_cast<size_type>(capacity()));

			std::move(where, end(), where + (last - first));
			
			while (first != last) {
				construct_at(static_cast<size_type>(where - begin()), *first);

				++first;
				++where;
			}

			count += new_elements_count;
		}

		void resize_no_init(const std::size_t s) {
			static_assert(is_trivially_copyable);
			count = static_cast<size_type>(s);
		}

		void resize(const std::size_t s) {
			ensure_leq(s, capacity());

			while (count < s) {
				new (raw + count) value_type;
				++count;
			}

			if constexpr(!is_trivially_copyable) {
				while (count > s) {
					_pop_back();
				}
			}
			else {
				count = static_cast<size_type>(s);
			}
		}

		value_type* data() {
			return nth_ptr(0);
		}

		const value_type* data() const {
			return nth_ptr(0);
		}

		auto begin() {
			return iterator(nth_ptr(0));
		}

		auto end() {
			return begin() + size();
		}

		auto begin() const {
			return const_iterator(nth_ptr(0));
		}

		auto end() const {
			return begin() + size();
		}

		auto rbegin() {
			return reverse_iterator(nth_ptr(size()));
		}

		auto rend() {
			return rbegin() + size();
		}

		auto rbegin() const {
			return const_reverse_iterator(nth_ptr(size()));
		}

		auto rend() const {
			return rbegin() + size();
		}

		std::size_t size() const {
			return static_cast<std::size_t>(count);
		}

		constexpr std::size_t max_size() const noexcept {
			return const_count;
		}

		constexpr std::size_t capacity() const noexcept {
			return const_count;
		}

		bool empty() const {
			return size() == 0;
		}

		void reserve(const std::size_t s) {
			(void)s;
			ensure_leq(s, max_size());
			// no-op
		}

		void pop_back() {
			ensure_greater(count, static_cast<size_type>(0));
			_pop_back();	
		}

		void clear() {
			if constexpr(!is_trivially_copyable) {
				while (count) {
					_pop_back();
				}
			}
			else {
				count = 0;
			}

#if DEBUG_DESYNCS
			memset_zero();
#endif
		}

		operator std::vector<value_type>() const {
			return { begin(), end() };
		}
	};

	template <class T, unsigned N, bool force_nontrivial_interface>
	class constant_size_vector<T, N, force_nontrivial_interface, std::enable_if_t<!force_nontrivial_interface && std::is_trivially_copyable_v<T>>>
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

	template <class T, unsigned N, bool force_nontrivial_interface>
	class constant_size_vector<T, N, force_nontrivial_interface, std::enable_if_t<force_nontrivial_interface || !std::is_trivially_copyable_v<T>>>
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

		constant_size_vector(const constant_size_vector& b) : base() {
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

	template <class T, unsigned C, bool F, class = std::enable_if_t<is_comparable_v<T, T>>>
	bool operator==(
		const constant_size_vector<T, C, F>& a,
		const constant_size_vector<T, C, F>& b
	) {
		return ranges_equal(a, b);
	}

	template <class T, unsigned C, bool F, class = std::enable_if_t<is_comparable_v<T, T>>>
	bool operator==(
		const constant_size_vector<T, C, F>& a,
		const std::vector<T>& b
	) {
		return ranges_equal(a, b);
	}
}
